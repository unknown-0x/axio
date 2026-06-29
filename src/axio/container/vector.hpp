#ifndef AXIO_CONTAINER_VECTOR_HPP_
#define AXIO_CONTAINER_VECTOR_HPP_

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

#include "detail/allocator_holder.hpp"
#include "detail/iterator_traits.hpp"

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"
#include "../memory/allocator.hpp"

#include "../utility/forward.hpp"
#include "../utility/move.hpp"

#include "../string/axio_repr.hpp"

namespace axio {
/**
 * @brief A dynamically-sized, contiguous, growable array, analogous to
 *        std::vector.
 * @tparam T Element type.
 * @tparam A Allocator type.
 */
template <typename T, typename A = axio::Allocator<T>>
class Vector : private detail::AllocatorHolder<A> {
  using AllocatorHolder = detail::AllocatorHolder<A>;
  using AllocatorTraits = std::allocator_traits<A>;

  template <typename It>
  using EnableIfForwardIt = EnableIf_T<IsForwardIterator_V<It>, int>;

  template <typename It>
  using EnableIfNotForwardIt =
      EnableIf_T<IsInputIterator_V<It> && !IsForwardIterator_V<It>, int>;

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

  /** Capacity multiplier applied on growth reallocation. */
  static constexpr SizeType kGrowthFactor = SizeType(2);

  /** Constructs an empty Vector with a default-constructed allocator. */
  Vector() noexcept(noexcept(AllocatorType())) : Vector(AllocatorType()) {}

  /** Constructs an empty Vector using the given allocator. */
  explicit Vector(const AllocatorType& allocator)
      : AllocatorHolder(allocator),
        begin_(nullptr),
        end_(nullptr),
        storage_end_(nullptr) {}

  /** Constructs a Vector with count default-constructed elements. */
  explicit Vector(SizeType count,
                  const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    auto& alloc = Initialize(count);
    FillElements(alloc, begin_, end_);
  }

  /** Constructs a Vector with count copies of value. */
  Vector(SizeType count,
         ConstReference value,
         const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    auto& alloc = Initialize(count);
    FillElements(alloc, begin_, end_, value);
  }

  /** Constructs a Vector from an input-iterator range (single-pass). */
  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  Vector(InputIt first,
         InputIt last,
         const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator),
        begin_(nullptr),
        end_(nullptr),
        storage_end_(nullptr) {
    while (first != last) {
      Push(*first);
      ++first;
    }
  }

  /** Constructs a Vector from a forward-iterator range (multi-pass). */
  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  Vector(ForwardIt first,
         ForwardIt last,
         const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    const auto count = static_cast<SizeType>(std::distance(first, last));
    auto& alloc = Initialize(count);
    CopyElements(alloc, begin_, first, last);
  }

  /** Constructs a Vector from an initializer list. */
  Vector(std::initializer_list<ValueType> values,
         const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    auto& alloc = Initialize(static_cast<SizeType>(values.size()));
    CopyElements(alloc, begin_, values.begin(), values.end());
  }

  /** Copy-constructs, reusing other's allocator. */
  Vector(const Vector& other) : Vector(other, other.GetAlloc()) {}

  /** Copy-constructs using the given allocator. */
  Vector(const Vector& other, const AllocatorType& allocator)
      : AllocatorHolder(allocator) {
    auto& alloc = Initialize(other.Size());
    CopyElements(alloc, begin_, other.begin_, other.end_);
  }

  /** Move-constructs, taking ownership of other's storage. */
  Vector(Vector&& other) noexcept
      : AllocatorHolder(Move(other.GetAlloc())),
        begin_(other.begin_),
        end_(other.end_),
        storage_end_(other.storage_end_) {
    other.begin_ = nullptr;
    other.end_ = nullptr;
    other.storage_end_ = nullptr;
  }

  /** Move-constructs using the given allocator. Steals other's storage
   *  when allocators compare equal, otherwise moves elements one by one. */
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

  /** Copy-assigns the contents of other into this Vector. */
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

  /** Move-assigns the contents of other into this Vector. */
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

  /** Replaces the contents with an initializer list. */
  Vector& operator=(std::initializer_list<ValueType> values) {
    Assign(values.begin(), values.end());
    return *this;
  }

  /** Replaces the contents with elements from an input-iterator range. */
  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  void Assign(InputIt first, InputIt last) {
    auto beg = begin_;
    while (beg < end_ && first != last) {
      *beg++ = *first;
      ++first;
    }

    if (beg == end_) {
      while (first != last) {
        Push(*first);
        ++first;
      }
    } else {
      DestroyElements(this->GetAlloc(), beg, end_);
      end_ = beg;
    }
  }

  /** Replaces the contents with elements from a forward-iterator range. */
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

  /** Replaces the contents with count copies of value. */
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

  /** Replaces the contents with an initializer list. */
  void Assign(std::initializer_list<ValueType> values) {
    Assign(values.begin(), values.end());
  }

  /** Destroys all elements, leaving the Vector empty without changing
   *  capacity. */
  void Clear() {
    if (end_ > begin_) {
      DestroyElements(this->GetAlloc(), begin_, end_);
      end_ = begin_;
    }
  }

  /** Resizes to new_size, default-constructing any new elements. */
  void Resize(SizeType new_size) { ResizeImpl(new_size); }

  /** Resizes to new_size, copy-constructing any new elements from value. */
  void Resize(SizeType new_size, ConstReference value) {
    ResizeImpl(new_size, value);
  }

  /** Ensures capacity is at least new_capacity, reallocating if needed. */
  void Reserve(SizeType new_capacity) {
    if (Capacity() < new_capacity) {
      Reallocate(new_capacity);
    }
  }

  /** Reduces capacity to fit the current size. */
  void Shrink() {
    if (storage_end_ > end_) {
      Reallocate(static_cast<SizeType>(end_ - begin_));
    }
  }

  /** Returns a copy of the allocator. */
  AllocatorType GetAllocator() const { return this->GetAlloc(); }

  /** Returns true if the Vector has no elements. */
  Bool IsEmpty() const noexcept { return begin_ == end_; }

  /** Returns the number of elements. */
  SizeType Size() const noexcept {
    return static_cast<SizeType>(end_ - begin_);
  }

  /** Returns the number of elements the current storage can hold. */
  SizeType Capacity() const noexcept {
    return static_cast<SizeType>(storage_end_ - begin_);
  }

  /** Returns a pointer to the underlying element storage. */
  Pointer Data() noexcept { return begin_; }
  /** Returns a const pointer to the underlying element storage. */
  ConstPointer Data() const noexcept { return begin_; }

  /** Returns the maximum number of elements the Vector could hold. */
  SizeType MaxSize() const noexcept {
    static constexpr auto kMaxSz =
        std::numeric_limits<SizeType>::max() / sizeof(ValueType);
    return std::min(kMaxSz, AllocatorTraits::max_size(this->GetAlloc()));
  }

  /** Returns a reference to the element at pos, throwing
   *  std::out_of_range if pos is out of bounds. */
  Reference At(SizeType pos) {
    if (AXIO_UNLIKELY(pos >= Size())) {
      throw std::out_of_range("Vector::At(SizeType): index " +
                              std::to_string(pos) + " out of range");
    }
    return begin_[pos];
  }

  /** Const overload of At(). */
  ConstReference At(SizeType pos) const {
    if (AXIO_UNLIKELY(pos >= Size())) {
      throw std::out_of_range("Vector::At(SizeType) const: index " +
                              std::to_string(pos) + " out of range");
    }
    return begin_[pos];
  }

  /** Returns a reference to the element at pos. Asserts pos is in range. */
  Reference operator[](SizeType pos) {
    AXIO_ASSERT(pos < Size());
    return *(begin_ + pos);
  }

  /** Const overload of operator[](). */
  ConstReference operator[](SizeType pos) const {
    AXIO_ASSERT(pos < Size());
    return *(begin_ + pos);
  }

  /** Returns a reference to the first element. Asserts non-empty. */
  Reference Front() {
    AXIO_ASSERT(!IsEmpty());
    return *begin_;
  }

  /** Const overload of Front(). */
  ConstReference Front() const {
    AXIO_ASSERT(!IsEmpty());
    return *begin_;
  }

  /** Returns a reference to the last element. Asserts non-empty. */
  Reference Back() {
    AXIO_ASSERT(!IsEmpty());
    return *(end_ - 1);
  }

  /** Const overload of Back(). */
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

  /**
   * Constructs a new element in-place at the end, growing storage if
   * needed. Returns a reference to the new element.
   */
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

  /** Appends elements from an input-iterator range to the end. */
  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  void Append(InputIt first, InputIt last) {
    while (first != last) {
      Push(*first);
      ++first;
    }
  }

  /** Appends elements from a forward-iterator range to the end. */
  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  void Append(ForwardIt first, ForwardIt last) {
    const auto count = static_cast<SizeType>(std::distance(first, last));
    if (end_ + count > storage_end_) {
      auto& allocator = this->GetAlloc();
      const auto size = Size();
      const auto capacity =
          ComputeCapacity(static_cast<SizeType>(storage_end_ - begin_), count);
      auto new_begin = AllocatorTraits::allocate(allocator, capacity);
      CopyElements(allocator, new_begin + size, first, last);
      MoveElements(allocator, new_begin, begin_, end_);
      Release(allocator);
      SetStorage(new_begin, size + count, capacity);
    } else {
      CopyElements(this->GetAlloc(), end_, first, last);
      end_ += count;
    }
  }

  /** Appends elements from an initializer list to the end. */
  void Append(std::initializer_list<ValueType> values) {
    Append(values.begin(), values.end());
  }

  /** Appends count copies of value to the end. */
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

  /** Removes the element at pos, shifting subsequent elements left.
   *  Returns an iterator to the element following the removed one. */
  Iterator Remove(ConstIterator pos) {
    AXIO_ASSERT(pos >= begin_ && pos < end_);
    Iterator pos_it = begin_ + (pos - begin_);
    MoveAssignElements(pos_it, pos_it + 1, end_);
    AllocatorTraits::destroy(this->GetAlloc(), --end_);
    return pos_it;
  }

  /** Removes the elements in [first, last), shifting subsequent elements
   *  left. Returns an iterator to the element following the removed range. */
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

  /** Removes the last element. Asserts the Vector is non-empty. */
  void Pop() {
    AXIO_ASSERT(end_ != begin_);
    AllocatorTraits::destroy(this->GetAlloc(), --end_);
  }

  /** Constructs a new element in-place before pos, growing storage if
   *  needed. Returns an iterator to the inserted element. */
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

  /** Inserts count copies of value before pos. Returns an iterator to the
   *  first inserted element. */
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

  /** Inserts elements of an initializer list before pos. */
  Iterator Insert(ConstIterator pos, std::initializer_list<ValueType> values) {
    return Insert(pos, values.begin(), values.end());
  }

  /** Inserts elements from an input-iterator range before pos. */
  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  Iterator Insert(ConstIterator pos, InputIt first, InputIt last) {
    auto idx = static_cast<SizeType>(pos - begin_);
    const auto temp_idx = idx;
    while (first != last) {
      Emplace(begin_ + idx++, *first);
      ++first;
    }
    return begin_ + temp_idx;
  }

  /** Inserts elements from a forward-iterator range before pos.
   *  @note: This function does not support self-insertion. Passing
   *  iterators from the same vector instance results in undefined
   *  behavior. */
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
  /** Trait: true when copying/moving InputIt -> DstPointer can be done via
   *  memcpy (trivially copyable, matching value types, contiguous source). */
  template <typename DstPointer, typename InputIt>
  struct ShouldUseMemcpy {
    using ValueType =
        RemoveCV_T<typename std::iterator_traits<InputIt>::value_type>;
    using DestType =
        RemoveCV_T<typename std::pointer_traits<DstPointer>::element_type>;

    static constexpr Bool value = Conjunction_V<IsTriviallyCopyable<DestType>,
                                                IsSame<DestType, ValueType>,
                                                IsPointer<InputIt>>;
  };

  /** Computes the new capacity needed to add add_size elements, applying
   *  the growth factor and clamping to MaxSize(). */
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

  /** Destroys all elements and deallocates storage, if any. */
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

  /** Sets begin_/end_/storage_end_ from a freshly allocated buffer. */
  void SetStorage(Pointer new_begin, SizeType new_size, SizeType new_capacity) {
    begin_ = new_begin;
    end_ = begin_ + new_size;
    storage_end_ = begin_ + new_capacity;
  }

  /** Reallocates storage to the given capacity, moving existing elements. */
  void Reallocate(SizeType capacity) {
    auto& allocator = this->GetAlloc();
    auto new_begin = AllocatorTraits::allocate(allocator, capacity);
    auto size = static_cast<SizeType>(end_ - begin_);
    MoveElements(allocator, new_begin, begin_, end_);
    Release(allocator);
    SetStorage(new_begin, size, capacity);
  }

  /**
   * Allocates storage for count elements and sets size == capacity == count.
   */
  AllocatorType& Initialize(SizeType count) {
    auto& allocator = this->GetAlloc();
    if (count > 0) {
      SetStorage(AllocatorTraits::allocate(allocator, count), count, count);
    } else {
      begin_ = end_ = storage_end_ = nullptr;
    }
    return allocator;
  }

  /** Shared implementation for both Resize() overloads. */
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

  /**
   * Destroys elements in [first, last); no-op for trivially-destructible
   * types.
   */
  static void DestroyElements(AllocatorType& allocator,
                              Pointer first,
                              Pointer last) {
    if constexpr (!IsTriviallyDestructible_V<ValueType>) {
      while (first != last) {
        AllocatorTraits::destroy(allocator, first++);
      }
    }
  }

  /**
   * Copy-constructs elements at dst from [first, last), using memcpy
   * when possible. Cleans up on exception.
   */
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

  /**
   * Copy-assigns elements in [first, last) from source, using memcpy
   *  when possible. Optionally returns the advanced source iterator.
   */
  template <Bool RETURN_INPUT_IT, typename InputIt>
  static Conditional_T<RETURN_INPUT_IT, InputIt, void>
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

  /**
   * Move-constructs elements at dst from [first, last), using memcpy
   *  when possible. Cleans up on exception.
   */
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

  /**
   * Move-assigns elements at dst from [first, last), using memcpy when
   *  possible. Does not handle overlapping forward ranges.
   */
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

  /**
   * Move-assigns elements from [first, last) ending at dst, iterating
   *  backward; safe for overlapping ranges where dst > first.
   */
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

  /**
   * Default-constructs elements in [first, last); zero-fills for scalar
   *  types. Cleans up on exception.
   */
  static void FillElements(AllocatorType& allocator,
                           Pointer first,
                           Pointer last) {
    using DestType =
        RemoveCV_T<typename std::pointer_traits<Pointer>::element_type>;
    if constexpr (IsScalar_V<ValueType>) {
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

  /**
   *  Constructs elements in [first, last) as copies of value; uses
   *  memset where applicable for scalar types. Cleans up on exception.
   */
  static void FillElements(AllocatorType& allocator,
                           Pointer first,
                           Pointer last,
                           ConstReference value) {
    using DestType =
        RemoveCV_T<typename std::pointer_traits<Pointer>::element_type>;

    if constexpr (IsScalar_V<DestType>) {
      if constexpr (sizeof(DestType) == 1) {
        std::memset(first, static_cast<unsigned char>(value),
                    static_cast<SizeType>(last - first));
        return;
      }
      if (value == static_cast<DestType>(0)) {
        std::memset(first, 0,
                    static_cast<SizeType>(last - first) * sizeof(DestType));
        return;
      }
    }

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

  /**
   * Assigns value to existing elements in [first, last); uses memset
   *  where applicable for scalar types.
   */
  static void FillAssignElements(Pointer first,
                                 Pointer last,
                                 ConstReference value) {
    using DestType =
        RemoveCV_T<typename std::pointer_traits<Pointer>::element_type>;
    if constexpr (IsScalar_V<DestType>) {
      if constexpr (sizeof(DestType) == 1) {
        std::memset(first, static_cast<unsigned char>(value),
                    static_cast<SizeType>(last - first));
        return;
      }
      if (value == static_cast<DestType>(0)) {
        std::memset(first, 0,
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

/** Returns true if lhs and rhs have equal size and elements. */
template <typename T, typename A>
Bool operator==(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return lhs.Size() == rhs.Size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/** Returns true if lhs and rhs differ in size or elements. */
template <typename T, typename A>
Bool operator!=(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return !(lhs == rhs);
}

/** Lexicographically compares lhs and rhs. */
template <typename T, typename A>
Bool operator<(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                      rhs.end());
}

/** Lexicographically compares lhs and rhs. */
template <typename T, typename A>
Bool operator<=(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return !(rhs < lhs);
}

/** Lexicographically compares lhs and rhs. */
template <typename T, typename A>
Bool operator>(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return rhs < lhs;
}

/** Lexicographically compares lhs and rhs. */
template <typename T, typename A>
Bool operator>=(const Vector<T, A>& lhs, const Vector<T, A>& rhs) {
  return !(lhs < rhs);
}

/** Appends the textual representation of vector, e.g. "[1, 2, 3]". */
template <typename Output, typename T, typename A>
void AxioRepr(Output& output, const Vector<T, A>& vector) {
  using SizeType = typename Vector<T, A>::SizeType;
  const auto size = vector.Size();
  if (size == 0) {
    output.Append("[]", 2);
    return;
  }
  output.Append(1, '[');
  SizeType i = 0;
  for (const auto n = size - 1; i < n; ++i) {
    AppendToOutput(output, vector[i], ", ");
  }
  AppendToOutput(output, vector[i], ']');
}
}  // namespace axio

#endif