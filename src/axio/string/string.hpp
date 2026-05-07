#ifndef AXIO_STRING_STRING_HPP_
#define AXIO_STRING_STRING_HPP_

#include <string>

#include "../base/endianness.hpp"
#include "../container/tuple.hpp"
#include "../memory/allocator.hpp"

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
  static constexpr ValueType kNullTerminator = ValueType();

  static_assert(!IsArray<ValueType>::value,
                "ValueType must not be an array type");
  static_assert(IsStandardLayout<ValueType>::value,
                "ValueType must be a standard-layout type");
  static_assert(IsTriviallyCopyable<ValueType>::value,
                "ValueType must be trivially copyable");

  BasicString() noexcept(noexcept(AllocatorType())) : AllocatorHolder() {
    SetModeAsSSO(0);
  }

  explicit BasicString(const AllocatorType& allocator)
      : AllocatorHolder(allocator) {
    SetModeAsSSO(0);
  }

  BasicString(ConstPointer s, const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    const SizeType n = Traits::length(s);
    Traits::copy(InitWithSize(n), s, n);
  }

  BasicString(ConstPointer s,
              SizeType count,
              const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    Traits::copy(InitWithSize(count), s, count);
  }

  BasicString(SizeType count,
              ValueType c,
              const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    Traits::assign(InitWithSize(count), count, c);
  }

  template <typename InputIt>
  BasicString(InputIt first,
              InputIt last,
              const AllocatorType& allocator = AllocatorType());

  BasicString(std::initializer_list<ValueType> values,
              const AllocatorType& allocator = AllocatorType())
      : AllocatorHolder(allocator) {
    const auto n = static_cast<SizeType>(values.size());
    Traits::copy(InitWithSize(n), values.begin(), n);
  }

  BasicString(const BasicString& other) : AllocatorHolder(other.GetAlloc()) {
    const auto n = other.Size();
    Traits::copy(InitWithSize(n), other.Data(), n);
  }

  BasicString(BasicString&& other) noexcept
      : AllocatorHolder(Move(other.GetAlloc())) {
    storage_ = other.storage_;
    other.SetModeAsSSO(0);
  }

  BasicString(const BasicString& other, const AllocatorType& allocator)
      : AllocatorHolder(allocator) {
    const auto n = other.Size();
    Traits::copy(InitWithSize(n), other.Data(), n);
  }

  BasicString(BasicString&& other, const AllocatorType& allocator)
      : AllocatorHolder(allocator) {
    if (this->GetAlloc() == other.GetAlloc()) {
      storage_ = other.storage_;
      other.SetModeAsSSO(0);
      return;
    }

    const auto n = other.Size();
    Traits::copy(InitWithSize(n), other.Data(), n);
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
    Traits::copy(InitWithSize(actual_count), other.Data() + pos, actual_count);
  }

  BasicString(NullPtrT) = delete;

  ~BasicString() {
    if (!IsSSO()) {
      AllocatorTraits::deallocate(this->GetAlloc(), storage_.heap.data,
                                  storage_.heap.capacity + 1);
    }
  }

  SizeType Size() const noexcept {
    return IsSSO() ? kSSOCapacity - storage_.raw[kModeByteOffset]
                   : storage_.heap.size;
  }

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
};

using String = BasicString<char>;
using WString = BasicString<wchar_t>;
#ifdef __cpp_char8_t
using U8String = BasicString<char8_t>;
#endif
using U16String = BasicString<char16_t>;
using U32String = BasicString<char32_t>;
}  // namespace axio

#endif