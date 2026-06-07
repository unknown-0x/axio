#ifndef AXIO_CONTAINER_SPAN_HPP_
#define AXIO_CONTAINER_SPAN_HPP_

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"
#include "../base/types.hpp"

#include "array.hpp"

#include <array>
#include <iterator>

namespace axio {
inline constexpr SizeT kDynamicExtent = static_cast<SizeT>(-1);

template <typename T, SizeT EXTENT = kDynamicExtent>
class Span;

namespace detail {
template <typename T, SizeT EXTENT>
struct SpanStorage {
  constexpr SpanStorage() noexcept : data(nullptr) {}
  constexpr explicit SpanStorage(T* ptr, SizeT) noexcept : data(ptr) {}
  static constexpr SizeT GetSize() noexcept { return EXTENT; }

  T* data;
};

template <typename T>
struct SpanStorage<T, kDynamicExtent> {
  constexpr SpanStorage() noexcept : data(nullptr), size(0) {}
  constexpr explicit SpanStorage(T* ptr, SizeT n) noexcept
      : data(ptr), size(n) {}
  constexpr SizeT GetSize() const noexcept { return size; }

  T* data;
  SizeT size;
};

template <typename T>
struct IsSpan : FalseType {};

template <typename T, SizeT E>
struct IsSpan<Span<T, E>> : TrueType {};

template <typename T>
struct IsArray : FalseType {};

template <typename T, SizeT N>
struct IsArray<std::array<T, N>> : TrueType {};

template <typename T, SizeT N>
struct IsArray<Array<T, N>> : TrueType {};

template <typename C, typename = void>
struct Has_Data_Size : FalseType {};

template <typename C>
struct Has_Data_Size<C,
                     Void_T<decltype(std::declval<C>().Data()),
                            decltype(std::declval<C>().Size())>> : TrueType {};

template <typename C, typename = void>
struct Has_data_size : FalseType {};

template <typename C>
struct Has_data_size<C,
                     Void_T<decltype(std::declval<C>().data()),
                            decltype(std::declval<C>().size())>> : TrueType {};

template <typename Container,
          EnableIf_T<Has_Data_Size<Container>::value, int> = 0>
constexpr auto GetSize(Container& c) noexcept(noexcept(c.Size()))
    -> decltype(c.Size()) {
  return c.Size();
}

template <typename Container,
          EnableIf_T<Has_data_size<Container>::value, int> = 0>
constexpr auto GetSize(Container& c) noexcept(noexcept(c.size()))
    -> decltype(c.size()) {
  return c.size();
}

template <typename Container,
          EnableIf_T<Has_Data_Size<Container>::value, int> = 0>
constexpr auto GetData(Container& c) noexcept(noexcept(c.Data()))
    -> decltype(c.Data()) {
  return c.Data();
}

template <typename Container,
          EnableIf_T<Has_data_size<Container>::value, int> = 0>
constexpr auto GetData(Container& c) noexcept(noexcept(c.data()))
    -> decltype(c.data()) {
  return c.data();
}
}  // namespace detail

template <typename T, SizeT EXTENT>
class Span : private detail::SpanStorage<T, EXTENT> {
  using Base = detail::SpanStorage<T, EXTENT>;

 public:
  using ElementType = T;
  using ValueType = RemoveCV_T<T>;
  using SizeType = SizeT;
  using DifferenceType = PtrDiffT;
  using Pointer = T*;
  using ConstPointer = const T*;
  using Reference = T&;
  using ConstReference = const T&;

  using Iterator = Pointer;
  using ConstIterator = ConstPointer;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  static constexpr SizeType kExtent = EXTENT;

  template <SizeT E = EXTENT,
            EnableIf_T<(E == kDynamicExtent || E == 0), int> = 0>
  constexpr Span() noexcept : Base() {}

  constexpr Span(Pointer ptr, SizeType count) noexcept : Base(ptr, count) {
    AXIO_ASSERT((kExtent == kDynamicExtent || count == kExtent));
  }

  constexpr Span(Pointer first, Pointer last) noexcept
      : Span(first, static_cast<SizeType>(last - first)) {}

  template <SizeT N,
            EnableIf_T<(EXTENT == kDynamicExtent || EXTENT == N), int> = 0>
  constexpr Span(ElementType (&arr)[N]) noexcept : Span(arr, N) {}

  template <SizeT N,
            EnableIf_T<(EXTENT == kDynamicExtent || EXTENT == N), int> = 0>
  constexpr Span(std::array<ValueType, N>& arr) noexcept
      : Span(arr.data(), N) {}

  template <
      SizeT N,
      EnableIf_T<(EXTENT == kDynamicExtent || EXTENT == N) && IsConst_V<T>,
                 int> = 0>
  constexpr Span(const std::array<ValueType, N>& arr) noexcept
      : Span(arr.data(), N) {}

  template <SizeT N,
            EnableIf_T<(EXTENT == kDynamicExtent || EXTENT == N), int> = 0>
  constexpr Span(Array<ValueType, N>& arr) noexcept : Span(arr.Data(), N) {}

  template <
      SizeT N,
      EnableIf_T<(EXTENT == kDynamicExtent || EXTENT == N) && IsConst_V<T>,
                 int> = 0>
  constexpr Span(const Array<ValueType, N>& arr) noexcept
      : Span(arr.Data(), N) {}

  template <typename Container,
            EnableIf_T<!IsArray_V<Container> &&
                           !detail::IsArray<RemoveCV_T<Container>>::value &&
                           !detail::IsSpan<RemoveCV_T<Container>>::value &&
                           (detail::Has_Data_Size<Container>::value ||
                            detail::Has_data_size<Container>::value) &&
                           IsConvertible_V<decltype(detail::GetData(
                                               std::declval<Container&>())),
                                           ElementType*>,
                       int> = 0>
  constexpr Span(Container& c) noexcept(noexcept(detail::GetData(c),
                                                 detail::GetSize(c)))
      : Span(detail::GetData(c), detail::GetSize(c)) {}

  template <typename U,
            SizeT E,
            EnableIf_T<(EXTENT == kDynamicExtent || E == kDynamicExtent ||
                        EXTENT == E) &&
                           IsConvertible_V<U (*)[], T (*)[]>,
                       int> = 0>
  constexpr Span(const Span<U, E>& other) noexcept
      : Span(other.Data(), other.Size()) {}

  constexpr Span(const Span&) noexcept = default;
  constexpr Span& operator=(const Span&) noexcept = default;

  AXIO_NODISCARD constexpr SizeType Size() const noexcept {
    return this->GetSize();
  }

  AXIO_NODISCARD constexpr SizeType SizeBytes() const noexcept {
    return this->GetSize() * sizeof(T);
  }

  AXIO_NODISCARD constexpr bool IsEmpty() const noexcept {
    return this->GetSize() == 0;
  }

  AXIO_NODISCARD constexpr Pointer Data() const noexcept { return this->data; }

  AXIO_NODISCARD constexpr Reference Front() const noexcept {
    AXIO_ASSERT(!IsEmpty());
    return this->data[0];
  }

  AXIO_NODISCARD constexpr Reference Back() const noexcept {
    AXIO_ASSERT(!IsEmpty());
    return this->data[this->GetSize() - 1];
  }

  AXIO_NODISCARD constexpr Reference operator[](SizeType i) const noexcept {
    AXIO_ASSERT(i < this->GetSize());
    return this->data[i];
  }

  AXIO_NODISCARD Reference At(SizeType i) const {
    if (i >= this->GetSize()) {
      throw std::out_of_range("Span::At(SizeType) index " + std::to_string(i) +
                              " out of range");
    }
    return this->data[i];
  }

  template <SizeT N>
  AXIO_NODISCARD constexpr Span<T, N> First() const noexcept {
    static_assert(N != kDynamicExtent);
    AXIO_ASSERT(N <= Size());
    return {this->data, N};
  }

  AXIO_NODISCARD constexpr Span<T> First(SizeType n) const noexcept {
    AXIO_ASSERT(n <= Size());
    return {this->data, n};
  }

  template <SizeT N>
  AXIO_NODISCARD constexpr Span<T, N> Last() const noexcept {
    static_assert(N != kDynamicExtent);
    AXIO_ASSERT(N <= Size());
    return {this->data + (Size() - N), N};
  }

  AXIO_NODISCARD constexpr Span<T> Last(SizeType n) const noexcept {
    AXIO_ASSERT(n <= Size());
    return {this->data + (Size() - n), n};
  }

  template <SizeT OFFSET, SizeT N = kDynamicExtent>
  AXIO_NODISCARD constexpr auto Subspan() const noexcept {
    AXIO_ASSERT(OFFSET <= Size());
    AXIO_ASSERT((N == kDynamicExtent || N <= Size() - OFFSET));

    constexpr auto kNewExtent =
        (N != kDynamicExtent)
            ? N
            : (EXTENT != kDynamicExtent ? EXTENT - OFFSET : kDynamicExtent);

    return Span<T, kNewExtent>{this->data + OFFSET,
                               N != kDynamicExtent ? N : Size() - OFFSET};
  }

  AXIO_NODISCARD constexpr Span<T> Subspan(
      SizeType offset,
      SizeType count = kDynamicExtent) const noexcept {
    AXIO_ASSERT(offset <= Size());
    const auto len = (count == kDynamicExtent) ? Size() - offset : count;
    AXIO_ASSERT(len <= Size() - offset);
    return {this->data + offset, len};
  }

  AXIO_NODISCARD constexpr Iterator begin() const noexcept {
    return this->data;
  }

  AXIO_NODISCARD constexpr Iterator end() const noexcept {
    return this->data + this->GetSize();
  }

  AXIO_NODISCARD constexpr ConstIterator cbegin() const noexcept {
    return this->data;
  }

  AXIO_NODISCARD constexpr ConstIterator cend() const noexcept {
    return this->data + this->GetSize();
  }

  AXIO_NODISCARD constexpr ReverseIterator rbegin() const noexcept {
    return ReverseIterator{end()};
  }

  AXIO_NODISCARD constexpr ReverseIterator rend() const noexcept {
    return ReverseIterator{begin()};
  }

  AXIO_NODISCARD constexpr ConstReverseIterator crbegin() const noexcept {
    return ConstReverseIterator{cend()};
  }

  AXIO_NODISCARD constexpr ConstReverseIterator crend() const noexcept {
    return ConstReverseIterator{cbegin()};
  }

  AXIO_NODISCARD
  Span<const Byte,
       EXTENT == kDynamicExtent ? kDynamicExtent : EXTENT * sizeof(T)>
  AsBytes() const noexcept {
    return {reinterpret_cast<const Byte*>(this->data), SizeBytes()};
  }

  template <typename U = T, EnableIf_T<!IsConst_V<U>, int> = 0>
  AXIO_NODISCARD
      Span<Byte, EXTENT == kDynamicExtent ? kDynamicExtent : EXTENT * sizeof(T)>
      AsWritableBytes() const noexcept {
    return {reinterpret_cast<Byte*>(this->data), SizeBytes()};
  }

  template <typename U>
  AXIO_NODISCARD Span<const U> As() const noexcept {
    static_assert(!IsReference_V<U>, "U must not be a reference type");
    AXIO_ASSERT(SizeBytes() % sizeof(U) == 0);
    return {reinterpret_cast<const U*>(this->data), SizeBytes() / sizeof(U)};
  }

  template <typename U, typename V = T, EnableIf_T<!IsConst_V<V>, int> = 0>
  AXIO_NODISCARD Span<U> AsWritable() const noexcept {
    static_assert(!IsReference_V<U>, "U must not be a reference type");
    AXIO_ASSERT(SizeBytes() % sizeof(U) == 0);
    return {reinterpret_cast<U*>(this->data), SizeBytes() / sizeof(U)};
  }

  template <SizeT INDEX>
  AXIO_NODISCARD constexpr auto Split() const noexcept {
    static_assert(INDEX != kDynamicExtent,
                  "Split<INDEX>() requires a compile-time index");
    static_assert(EXTENT == kDynamicExtent || INDEX <= EXTENT,
                  "Split<INDEX>() index exceeds static extent");
    AXIO_ASSERT(INDEX <= Size());

    constexpr auto kTailExtent =
        (EXTENT != kDynamicExtent) ? (EXTENT - INDEX) : kDynamicExtent;

    return std::pair<Span<T, INDEX>, Span<T, kTailExtent>>{
        Span<T, INDEX>{this->data, INDEX},
        Span<T, kTailExtent>{this->data + INDEX, Size() - INDEX},
    };
  }

  AXIO_NODISCARD constexpr std::pair<Span<T>, Span<T>> Split(
      SizeType index) const noexcept {
    AXIO_ASSERT(index <= Size());
    return {
        Span<T>{this->data, index},
        Span<T>{this->data + index, Size() - index},
    };
  }
};

template <typename T, SizeT N>
Span(T (&)[N]) -> Span<T, N>;

template <typename T, SizeT N>
Span(std::array<T, N>&) -> Span<T, N>;

template <typename T, SizeT N>
Span(const std::array<T, N>&) -> Span<const T, N>;

template <typename T, SizeT N>
Span(Array<T, N>&) -> Span<T, N>;

template <typename T, SizeT N>
Span(const Array<T, N>&) -> Span<const T, N>;

template <typename Container>
Span(Container&) -> Span<
    RemovePointer_T<decltype(detail::GetData(std::declval<Container&>()))>>;
}  // namespace axio

#endif