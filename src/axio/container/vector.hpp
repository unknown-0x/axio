#ifndef AXIO_CONTAINER_VECTOR_HPP_
#define AXIO_CONTAINER_VECTOR_HPP_

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>

#include "compressed_tuple.hpp"
#include "detail/iterator_traits.hpp"

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"
#include "../memory/allocator.hpp"

namespace axio {
template <typename T, typename A = axio::Allocator<T>>
class Vector {
  using AllocatorTraits = std::allocator_traits<A>;

  template <typename It>
  using EnableIfForwardIt =
      typename EnableIf<IsForwardIterator<It>::value, int>::type;

  template <typename It>
  using EnableIfNotForwardIt =
      typename EnableIf<IsInputIterator<It>::value &&
                            !IsForwardIterator<It>::value,
                        int>::type;

 public:
  using ValueType = T;
  using AllocatorType = A;
  using SizeType = typename AllocatorTraits::size_type;
  using DifferenceType = typename AllocatorTraits::difference_type;
  using Reference = ValueType&;
  using ConstReference = const ValueType&;
  using Pointer = typename AllocatorTraits::pointer;
  using ConstPointer = typename AllocatorTraits::const_pointer;
  using Iterator = Pointer;
  using ConstIterator = ConstPointer;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  static constexpr SizeType kGrowthFactor = SizeType(2);

  Vector() noexcept(noexcept(AllocatorType())) : Vector(AllocatorType()) {}

  explicit Vector(const AllocatorType& allocator)
      : storage_(allocator, nullptr), begin_(nullptr), end_(nullptr) {}

  explicit Vector(SizeType count,
                  const AllocatorType& allocator = AllocatorType())
      : storage_(allocator, nullptr) {
    auto& alloc = Initialize(count);
    FillElements(alloc, begin_, end_);
  }

  Vector(SizeType count,
         ConstReference value,
         const AllocatorType& allocator = AllocatorType())
      : storage_(allocator, nullptr) {
    auto& alloc = Initialize(count);
    FillElements(alloc, begin_, end_, value);
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  Vector(InputIt first,
         InputIt last,
         const AllocatorType& allocator = AllocatorType())
      : storage_(allocator, nullptr), begin_(nullptr), end_(nullptr) {
    while (first != last) {
      Push(*first++);
    }
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  Vector(ForwardIt first,
         ForwardIt last,
         const AllocatorType& allocator = AllocatorType())
      : storage_(allocator, nullptr) {
    const auto count = static_cast<SizeType>(std::distance(first, last));
    auto& alloc = Initialize(count);
    CopyElements(alloc, begin_, first, last);
  }

  Vector(std::initializer_list<ValueType> values,
         const AllocatorType& allocator = AllocatorType())
      : storage_(allocator, nullptr) {
    auto& alloc = Initialize(static_cast<SizeType>(values.size()));
    CopyElements(alloc, begin_, values.begin(), values.end());
  }

  Vector(const Vector& other) : Vector(other, other.InternalAllocator()) {}

  Vector(const Vector& other, const AllocatorType& allocator)
      : storage_(allocator, nullptr) {
    auto& alloc = Initialize(other.Size());
    CopyElements(alloc, begin_, other.begin_, other.end_);
  }

  Vector(Vector&& other) noexcept
      : storage_(Move(other.InternalAllocator()), other.GetEndOfStorage()),
        begin_(other.begin_),
        end_(other.end_) {
    other.begin_ = nullptr;
    other.end_ = nullptr;
    other.GetEndOfStorage() = nullptr;
  }

  Vector(Vector&& other, const AllocatorType& allocator)
      : storage_(allocator, nullptr) {
    if (!other.begin_) {
      return;
    }

    if (InternalAllocator() == other.InternalAllocator()) {
      auto& other_end_of_storage = other.GetEndOfStorage();

      begin_ = other.begin_;
      end_ = other.end_;
      GetEndOfStorage() = other_end_of_storage;

      other.begin_ = nullptr;
      other.end_ = nullptr;
      other_end_of_storage = nullptr;
      return;
    }

    auto& alloc = Initialize(other.Size());
    MoveElements(alloc, begin_, other.begin_, other.end_);
    other.Clear();
  }

  ~Vector() { Release(InternalAllocator(), begin_, end_, GetEndOfStorage()); }

  Vector& operator=(const Vector& other) {
    if (AXIO_LIKELY(this != &other)) {
      if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::
                        value) {
        auto& allocator = InternalAllocator();
        const auto& other_allocator = other.InternalAllocator();
        if (allocator != other_allocator) {
          Pointer& end_of_storage = GetEndOfStorage();
          Release(InternalAllocator(), begin_, end_, end_of_storage);
          begin_ = nullptr;
          end_ = nullptr;
          end_of_storage = nullptr;
          InternalAllocator() = other.InternalAllocator();
        }
      }
      Assign(other.begin_, other.end_);
    }
    return *this;
  }

  Vector& operator=(Vector&& other) noexcept(
      AllocatorTraits::propagate_on_container_move_assignment::value ||
      AllocatorTraits::is_always_equal::value) {
    if (AXIO_LIKELY(this != &other)) {
      auto& allocator = InternalAllocator();
      const auto& other_allocator = other.InternalAllocator();
      constexpr bool can_propagate =
          AllocatorTraits::propagate_on_container_move_assignment::value;

      if (can_propagate || allocator == other_allocator) {
        Pointer& end_of_storage = GetEndOfStorage();
        Release(allocator, begin_, end_, end_of_storage);
        if constexpr (can_propagate) {
          allocator = Move(other_allocator);
        }
        begin_ = other.begin_;
        end_ = other.end_;
        end_of_storage = other.GetEndOfStorage();
        other.begin_ = other.end_ = other.GetEndOfStorage() = nullptr;
      } else {
        Assign(std::make_move_iterator(other.begin_),
               std::make_move_iterator(other.end_));
        other.Clear();
      }
    }
    return *this;
  }

  Vector& operator=(std::initializer_list<ValueType> values) {
    Assign(values.begin(), values.end());
    return *this;
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  void Assign(InputIt first, InputIt last) {
    Pointer beg = begin_;
    while (beg < end_ && first != last) {
      *beg++ = *first++;
    }

    if (beg == end_) {
      while (first != last) {
        Push(*first++);
      }
    } else {
      DestroyElements(InternalAllocator(), beg, end_);
      end_ = beg;
    }
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  void Assign(ForwardIt first, ForwardIt last) {
    const SizeType count = std::distance(first, last);
    const SizeType old_size = Size();
    if (AXIO_LIKELY(count > old_size)) {
      Pointer old_end_of_storage = GetEndOfStorage();
      const SizeType current_capacity = old_end_of_storage - begin_;
      if (AXIO_LIKELY(count > current_capacity)) {
        auto& allocator = InternalAllocator();
        Pointer old_begin = Allocate(allocator, count);
        CopyElements(allocator, begin_, first, last);
        Release(allocator, old_begin, end_, old_end_of_storage);
        end_ = begin_ + count;
      } else {
        first = CopyAssignElements<true>(begin_, end_, first);
        CopyElements(InternalAllocator(), end_, first, last);
        end_ = begin_ + count;
      }
    } else {
      Pointer new_end = begin_ + count;
      CopyAssignElements<false>(begin_, new_end, first);
      DestroyElements(InternalAllocator(), new_end, end_);
      end_ = new_end;
    }
  }

  void Assign(SizeType count, ConstReference value) {
    const SizeType old_size = Size();
    if (AXIO_LIKELY(count > old_size)) {
      Pointer old_end_of_storage = GetEndOfStorage();
      const SizeType current_capacity = old_end_of_storage - begin_;
      if (AXIO_LIKELY(count > current_capacity)) {
        auto& allocator = InternalAllocator();
        Pointer old_end = end_;
        Pointer old_begin = Allocate(allocator, count);
        end_ = begin_ + count;
        FillElements(allocator, begin_, end_, value);
        Release(allocator, old_begin, old_end, old_end_of_storage);
      } else {
        Pointer new_end = begin_ + count;
        FillAssignElements(begin_, end_, value);
        FillElements(InternalAllocator(), end_, new_end, value);
        end_ = new_end;
      }
    } else {
      Pointer new_end = begin_ + count;
      FillAssignElements(begin_, new_end, value);
      DestroyElements(InternalAllocator(), new_end, end_);
      end_ = new_end;
    }
  }

  void Assign(std::initializer_list<ValueType> values) {
    Assign(values.begin(), values.end());
  }

  void Clear() {
    if (end_ > begin_) {
      DestroyElements(InternalAllocator(), begin_, end_);
      end_ = begin_;
    }
  }

  void Resize(SizeType new_size) { ResizeImpl(new_size); }

  void Resize(SizeType new_size, ConstReference value) {
    ResizeImpl(new_size, value);
  }

  void Reserve(SizeType new_capacity) {
    if (Capacity() < new_capacity) {
      Reallocate(new_capacity);
    }
  }

  void Shrink() {
    if (GetEndOfStorage() > end_) {
      Reallocate(end_ - begin_);
    }
  }

  AllocatorType GetAllocator() const { return storage_.template Get<0>(); }

  Bool IsEmpty() const noexcept { return begin_ == end_; }

  SizeType Size() const noexcept { return end_ - begin_; }

  SizeType Capacity() const noexcept { return GetEndOfStorage() - begin_; }

  Pointer Data() noexcept { return begin_; }
  ConstPointer Data() const noexcept { return begin_; }

  SizeType MaxSize() const noexcept {
    static constexpr auto kMaxSize =
        std::numeric_limits<SizeType>::max() / sizeof(ValueType);
    return std::min(kMaxSize, AllocatorTraits::max_size(InternalAllocator()));
  }

  Reference At(SizeType pos) {
    if (AXIO_LIKELY(pos >= Size())) {
      throw std::out_of_range("Vector::At(SizeType) - index out of range");
    }
    return begin_[pos];
  }

  ConstReference At(SizeType pos) const {
    if (AXIO_LIKELY(pos >= Size())) {
      throw std::out_of_range(
          "Vector::At(SizeType) const - index out of range");
    }
    return begin_[pos];
  }

  Reference operator[](SizeType pos) {
    AXIO_ASSERT(pos < Size());
    return *(begin_ + pos);
  }

  ConstReference operator[](SizeType pos) const {
    AXIO_ASSERT(pos < Size());
    return *(begin_ + pos);
  }

  Reference Front() {
    AXIO_ASSERT(!IsEmpty());
    return *begin_;
  }

  ConstReference Front() const {
    AXIO_ASSERT(!IsEmpty());
    return *begin_;
  }

  Reference Back() {
    AXIO_ASSERT(!IsEmpty());
    return *(end_ - 1);
  }

  ConstReference Back() const {
    AXIO_ASSERT(!IsEmpty());
    return *(end_ - 1);
  }

  Iterator begin() noexcept { return begin_; }
  Iterator end() noexcept { return end_; }

  ConstIterator begin() const noexcept { return begin_; }
  ConstIterator end() const noexcept { return end_; }

  ConstIterator cbegin() const noexcept { return begin_; }
  ConstIterator cend() const noexcept { return end_; }

  ReverseIterator rbegin() noexcept { return ReverseIterator(end_); }
  ReverseIterator rend() noexcept { return ReverseIterator(begin_); }

  ConstReverseIterator rbegin() const noexcept {
    return ConstReverseIterator(end_);
  }
  ConstReverseIterator rend() const noexcept {
    return ConstReverseIterator(begin_);
  }

  ConstReverseIterator crbegin() const noexcept {
    return ConstReverseIterator(end_);
  }
  ConstReverseIterator crend() const noexcept {
    return ConstReverseIterator(begin_);
  }

  template <typename... ArgTypes>
  Reference Push(ArgTypes&&... args) {
    Pointer end_of_storage = GetEndOfStorage();
    if (end_ == end_of_storage) {
      auto& allocator = InternalAllocator();
      SizeType size = Size();
      Pointer old_end = end_;
      Pointer old_begin = Allocate(
          allocator,
          ComputeCapacity(static_cast<SizeType>(end_of_storage - begin_), 1));
      end_ = begin_ + size;
      AllocatorTraits::construct(allocator, end_, Forward<ArgTypes>(args)...);
      MoveElements(allocator, begin_, old_begin, old_end);
      Release(allocator, old_begin, old_end, end_of_storage);
    } else {
      AllocatorTraits::construct(InternalAllocator(), end_,
                                 Forward<ArgTypes>(args)...);
    }
    return *(end_++);
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  void Append(InputIt first, InputIt last) {
    while (first != last) {
      Push(*first++);
    }
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  void Append(ForwardIt first, ForwardIt last) {
    const SizeType count = std::distance(first, last);
    Pointer end_of_storage = GetEndOfStorage();
    if (end_ + count > end_of_storage) {
      auto& allocator = InternalAllocator();
      SizeType size = Size();
      Pointer old_end = end_;
      Pointer old_begin =
          Allocate(allocator,
                   ComputeCapacity(
                       static_cast<SizeType>(end_of_storage - begin_), count));
      end_ = begin_ + size;
      CopyElements(allocator, end_, first, last);
      MoveElements(allocator, begin_, old_begin, old_end);
      Release(allocator, old_begin, old_end, end_of_storage);
    } else {
      CopyElements(InternalAllocator(), end_, first, last);
    }
    end_ += count;
  }

  void Append(std::initializer_list<ValueType> values) {
    Append(values.begin(), values.end());
  }

  void Append(SizeType count, ConstReference value) {
    Pointer end_of_storage = GetEndOfStorage();
    if (end_ + count > end_of_storage) {
      auto& allocator = InternalAllocator();
      SizeType size = Size();
      Pointer old_end = end_;
      Pointer old_begin =
          Allocate(allocator,
                   ComputeCapacity(
                       static_cast<SizeType>(end_of_storage - begin_), count));
      end_ = begin_ + size;
      FillElements(allocator, end_, end_ + count, value);
      MoveElements(allocator, begin_, old_begin, old_end);
      Release(allocator, old_begin, old_end, end_of_storage);
    } else {
      FillElements(InternalAllocator(), end_, end_ + count, value);
    }
    end_ += count;
  }

  Iterator Remove(ConstIterator pos) {
    AXIO_ASSERT(pos >= begin_ && pos < end_);
    Iterator pos_it = begin_ + (pos - begin_);
    MoveAssignElements(pos_it, pos_it + 1, end_);
    AllocatorTraits::destroy(InternalAllocator(), --end_);
    return pos_it;
  }

  Iterator Remove(ConstIterator first, ConstIterator last) {
    AXIO_ASSERT(first >= begin_ && first <= last && last <= end_);
    const SizeType count = static_cast<SizeType>(last - first);
    Iterator pos_it = begin_ + (first - begin_);
    MoveAssignElements(pos_it, pos_it + count, end_);
    Pointer new_end = end_ - count;
    DestroyElements(InternalAllocator(), new_end, end_);
    end_ = new_end;
    return begin_ + (first - begin_);
  }

  void Pop() {
    AXIO_ASSERT(end_ != begin_);
    AllocatorTraits::destroy(InternalAllocator(), --end_);
  }

  template <typename... ArgTypes>
  Iterator Emplace(ConstIterator pos, ArgTypes&&... args) {
    AXIO_ASSERT(pos >= begin_ && pos <= end_);

    if (pos == end_) {
      Push(Forward<ArgTypes>(args)...);
      return end_ - 1;
    }

    SizeType idx = static_cast<SizeType>(pos - begin_);
    Iterator pos_it = begin_ + idx;
    Pointer end_of_storage = GetEndOfStorage();
    auto& allocator = InternalAllocator();
    if (end_ == end_of_storage) {
      Pointer old_end = end_;
      Pointer old_begin = Allocate(
          allocator,
          ComputeCapacity(static_cast<SizeType>(end_of_storage - begin_), 1));
      end_ = begin_ + (old_end - old_begin) + 1;
      Iterator final_it = begin_ + idx;
      AllocatorTraits::construct(allocator, final_it,
                                 Forward<ArgTypes>(args)...);
      MoveElements(allocator, begin_, old_begin, pos_it);
      MoveElements(allocator, final_it + 1, pos_it, old_end);
      Release(allocator, old_begin, old_end, end_of_storage);
      return final_it;
    } else {
      ValueType value(Forward<ArgTypes>(args)...);
      AllocatorTraits::construct(allocator, end_, Move(*(end_ - 1)));
      MoveAssignBackward(end_, pos_it, end_ - 1);
      *pos_it = Move(value);
      ++end_;
    }
    return pos_it;
  }

  Iterator Insert(ConstIterator pos, SizeType count, ConstReference value) {
    AXIO_ASSERT(pos >= begin_ && pos <= end_);
    if (count == 0) {
      return begin_ + (pos - begin_);
    }
    if (pos == end_) {
      Append(count, value);
      return end_ - count;
    }

    SizeType idx = static_cast<SizeType>(pos - begin_);
    Iterator pos_it = begin_ + idx;
    Pointer end_of_storage = GetEndOfStorage();
    auto& allocator = InternalAllocator();
    if (end_ + count > end_of_storage) {
      Pointer old_end = end_;
      Pointer old_begin =
          Allocate(allocator,
                   ComputeCapacity(
                       static_cast<SizeType>(end_of_storage - begin_), count));
      end_ = begin_ + (old_end - old_begin) + count;
      Iterator first = begin_ + idx;
      Iterator last = first + count;
      FillElements(allocator, first, last, value);
      MoveElements(allocator, begin_, old_begin, old_begin + idx);
      MoveElements(allocator, last, old_begin + idx, old_end);
      Release(allocator, old_begin, old_end, end_of_storage);
      return first;
    } else {
      const ValueType copy = value;
      const auto size = static_cast<SizeType>(end_ - begin_);
      const auto insert_end = idx + count;
      if (insert_end > size) {
        auto dst = end_ + idx;
        MoveElements(allocator, dst, dst - size, end_);
        FillAssignElements(pos_it, pos_it + (size - idx), copy);
        FillElements(allocator, end_, dst, copy);
      } else {
        MoveElements(allocator, end_, end_ - count, end_);
        MoveAssignBackward(end_, pos_it, end_ - count);
        FillAssignElements(pos_it, pos_it + count, copy);
      }
      end_ += count;
    }
    return pos_it;
  }

  Iterator Insert(ConstIterator pos, std::initializer_list<ValueType> values) {
    return Insert(pos, values.begin(), values.end());
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  Iterator Insert(ConstIterator pos, InputIt first, InputIt last) {
    auto idx = static_cast<SizeType>(pos - begin_);
    const auto temp_idx = idx;
    while (first != last) {
      Emplace(begin_ + idx++, *first++);
    }
    return begin_ + temp_idx;
  }

  // Note: This function does not support self-insertion. Passing iterators from
  // the same vector instance results in undefined behavior.
  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  Iterator Insert(ConstIterator pos, ForwardIt first, ForwardIt last) {
    AXIO_ASSERT(pos >= begin_ && pos <= end_);
    const auto count = static_cast<SizeType>(std::distance(first, last));
    if (count == 0) {
      return begin_ + (pos - begin_);
    }

    SizeType idx = static_cast<SizeType>(pos - begin_);
    Iterator pos_it = begin_ + idx;
    Pointer end_of_storage = GetEndOfStorage();
    auto& allocator = InternalAllocator();
    if (end_ + count > end_of_storage) {
      Pointer old_end = end_;
      Pointer old_begin =
          Allocate(allocator,
                   ComputeCapacity(
                       static_cast<SizeType>(end_of_storage - begin_), count));
      end_ = begin_ + (old_end - old_begin) + count;
      auto dst = begin_ + idx;
      auto old_pos = old_begin + idx;
      CopyElements(allocator, dst, first, last);
      MoveElements(allocator, begin_, old_begin, old_pos);
      MoveElements(allocator, dst + count, old_pos, old_end);
      Release(allocator, old_begin, old_end, end_of_storage);
      return dst;
    } else {
      const auto after_elems = static_cast<SizeType>(end_ - pos);
      auto old_end = end_;
      if (after_elems > count) {
        MoveElements(allocator, end_, end_ - count, end_);
        MoveAssignBackward(old_end, pos_it, old_end - count);
        CopyAssignElements<false>(pos_it, pos_it + count, first);
      } else {
        MoveElements(allocator, end_ + (count - after_elems), pos_it, end_);
        first = CopyAssignElements<true>(pos_it, pos_it + after_elems, first);
        CopyElements(allocator, end_, first, last);
      }
      end_ += count;
    }
    return pos_it;
  }

 private:
  template <typename DstPointer, typename InputIt>
  struct ShouldUseMemcpy {
    using ValueType =
        axio::T<RemoveCV<typename std::iterator_traits<InputIt>::value_type>>;
    using DestType = axio::T<
        RemoveCV<typename std::pointer_traits<DstPointer>::element_type>>;

    static constexpr Bool value = Conjunction<IsTriviallyCopyable<DestType>,
                                              IsSame<DestType, ValueType>,
                                              IsPointer<InputIt>>::value;
  };

  SizeType ComputeCapacity(SizeType old_capacity, SizeType add_size) {
    if (old_capacity == 0) {
      return AXIO_MAX(add_size, SizeType(8));
    }

    const SizeType max_size = MaxSize();
    if ((old_capacity > max_size / kGrowthFactor) ||
        (old_capacity > max_size - add_size)) {
      return max_size;
    }

    const SizeType grown = old_capacity * kGrowthFactor;
    const SizeType required = old_capacity + add_size;
    return AXIO_MAX(grown, required);
  }

  static void Release(AllocatorType& allocator,
                      Pointer old_begin,
                      Pointer old_end,
                      Pointer old_end_of_storage) {
    if (!old_begin) {
      return;
    }
    if (old_begin != old_end) {
      DestroyElements(allocator, old_begin, old_end);
    }
    AllocatorTraits::deallocate(allocator, old_begin,
                                old_end_of_storage - old_begin);
  }

  Pointer Allocate(AllocatorType& allocator, SizeType capacity) {
    Pointer old_begin = begin_;
    begin_ = AllocatorTraits::allocate(allocator, capacity);
    GetEndOfStorage() = begin_ + capacity;
    return old_begin;
  }

  void Reallocate(SizeType new_capacity) {
    auto& allocator = InternalAllocator();
    Pointer old_end_of_storage = GetEndOfStorage();
    Pointer old_begin = Allocate(allocator, new_capacity);
    Pointer old_end = end_;
    SizeType size = old_end - old_begin;
    MoveElements(allocator, begin_, old_begin, old_end);
    Release(allocator, old_begin, old_end, old_end_of_storage);
    end_ = begin_ + size;
  }

  AllocatorType& Initialize(SizeType count) {
    auto& allocator = InternalAllocator();
    begin_ = AllocatorTraits::allocate(allocator, count);
    end_ = begin_ + count;
    GetEndOfStorage() = end_;
    return allocator;
  }

  template <typename... ArgTypes>
  void ResizeImpl(SizeType new_size, ArgTypes&&... args) {
    const auto current_size = Size();
    if (new_size == current_size) {
      return;
    }

    if (new_size > current_size) {
      if (new_size > Capacity()) {
        Reallocate(new_size);
      }
      Pointer new_end = begin_ + new_size;
      FillElements(InternalAllocator(), end_, new_end,
                   Forward<ArgTypes>(args)...);
      end_ = new_end;
    } else {
      Pointer new_end = begin_ + new_size;
      DestroyElements(InternalAllocator(), new_end, end_);
      end_ = new_end;
    }
  }

  Pointer& GetEndOfStorage() { return storage_.template Get<1>(); }
  const Pointer& GetEndOfStorage() const { return storage_.template Get<1>(); }

  AllocatorType& InternalAllocator() { return storage_.template Get<0>(); }
  const AllocatorType& InternalAllocator() const {
    return storage_.template Get<0>();
  }

  static void DestroyElements(AllocatorType& allocator,
                              Pointer first,
                              Pointer last) {
    if constexpr (!axio::V<IsTriviallyCopyConstructible<ValueType>>) {
      while (first != last) {
        AllocatorTraits::destroy(allocator, first++);
      }
    }
  }

  template <typename InputIt>
  static void CopyElements(AllocatorType& allocator,
                           Pointer dst,
                           InputIt first,
                           InputIt last) {
    using UseMemcpy = ShouldUseMemcpy<Pointer, InputIt>;
    if constexpr (UseMemcpy::value) {
      AXIO_IGNORE(allocator);
      std::memcpy(dst, first,
                  static_cast<SizeT>(last - first) *
                      sizeof(typename UseMemcpy::DestType));
    } else {
      Pointer current = dst;
      try {
        while (first != last) {
          AllocatorTraits::construct(allocator, current++, *first++);
        }
      } catch (...) {
        while (dst != current) {
          AllocatorTraits::destroy(allocator, dst++);
        }
        throw;
      }
    }
  }

  template <Bool RETURN_INPUT_IT, typename InputIt>
  static axio::T<Conditional<RETURN_INPUT_IT, InputIt, void>>
  CopyAssignElements(Pointer first, Pointer last, InputIt source) {
    using UseMemcpy = ShouldUseMemcpy<Pointer, InputIt>;
    if constexpr (UseMemcpy::value) {
      SizeType count = static_cast<SizeType>(last - first);
      std::memcpy(first, source, count * sizeof(typename UseMemcpy::DestType));
      if constexpr (RETURN_INPUT_IT) {
        return source + count;
      }
    } else {
      while (first != last) {
        *first++ = *source++;
      }
      if constexpr (RETURN_INPUT_IT) {
        return source;
      }
    }
  }

  template <typename InputIt>
  static void MoveElements(AllocatorType& allocator,
                           Pointer dst,
                           InputIt first,
                           InputIt last) {
    using UseMemcpy = ShouldUseMemcpy<Pointer, InputIt>;

    if constexpr (UseMemcpy::value) {
      AXIO_IGNORE(allocator);
      std::memcpy(dst, first,
                  static_cast<SizeT>(last - first) *
                      sizeof(typename UseMemcpy::DestType));
    } else {
      Pointer current = dst;
      try {
        while (first != last) {
          AllocatorTraits::construct(allocator, current++, Move(*first++));
        }
      } catch (...) {
        while (dst != current) {
          AllocatorTraits::destroy(allocator, dst++);
        }
        throw;
      }
    }
  }

  template <typename InputIt>
  static void MoveAssignElements(Pointer dst, InputIt first, InputIt last) {
    using UseMemcpy = ShouldUseMemcpy<Pointer, InputIt>;

    if constexpr (UseMemcpy::value) {
      std::memcpy(dst, first,
                  static_cast<SizeT>(last - first) *
                      sizeof(typename UseMemcpy::DestType));
    } else {
      while (first != last) {
        *dst++ = Move(*first++);
      }
    }
  }

  template <typename InputIt>
  static void MoveAssignBackward(Pointer dst, InputIt first, InputIt last) {
    using UseMemcpy = ShouldUseMemcpy<Pointer, InputIt>;

    if constexpr (UseMemcpy::value) {
      const auto count = static_cast<SizeType>(last - first);
      std::memmove(dst - count, first,
                   count * sizeof(typename UseMemcpy::DestType));
    } else {
      while (first != last) {
        *--dst = Move(*--last);
      }
    }
  }

  static void FillElements(AllocatorType& allocator,
                           Pointer first,
                           Pointer last) {
    using DestType =
        axio::T<RemoveCV<typename std::pointer_traits<Pointer>::element_type>>;
    if constexpr (IsScalar<ValueType>::value) {
      std::memset(first, 0,
                  static_cast<SizeType>(last - first) * sizeof(DestType));
    } else {
      Pointer current = first;
      try {
        while (current != last) {
          AllocatorTraits::construct(allocator, current++);
        }
      } catch (...) {
        while (first != current) {
          AllocatorTraits::destroy(allocator, first++);
        }
        throw;
      }
    }
  }

  static void FillElements(AllocatorType& allocator,
                           Pointer first,
                           Pointer last,
                           ConstReference value) {
    using DestType =
        axio::T<RemoveCV<typename std::pointer_traits<Pointer>::element_type>>;

    if constexpr (IsScalar<DestType>::value) {
      if (AXIO_LIKELY(value == static_cast<DestType>(0) ||
                      sizeof(DestType) == 1)) {
        std::memset(first, value,
                    static_cast<SizeType>(last - first) * sizeof(DestType));
      } else {
        while (first != last) {
          AllocatorTraits::construct(allocator, first++, value);
        }
      }
    } else {
      Pointer current = first;
      try {
        while (current != last) {
          AllocatorTraits::construct(allocator, current++, value);
        }
      } catch (...) {
        while (first != current) {
          AllocatorTraits::destroy(allocator, first++);
        }
        throw;
      }
    }
  }

  static void FillAssignElements(Pointer first,
                                 Pointer last,
                                 ConstReference value) {
    using DestType =
        axio::T<RemoveCV<typename std::pointer_traits<Pointer>::element_type>>;
    if constexpr (IsScalar<DestType>::value) {
      if (AXIO_LIKELY(value == static_cast<DestType>(0) ||
                      sizeof(DestType) == 1)) {
        std::memset(first, value,
                    static_cast<SizeType>(last - first) * sizeof(DestType));
        return;
      }
    }
    while (first != last) {
      *first++ = value;
    }
  }

  CompressedPair<AllocatorType, Pointer /* end of storage */> storage_;
  Pointer begin_;
  Pointer end_;
};

template <typename T, typename A>
Bool operator==(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return lhs.Size() == rhs.Size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename A>
Bool operator!=(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return !(lhs == rhs);
}

template <typename T, typename A>
Bool operator<(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                      rhs.end());
}

template <typename T, typename A>
Bool operator<=(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return !(rhs < lhs);
}

template <typename T, typename A>
Bool operator>(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return rhs < lhs;
}

template <typename T, typename A>
Bool operator>=(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return !(lhs < rhs);
}
}  // namespace axio

#endif