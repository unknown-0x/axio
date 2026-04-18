#ifndef AXIO_CONTAINER_ARRAY_HPP_
#define AXIO_CONTAINER_ARRAY_HPP_

#include <algorithm>
#include <iterator>

#include "../base/macros.hpp"
#include "../base/types.hpp"
#include "../utility/move.hpp"

#include "detail/tuple_fwd.hpp"

namespace axio {
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

  ValueType values[N];

  constexpr Bool IsEmpty() const noexcept { return N == 0; }
  constexpr SizeType Size() const noexcept { return N; }
  constexpr SizeType MaxSize() const noexcept { return N; }

  constexpr Pointer Data() noexcept { return values; }
  constexpr ConstPointer Data() const noexcept { return values; }

  constexpr Reference At(SizeType i) { return AXIO_ASSERT(i < N), values[i]; }

  constexpr ConstReference At(SizeType i) const {
    return AXIO_ASSERT(i < N), values[i];
  }

  constexpr Reference operator[](SizeType i) {
    return AXIO_ASSERT(i < N), values[i];
  }

  constexpr ConstReference operator[](SizeType i) const {
    return AXIO_ASSERT(i < N), values[i];
  }

  constexpr Reference Front() {
    static_assert(N > 0, "Array size must be greater than 0");
    return values[0];
  }

  constexpr ConstReference Front() const {
    static_assert(N > 0, "Array size must be greater than 0");
    return values[0];
  }

  constexpr Reference Back() {
    static_assert(N > 0, "Array size must be greater than 0");
    return values[N - 1];
  }

  constexpr ConstReference Back() const {
    static_assert(N > 0, "Array size must be greater than 0");
    return values[N - 1];
  }

  constexpr Pointer begin() noexcept { return values; }
  constexpr ConstPointer begin() const noexcept { return values; }
  constexpr ReverseIterator rbegin() noexcept {
    return ReverseIterator{values + N};
  }
  constexpr ConstReverseIterator rbegin() const noexcept {
    return ConstReverseIterator{values + N};
  }

  constexpr Pointer end() noexcept { return values + N; }
  constexpr ConstPointer end() const noexcept { return values + N; }
  constexpr ReverseIterator rend() noexcept { return ReverseIterator{values}; }
  constexpr ConstReverseIterator rend() const noexcept {
    return ConstReverseIterator{values};
  }
};

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

  constexpr Bool IsEmpty() const noexcept { return true; }
  constexpr SizeType Size() const noexcept { return 0; }
  constexpr SizeType MaxSize() const noexcept { return 0; }

  constexpr Pointer Data() noexcept { return nullptr; }
  constexpr ConstPointer Data() const noexcept { return nullptr; }

  constexpr Reference At(SizeType i) { return *Data(); }
  constexpr ConstReference At(SizeType i) const { return *Data(); }
  constexpr Reference operator[](SizeType i) { return *Data(); }
  constexpr ConstReference operator[](SizeType i) const { return *Data(); }

  constexpr Reference Front() { return *Data(); }
  constexpr ConstReference Front() const { return *Data(); }
  constexpr Reference Back() { return *Data(); }
  constexpr ConstReference Back() const { return *Data(); }

  constexpr Pointer begin() noexcept { return nullptr; }
  constexpr ConstPointer begin() const noexcept { return nullptr; }
  constexpr ReverseIterator rbegin() noexcept {
    return ConstReverseIterator{nullptr};
  }
  constexpr ConstReverseIterator rbegin() const noexcept {
    return ConstReverseIterator{nullptr};
  }

  constexpr Pointer end() noexcept { return nullptr; }
  constexpr ConstPointer end() const noexcept { return nullptr; }
  constexpr ReverseIterator rend() noexcept {
    return ConstReverseIterator{nullptr};
  }
  constexpr ConstReverseIterator rend() const noexcept {
    return ConstReverseIterator{nullptr};
  }
};

template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator==(const Array<T, N>& lhs,
                                      const Array<T, N>& rhs) {
  return std::equal(lhs.values, lhs.values + N, rhs.values);
}

template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator!=(const Array<T, N>& lhs,
                                      const Array<T, N>& rhs) {
  return !(lhs == rhs);
}

template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator<(const Array<T, N>& lhs,
                                     const Array<T, N>& rhs) {
  return std::lexicographical_compare(lhs.values, lhs.values + N, rhs.values,
                                      rhs.values + N);
}

template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator>(const Array<T, N>& lhs,
                                     const Array<T, N>& rhs) {
  return rhs < lhs;
}

template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator<=(const Array<T, N>& lhs,
                                      const Array<T, N>& rhs) {
  return !(rhs < lhs);
}

template <typename T, SizeT N>
AXIO_NODISCARD inline Bool operator>=(const Array<T, N>& lhs,
                                      const Array<T, N>& rhs) {
  return !(lhs < rhs);
}

template <typename T>
AXIO_NODISCARD inline Bool operator==(const Array<T, 0>&, const Array<T, 0>&) {
  return true;
}

template <typename T>
AXIO_NODISCARD inline Bool operator!=(const Array<T, 0>&, const Array<T, 0>&) {
  return false;
}

template <typename T>
AXIO_NODISCARD inline Bool operator<(const Array<T, 0>&, const Array<T, 0>&) {
  return false;
}

template <typename T>
AXIO_NODISCARD inline Bool operator>(const Array<T, 0>&, const Array<T, 0>&) {
  return false;
}

template <typename T>
AXIO_NODISCARD inline Bool operator<=(const Array<T, 0>&, const Array<T, 0>&) {
  return true;
}

template <typename T>
AXIO_NODISCARD inline Bool operator>=(const Array<T, 0>&, const Array<T, 0>&) {
  return true;
}

template <typename T, SizeT N>
struct TupleSize<Array<T, N>> : IntegralConstant<SizeT, N> {};

template <SizeT I, typename T, SizeT N>
struct TupleElement<I, Array<T, N>> {
  using type = T;
};

template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) Get(Array<T, N>& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return arr.values[I];
}

template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) Get(const Array<T, N>& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return arr.values[I];
}

template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) Get(Array<T, N>&& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return Move(arr.values[I]);
}

template <SizeT I, typename T, SizeT N>
constexpr decltype(auto) Get(const Array<T, N>&& arr) noexcept {
  static_assert(I < N, "Index exceeds array size");
  return Move(arr.values[I]);
}
}  // namespace axio

#endif