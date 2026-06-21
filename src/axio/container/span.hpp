/**
 * @file span.hpp
 * @brief Non-owning view over a contiguous sequence of elements.
 */
#ifndef AXIO_CONTAINER_SPAN_HPP_
#define AXIO_CONTAINER_SPAN_HPP_

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"
#include "../base/types.hpp"

#include "array.hpp"

#include <array>
#include <iterator>

namespace axio {
/**
 * @brief Sentinel value used as the EXTENT template parameter to indicate
 * that a Span's size is determined at runtime rather than compile time.
 */
inline constexpr SizeT kDynamicExtent = static_cast<SizeT>(-1);

template <typename T, SizeT EXTENT = kDynamicExtent>
class Span;

namespace detail {
/**
 * @brief Storage for a Span with a fixed, compile-time EXTENT.
 *
 * Only holds the data pointer; the size is known statically via EXTENT.
 *
 * @tparam T Element type.
 * @tparam EXTENT Compile-time extent.
 */
template <typename T, SizeT EXTENT>
struct SpanStorage {
  /// @brief Constructs an empty storage with a `nullptr` data pointer.
  constexpr SpanStorage() noexcept : data(nullptr) {}
  /**
   * @brief Constructs storage pointing at @p ptr.
   * @param ptr Pointer to the first element.
   */
  constexpr explicit SpanStorage(T* ptr, SizeT) noexcept : data(ptr) {}
  /**
   * @brief Returns the compile-time size.
   * @return EXTENT.
   */
  static constexpr SizeT GetSize() noexcept { return EXTENT; }

  T* data;
};

/**
 * @brief Storage specialization for a Span with kDynamicExtent.
 *
 * Holds both the data pointer and a runtime size.
 *
 * @tparam T Element type.
 */
template <typename T>
struct SpanStorage<T, kDynamicExtent> {
  /// @brief Constructs an empty storage with a `nullptr` data pointer and
  /// size 0.
  constexpr SpanStorage() noexcept : data(nullptr), size(0) {}
  /**
   * @brief Constructs storage pointing at @p ptr with @p n elements.
   * @param ptr Pointer to the first element.
   * @param n Number of elements.
   */
  constexpr explicit SpanStorage(T* ptr, SizeT n) noexcept
      : data(ptr), size(n) {}

  /**
   * @brief Returns the runtime size.
   * @return Number of elements.
   */
  constexpr SizeT GetSize() const noexcept { return size; }

  T* data;
  SizeT size;
};

/**
 * @brief Checks whether T is a specialization of axio::Span.
 * @tparam T Type to check.
 */
template <typename T>
struct IsSpan : FalseType {};

template <typename T, SizeT E>
struct IsSpan<Span<T, E>> : TrueType {};

/**
 * @brief Checks whether T is a std::array or axio::Array.
 * @tparam T Type to check.
 */
template <typename T>
struct IsArray : FalseType {};

template <typename T, SizeT N>
struct IsArray<std::array<T, N>> : TrueType {};

template <typename T, SizeT N>
struct IsArray<Array<T, N>> : TrueType {};

/**
 * @brief Checks whether C exposes `Data()`/`Size()` (PascalCase) accessors.
 * @tparam C Container type to check.
 */
template <typename C, typename = void>
struct Has_Data_Size : FalseType {};

template <typename C>
struct Has_Data_Size<C,
                     Void_T<decltype(std::declval<C>().Data()),
                            decltype(std::declval<C>().Size())>> : TrueType {};

/**
 * @brief Checks whether C exposes `data()`/`size()` (STL-style) accessors.
 * @tparam C Container type to check.
 */
template <typename C, typename = void>
struct Has_data_size : FalseType {};

template <typename C>
struct Has_data_size<C,
                     Void_T<decltype(std::declval<C>().data()),
                            decltype(std::declval<C>().size())>> : TrueType {};

/**
 * @brief Returns `c.Size()` for containers with a `Size()` accessor.
 * @tparam Container Container type exposing `Size()`.
 * @param c Container instance.
 * @return Result of `c.Size()`.
 */
template <typename Container,
          EnableIf_T<Has_Data_Size<Container>::value, int> = 0>
constexpr auto GetSize(Container& c) noexcept(noexcept(c.Size()))
    -> decltype(c.Size()) {
  return c.Size();
}

/**
 * @brief Returns `c.size()` for containers with a `size()` accessor.
 * @tparam Container Container type exposing `size()`.
 * @param c Container instance.
 * @return Result of `c.size()`.
 */
template <typename Container,
          EnableIf_T<Has_data_size<Container>::value, int> = 0>
constexpr auto GetSize(Container& c) noexcept(noexcept(c.size()))
    -> decltype(c.size()) {
  return c.size();
}

/**
 * @brief Returns `c.Data()` for containers with a `Data()` accessor.
 * @tparam Container Container type exposing `Data()`.
 * @param c Container instance.
 * @return Result of `c.Data()`.
 */
template <typename Container,
          EnableIf_T<Has_Data_Size<Container>::value, int> = 0>
constexpr auto GetData(Container& c) noexcept(noexcept(c.Data()))
    -> decltype(c.Data()) {
  return c.Data();
}

/**
 * @brief Returns `c.data()` for containers with a `data()` accessor.
 * @tparam Container Container type exposing `data()`.
 * @param c Container instance.
 * @return Result of `c.data()`.
 */
template <typename Container,
          EnableIf_T<Has_data_size<Container>::value, int> = 0>
constexpr auto GetData(Container& c) noexcept(noexcept(c.data()))
    -> decltype(c.data()) {
  return c.data();
}
}  // namespace detail

/**
 * @brief A non-owning view over a contiguous sequence of T.
 *
 * Modeled after std::span. EXTENT may be a compile-time size or
 * kDynamicExtent for a runtime-determined size.
 *
 * @tparam T Element type.
 * @tparam EXTENT Compile-time extent, or kDynamicExtent for runtime size.
 */
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

  /// @brief The compile-time extent of this Span (may be kDynamicExtent).
  static constexpr SizeType kExtent = EXTENT;

  /**
   * @brief Constructs an empty Span.
   *
   * Only available when EXTENT is dynamic or 0.
   *
   * @tparam E Defaulted to EXTENT; used to constrain availability.
   */
  template <SizeT E = EXTENT,
            EnableIf_T<(E == kDynamicExtent || E == 0), int> = 0>
  constexpr Span() noexcept : Base() {}

  /**
   * @brief Constructs a Span over @p count elements starting at @p ptr.
   * @param ptr Pointer to the first element.
   * @param count Number of elements.
   */
  constexpr Span(Pointer ptr, SizeType count) noexcept : Base(ptr, count) {
    AXIO_ASSERT((kExtent == kDynamicExtent || count == kExtent));
  }

  /**
   * @brief Constructs a Span over the half-open range [first, last).
   * @param first Pointer to the first element.
   * @param last Pointer one past the last element.
   */
  constexpr Span(Pointer first, Pointer last) noexcept
      : Span(first, static_cast<SizeType>(last - first)) {}

  /**
   * @brief Constructs a Span from a C array.
   * @tparam N Array size.
   * @param arr Source array.
   */
  template <SizeT N,
            EnableIf_T<(EXTENT == kDynamicExtent || EXTENT == N), int> = 0>
  constexpr Span(ElementType (&arr)[N]) noexcept : Span(arr, N) {}

  /**
   * @brief Constructs a Span from a mutable std::array.
   * @tparam N Array size.
   * @param arr Source array.
   */
  template <SizeT N,
            EnableIf_T<(EXTENT == kDynamicExtent || EXTENT == N), int> = 0>
  constexpr Span(std::array<ValueType, N>& arr) noexcept
      : Span(arr.data(), N) {}

  /**
   * @brief Constructs a Span from a const std::array.
   *
   * Only valid for `T == const ValueType`.
   *
   * @tparam N Array size.
   * @param arr Source array.
   */
  template <
      SizeT N,
      EnableIf_T<(EXTENT == kDynamicExtent || EXTENT == N) && IsConst_V<T>,
                 int> = 0>
  constexpr Span(const std::array<ValueType, N>& arr) noexcept
      : Span(arr.data(), N) {}

  /**
   * @brief Constructs a Span from a mutable axio::Array.
   * @tparam N Array size.
   * @param arr Source array.
   */
  template <SizeT N,
            EnableIf_T<(EXTENT == kDynamicExtent || EXTENT == N), int> = 0>
  constexpr Span(Array<ValueType, N>& arr) noexcept : Span(arr.Data(), N) {}

  /**
   * @brief Constructs a Span from a const axio::Array.
   *
   * Only valid for `T == const ValueType`.
   *
   * @tparam N Array size.
   * @param arr Source array.
   */
  template <
      SizeT N,
      EnableIf_T<(EXTENT == kDynamicExtent || EXTENT == N) && IsConst_V<T>,
                 int> = 0>
  constexpr Span(const Array<ValueType, N>& arr) noexcept
      : Span(arr.Data(), N) {}

  /**
   * @brief Constructs a Span from any contiguous container exposing
   * `Data()`/`Size()` or `data()`/`size()`.
   * @tparam Container Source container type.
   * @param c Source container.
   */
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

  /**
   * @brief Converting constructor from a compatible Span<U, E>.
   * @tparam U Source element type.
   * @tparam E Source extent.
   * @param other Source span.
   */
  template <typename U,
            SizeT E,
            EnableIf_T<(EXTENT == kDynamicExtent || E == kDynamicExtent ||
                        EXTENT == E) &&
                           IsConvertible_V<U (*)[], T (*)[]>,
                       int> = 0>
  constexpr Span(const Span<U, E>& other) noexcept
      : Span(other.Data(), other.Size()) {}

  /// @brief Copy-constructs a Span (shallow; shares the referenced data).
  constexpr Span(const Span&) noexcept = default;
  /// @brief Copy-assigns a Span (shallow; shares the referenced data).
  constexpr Span& operator=(const Span&) noexcept = default;

  /**
   * @brief Returns the number of elements in the Span.
   * @return Element count.
   */
  AXIO_NODISCARD constexpr SizeType Size() const noexcept {
    return this->GetSize();
  }

  /**
   * @brief Returns the size of the Span in bytes.
   * @return `Size() * sizeof(T)`.
   */
  AXIO_NODISCARD constexpr SizeType SizeBytes() const noexcept {
    return this->GetSize() * sizeof(T);
  }

  /**
   * @brief Checks whether the Span has zero elements.
   * @return `true` if the Span is empty.
   */
  AXIO_NODISCARD constexpr bool IsEmpty() const noexcept {
    return this->GetSize() == 0;
  }

  /**
   * @brief Returns a pointer to the first element.
   * @return Pointer to the underlying data.
   */
  AXIO_NODISCARD constexpr Pointer Data() const noexcept { return this->data; }

  /**
   * @brief Returns a reference to the first element.
   * @return Reference to the first element.
   * @note Triggers AXIO_ASSERT if the Span is empty.
   */
  AXIO_NODISCARD constexpr Reference Front() const noexcept {
    AXIO_ASSERT(!IsEmpty());
    return this->data[0];
  }

  /**
   * @brief Returns a reference to the last element.
   * @return Reference to the last element.
   * @note Triggers AXIO_ASSERT if the Span is empty.
   */
  AXIO_NODISCARD constexpr Reference Back() const noexcept {
    AXIO_ASSERT(!IsEmpty());
    return this->data[this->GetSize() - 1];
  }

  /**
   * @brief Accesses the element at index @p i.
   * @param i Index. Triggers AXIO_ASSERT if out of range.
   * @return Reference to the element at index @p i.
   */
  AXIO_NODISCARD constexpr Reference operator[](SizeType i) const noexcept {
    AXIO_ASSERT(i < this->GetSize());
    return this->data[i];
  }

  /**
   * @brief Accesses the element at index @p i, with bounds checking.
   * @param i Index.
   * @return Reference to the element at index @p i.
   * @throws std::out_of_range if @p i is out of bounds.
   */
  AXIO_NODISCARD Reference At(SizeType i) const {
    if (i >= this->GetSize()) {
      throw std::out_of_range("Span::At(SizeType) index " + std::to_string(i) +
                              " out of range");
    }
    return this->data[i];
  }

  /**
   * @brief Returns a Span over the first N elements (compile-time size).
   * @tparam N Number of elements to take. Must not be kDynamicExtent.
   * @return Sub-span over the first N elements.
   * @note Triggers AXIO_ASSERT if `N > Size()`.
   */
  template <SizeT N>
  AXIO_NODISCARD constexpr Span<T, N> First() const noexcept {
    static_assert(N != kDynamicExtent);
    AXIO_ASSERT(N <= Size());
    return {this->data, N};
  }

  /**
   * @brief Returns a Span over the first @p n elements (runtime size).
   * @param n Number of elements to take.
   * @return Sub-span over the first @p n elements.
   * @note Triggers AXIO_ASSERT if `n > Size()`.
   */
  AXIO_NODISCARD constexpr Span<T> First(SizeType n) const noexcept {
    AXIO_ASSERT(n <= Size());
    return {this->data, n};
  }

  /**
   * @brief Returns a Span over the last N elements (compile-time size).
   * @tparam N Number of elements to take. Must not be kDynamicExtent.
   * @return Sub-span over the last N elements.
   * @note Triggers AXIO_ASSERT if `N > Size()`.
   */
  template <SizeT N>
  AXIO_NODISCARD constexpr Span<T, N> Last() const noexcept {
    static_assert(N != kDynamicExtent);
    AXIO_ASSERT(N <= Size());
    return {this->data + (Size() - N), N};
  }

  /**
   * @brief Returns a Span over the last @p n elements (runtime size).
   * @param n Number of elements to take.
   * @return Sub-span over the last @p n elements.
   * @note Triggers AXIO_ASSERT if `n > Size()`.
   */
  AXIO_NODISCARD constexpr Span<T> Last(SizeType n) const noexcept {
    AXIO_ASSERT(n <= Size());
    return {this->data + (Size() - n), n};
  }

  /**
   * @brief Returns a sub-span starting at OFFSET with N elements
   * (compile-time).
   * @tparam OFFSET Starting offset.
   * @tparam N Number of elements, or kDynamicExtent for the remainder.
   * @return Sub-span of the requested extent.
   * @note Triggers AXIO_ASSERT if @p OFFSET or @p N is out of range.
   */
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

  /**
   * @brief Returns a sub-span starting at @p offset with @p count elements
   * (runtime).
   * @param offset Starting offset.
   * @param count Number of elements, or kDynamicExtent for the remainder.
   * @return Sub-span of the requested extent.
   * @note Triggers AXIO_ASSERT if @p offset or @p count is out of range.
   */
  AXIO_NODISCARD constexpr Span<T> Subspan(
      SizeType offset,
      SizeType count = kDynamicExtent) const noexcept {
    AXIO_ASSERT(offset <= Size());
    const auto len = (count == kDynamicExtent) ? Size() - offset : count;
    AXIO_ASSERT(len <= Size() - offset);
    return {this->data + offset, len};
  }

  /// @return Iterator to the first element.
  AXIO_NODISCARD constexpr Iterator begin() const noexcept {
    return this->data;
  }

  /// @return Iterator past the last element.
  AXIO_NODISCARD constexpr Iterator end() const noexcept {
    return this->data + this->GetSize();
  }

  /// @return Const iterator to the first element.
  AXIO_NODISCARD constexpr ConstIterator cbegin() const noexcept {
    return this->data;
  }

  /// @return Const iterator past the last element.
  AXIO_NODISCARD constexpr ConstIterator cend() const noexcept {
    return this->data + this->GetSize();
  }

  /// @return Reverse iterator to the last element.
  AXIO_NODISCARD constexpr ReverseIterator rbegin() const noexcept {
    return ReverseIterator{end()};
  }

  /// @return Reverse iterator past the first element.
  AXIO_NODISCARD constexpr ReverseIterator rend() const noexcept {
    return ReverseIterator{begin()};
  }

  /// @return Const reverse iterator to the last element.
  AXIO_NODISCARD constexpr ConstReverseIterator crbegin() const noexcept {
    return ConstReverseIterator{cend()};
  }

  /// @return Const reverse iterator past the first element.
  AXIO_NODISCARD constexpr ConstReverseIterator crend() const noexcept {
    return ConstReverseIterator{cbegin()};
  }

  /**
   * @brief Reinterprets the Span as a read-only view of raw bytes.
   * @return Span of `const Byte` covering the same memory.
   */
  AXIO_NODISCARD
  Span<const Byte,
       EXTENT == kDynamicExtent ? kDynamicExtent : EXTENT * sizeof(T)>
  AsBytes() const noexcept {
    return {reinterpret_cast<const Byte*>(this->data), SizeBytes()};
  }

  /**
   * @brief Reinterprets the Span as a writable view of raw bytes.
   *
   * Only available when T is non-const.
   *
   * @tparam U Defaulted to T; used to constrain availability.
   * @return Span of `Byte` covering the same memory.
   */
  template <typename U = T, EnableIf_T<!IsConst_V<U>, int> = 0>
  AXIO_NODISCARD
      Span<Byte, EXTENT == kDynamicExtent ? kDynamicExtent : EXTENT * sizeof(T)>
      AsWritableBytes() const noexcept {
    return {reinterpret_cast<Byte*>(this->data), SizeBytes()};
  }

  /**
   * @brief Reinterprets the Span's bytes as a read-only Span of U.
   * @tparam U Target element type. Must not be a reference type.
   * @return Span of `const U` covering the same memory.
   * @note Triggers AXIO_ASSERT if the byte size is not a multiple of
   * `sizeof(U)`.
   */
  template <typename U>
  AXIO_NODISCARD Span<const U> As() const noexcept {
    static_assert(!IsReference_V<U>, "U must not be a reference type");
    AXIO_ASSERT(SizeBytes() % sizeof(U) == 0);
    return {reinterpret_cast<const U*>(this->data), SizeBytes() / sizeof(U)};
  }

  /**
   * @brief Reinterprets the Span's bytes as a writable Span of U.
   *
   * Only available when T is non-const.
   *
   * @tparam U Target element type. Must not be a reference type.
   * @tparam V Defaulted to T; used to constrain availability.
   * @return Span of `U` covering the same memory.
   * @note Triggers AXIO_ASSERT if the byte size is not a multiple of
   * `sizeof(U)`.
   */
  template <typename U, typename V = T, EnableIf_T<!IsConst_V<V>, int> = 0>
  AXIO_NODISCARD Span<U> AsWritable() const noexcept {
    static_assert(!IsReference_V<U>, "U must not be a reference type");
    AXIO_ASSERT(SizeBytes() % sizeof(U) == 0);
    return {reinterpret_cast<U*>(this->data), SizeBytes() / sizeof(U)};
  }

  /**
   * @brief Splits the Span at compile-time INDEX.
   * @tparam INDEX Split point. Must not be kDynamicExtent.
   * @return Pair of sub-spans: `[0, INDEX)` and `[INDEX, Size())`.
   * @note Triggers AXIO_ASSERT if `INDEX > Size()`.
   */
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

  /**
   * @brief Splits the Span at a runtime index.
   * @param index Split point.
   * @return Pair of sub-spans: `[0, index)` and `[index, Size())`.
   * @note Triggers AXIO_ASSERT if `index > Size()`.
   */
  AXIO_NODISCARD constexpr std::pair<Span<T>, Span<T>> Split(
      SizeType index) const noexcept {
    AXIO_ASSERT(index <= Size());
    return {
        Span<T>{this->data, index},
        Span<T>{this->data + index, Size() - index},
    };
  }
};

/// @brief Deduces a static-extent Span from a C array.
template <typename T, SizeT N>
Span(T (&)[N]) -> Span<T, N>;

/// @brief Deduces a static-extent Span from a mutable std::array.
template <typename T, SizeT N>
Span(std::array<T, N>&) -> Span<T, N>;

/// @brief Deduces a static-extent, const-element Span from a const
/// std::array.
template <typename T, SizeT N>
Span(const std::array<T, N>&) -> Span<const T, N>;

/// @brief Deduces a static-extent Span from a mutable axio::Array.
template <typename T, SizeT N>
Span(Array<T, N>&) -> Span<T, N>;

/// @brief Deduces a static-extent, const-element Span from a const
/// axio::Array.
template <typename T, SizeT N>
Span(const Array<T, N>&) -> Span<const T, N>;

/// @brief Deduces a dynamic-extent Span from any contiguous container.
template <typename Container>
Span(Container&) -> Span<
    RemovePointer_T<decltype(detail::GetData(std::declval<Container&>()))>>;
}  // namespace axio

#endif