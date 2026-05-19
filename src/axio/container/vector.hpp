#ifndef AXIO_CONTAINER_VECTOR_HPP_
#define AXIO_CONTAINER_VECTOR_HPP_

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>

#include "detail/allocator_holder.hpp"
#include "detail/iterator_traits.hpp"

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"
#include "../memory/allocator.hpp"

#include "../utility/forward.hpp"
#include "../utility/move.hpp"

#include "../string/axio_repr.hpp"

namespace axio {
template <typename T, typename A = axio::Allocator<T>>
class Vector : private detail::AllocatorHolder<A> {
  using AllocatorHolder = detail::AllocatorHolder<A>;
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
      : AllocatorHolder(allocator),
        begin_(nullptr),
        end_(nullptr),
        storage_end_(nullptr) {}

  explicit Vector(SizeType count,
                  const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    auto& alloc = Initialize(count);
    FillElements(alloc, begin_, end_);
  }

  Vector(SizeType count,
         ConstReference value,
         const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    auto& alloc = Initialize(count);
    FillElements(alloc, begin_, end_, value);
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  Vector(InputIt first,
         InputIt last,
         const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator),
        begin_(nullptr),
        end_(nullptr),
        storage_end_(nullptr) {
    while (first != last) {
      Push(*first++);
    }
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  Vector(ForwardIt first,
         ForwardIt last,
         const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    const auto count = static_cast<SizeType>(std::distance(first, last));
    auto& alloc = Initialize(count);
    CopyElements(alloc, begin_, first, last);
  }

  Vector(std::initializer_list<ValueType> values,
         const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    auto& alloc = Initialize(static_cast<SizeType>(values.size()));
    CopyElements(alloc, begin_, values.begin(), values.end());
  }

  Vector(const Vector& other) : Vector(other, other.GetAlloc()) {}

  Vector(const Vector& other, const AllocatorType& allocator)
      : AllocatorHolder(allocator) {
    auto& alloc = Initialize(other.Size());
    CopyElements(alloc, begin_, other.begin_, other.end_);
  }

  Vector(Vector&& other) noexcept
      : AllocatorHolder(Move(other.GetAlloc())),
        begin_(other.begin_),
        end_(other.end_),
        storage_end_(other.storage_end_) {
    other.begin_ = nullptr;
    other.end_ = nullptr;
    other.storage_end_ = nullptr;
  }

  Vector(Vector&& other, const AllocatorType& allocator)
      : AllocatorHolder(allocator) {
    if (!other.begin_) {
      return;
    }

    if (this->GetAlloc() == other.GetAlloc()) {
      begin_ = other.begin_;
      end_ = other.end_;
      storage_end_ = other.storage_end_;

      other.begin_ = nullptr;
      other.end_ = nullptr;
      other.storage_end_ = nullptr;
      return;
    }

    auto& alloc = Initialize(other.Size());
    MoveElements(alloc, begin_, other.begin_, other.end_);
    other.Clear();
  }

  ~Vector() { Release(this->GetAlloc()); }

  Vector& operator=(const Vector& other) {
    if (AXIO_LIKELY(this != &other)) {
      if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::
                        value) {
        auto& allocator = this->GetAlloc();
        const auto& other_allocator = other.GetAlloc();
        if (allocator != other_allocator) {
          Release(allocator);
          begin_ = nullptr;
          end_ = nullptr;
          storage_end_ = nullptr;
          allocator = other_allocator;
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
      auto& allocator = this->GetAlloc();
      const auto& other_allocator = other.GetAlloc();
      static constexpr bool kCanPropagate =
          AllocatorTraits::propagate_on_container_move_assignment::value;

      if (kCanPropagate || allocator == other_allocator) {
        Release(allocator);
        if constexpr (kCanPropagate) {
          allocator = Move(other_allocator);
        }
        begin_ = other.begin_;
        end_ = other.end_;
        storage_end_ = other.storage_end_;
        other.begin_ = other.end_ = other.storage_end_ = nullptr;
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
    auto beg = begin_;
    while (beg < end_ && first != last) {
      *beg++ = *first++;
    }

    if (beg == end_) {
      while (first != last) {
        Push(*first++);
      }
    } else {
      DestroyElements(this->GetAlloc(), beg, end_);
      end_ = beg;
    }
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  void Assign(ForwardIt first, ForwardIt last) {
    const auto count = static_cast<SizeType>(std::distance(first, last));
    const auto size = Size();
    if (AXIO_LIKELY(count > size)) {
      const auto capacity = static_cast<SizeType>(storage_end_ - begin_);
      if (AXIO_LIKELY(count > capacity)) {
        auto& allocator = this->GetAlloc();
        auto new_begin = AllocatorTraits::allocate(allocator, count);
        CopyElements(allocator, new_begin, first, last);
        Release(allocator);
        SetStorage(new_begin, count, count);
      } else {
        first = CopyAssignElements<true>(begin_, end_, first);
        CopyElements(this->GetAlloc(), end_, first, last);
        end_ = begin_ + count;
      }
    } else {
      auto new_end = begin_ + count;
      CopyAssignElements<false>(begin_, new_end, first);
      DestroyElements(this->GetAlloc(), new_end, end_);
      end_ = new_end;
    }
  }

  void Assign(SizeType count, ConstReference value) {
    const auto size = Size();
    if (AXIO_LIKELY(count > size)) {
      const auto capacity = static_cast<SizeType>(storage_end_ - begin_);
      if (AXIO_LIKELY(count > capacity)) {
        auto& allocator = this->GetAlloc();
        auto new_begin = AllocatorTraits::allocate(allocator, count);
        FillElements(allocator, new_begin, new_begin + count, value);
        Release(allocator);
        SetStorage(new_begin, count, count);
      } else {
        auto new_end = begin_ + count;
        FillAssignElements(begin_, end_, value);
        FillElements(this->GetAlloc(), end_, new_end, value);
        end_ = new_end;
      }
    } else {
      auto new_end = begin_ + count;
      FillAssignElements(begin_, new_end, value);
      DestroyElements(this->GetAlloc(), new_end, end_);
      end_ = new_end;
    }
  }

  void Assign(std::initializer_list<ValueType> values) {
    Assign(values.begin(), values.end());
  }

  void Clear() {
    if (end_ > begin_) {
      DestroyElements(this->GetAlloc(), begin_, end_);
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
    if (storage_end_ > end_) {
      Reallocate(static_cast<SizeType>(end_ - begin_));
    }
  }

  AllocatorType GetAllocator() const { return this->GetAlloc(); }

  Bool IsEmpty() const noexcept { return begin_ == end_; }

  SizeType Size() const noexcept {
    return static_cast<SizeType>(end_ - begin_);
  }

  SizeType Capacity() const noexcept {
    return static_cast<SizeType>(storage_end_ - begin_);
  }

  Pointer Data() noexcept { return begin_; }
  ConstPointer Data() const noexcept { return begin_; }

  SizeType MaxSize() const noexcept {
    static constexpr auto kMaxSz =
        std::numeric_limits<SizeType>::max() / sizeof(ValueType);
    return std::min(kMaxSz, AllocatorTraits::max_size(this->GetAlloc()));
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
    if (end_ == storage_end_) {
      auto& allocator = this->GetAlloc();
      auto size = Size();
      auto capacity =
          ComputeCapacity(static_cast<SizeType>(storage_end_ - begin_), 1);
      auto new_begin = AllocatorTraits::allocate(allocator, capacity);
      AllocatorTraits::construct(allocator, new_begin + size,
                                 Forward<ArgTypes>(args)...);
      MoveElements(allocator, new_begin, begin_, end_);
      Release(allocator);
      SetStorage(new_begin, size, capacity);
    } else {
      AllocatorTraits::construct(this->GetAlloc(), end_,
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
    const auto count = static_cast<SizeType>(std::distance(first, last));
    if (end_ + count > storage_end_) {
      auto& allocator = this->GetAlloc();
      auto size = Size();
      auto capacity =
          ComputeCapacity(static_cast<SizeType>(storage_end_ - begin_), count);
      auto new_begin = AllocatorTraits::allocate(allocator, capacity);
      CopyElements(allocator, new_begin + size, first, last);
      MoveElements(allocator, new_begin, begin_, end_);
      Release(allocator);
      SetStorage(new_begin, size, capacity);
    } else {
      CopyElements(this->GetAlloc(), end_, first, last);
    }
    end_ += count;
  }

  void Append(std::initializer_list<ValueType> values) {
    Append(values.begin(), values.end());
  }

  void Append(SizeType count, ConstReference value) {
    if (end_ + count > storage_end_) {
      auto& allocator = this->GetAlloc();
      auto size = Size();
      auto capacity =
          ComputeCapacity(static_cast<SizeType>(storage_end_ - begin_), count);
      auto new_begin = AllocatorTraits::allocate(allocator, capacity);
      auto new_end = new_begin + size;
      FillElements(allocator, new_end, new_end + count, value);
      MoveElements(allocator, new_begin, begin_, end_);
      Release(allocator);
      SetStorage(new_begin, size, capacity);
    } else {
      FillElements(this->GetAlloc(), end_, end_ + count, value);
    }
    end_ += count;
  }

  Iterator Remove(ConstIterator pos) {
    AXIO_ASSERT(pos >= begin_ && pos < end_);
    Iterator pos_it = begin_ + (pos - begin_);
    MoveAssignElements(pos_it, pos_it + 1, end_);
    AllocatorTraits::destroy(this->GetAlloc(), --end_);
    return pos_it;
  }

  Iterator Remove(ConstIterator first, ConstIterator last) {
    AXIO_ASSERT(first >= begin_ && first <= last && last <= end_);
    const SizeType count = static_cast<SizeType>(last - first);
    Iterator pos_it = begin_ + (first - begin_);
    MoveAssignElements(pos_it, pos_it + count, end_);
    Pointer new_end = end_ - count;
    DestroyElements(this->GetAlloc(), new_end, end_);
    end_ = new_end;
    return begin_ + (first - begin_);
  }

  void Pop() {
    AXIO_ASSERT(end_ != begin_);
    AllocatorTraits::destroy(this->GetAlloc(), --end_);
  }

  template <typename... ArgTypes>
  Iterator Emplace(ConstIterator pos, ArgTypes&&... args) {
    AXIO_ASSERT(pos >= begin_ && pos <= end_);

    if (pos == end_) {
      Push(Forward<ArgTypes>(args)...);
      return end_ - 1;
    }

    auto& allocator = this->GetAlloc();
    auto index = static_cast<SizeType>(pos - begin_);
    auto pos_it = begin_ + index;
    if (end_ == storage_end_) {
      auto size = Size();
      auto capacity =
          ComputeCapacity(static_cast<SizeType>(storage_end_ - begin_), 1);
      auto new_begin = AllocatorTraits::allocate(allocator, capacity);
      auto final_it = new_begin + index;
      AllocatorTraits::construct(allocator, final_it,
                                 Forward<ArgTypes>(args)...);
      MoveElements(allocator, new_begin, begin_, pos_it);
      MoveElements(allocator, final_it + 1, pos_it, end_);
      Release(allocator);
      SetStorage(new_begin, size + 1, capacity);
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

    auto& allocator = this->GetAlloc();
    auto index = static_cast<SizeType>(pos - begin_);
    auto pos_it = begin_ + index;
    if (end_ + count > storage_end_) {
      auto size = Size();
      auto capacity =
          ComputeCapacity(static_cast<SizeType>(storage_end_ - begin_), count);
      auto new_begin = AllocatorTraits::allocate(allocator, capacity);
      auto first = new_begin + index;
      auto last = first + count;
      FillElements(allocator, first, last, value);
      MoveElements(allocator, new_begin, begin_, begin_ + index);
      MoveElements(allocator, last, begin_ + index, end_);
      Release(allocator);
      SetStorage(new_begin, size + count, capacity);
      return first;
    } else {
      const ValueType copy = value;
      const auto size = static_cast<SizeType>(end_ - begin_);
      const auto insert_end = index + count;
      if (insert_end > size) {
        auto dst = end_ + index;
        MoveElements(allocator, dst, dst - size, end_);
        FillAssignElements(pos_it, pos_it + (size - index), copy);
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

    auto index = static_cast<SizeType>(pos - begin_);
    auto pos_it = begin_ + index;
    auto& allocator = this->GetAlloc();
    if (end_ + count > storage_end_) {
      auto size = Size();
      auto capacity =
          ComputeCapacity(static_cast<SizeType>(storage_end_ - begin_), count);
      auto new_begin = AllocatorTraits::allocate(allocator, capacity);
      auto dest = new_begin + index;
      auto old_pos = begin_ + index;
      CopyElements(allocator, dest, first, last);
      MoveElements(allocator, new_begin, begin_, old_pos);
      MoveElements(allocator, dest + count, old_pos, end_);
      Release(allocator);
      SetStorage(new_begin, size + count, capacity);
      return dest;
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

  void Release(AllocatorType& allocator) {
    if (!begin_) {
      return;
    }
    if (begin_ != end_) {
      DestroyElements(allocator, begin_, end_);
    }
    AllocatorTraits::deallocate(allocator, begin_,
                                static_cast<SizeType>(storage_end_ - begin_));
  }

  void SetStorage(Pointer new_begin, SizeType new_size, SizeType new_capacity) {
    begin_ = new_begin;
    end_ = begin_ + new_size;
    storage_end_ = begin_ + new_capacity;
  }

  void Reallocate(SizeType capacity) {
    auto& allocator = this->GetAlloc();
    auto new_begin = AllocatorTraits::allocate(allocator, capacity);
    auto size = static_cast<SizeType>(end_ - begin_);
    MoveElements(allocator, new_begin, begin_, end_);
    Release(allocator);
    SetStorage(new_begin, size, capacity);
  }

  AllocatorType& Initialize(SizeType count) {
    auto& allocator = this->GetAlloc();
    SetStorage(AllocatorTraits::allocate(allocator, count), count, count);
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
      auto new_end = begin_ + new_size;
      FillElements(this->GetAlloc(), end_, new_end, Forward<ArgTypes>(args)...);
      end_ = new_end;
    } else {
      auto new_end = begin_ + new_size;
      DestroyElements(this->GetAlloc(), new_end, end_);
      end_ = new_end;
    }
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

  Pointer begin_;
  Pointer end_;
  Pointer storage_end_;
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

template <typename Output, typename T, typename A>
void AxioRepr(Output& output, const Vector<T, A>& vector) {
  using SizeType = typename Vector<T, A>::SizeType;
  const auto size = vector.Size();
  if (size == 0) {
    output.Append("[]", 2);
    return;
  }
  output.Append('[');
  SizeType i = 0;
  for (const auto n = size - 1; i < n; ++i) {
    AppendToOutput(output, vector[i], ", ");
  }
  AppendToOutput(output, vector[i], ']');
}
}  // namespace axio

#endif