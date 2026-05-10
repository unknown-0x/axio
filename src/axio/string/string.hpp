#ifndef AXIO_STRING_STRING_HPP_
#define AXIO_STRING_STRING_HPP_

#include <limits>
#include <stdexcept>
#include <string>

#include "../base/endianness.hpp"
#include "../container/detail/iterator_traits.hpp"
#include "../memory/allocator.hpp"
#include "../utility/move.hpp"

namespace axio {
namespace internal {
template <typename A, Bool = ShouldUseEBO<A>::value>
struct AllocatorHolder : public A {
  AllocatorHolder() noexcept(noexcept(A())) = default;
  AllocatorHolder(const A& other) noexcept : A(other) {}

  A& GetAlloc() noexcept { return *this; }
  const A& GetAlloc() const noexcept { return *this; }
};

template <typename A>
struct AllocatorHolder<A, false> {
  A alloc;

  AllocatorHolder() noexcept(noexcept(A())) = default;
  AllocatorHolder(const A& other) : alloc(other) {}

  A& GetAlloc() { return alloc; }
  const A& GetAlloc() const { return alloc; }
};
}  // namespace internal

template <typename T,
          typename Traits = std::char_traits<T>,
          typename A = Allocator<T>>
class BasicString : private internal::AllocatorHolder<A> {
  using AllocatorHolder = internal::AllocatorHolder<A>;
  using AllocatorTraits = std::allocator_traits<A>;

  template <typename It>
  using EnableIfForwardIt =
      typename EnableIf<IsForwardIterator<It>::value, int>::type;

  template <typename It>
  using EnableIfNotForwardIt =
      typename EnableIf<IsInputIterator<It>::value &&
                            !IsForwardIterator<It>::value,
                        int>::type;

  template <typename U>
  struct IsContiguousIterator : FalseType {};

  template <typename U>
  struct IsContiguousIterator<U*> : TrueType {};

  template <typename U>
  struct IsContiguousIterator<const U*> : TrueType {};

 public:
  using TraitsType = Traits;
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

  static constexpr SizeType kNpos = SizeType(-1);
  static constexpr SizeType kGrowthFactor = SizeType(2);
  static constexpr ValueType kNullTerminator = ValueType();

  static_assert(!IsArray<ValueType>::value,
                "ValueType must not be an array type");
  static_assert(IsStandardLayout<ValueType>::value,
                "ValueType must be a standard-layout type");
  static_assert(IsTriviallyCopyable<ValueType>::value,
                "ValueType must be trivially copyable");

 private:
  using StringViewType = std::basic_string_view<ValueType, TraitsType>;

  template <typename StringViewLike, typename Dummy>
  using EnableIfIsStringViewLike = typename EnableIf<
      IsConvertible<const StringViewLike&, StringViewType>::value &&
          !IsConvertible<const StringViewLike&, ConstPointer>::value,
      Dummy>::type;

 public:
  BasicString() noexcept(noexcept(AllocatorType())) : AllocatorHolder() {
    SetModeAsSSO(0);
  }

  explicit BasicString(const AllocatorType& allocator)
      : AllocatorHolder(allocator) {
    SetModeAsSSO(0);
  }

  BasicString(ConstPointer s, const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    const SizeType n = TraitsType::length(s);
    TraitsType::copy(InitWithSize(n), s, n);
  }

  BasicString(ConstPointer s,
              SizeType count,
              const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    TraitsType::copy(InitWithSize(count), s, count);
  }

  BasicString(SizeType count,
              ValueType c,
              const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    TraitsType::assign(InitWithSize(count), count, c);
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  BasicString(InputIt first,
              InputIt last,
              const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    SetModeAsSSO(0);
    while (first != last) {
      Push(*first++);
    }
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  BasicString(ForwardIt first,
              ForwardIt last,
              const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    const auto n = static_cast<SizeType>(std::distance(first, last));
    Copy(InitWithSize(n), first, n);
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  explicit BasicString(const StringViewLike& sv,
                       const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    const StringViewType view(sv);
    const auto n = static_cast<SizeType>(view.size());
    TraitsType::copy(InitWithSize(n), view.data(), n);
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString(const StringViewLike& sv,
              SizeType pos,
              SizeType count = kNpos,
              const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    const StringViewType view(sv);
    const auto size = static_cast<SizeType>(view.size());
    const auto n =
        (count == kNpos || pos + count > size) ? (size - pos) : count;
    TraitsType::copy(InitWithSize(n), view.data() + pos, n);
  }

  BasicString(std::initializer_list<ValueType> values,
              const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    const auto n = static_cast<SizeType>(values.size());
    TraitsType::copy(InitWithSize(n), values.begin(), n);
  }

  BasicString(const BasicString& other) : AllocatorHolder(other.GetAlloc()) {
    const auto n = other.Size();
    TraitsType::copy(InitWithSize(n), other.Data(), n);
  }

  BasicString(BasicString&& other) noexcept
      : AllocatorHolder(Move(other.GetAlloc())) {
    storage_ = other.storage_;
    other.SetModeAsSSO(0);
  }

  BasicString(const BasicString& other, const AllocatorType& allocator)
      : AllocatorHolder(allocator) {
    const auto n = other.Size();
    TraitsType::copy(InitWithSize(n), other.Data(), n);
  }

  BasicString(BasicString&& other, const AllocatorType& allocator)
      : AllocatorHolder(allocator) {
    if (this->GetAlloc() == other.GetAlloc()) {
      storage_ = other.storage_;
      other.SetModeAsSSO(0);
      return;
    }

    const auto n = other.Size();
    TraitsType::copy(InitWithSize(n), other.Data(), n);
  }

  BasicString(const BasicString& other,
              SizeType pos,
              SizeType count = kNpos,
              const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    const auto n = other.Size();
    AXIO_ASSERT(pos <= n);
    const auto actual_count =
        (count == kNpos || pos + count > n) ? (n - pos) : count;
    TraitsType::copy(InitWithSize(actual_count), other.Data() + pos,
                     actual_count);
  }

  BasicString(NullPtrT) = delete;

  ~BasicString() { Release(this->GetAlloc()); }

  operator StringViewType() const noexcept {
    return StringViewType(Data(), Size());
  }

  BasicString& operator=(const BasicString& other) {
    if (this == &other) {
      return *this;
    }

    if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::
                      value) {
      auto& allocator = this->GetAlloc();
      const auto& other_allocator = other.GetAlloc();
      if (allocator != other_allocator) {
        Release(allocator);
        allocator = other_allocator;
        const auto size = other.Size();
        Traits::copy(InitWithSize(size), other.Data(), size);
        return *this;
      }
    }

    Assign(other.Data(), other.Size());
    return *this;
  }

  BasicString& operator=(BasicString&& other) noexcept(
      noexcept(AllocatorTraits::propagate_on_container_move_assignment::value ||
               AllocatorTraits::is_always_equal::value)) {
    if (this == &other) {
      return *this;
    }

    auto& allocator = this->GetAlloc();

    constexpr bool kCanStealStorage =
        AllocatorTraits::propagate_on_container_move_assignment::value ||
        AllocatorTraits::is_always_equal::value;

    if (kCanStealStorage || allocator == other.GetAlloc()) {
      Release(allocator);
      if constexpr (AllocatorTraits::propagate_on_container_move_assignment::
                        value) {
        allocator = Move(other.GetAlloc());
      }
      storage_ = other.storage_;
      other.SetModeAsSSO(0);
    } else {
      Assign(other.Data(), other.Size());
    }

    return *this;
  }

  BasicString& operator=(ConstPointer s) {
    return Assign(s, TraitsType::length(s));
  }

  BasicString& operator=(ValueType c) {
    if (IsSSO()) {
      TraitsType::assign(storage_.sso.data[0], c);
      SetModeAsSSO(1);
    } else {
      TraitsType::assign(storage_.heap.data[0], c);
      SetHeapSize(1);
    }
    return *this;
  }

  BasicString& operator=(std::initializer_list<ValueType> values) {
    return Assign(values.begin(), static_cast<SizeType>(values.size()));
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& operator=(const StringViewLike& sv) {
    const StringViewType view(sv);
    return Assign(view.data(), static_cast<SizeType>(view.size()));
  }

  BasicString& operator=(NullPtrT) = delete;

  BasicString& Assign(const BasicString& other) { return operator=(other); }

  BasicString& Assign(BasicString&& other) noexcept(
      AllocatorTraits::propagate_on_container_move_assignment::value ||
      AllocatorTraits::is_always_equal::value) {
    return operator=(Move(other));
  }

  BasicString& Assign(SizeType n, ValueType c) {
    if (n <= Capacity()) {
      if (IsSSO()) {
        SetModeAsSSO(static_cast<unsigned char>(n));
        TraitsType::assign(storage_.sso.data, n, c);
      } else {
        SetHeapSize(n);
        TraitsType::assign(storage_.heap.data, n, c);
      }
      return *this;
    }
    Release(this->GetAlloc());
    TraitsType::assign(InitWithSize(n), n, c);
    return *this;
  }

  BasicString& Assign(ConstPointer s) {
    return Assign(s, TraitsType::length(s));
  }

  BasicString& Assign(ConstPointer s, SizeType n) {
    if (n <= Capacity()) {
      if (IsSSO()) {
        SetModeAsSSO(static_cast<unsigned char>(n));
        TraitsType::copy(storage_.sso.data, s, n);
      } else {
        SetHeapSize(n);
        TraitsType::copy(storage_.heap.data, s, n);
      }
      return *this;
    }
    Release(this->GetAlloc());
    TraitsType::copy(InitWithSize(n), s, n);
    return *this;
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& Assign(const StringViewLike& sv) {
    const StringViewType view(sv);
    return Assign(view.data(), static_cast<SizeType>(view.size()));
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& Assign(const StringViewLike& sv,
                      SizeType pos,
                      SizeType count = kNpos) {
    const StringViewType view(sv);
    const auto size = static_cast<SizeType>(view.size());
    const auto n =
        (count == kNpos || pos + count > size) ? (size - pos) : count;
    return Assign(view.data() + pos, n);
  }

  BasicString& Assign(const BasicString& other,
                      SizeType pos,
                      SizeType count = kNpos) {
    const auto size = other.Size();
    const auto n =
        (count == kNpos || pos + count > size) ? (size - pos) : count;
    return Assign(other.Data() + pos, n);
  }

  BasicString& Assign(std::initializer_list<ValueType> values) {
    return Assign(values.begin(), static_cast<SizeType>(values.size()));
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  BasicString& Assign(InputIt first, InputIt last) {
    Clear();
    while (first != last) {
      Push(*first++);
    }
    return *this;
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  BasicString& Assign(ForwardIt first, ForwardIt last) {
    const auto n = static_cast<SizeType>(std::distance(first, last));
    if (n > Capacity()) {
      Release(this->GetAlloc());
      InitWithSize(n);
    }

    Copy(Data(), first, n);
    IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(n)) : SetHeapSize(n);
    return *this;
  }

  AllocatorType GetAllocator() const { return this->GetAlloc(); }

  Bool IsEmpty() const noexcept { return Size() == 0; }

  SizeType Size() const noexcept {
    return IsSSO() ? kSSOCapacity - storage_.raw[kModeByteOffset]
                   : storage_.heap.size;
  }

  SizeType Length() const noexcept { return Size(); }

  SizeType Capacity() const noexcept {
    return IsSSO() ? kSSOCapacity : GetHeapCapacity();
  }

  Pointer Data() noexcept {
    return IsSSO() ? storage_.sso.data : storage_.heap.data;
  }

  ConstPointer Data() const noexcept {
    return IsSSO() ? storage_.sso.data : storage_.heap.data;
  }

  ConstPointer CStr() const noexcept { return Data(); }

  SizeType MaxSize() const noexcept {
    static constexpr auto kMaxSz =
        std::numeric_limits<SizeType>::max() / sizeof(ValueType);
    return std::min(kMaxSz, AllocatorTraits::max_size(this->GetAlloc())) - 1;
  }

  Reference At(SizeType pos) {
    if (AXIO_LIKELY(pos >= Size())) {
      throw std::out_of_range("BasicString::At(SizeType) - index out of range");
    }
    return Data()[pos];
  }

  ConstReference At(SizeType pos) const {
    if (AXIO_LIKELY(pos >= Size())) {
      throw std::out_of_range(
          "BasicString::At(SizeType) const - index out of range");
    }
    return Data()[pos];
  }

  Reference operator[](SizeType pos) {
    AXIO_ASSERT(pos < Size());
    return *(Data() + pos);
  }

  ConstReference operator[](SizeType pos) const {
    AXIO_ASSERT(pos < Size());
    return *(Data() + pos);
  }

  Reference Front() {
    AXIO_ASSERT(!IsEmpty());
    return *begin();
  }

  ConstReference Front() const {
    AXIO_ASSERT(!IsEmpty());
    return *begin();
  }

  Reference Back() {
    AXIO_ASSERT(!IsEmpty());
    return *(end() - 1);
  }

  ConstReference Back() const {
    AXIO_ASSERT(!IsEmpty());
    return *(end() - 1);
  }

  Iterator begin() noexcept { return Data(); }
  Iterator end() noexcept { return Data() + Size(); }

  ConstIterator begin() const noexcept { return Data(); }
  ConstIterator end() const noexcept { return Data() + Size(); }

  ConstIterator cbegin() const noexcept { return Data(); }
  ConstIterator cend() const noexcept { return Data() + Size(); }

  ReverseIterator rbegin() noexcept { return ReverseIterator(Data() + Size()); }
  ReverseIterator rend() noexcept { return ReverseIterator(Data()); }

  ConstReverseIterator rbegin() const noexcept {
    return ConstReverseIterator(Data() + Size());
  }
  ConstReverseIterator rend() const noexcept {
    return ConstReverseIterator(Data());
  }

  ConstReverseIterator crbegin() const noexcept {
    return ConstReverseIterator(Data() + Size());
  }
  ConstReverseIterator crend() const noexcept {
    return ConstReverseIterator(Data());
  }

  void Clear() { IsSSO() ? SetModeAsSSO(0) : SetHeapSize(0); }

  void Resize(SizeType n, ValueType c = kNullTerminator) {
    const auto old_size = Size();
    if (n == old_size)
      return;

    if (n < old_size) {
      IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(n)) : SetHeapSize(n);
      return;
    }

    if (n > Capacity()) {
      Reallocate<false>(n, old_size);
    }

    if (IsSSO()) {
      TraitsType::assign(storage_.sso.data + old_size, n - old_size, c);
      SetModeAsSSO(static_cast<unsigned char>(n));
      return;
    }

    TraitsType::assign(storage_.heap.data + old_size, n - old_size, c);
    SetHeapSize(n);
  }

  void Reserve(SizeType new_capacity) {
    if (new_capacity > Capacity()) {
      Reallocate(new_capacity, Size());
    }
  }

  void Shrink() {
    const auto size = Size();
    const auto is_sso = IsSSO();
    if ((is_sso && size <= kSSOCapacity) || (!is_sso && size == Capacity())) {
      return;
    }

    if (size <= kSSOCapacity) {
      auto old_data = storage_.heap.data;
      const auto old_capacity = storage_.heap.capacity;
      TraitsType::copy(storage_.sso.data, old_data, size);
      AllocatorTraits::deallocate(this->GetAlloc(), old_data, old_capacity + 1);
      SetModeAsSSO(static_cast<unsigned char>(size));
      return;
    }

    Reallocate(size, size);
  }

  Reference Push(ValueType c) {
    auto size = Size();
    const auto capacity = Capacity();
    if (size == capacity) {
      Reallocate<false>(ComputeCapacity(capacity, 1), size);
    }

    auto data = Data();
    data[size++] = c;

    IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(size))
            : SetHeapSize(size);

    return data[size - 1];
  }

  BasicString& Remove(SizeType index = 0, SizeType count = kNpos) {
    const auto size = Size();
    AXIO_ASSERT(index <= size);

    if (index == size || count == 0) {
      return *this;
    }

    Pointer data = Data();
    SizeType new_size = 0;
    if (count == kNpos || index + count >= size) {
      new_size = index;
    } else {
      new_size = size - count;
      auto dest = data + index;
      TraitsType::copy(dest, dest + count, new_size - index);
    }

    IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(new_size))
            : SetHeapSize(new_size);
    return *this;
  }

  Iterator Remove(ConstIterator pos) {
    auto b = begin();
    AXIO_ASSERT(pos >= b && pos < end());
    const auto index = static_cast<SizeType>(pos - b);
    Remove(index, 1);
    return b + index;
  }

  Iterator Remove(ConstIterator first, ConstIterator last) {
    auto b = begin();
    AXIO_ASSERT(first >= b && first <= last && last <= end());
    const auto index = static_cast<SizeType>(first - b);
    const auto count = static_cast<SizeType>(last - first);
    Remove(index, count);
    return b + index;
  }

  void Pop() {
    AXIO_ASSERT(!IsEmpty());
    const auto new_size = Size() - 1;
    IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(new_size))
            : SetHeapSize(new_size);
  }

  BasicString& Append(SizeType n, ValueType c) {
    const auto size = Size();
    const auto capacity = Capacity();
    if (n > capacity - size) {
      Reallocate<false>(ComputeCapacity(capacity, n), size);
    }

    TraitsType::assign(Data() + size, n, c);
    IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(size + n))
            : SetHeapSize(size + n);

    return *this;
  }

  BasicString& Append(ConstPointer s, SizeType n) { return Append(s, s + n); }

  BasicString& Append(ConstPointer s) {
    return Append(s, s + TraitsType::length(s));
  }

  BasicString& Append(std::initializer_list<ValueType> values) {
    return Append(values.begin(), values.end());
  }

  BasicString& Append(const BasicString& other) {
    const auto other_data = other.Data();
    return Append(other_data, other_data + other.Size());
  }

  BasicString& Append(const BasicString& other,
                      SizeType pos,
                      SizeType count = kNpos) {
    const auto other_size = other.Size();
    AXIO_ASSERT(pos <= other_size);
    const auto append_count = (count == kNpos || pos + count > other_size)
                                  ? (other_size - pos)
                                  : count;
    const auto data_pos = other.Data() + pos;
    return Append(data_pos, data_pos + append_count);
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& Append(const StringViewLike& sv) {
    const StringViewType view(sv);
    return Append(view.begin(), view.end());
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& Append(const StringViewLike& sv,
                      SizeType pos,
                      SizeType count = kNpos) {
    const StringViewType view(sv);
    const auto view_size = view.size();
    AXIO_ASSERT(pos <= view_size);
    const auto append_count =
        (count == kNpos || pos + count > view_size) ? (view_size - pos) : count;
    const auto data_pos = view.begin() + pos;
    return Append(data_pos, data_pos + append_count);
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  BasicString& Append(InputIt first, InputIt last) {
    while (first != last) {
      Push(*first++);
    }
    return *this;
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  BasicString& Append(ForwardIt first, ForwardIt last) {
    const auto n = static_cast<SizeType>(std::distance(first, last));
    const auto size = Size();
    const auto capacity = Capacity();
    if (n > capacity - size) {
      const auto new_capacity = ComputeCapacity(capacity, n);
      auto& allocator = this->GetAlloc();
      auto new_data = AllocatorTraits::allocate(allocator, new_capacity + 1);
      auto old_data = Data();
      TraitsType::copy(new_data, old_data, size);
      Copy(new_data + size, first, n);
      Release(allocator);

      storage_.heap.data = new_data;
      storage_.heap.capacity = new_capacity;
      SetModeAsHeap();
      SetHeapSize(size + n);
    } else {
      Copy(Data() + size, first, n);
      IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(size + n))
              : SetHeapSize(size + n);
    }

    return *this;
  }

 private:
  struct HeapStorage {
    Pointer data;
    SizeType size;
    SizeType capacity;
  };

 public:
  static constexpr SizeType kSSOCapacity =
      sizeof(HeapStorage) / sizeof(ValueType) - 1;

 private:
  struct SSOStorage {
    ValueType data[kSSOCapacity + 1];  // +1 for null terminator
  };

  static_assert(sizeof(SSOStorage) == sizeof(HeapStorage),
                "SSOStorage and HeapStorage must have the same size");

  union Storage {
    HeapStorage heap;
    SSOStorage sso;
    unsigned char raw[sizeof(HeapStorage)];
  };

  Storage storage_;

  static constexpr unsigned char kHeapMask = 0x40;

  static constexpr SizeType kModeByteOffset =
      Endian::kNative == Endian::kLittle ? sizeof(HeapStorage) - 1 : 0;

  static constexpr SizeType kCapacityMask =
      Endian::kNative == Endian::kLittle
          ? ~(SizeType(0xFF) << ((sizeof(SizeType) - 1) * 8))
          : ~SizeType(0xFF);

  Bool IsSSO() const noexcept {
    return (storage_.raw[kModeByteOffset] & kHeapMask) == 0;
  }

  void SetModeAsSSO(unsigned char len) {
    storage_.sso.data[len] = kNullTerminator;
    storage_.raw[kModeByteOffset] =
        static_cast<unsigned char>(kSSOCapacity - len);
  }

  void SetModeAsHeap() { storage_.raw[kModeByteOffset] |= kHeapMask; }

  void SetHeapCapacity(SizeType n) {
    storage_.heap.capacity =
        (n & kCapacityMask) | (storage_.heap.capacity & ~kCapacityMask);
  }

  SizeType GetHeapCapacity() const noexcept {
    return storage_.heap.capacity & kCapacityMask;
  }

  void SetHeapSize(const SizeType n) {
    storage_.heap.size = n;
    storage_.heap.data[n] = kNullTerminator;
  }

  Pointer InitWithSize(const SizeType n) {
    if (n <= kSSOCapacity) {
      SetModeAsSSO(static_cast<unsigned char>(n));
      return storage_.sso.data;
    }

    SetModeAsHeap();
    storage_.heap.data = AllocatorTraits::allocate(this->GetAlloc(), n + 1);
    SetHeapCapacity(n);
    SetHeapSize(n);
    return storage_.heap.data;
  }

  template <Bool SET_HEAP_SIZE_NOW = true>
  void Reallocate(const SizeType new_capacity, const SizeType old_size) {
    auto& allocator = this->GetAlloc();
    auto new_data = AllocatorTraits::allocate(allocator, new_capacity + 1);
    TraitsType::copy(new_data, Data(), old_size);
    Release(allocator);

    storage_.heap.data = new_data;
    storage_.heap.capacity = new_capacity;

    if constexpr (SET_HEAP_SIZE_NOW) {
      SetHeapSize(old_size);
    }
    SetModeAsHeap();
  }

  SizeType ComputeCapacity(SizeType old_capacity, SizeType add_size) {
    const auto remaining = MaxSize() - old_capacity;
    if (add_size > remaining) {
      throw std::length_error("BasicString capacity overflow");
    }
    auto required = old_capacity + add_size;
    auto new_capacity = old_capacity * kGrowthFactor;
    return AXIO_MAX(required, new_capacity);
  }

  void Release(AllocatorType& allocator) {
    if (!IsSSO()) {
      AllocatorTraits::deallocate(allocator, storage_.heap.data,
                                  storage_.heap.capacity + 1);
    }
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  static void Copy(Pointer dst, ForwardIt first, SizeType n) {
    if constexpr (IsContiguousIterator<
                      typename Decay<ForwardIt>::type>::value) {
      TraitsType::copy(dst, first, n);
    } else {
      const auto end = dst + n;
      for (; dst != end; ++first, ++dst) {
        TraitsType::assign(*dst, *first);
      }
    }
  }
};

// ========================================================= //

template <typename T, typename Traits, typename A>
Bool operator==(const BasicString<T, Traits, A>& lhs,
                const BasicString<T, Traits, A>& rhs);

template <typename T, typename Traits, typename A>
Bool operator!=(const BasicString<T, Traits, A>& lhs,
                const BasicString<T, Traits, A>& rhs);

template <typename T, typename Traits, typename A>
Bool operator>=(const BasicString<T, Traits, A>& lhs,
                const BasicString<T, Traits, A>& rhs);

template <typename T, typename Traits, typename A>
Bool operator<=(const BasicString<T, Traits, A>& lhs,
                const BasicString<T, Traits, A>& rhs);

template <typename T, typename Traits, typename A>
Bool operator>(const BasicString<T, Traits, A>& lhs,
               const BasicString<T, Traits, A>& rhs);

template <typename T, typename Traits, typename A>
Bool operator<(const BasicString<T, Traits, A>& lhs,
               const BasicString<T, Traits, A>& rhs);

// ========================================================= //

template <typename T, typename Traits, typename A>
Bool operator==(const BasicString<T, Traits, A>& lhs, const T* rhs);

template <typename T, typename Traits, typename A>
Bool operator!=(const BasicString<T, Traits, A>& lhs, const T* rhs);

template <typename T, typename Traits, typename A>
Bool operator>=(const BasicString<T, Traits, A>& lhs, const T* rhs);

template <typename T, typename Traits, typename A>
Bool operator<=(const BasicString<T, Traits, A>& lhs, const T* rhs);

template <typename T, typename Traits, typename A>
Bool operator>(const BasicString<T, Traits, A>& lhs, const T* rhs);

template <typename T, typename Traits, typename A>
Bool operator<(const BasicString<T, Traits, A>& lhs, const T* rhs);

// ========================================================= //

template <typename T, typename Traits, typename A>
Bool operator==(const T* lhs, const BasicString<T, Traits, A>& rhs);

template <typename T, typename Traits, typename A>
Bool operator!=(const T* lhs, const BasicString<T, Traits, A>& rhs);

template <typename T, typename Traits, typename A>
Bool operator>=(const T* lhs, const BasicString<T, Traits, A>& rhs);

template <typename T, typename Traits, typename A>
Bool operator<=(const T* lhs, const BasicString<T, Traits, A>& rhs);

template <typename T, typename Traits, typename A>
Bool operator>(const T* lhs, const BasicString<T, Traits, A>& rhs);

template <typename T, typename Traits, typename A>
Bool operator<(const T* lhs, const BasicString<T, Traits, A>& rhs);

// ========================================================= //

using String = BasicString<char>;
using WString = BasicString<wchar_t>;
#ifdef __cpp_char8_t
using U8String = BasicString<char8_t>;
#endif
using U16String = BasicString<char16_t>;
using U32String = BasicString<char32_t>;
}  // namespace axio

#endif