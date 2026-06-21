/**
 * @file array.hpp
 * @brief Fixed-size array container with STL-compatible interface.
 */

#ifndef AXIO_CONTAINER_ARRAY_HPP_
#define AXIO_CONTAINER_ARRAY_HPP_

#include <algorithm>
#include <iterator>

#include "../base/macros.hpp"
#include "../base/types.hpp"
#include "../string/axio_repr.hpp"
#include "../utility/move.hpp"

#include "detail/tuple_fwd.hpp"

namespace axio {
/**
 * @brief A fixed-size aggregate array of N elements of type T.
 *
 * Provides bounds-checked access, iterator support, and structured-binding
 * (tuple protocol) integration. Specialized for N == 0 to avoid zero-length
 * array UB.
 *
 * @tparam T Element type.
 * @tparam N Number of elements.
 */
template <typename T, SizeT N>
struct Array {
  using ValueType = T;
  using SizeType = SizeT;
  using DifferenceType = PtrDiffT;

  using Reference = T&;
  using ConstReference = const T&;
  using Pointer = T*;
  using ConstPointer = const T*;
  using Iterator = T*;
  using ConstIterator = const T*;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  /**
   * @brief Raw storage.
   *
   * Aggregate-initialize directly: `Array<int, 3>{{1, 2, 3}}`.
   */
  ValueType values[N];

  /**
   * @brief Checks whether the array has zero elements.
   * @return `true` if N == 0, otherwise `false`.
   */
  constexpr Bool IsEmpty() const noexcept { return N == 0; }

  /**
   * @brief Returns the number of elements in the array.
   * @return N.
   */
  constexpr SizeType Size() const noexcept { return N; }

  /**
   * @brief Returns the maximum number of elements the array can hold.
   * @return N.
   */
  constexpr SizeType MaxSize() const noexcept { return N; }

  /**
   * @brief Returns a pointer to the underlying storage.
   * @return Mutable pointer to the first element.
   */
  constexpr Pointer Data() noexcept { return values; }

  /**
   * @brief Returns a pointer to the underlying storage.
   * @return Const pointer to the first element.
   */
  constexpr ConstPointer Data() const noexcept { return values; }

  /**
   * @brief Accesses the element at index @p i.
   * @param i Index. Triggers AXIO_ASSERT if `i >= N`.
   * @return Mutable reference to the element at index @p i.
   */
  constexpr Reference At(SizeType i) { return AXIO_ASSERT(i < N), values[i]; }

  /**
   * @brief Accesses the element at index @p i.
   * @param i Index. Triggers AXIO_ASSERT if `i >= N`.
   * @return Const reference to the element at index @p i.
   */
  constexpr ConstReference At(SizeType i) const {
    return AXIO_ASSERT(i < N), values[i];
  }

  /**
   * @brief Accesses the element at index @p i.
   * @param i Index. Triggers AXIO_ASSERT if `i >= N`.
   * @return Mutable reference to the element at index @p i.
   */
  constexpr Reference operator[](SizeType i) {
    return AXIO_ASSERT(i < N), values[i];
  }

  /**
   * @brief Accesses the element at index @p i.
   * @param i Index. Triggers AXIO_ASSERT if `i >= N`.
   * @return Const reference to the element at index @p i.
   */
  constexpr ConstReference operator[](SizeType i) const {
    return AXIO_ASSERT(i < N), values[i];
  }

  /**
   * @brief Accesses the first element.
   * @return Mutable reference to the first element.
   * @warning static_assert fires if N == 0.
   */
  constexpr Reference Front() {
    static_assert(N > 0, "Array size must be greater than 0");
    return values[0];
  }

  /**
   * @brief Accesses the first element.
   * @return Const reference to the first element.
   * @warning static_assert fires if N == 0.
   */
  constexpr ConstReference Front() const {
    static_assert(N > 0, "Array size must be greater than 0");
    return values[0];
  }

  /**
   * @brief Accesses the last element.
   * @return Mutable reference to the last element.
   * @warning static_assert fires if N == 0.
   */
  constexpr Reference Back() {
    static_assert(N > 0, "Array size must be greater than 0");
    return values[N - 1];
  }

  /**
   * @brief Accesses the last element.
   * @return Const reference to the last element.
   * @warning static_assert fires if N == 0.
   */
  constexpr ConstReference Back() const {
    static_assert(N > 0, "Array size must be greater than 0");
    return values[N - 1];
  }

  /// @return Mutable iterator to the first element.
  constexpr Pointer begin() noexcept { return values; }
  /// @return Const iterator to the first element.
  constexpr ConstPointer begin() const noexcept { return values; }
  /// @return Mutable reverse iterator to the last element.
  constexpr ReverseIterator rbegin() noexcept {
    return ReverseIterator{values + N};
  }
  /// @return Const reverse iterator to the last element.
  constexpr ConstReverseIterator rbegin() const noexcept {
    return ConstReverseIterator{values + N};
  }

  /// @return Mutable iterator past the last element.
  constexpr Pointer end() noexcept { return values + N; }
  /// @return Const iterator past the last element.
  constexpr ConstPointer end() const noexcept { return values + N; }
  /// @return Mutable reverse iterator past the first element.
  constexpr ReverseIterator rend() noexcept { return ReverseIterator{values}; }
  /// @return Const reverse iterator past the first element.
  constexpr ConstReverseIterator rend() const noexcept {
    return ConstReverseIterator{values};
  }
};

/**
 * @brief Zero-element specialization of Array, avoiding zero-length array UB.
 *
 * All accessors return a dereference of a `nullptr` Data() pointer guarded
 * by the fact that they are never actually called for a well-formed empty
 * array; size queries report 0 and iterators report `nullptr`.
 *
 * @tparam T Element type.
 */
template <typename T>
struct Array<T, 0> {
  using ValueType = T;
  using SizeType = SizeT;
  using DifferenceType = PtrDiffT;

  using Reference = T&;
  using ConstReference = const T&;
  using Pointer = T*;
  using ConstPointer = const T*;
  using Iterator = T*;
  using ConstIterator = const T*;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  /// @return Always `true`.
  constexpr Bool IsEmpty() const noexcept { return true; }
  /// @return Always 0.
  constexpr SizeType Size() const noexcept { return 0; }
  /// @return Always 0.
  constexpr SizeType MaxSize() const noexcept { return 0; }

  /// @return Always `nullptr`.
  constexpr Pointer Data() noexcept { return nullptr; }
  /// @return Always `nullptr`.
  constexpr ConstPointer Data() const noexcept { return nullptr; }

  constexpr Reference At(SizeType) { return *Data(); }
  constexpr ConstReference At(SizeType) const { return *Data(); }
  constexpr Reference operator[](SizeType) { return *Data(); }
  constexpr ConstReference operator[](SizeType) const { return *Data(); }

  constexpr Reference Front() { return *Data(); }
  constexpr ConstReference Front() const { return *Data(); }
  constexpr Reference Back() { return *Data(); }
  constexpr ConstReference Back() const { return *Data(); }

  /// @return Always `nullptr`.
  constexpr Pointer begin() noexcept { return nullptr; }
  /// @return Always `nullptr`.
  constexpr ConstPointer begin() const noexcept { return nullptr; }
  /// @return Reverse iterator wrapping `nullptr`.
  constexpr ReverseIterator rbegin() noexcept {
    return ConstReverseIterator{nullptr};
  }
  /// @return Reverse iterator wrapping `nullptr`.
  constexpr ConstReverseIterator rbegin() const noexcept {
    return ConstReverseIterator{nullptr};
  }

  /// @return Always `nullptr`.
  constexpr Pointer end() noexcept { return nullptr; }
  /// @return Always `nullptr`.
  constexpr ConstPointer end() const noexcept { return nullptr; }
  /// @return Reverse iterator wrapping `nullptr`.
  constexpr ReverseIterator rend() noexcept {
    return ConstReverseIterator{nullptr};
  }
  /// @return Reverse iterator wrapping `nullptr`.
  constexpr ConstReverseIterator rend() const noexcept {
    return ConstReverseIterator{nullptr};
  }
};

/**
 * @brief Checks whether two arrays are element-wise equal.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @return `true` if all elements compare equal.
 */
template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator==(const Array<T, N>& lhs,
                                      const Array<T, N>& rhs) {
  return std::equal(lhs.values, lhs.values + N, rhs.values);
}

/**
 * @brief Checks whether two arrays are not element-wise equal.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @return `true` if any element differs.
 */
template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator!=(const Array<T, N>& lhs,
                                      const Array<T, N>& rhs) {
  return !(lhs == rhs);
}

/**
 * @brief Lexicographically compares two arrays.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @return `true` if @p lhs is lexicographically less than @p rhs.
 */
template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator<(const Array<T, N>& lhs,
                                     const Array<T, N>& rhs) {
  return std::lexicographical_compare(lhs.values, lhs.values + N, rhs.values,
                                      rhs.values + N);
}

/**
 * @brief Lexicographically compares two arrays.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @return `true` if @p lhs is lexicographically greater than @p rhs.
 */
template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator>(const Array<T, N>& lhs,
                                     const Array<T, N>& rhs) {
  return rhs < lhs;
}

/**
 * @brief Lexicographically compares two arrays.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @return `true` if @p lhs is lexicographically less than or equal to
 * @p rhs.
 */
template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator<=(const Array<T, N>& lhs,
                                      const Array<T, N>& rhs) {
  return !(rhs < lhs);
}

/**
 * @brief Lexicographically compares two arrays.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @return `true` if @p lhs is lexicographically greater than or equal to
 * @p rhs.
 */
template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator>=(const Array<T, N>& lhs,
                                      const Array<T, N>& rhs) {
  return !(lhs < rhs);
}

// N=0 specializations: comparisons resolve to the mathematically correct
// constant for an empty sequence, without touching any storage.

/// @return Always `true` (two empty arrays are equal).
template <typename T>
AXIO_NODISCARD inline Bool operator==(const Array<T, 0>&, const Array<T, 0>&) {
  return true;
}

/// @return Always `false`.
template <typename T>
AXIO_NODISCARD inline Bool operator!=(const Array<T, 0>&, const Array<T, 0>&) {
  return false;
}

/// @return Always `false`.
template <typename T>
AXIO_NODISCARD inline Bool operator<(const Array<T, 0>&, const Array<T, 0>&) {
  return false;
}

/// @return Always `false`.
template <typename T>
AXIO_NODISCARD inline Bool operator>(const Array<T, 0>&, const Array<T, 0>&) {
  return false;
}

/// @return Always `true`.
template <typename T>
AXIO_NODISCARD inline Bool operator<=(const Array<T, 0>&, const Array<T, 0>&) {
  return true;
}

/// @return Always `true`.
template <typename T>
AXIO_NODISCARD inline Bool operator>=(const Array<T, 0>&, const Array<T, 0>&) {
  return true;
}

/**
 * @brief Tuple-protocol size for Array, equal to N.
 * @tparam T Element type.
 * @tparam N Number of elements.
 */
template <typename T, SizeT N>
struct TupleSize<Array<T, N>> : IntegralConstant<SizeT, N> {};

/**
 * @brief Tuple-protocol element type for Array; always T regardless of I.
 * @tparam I Tuple index (unused, all elements share type T).
 * @tparam T Element type.
 * @tparam N Number of elements.
 */
template <SizeT I, typename T, SizeT N>
struct TupleElement<I, Array<T, N>> {
  using type = T;
};

/**
 * @brief Gets element I of an lvalue Array.
 * @tparam I Index. Must satisfy `I < N`.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param arr Array to access.
 * @return Mutable reference to element I.
 */
template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) Get(Array<T, N>& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return arr.values[I];
}

/**
 * @brief Gets element I of a const lvalue Array.
 * @tparam I Index. Must satisfy `I < N`.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param arr Array to access.
 * @return Const reference to element I.
 */
template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) Get(const Array<T, N>& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return arr.values[I];
}

/**
 * @brief Gets element I of an rvalue Array.
 * @tparam I Index. Must satisfy `I < N`.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param arr Array to access.
 * @return Moved-from rvalue reference to element I.
 */
template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) Get(Array<T, N>&& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return Move(arr.values[I]);
}

/**
 * @brief Gets element I of a const rvalue Array.
 * @tparam I Index. Must satisfy `I < N`.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param arr Array to access.
 * @return Moved-from const rvalue reference to element I.
 */
template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) Get(const Array<T, N>&& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return Move(arr.values[I]);
}

/**
 * @brief Gets element I of an lvalue Array, for structured bindings.
 * @tparam I Index. Must satisfy `I < N`.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param arr Array to access.
 * @return Mutable reference to element I.
 */
template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) get(Array<T, N>& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return arr.values[I];
}

/**
 * @brief Gets element I of a const lvalue Array, for structured bindings.
 * @tparam I Index. Must satisfy `I < N`.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param arr Array to access.
 * @return Const reference to element I.
 */
template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) get(const Array<T, N>& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return arr.values[I];
}

/**
 * @brief Gets element I of an rvalue Array, for structured bindings.
 * @tparam I Index. Must satisfy `I < N`.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param arr Array to access.
 * @return Moved-from rvalue reference to element I.
 */
template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) get(Array<T, N>&& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return Move(arr.values[I]);
}

/**
 * @brief Gets element I of a const rvalue Array, for structured bindings.
 * @tparam I Index. Must satisfy `I < N`.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param arr Array to access.
 * @return Moved-from const rvalue reference to element I.
 */
template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) get(const Array<T, N>&& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return Move(arr.values[I]);
}

/**
 * @brief Appends a human-readable representation of an empty Array.
 * @tparam Output Output buffer type.
 * @tparam T Element type.
 * @param output Buffer to append to.
 *
 * Always appends `[]`.
 */
template <typename Output, typename T>
void AxioRepr(Output& output, Array<T, 0>) {
  output.Append("[]", 2);
}

/**
 * @brief Appends a human-readable representation of a single-element Array.
 * @tparam Output Output buffer type.
 * @tparam T Element type.
 * @param output Buffer to append to.
 * @param a Array to format.
 *
 * Format: `[x]`.
 */
template <typename Output, typename T>
void AxioRepr(Output& output, const Array<T, 1>& a) {
  AppendToOutput(output, '[', a[0], ']');
}

namespace internal {
/**
 * @brief Appends each element of @p a, followed by a separator, in order.
 * @tparam Output Output buffer type.
 * @tparam T Element type.
 * @tparam N Number of elements in @p a.
 * @tparam Is Index sequence selecting which elements to append.
 * @param output Buffer to append to.
 * @param a Array to format.
 */
template <typename Output, typename T, SizeT N, SizeT... Is>
void AppendSequence(Output& output,
                    const Array<T, N>& a,
                    std::index_sequence<Is...>) {
  (AppendToOutput(output, a[Is], ", "), ...);
}
}  // namespace internal

/**
 * @brief Appends a human-readable representation of an Array with two or
 * more elements.
 * @tparam Output Output buffer type.
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @param output Buffer to append to.
 * @param a Array to format.
 *
 * Format: `[a, b, ..., z]`.
 */
template <typename Output, typename T, SizeT N>
void AxioRepr(Output& output, const Array<T, N>& a) {
  output.Append(1, '[');
  internal::AppendSequence(output, a, std::make_index_sequence<N - 1>{});
  AppendToOutput(output, a[N - 1], ']');
}
}  // namespace axio

namespace std {
/** Specializes std::tuple_size for structured bindings support. */
template <typename T, size_t N>
struct tuple_size<axio::Array<T, N>> : std::integral_constant<size_t, N> {};

/** Specializes std::tuple_element for structured bindings support. */
template <size_t I, typename T, size_t N>
struct tuple_element<I, axio::Array<T, N>> {
  using type = T;
};
}  // namespace std

#endif