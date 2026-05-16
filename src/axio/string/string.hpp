#ifndef AXIO_STRING_STRING_HPP_
#define AXIO_STRING_STRING_HPP_

#include <limits>
#include <stdexcept>
#include <string>

#include "../base/endianness.hpp"
#include "../container/detail/allocator_holder.hpp"
#include "../container/detail/iterator_traits.hpp"
#include "../memory/allocator.hpp"
#include "../utility/move.hpp"

namespace axio {
template <typename T,
          typename Traits = std::char_traits<T>,
          typename A = axio::Allocator<T>>
class BasicString : private detail::AllocatorHolder<A> {
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
      TraitsType::assign(storage_.sso[0], c);
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
        TraitsType::assign(storage_.sso, n, c);
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
        TraitsType::copy(storage_.sso, s, n);
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
    return IsSSO() ? storage_.sso : storage_.heap.data;
  }

  ConstPointer Data() const noexcept {
    return IsSSO() ? storage_.sso : storage_.heap.data;
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
      TraitsType::assign(storage_.sso + old_size, n - old_size, c);
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
      const auto old_capacity = GetHeapCapacity();
      TraitsType::copy(storage_.sso, old_data, size);
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

      SetModeAsHeap(new_data, new_capacity, size + n);
    } else {
      Copy(Data() + size, first, n);
      IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(size + n))
              : SetHeapSize(size + n);
    }

    return *this;
  }

  BasicString& Insert(SizeType index, SizeType count, ValueType value) {
    Insert(begin() + index, count, value);
    return *this;
  }

  BasicString& Insert(SizeType index, ConstPointer s) {
    Insert(begin() + index, s, s + TraitsType::length(s));
    return *this;
  }

  BasicString& Insert(SizeType index, ConstPointer s, SizeType count) {
    Insert(begin() + index, s, s + count);
    return *this;
  }

  BasicString& Insert(SizeType index, const BasicString& other) {
    const auto other_data = other.Data();
    Insert(begin() + index, other_data, other_data + other.Size());
    return *this;
  }

  BasicString& Insert(SizeType index,
                      const BasicString& str,
                      SizeType pos,
                      SizeType count = kNpos) {
    const auto str_size = str.Size();
    AXIO_ASSERT(pos <= str_size);
    const auto insert_count =
        (count == kNpos || pos + count > str_size) ? (str_size - pos) : count;
    const auto data_pos = str.Data() + pos;
    Insert(begin() + index, data_pos, data_pos + insert_count);
    return *this;
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& Insert(SizeType index, const StringViewLike& sv) {
    const StringViewType view(sv);
    Insert(begin() + index, view.begin(), view.end());
    return *this;
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& Insert(SizeType index,
                      const StringViewLike& sv,
                      SizeType pos,
                      SizeType count = kNpos) {
    const StringViewType view(sv);

    const auto view_size = static_cast<SizeType>(view.size());
    AXIO_ASSERT(pos <= view_size);
    const auto insert_count =
        (count == kNpos || pos + count > view_size) ? (view_size - pos) : count;
    const auto data_pos = view.data() + pos;
    Insert(begin() + index, data_pos, data_pos + insert_count);
    return *this;
  }

  Iterator Insert(ConstIterator pos, ValueType value) {
    auto b = begin();
    AXIO_ASSERT(pos >= b && pos <= end());
    const auto index = static_cast<SizeType>(pos - b);
    const auto capacity = Capacity();
    const auto size = Size();
    if (size == capacity) {
      Reallocate<false>(ComputeCapacity(capacity, 1), size);
    }
    auto dst = Data() + index;
    TraitsType::move(dst + 1, dst, size - index);
    TraitsType::assign(*dst, value);
    IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(size + 1))
            : SetHeapSize(size + 1);
    return dst;
  }

  Iterator Insert(ConstIterator pos, SizeType count, ValueType value) {
    auto b = begin();
    AXIO_ASSERT(pos >= b && pos <= end());
    const auto index = static_cast<SizeType>(pos - b);
    if (count == 0) {
      return b + index;
    }
    const auto capacity = Capacity();
    const auto size = Size();
    if (count > capacity - size) {
      Reallocate<false>(ComputeCapacity(capacity, count), size);
    }
    auto dst = Data() + index;
    TraitsType::move(dst + count, dst, size - index);
    TraitsType::assign(dst, count, value);
    IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(size + count))
            : SetHeapSize(size + count);
    return dst;
  }

  Iterator Insert(ConstIterator pos, std::initializer_list<ValueType> values) {
    return Insert(pos, values.begin(), values.end());
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  Iterator Insert(ConstIterator pos, InputIt first, InputIt last) {
    const auto index = static_cast<SizeType>(pos - begin());
    BasicString temp(first, last);
    return Insert(begin() + index, temp.begin(), temp.end());
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  Iterator Insert(ConstIterator pos, ForwardIt first, ForwardIt last) {
    auto b = begin();
    AXIO_ASSERT(pos >= b && pos <= end());

    const auto index = static_cast<SizeType>(pos - b);
    const auto n = static_cast<SizeType>(std::distance(first, last));

    if (n == 0) {
      return b + index;
    }

    const auto size = Size();
    const auto capacity = Capacity();

    if (n > capacity - size) {
      const auto new_capacity = ComputeCapacity(capacity, n);
      auto& allocator = this->GetAlloc();
      auto new_data = AllocatorTraits::allocate(allocator, new_capacity + 1);
      auto old_data = Data();

      TraitsType::copy(new_data, old_data, index);
      Copy(new_data + index, first, n);
      TraitsType::copy(new_data + index + n, old_data + index, size - index);

      Release(allocator);

      SetModeAsHeap(new_data, new_capacity, size + n);

      return storage_.heap.data + index;
    }
    auto dst = Data() + index;
    TraitsType::move(dst + n, dst, size - index);
    Copy(dst, first, n);
    IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(size + n))
            : SetHeapSize(size + n);
    return dst;
  }

  int Compare(const BasicString& s) const {
    return Compare(s.Data(), s.Size());
  }

  int Compare(SizeType pos1, SizeType count1, const BasicString& s) const {
    return Compare(pos1, count1, s.Data(), s.Size());
  }

  int Compare(SizeType pos1,
              SizeType count1,
              const BasicString& s,
              SizeType pos2,
              SizeType count2 = kNpos) const {
    const auto size2 = s.Size();
    AXIO_ASSERT(pos2 <= size2);
    const auto rhs_count =
        (count2 == kNpos || pos2 + count2 > size2) ? (size2 - pos2) : count2;
    return Compare(pos1, count1, s.Data() + pos2, rhs_count);
  }

  int Compare(ConstPointer s, SizeType count) const {
    const auto size = Size();
    const int result = TraitsType::compare(Data(), s, AXIO_MIN(size, count));
    return result != 0 ? result : static_cast<int>(size - count);
  }

  int Compare(ConstPointer s) const {
    return Compare(s, TraitsType::length(s));
  }

  int Compare(SizeType pos1, SizeType count1, ConstPointer s) const {
    return Compare(pos1, count1, s, TraitsType::length(s));
  }

  int Compare(SizeType pos1,
              SizeType count1,
              ConstPointer s,
              SizeType count2) const {
    const auto size = Size();
    AXIO_ASSERT(pos1 <= size);
    const auto lhs_count =
        (count1 == kNpos || pos1 + count1 > size) ? (size - pos1) : count1;
    const int result =
        TraitsType::compare(Data() + pos1, s, AXIO_MIN(lhs_count, count2));
    return result != 0 ? result : static_cast<int>(lhs_count - count2);
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  int Compare(const StringViewLike& s) const {
    const StringViewType view(s);
    return Compare(view.data(), static_cast<SizeType>(view.size()));
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  int Compare(SizeType pos1, SizeType count1, const StringViewLike& s) const {
    const StringViewType view(s);
    return Compare(pos1, count1, view.data(),
                   static_cast<SizeType>(view.size()));
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  int Compare(SizeType pos1,
              SizeType count1,
              const StringViewLike& s,
              SizeType pos2,
              SizeType count2 = kNpos) const {
    const StringViewType view(s);
    const auto view_size = static_cast<SizeType>(view.size());
    AXIO_ASSERT(pos2 <= view_size);
    const auto rhs_count = (count2 == kNpos || pos2 + count2 > view_size)
                               ? (view_size - pos2)
                               : count2;
    return Compare(pos1, count1, view.data() + pos2, rhs_count);
  }

  BasicString Substr(SizeType pos = 0, SizeType count = kNpos) const {
    const auto size = Size();
    AXIO_ASSERT(pos <= size);
    const auto substr_count =
        (count == kNpos || pos + count > size) ? (size - pos) : count;
    return BasicString(Data() + pos, substr_count);
  }

  Bool StartsWith(ValueType c) const noexcept {
    return !IsEmpty() && TraitsType::eq(Data()[0], c);
  }

  Bool StartsWith(ConstPointer s) const {
    const auto n = TraitsType::length(s);
    return Size() >= n && TraitsType::compare(Data(), s, n) == 0;
  }

  Bool StartsWith(const BasicString& s) const {
    const auto n = s.Size();
    return Size() >= n && TraitsType::compare(Data(), s.Data(), n) == 0;
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  Bool StartsWith(const StringViewLike& s) const {
    const StringViewType view(s);
    const auto n = static_cast<SizeType>(view.size());
    return Size() >= n && TraitsType::compare(Data(), view.data(), n) == 0;
  }

  Bool EndsWith(ValueType c) const noexcept {
    const auto size = Size();
    return size > 0 && TraitsType::eq(Data()[size - 1], c);
  }

  Bool EndsWith(ConstPointer s) const {
    const auto n = TraitsType::length(s);
    const auto size = Size();
    return size >= n && TraitsType::compare(Data() + (size - n), s, n) == 0;
  }

  Bool EndsWith(const BasicString& s) const {
    const auto n = s.Size();
    const auto size = Size();
    return size >= n &&
           TraitsType::compare(Data() + (size - n), s.Data(), n) == 0;
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  Bool EndsWith(const StringViewLike& s) const {
    const StringViewType view(s);
    const auto n = static_cast<SizeType>(view.size());
    const auto size = Size();
    return size >= n &&
           TraitsType::compare(Data() + (size - n), view.data(), n) == 0;
  }

  SizeType Find(const BasicString& str, SizeType pos = 0) const noexcept {
    return Find(str.Data(), pos, str.Size());
  }

  SizeType Find(ConstPointer s, SizeType pos, SizeType count) const {
    const auto size = Size();
    if (count == 0) {
      return pos;
    }
    if ((pos > size) || (count > size - pos)) {
      return kNpos;
    }

    const auto data = Data();
    const auto end = size - count;
    for (SizeType i = pos; i <= end; ++i) {
      if (TraitsType::compare(data + i, s, count) == 0) {
        return i;
      }
    }

    return kNpos;
  }

  SizeType Find(ConstPointer s, SizeType pos = 0) const {
    return Find(s, pos, TraitsType::length(s));
  }

  SizeType Find(ValueType ch, SizeType pos = 0) const noexcept {
    const auto size = Size();
    if (pos >= size) {
      return kNpos;
    }
    const auto result = TraitsType::find(Data() + pos, size - pos, ch);
    return result ? static_cast<SizeType>(result - Data()) : kNpos;
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  SizeType Find(const StringViewLike& s, SizeType pos = 0) const
      noexcept(noexcept(StringViewType(s))) {
    const StringViewType view(s);
    return Find(view.data(), pos, static_cast<SizeType>(view.size()));
  }

  SizeType RFind(const BasicString& str, SizeType pos = kNpos) const noexcept {
    return RFind(str.Data(), pos, str.Size());
  }

  SizeType RFind(ConstPointer s, SizeType pos, SizeType count) const {
    const auto size = Size();
    if (count == 0) {
      return AXIO_MIN(pos, size);
    }
    if (count > size) {
      return kNpos;
    }

    const auto last = AXIO_MIN(pos, size - count);
    const auto data = Data();
    for (SizeType i = last + 1; i-- > 0;) {
      if (TraitsType::compare(data + i, s, count) == 0) {
        return i;
      }
    }
    return kNpos;
  }

  SizeType RFind(ConstPointer s, SizeType pos = kNpos) const {
    return RFind(s, pos, TraitsType::length(s));
  }

  SizeType RFind(ValueType ch, SizeType pos = kNpos) const noexcept {
    const auto size = Size();
    if (size == 0) {
      return kNpos;
    }

    SizeType i = AXIO_MIN(pos, size - 1);
    const auto data = Data();
    do {
      if (TraitsType::eq(data[i], ch)) {
        return i;
      }
    } while (i-- != 0);
    return kNpos;
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  SizeType RFind(const StringViewLike& s, SizeType pos = kNpos) const
      noexcept(noexcept(StringViewType(s))) {
    const StringViewType view(s);
    return RFind(view.data(), pos, static_cast<SizeType>(view.size()));
  }

  SizeType FindFirstOf(const BasicString& str,
                       SizeType pos = 0) const noexcept {
    return FindFirstOf(str.Data(), pos, str.Size());
  }

  SizeType FindFirstOf(ConstPointer s, SizeType pos, SizeType count) const {
    const auto size = Size();
    if (pos >= size || count == 0) {
      return kNpos;
    }

    const auto data = Data();
    for (SizeType i = pos; i < size; ++i) {
      if (TraitsType::find(s, count, data[i]) != nullptr) {
        return i;
      }
    }
    return kNpos;
  }

  SizeType FindFirstOf(ConstPointer s, SizeType pos = 0) const {
    return FindFirstOf(s, pos, TraitsType::length(s));
  }

  SizeType FindFirstOf(ValueType ch, SizeType pos = 0) const noexcept {
    return Find(ch, pos);
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  SizeType FindFirstOf(const StringViewLike& s, SizeType pos = 0) const
      noexcept(noexcept(StringViewType(s))) {
    const StringViewType view(s);
    return FindFirstOf(view.data(), pos, static_cast<SizeType>(view.size()));
  }

  SizeType FindFirstNotOf(const BasicString& str,
                          SizeType pos = 0) const noexcept {
    return FindFirstNotOf(str.Data(), pos, str.Size());
  }

  SizeType FindFirstNotOf(ConstPointer s, SizeType pos, SizeType count) const {
    const auto size = Size();
    if (pos >= size) {
      return kNpos;
    }

    const auto data = Data();
    for (SizeType i = pos; i < size; ++i) {
      if (TraitsType::find(s, count, data[i]) == nullptr) {
        return i;
      }
    }
    return kNpos;
  }

  SizeType FindFirstNotOf(ConstPointer s, SizeType pos = 0) const {
    return FindFirstNotOf(s, pos, TraitsType::length(s));
  }

  SizeType FindFirstNotOf(ValueType ch, SizeType pos = 0) const noexcept {
    const auto size = Size();
    if (pos >= size) {
      return kNpos;
    }

    const auto data = Data();
    for (SizeType i = pos; i < size; ++i) {
      if (!TraitsType::eq(data[i], ch)) {
        return i;
      }
    }
    return kNpos;
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  SizeType FindFirstNotOf(const StringViewLike& s, SizeType pos = 0) const
      noexcept(noexcept(StringViewType(s))) {
    const StringViewType view(s);
    return FindFirstNotOf(view.data(), pos, static_cast<SizeType>(view.size()));
  }

  SizeType FindLastOf(const BasicString& str,
                      SizeType pos = kNpos) const noexcept {
    return FindLastOf(str.Data(), pos, str.Size());
  }

  SizeType FindLastOf(ConstPointer s, SizeType pos, SizeType count) const {
    const auto size = Size();
    if (size == 0 || count == 0) {
      return kNpos;
    }

    SizeType i = AXIO_MIN(pos, size - 1);
    const auto data = Data();
    do {
      if (TraitsType::find(s, count, data[i]) != nullptr) {
        return i;
      }
    } while (i-- != 0);
    return kNpos;
  }

  SizeType FindLastOf(ConstPointer s, SizeType pos = kNpos) const {
    return FindLastOf(s, pos, TraitsType::length(s));
  }

  SizeType FindLastOf(ValueType ch, SizeType pos = kNpos) const noexcept {
    return RFind(ch, pos);
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  SizeType FindLastOf(const StringViewLike& s, SizeType pos = kNpos) const
      noexcept(noexcept(StringViewType(s))) {
    const StringViewType view(s);
    return FindLastOf(view.data(), pos, static_cast<SizeType>(view.size()));
  }

  SizeType FindLastNotOf(const BasicString& str,
                         SizeType pos = kNpos) const noexcept {
    return FindLastNotOf(str.Data(), pos, str.Size());
  }

  SizeType FindLastNotOf(ConstPointer s, SizeType pos, SizeType count) const {
    const auto size = Size();
    if (size == 0) {
      return kNpos;
    }

    SizeType i = AXIO_MIN(pos, size - 1);
    const auto data = Data();
    do {
      if (TraitsType::find(s, count, data[i]) == nullptr) {
        return i;
      }
    } while (i-- != 0);
    return kNpos;
  }

  SizeType FindLastNotOf(ConstPointer s, SizeType pos = kNpos) const {
    return FindLastNotOf(s, pos, TraitsType::length(s));
  }

  SizeType FindLastNotOf(ValueType ch, SizeType pos = kNpos) const noexcept {
    const auto size = Size();
    if (size == 0) {
      return kNpos;
    }

    SizeType i = AXIO_MIN(pos, size - 1);
    const auto data = Data();
    do {
      if (!TraitsType::eq(data[i], ch)) {
        return i;
      }
    } while (i-- != 0);
    return kNpos;
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  SizeType FindLastNotOf(const StringViewLike& s, SizeType pos = kNpos) const
      noexcept(noexcept(StringViewType(s))) {
    const StringViewType view(s);
    return FindLastNotOf(view.data(), pos, static_cast<SizeType>(view.size()));
  }

  Bool Contains(ValueType c) const noexcept { return Find(c) != kNpos; }

  Bool Contains(ConstPointer s) const { return Find(s) != kNpos; }

  Bool Contains(const BasicString& s) const { return Find(s) != kNpos; }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  Bool Contains(const StringViewLike& s) const {
    const StringViewType view(s);
    return Find(view.data(), 0, static_cast<SizeType>(view.size())) != kNpos;
  }

  BasicString& Replace(SizeType pos, SizeType count, const BasicString& str) {
    const auto data = str.Data();
    return Replace(pos, count, data, data + str.Size());
  }

  BasicString& Replace(SizeType pos,
                       SizeType count,
                       const BasicString& str,
                       SizeType pos2,
                       SizeType count2 = kNpos) {
    const auto size = str.Size();
    AXIO_ASSERT(pos2 <= size);
    count2 = (count2 == kNpos || pos2 + count2 > size) ? (size - pos2) : count2;
    const auto data_pos = str.Data() + pos2;
    return Replace(pos, count, data_pos, data_pos + count2);
  }

  BasicString& Replace(SizeType pos, SizeType count, ConstPointer str) {
    return Replace(pos, count, str, str + TraitsType::length(str));
  }

  BasicString& Replace(SizeType pos,
                       SizeType count,
                       ConstPointer str,
                       SizeType count2) {
    return Replace(pos, count, str, str + count2);
  }

  BasicString& Replace(SizeType pos,
                       SizeType count,
                       std::initializer_list<ValueType> values) {
    return Replace(pos, count, values.begin(), values.end());
  }

  BasicString& Replace(SizeType pos,
                       SizeType count,
                       SizeType count2,
                       ValueType value) {
    const auto capacity = Capacity();
    const auto size = Size();
    AXIO_ASSERT(pos <= size);
    count = (count == kNpos || pos + count > size) ? (size - pos) : count;
    const auto growing = count2 > count;
    const auto offset = growing ? count2 - count : count - count2;

    if (growing && offset > capacity - size) {
      const auto new_capacity = ComputeCapacity(capacity, offset);
      auto& allocator = this->GetAlloc();
      auto new_data = AllocatorTraits::allocate(allocator, new_capacity + 1);
      auto old_data = Data();
      TraitsType::copy(new_data, old_data, pos);
      TraitsType::assign(new_data + pos, count2, value);
      TraitsType::copy(new_data + pos + count2, old_data + pos + count,
                       size - pos - count);
      Release(allocator);
      SetModeAsHeap(new_data, new_capacity, size + offset);
    } else {
      auto data = Data();
      auto data_pos = data + pos;
      if (growing) {
        TraitsType::move(data_pos + count2, data_pos + count,
                         size - pos - count);
      } else if (count2 < count) {
        TraitsType::copy(data_pos + count2, data_pos + count,
                         size - pos - count);
      }
      TraitsType::assign(data_pos, count2, value);
      IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(size - count + count2))
              : SetHeapSize(size - count + count2);
    }
    return *this;
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& Replace(SizeType pos, SizeType count, const StringViewLike& s) {
    return Replace(pos, count, s.data(), s.data() + s.size());
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& Replace(SizeType pos,
                       SizeType count,
                       const StringViewLike& s,
                       SizeType pos2,
                       SizeType count2 = kNpos) {
    const StringViewType sv(s);
    const auto view_size = static_cast<SizeType>(sv.size());
    AXIO_ASSERT(pos2 <= view_size);
    count2 = (count2 == kNpos || pos2 + count2 > view_size) ? (view_size - pos2)
                                                            : count2;
    const auto data_pos = sv.data() + pos2;
    return Replace(pos, count, data_pos, data_pos + count2);
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  BasicString& Replace(SizeType pos,
                       SizeType count,
                       InputIt first,
                       InputIt last) {
    const auto size = Size();
    AXIO_ASSERT(pos <= size);
    count = (count == kNpos || pos + count > size) ? (size - pos) : count;

    auto first1 = begin() + pos;
    auto last1 = first1 + count;
    auto current = first1;
    while (current != last1 && first != last) {
      *current++ = *first++;
    }

    if (first != last) {
      Insert(current, first, last);
    } else {
      Remove(current, last1);
    }
    return *this;
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  BasicString& Replace(SizeType pos,
                       SizeType count,
                       ForwardIt first,
                       ForwardIt last) {
    const auto n = static_cast<SizeType>(std::distance(first, last));
    const auto capacity = Capacity();
    const auto size = Size();
    AXIO_ASSERT(pos <= size);
    count = (count == kNpos || pos + count > size) ? (size - pos) : count;
    const auto growing = n > count;
    const auto offset = growing ? n - count : count - n;

    if (growing && offset > capacity - size) {
      const auto new_capacity = ComputeCapacity(capacity, offset);
      auto& allocator = this->GetAlloc();
      auto new_data = AllocatorTraits::allocate(allocator, new_capacity + 1);
      auto old_data = Data();
      TraitsType::copy(new_data, old_data, pos);
      Copy(new_data + pos, first, n);
      TraitsType::copy(new_data + pos + n, old_data + pos + count,
                       size - pos - count);
      Release(allocator);
      SetModeAsHeap(new_data, new_capacity, size + offset);
    } else {
      auto data = Data();
      auto data_pos = data + pos;
      if (growing) {
        TraitsType::move(data_pos + n, data_pos + count, size - pos - count);
      } else if (n < count) {
        TraitsType::copy(data_pos + n, data_pos + count, size - pos - count);
      }
      Copy(data + pos, first, n);
      IsSSO() ? SetModeAsSSO(static_cast<unsigned char>(size - count + n))
              : SetHeapSize(size - count + n);
    }
    return *this;
  }
  // -------------------------------------------------------------------- //
  BasicString& Replace(ConstIterator first,
                       ConstIterator last,
                       const BasicString& str) {
    const auto data = str.Data();
    return Replace(first, last, data, data + str.Size());
  }

  BasicString& Replace(ConstIterator first,
                       ConstIterator last,
                       const BasicString& str,
                       SizeType pos,
                       SizeType count = kNpos) {
    const auto size = str.Size();
    AXIO_ASSERT(pos <= size);
    count = (count == kNpos || pos + count > size) ? (size - pos) : count;
    const auto data_pos = str.Data() + pos;
    return Replace(first, last, data_pos, data_pos + count);
  }

  BasicString& Replace(ConstIterator first,
                       ConstIterator last,
                       ConstPointer str) {
    return Replace(first, last, str, str + TraitsType::length(str));
  }

  BasicString& Replace(ConstIterator first,
                       ConstIterator last,
                       ConstPointer str,
                       SizeType count) {
    return Replace(first, last, str, str + count);
  }

  BasicString& Replace(ConstIterator first,
                       ConstIterator last,
                       std::initializer_list<ValueType> values) {
    return Replace(first, last, values.begin(), values.end());
  }

  BasicString& Replace(ConstIterator first,
                       ConstIterator last,
                       SizeType count,
                       ValueType value) {
    auto b = begin();
    AXIO_ASSERT(b <= first);
    AXIO_ASSERT(first <= last);
    AXIO_ASSERT(last <= end());
    const auto pos = static_cast<SizeType>(first - b);
    const auto length = static_cast<SizeType>(last - first);
    return Replace(pos, length, count, value);
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& Replace(ConstIterator first,
                       ConstIterator last,
                       const StringViewLike& s) {
    const StringViewType sv(s);
    return Replace(first, last, sv.data(), s.data() + s.size());
  }

  template <typename StringViewLike,
            EnableIfIsStringViewLike<StringViewLike, int> = 0>
  BasicString& Replace(ConstIterator first,
                       ConstIterator last,
                       const StringViewLike& s,
                       SizeType pos,
                       SizeType count = kNpos) {
    const StringViewType view(s);
    const auto view_size = static_cast<SizeType>(view.size());
    AXIO_ASSERT(pos <= view_size);
    count =
        (count == kNpos || pos + count > view_size) ? (view_size - pos) : count;
    const auto data_pos = view.data() + pos;
    return Replace(first, last, data_pos, data_pos + count);
  }

  template <typename InputIt, EnableIfNotForwardIt<InputIt> = 0>
  BasicString& Replace(ConstIterator first,
                       ConstIterator last,
                       InputIt first2,
                       InputIt last2) {
    auto b = begin();
    AXIO_ASSERT(b <= first);
    AXIO_ASSERT(first <= last);
    AXIO_ASSERT(last <= end());
    const auto pos = static_cast<SizeType>(first - b);
    const auto count = static_cast<SizeType>(last - first);
    return Replace(pos, count, first2, last2);
  }

  template <typename ForwardIt, EnableIfForwardIt<ForwardIt> = 0>
  BasicString& Replace(ConstIterator first,
                       ConstIterator last,
                       ForwardIt first2,
                       ForwardIt last2) {
    auto b = begin();
    AXIO_ASSERT(b <= first);
    AXIO_ASSERT(first <= last);
    AXIO_ASSERT(last <= end());
    const auto pos = static_cast<SizeType>(first - b);
    const auto count = static_cast<SizeType>(last - first);
    return Replace(pos, count, first2, last2);
  }

 private:
  static constexpr ValueType kWhiteSpaces[]{ValueType(0x20), ValueType(0x09),
                                            ValueType(0x0a), ValueType(0x0d),
                                            ValueType(0x0c), ValueType(0x0b)};

 public:
  void Trim() {
    LTrim();
    RTrim();
  }

  void LTrim() {
    const auto pos =
        FindFirstNotOf(kWhiteSpaces, 0, AXIO_ARRAY_SIZE(kWhiteSpaces));
    Remove(0, pos);
  }

  void RTrim() {
    const auto pos =
        FindLastNotOf(kWhiteSpaces, kNpos, AXIO_ARRAY_SIZE(kWhiteSpaces));
    Remove(pos + 1);
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
  using SSOStorage = ValueType[kSSOCapacity + 1];  // +1 for null terminator

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
    storage_.sso[len] = kNullTerminator;
    storage_.raw[kModeByteOffset] =
        static_cast<unsigned char>(kSSOCapacity - len);
  }

  void SetModeAsHeap(Pointer data, SizeType capacity, SizeType size) {
    storage_.raw[kModeByteOffset] |= kHeapMask;
    storage_.heap.data = data;
    SetHeapCapacity(capacity);
    SetHeapSize(size);
  }

  void SetModeAsHeap(Pointer data, SizeType capacity) {
    storage_.raw[kModeByteOffset] |= kHeapMask;
    storage_.heap.data = data;
    SetHeapCapacity(capacity);
  }

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
      return storage_.sso;
    }
    SetModeAsHeap(AllocatorTraits::allocate(this->GetAlloc(), n + 1), n, n);
    return storage_.heap.data;
  }

  template <Bool SET_HEAP_SIZE_NOW = true>
  void Reallocate(const SizeType new_capacity, const SizeType old_size) {
    auto& allocator = this->GetAlloc();
    auto new_data = AllocatorTraits::allocate(allocator, new_capacity + 1);
    TraitsType::copy(new_data, Data(), old_size);
    Release(allocator);

    SetModeAsHeap(new_data, new_capacity);
    if constexpr (SET_HEAP_SIZE_NOW) {
      SetHeapSize(old_size);
    }
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
                                  GetHeapCapacity() + 1);
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

template <typename T, typename Traits, typename A>
inline Bool operator==(const BasicString<T, Traits, A>& lhs,
                       const BasicString<T, Traits, A>& rhs) {
  return lhs.Size() == rhs.Size() &&
         Traits::compare(lhs.Data(), rhs.Data(), lhs.Size()) == 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator!=(const BasicString<T, Traits, A>& lhs,
                       const BasicString<T, Traits, A>& rhs) {
  return !(lhs == rhs);
}

template <typename T, typename Traits, typename A>
inline Bool operator>=(const BasicString<T, Traits, A>& lhs,
                       const BasicString<T, Traits, A>& rhs) {
  return lhs.Compare(rhs) >= 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator<=(const BasicString<T, Traits, A>& lhs,
                       const BasicString<T, Traits, A>& rhs) {
  return lhs.Compare(rhs) <= 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator>(const BasicString<T, Traits, A>& lhs,
                      const BasicString<T, Traits, A>& rhs) {
  return lhs.Compare(rhs) > 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator<(const BasicString<T, Traits, A>& lhs,
                      const BasicString<T, Traits, A>& rhs) {
  return lhs.Compare(rhs) < 0;
}

// ========================================================= //

template <typename T, typename Traits, typename A>
inline Bool operator==(const BasicString<T, Traits, A>& lhs, const T* rhs) {
  const auto rhs_size = Traits::length(rhs);
  return lhs.Size() == rhs_size &&
         Traits::compare(lhs.Data(), rhs, rhs_size) == 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator!=(const BasicString<T, Traits, A>& lhs, const T* rhs) {
  return !(lhs == rhs);
}

template <typename T, typename Traits, typename A>
inline Bool operator>=(const BasicString<T, Traits, A>& lhs, const T* rhs) {
  return lhs.Compare(rhs) >= 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator<=(const BasicString<T, Traits, A>& lhs, const T* rhs) {
  return lhs.Compare(rhs) <= 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator>(const BasicString<T, Traits, A>& lhs, const T* rhs) {
  return lhs.Compare(rhs) > 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator<(const BasicString<T, Traits, A>& lhs, const T* rhs) {
  return lhs.Compare(rhs) < 0;
}

// ========================================================= //

template <typename T, typename Traits, typename A>
inline Bool operator==(const T* lhs, const BasicString<T, Traits, A>& rhs) {
  return rhs == lhs;
}

template <typename T, typename Traits, typename A>
inline Bool operator!=(const T* lhs, const BasicString<T, Traits, A>& rhs) {
  return !(lhs == rhs);
}

template <typename T, typename Traits, typename A>
inline Bool operator>=(const T* lhs, const BasicString<T, Traits, A>& rhs) {
  return rhs.Compare(lhs) <= 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator<=(const T* lhs, const BasicString<T, Traits, A>& rhs) {
  return rhs.Compare(lhs) >= 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator>(const T* lhs, const BasicString<T, Traits, A>& rhs) {
  return rhs.Compare(lhs) < 0;
}

template <typename T, typename Traits, typename A>
inline Bool operator<(const T* lhs, const BasicString<T, Traits, A>& rhs) {
  return rhs.Compare(lhs) > 0;
}

using String = BasicString<char>;
using WString = BasicString<wchar_t>;
#ifdef __cpp_char8_t
using U8String = BasicString<char8_t>;
#endif
using U16String = BasicString<char16_t>;
using U32String = BasicString<char32_t>;
}  // namespace axio

#endif