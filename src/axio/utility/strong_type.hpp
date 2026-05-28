#ifndef AXIO_UTILITY_STRONG_TYPE_HPP_
#define AXIO_UTILITY_STRONG_TYPE_HPP_

#include "move.hpp"

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"
#include "../string/axio_repr.hpp"

namespace axio {
template <typename T, typename Tag, template <typename> class... Skills>
struct StrongType : public Skills<StrongType<T, Tag, Skills...>>... {
  static_assert(!IsReference<T>::value, "Reference types are not supported");

  using ValueType = T;

  constexpr StrongType() : value_{} {}

  constexpr StrongType(const StrongType&) = default;
  constexpr StrongType(StrongType&&) = default;
  constexpr StrongType& operator=(const StrongType&) = default;
  constexpr StrongType& operator=(StrongType&&) = default;

  explicit constexpr StrongType(const T& value) noexcept(
      IsNothrowCopyConstructible_V<T>)
      : value_(value) {}
  explicit constexpr StrongType(T&& value) noexcept(
      IsNothrowMoveConstructible_V<T>)
      : value_(axio::Move(value)) {}

  AXIO_NODISCARD constexpr T& Get() noexcept { return value_; }
  AXIO_NODISCARD constexpr const T& Get() const noexcept { return value_; }

  AXIO_NODISCARD explicit constexpr operator T() const noexcept {
    return value_;
  }

 private:
  ValueType value_;
};

namespace detail {
template <typename T>
struct StrongTypeTraits;

template <typename T, typename Tag, template <typename> class... Skills>
struct StrongTypeTraits<StrongType<T, Tag, Skills...>> {
  using ValueType = T;
};

template <typename Ptr>
constexpr auto GetPointer(Ptr& p) -> decltype(p.operator->()) {
  return p.operator->();
}

template <typename Element>
constexpr auto GetPointer(Element* p) -> Element* {
  return p;
}
}  // namespace detail

template <typename T>
struct Incrementable {
  constexpr T& operator++() {
    ++static_cast<T*>(this)->Get();
    return *static_cast<T*>(this);
  }

  constexpr T operator++(int) {
    T copy(*static_cast<T*>(this));
    ++(*this);
    return copy;
  }
};

template <typename T>
struct Decrementable {
  constexpr T& operator--() {
    --static_cast<T*>(this)->Get();
    return *static_cast<T*>(this);
  }

  constexpr T operator--(int) {
    T copy(*static_cast<T*>(this));
    --(*this);
    return copy;
  }
};

template <typename T>
struct BinaryAddable {
  friend constexpr T operator+(const T& lhs, const T& rhs) {
    return T(lhs.Get() + rhs.Get());
  }

  friend constexpr T& operator+=(T& lhs, const T& rhs) {
    lhs.Get() += rhs.Get();
    return lhs;
  }
};

template <typename T>
struct BinarySubtractable {
  friend constexpr T operator-(const T& lhs, const T& rhs) {
    return T(lhs.Get() - rhs.Get());
  }

  friend constexpr T& operator-=(T& lhs, const T& rhs) {
    lhs.Get() -= rhs.Get();
    return lhs;
  }
};

template <typename T>
struct UnaryAddable {
  constexpr T operator+() const {
    return T(+static_cast<const T*>(this)->Get());
  }
};

template <typename T>
struct Negatable {
  constexpr T operator-() const {
    return T(-static_cast<const T*>(this)->Get());
  }
};

template <typename T>
struct Addable : BinaryAddable<T>, UnaryAddable<T>, Incrementable<T> {};

template <typename T>
struct Subtractable : BinarySubtractable<T>, Negatable<T>, Decrementable<T> {};

template <typename T>
struct Multiplicable {
  friend constexpr T operator*(const T& lhs, const T& rhs) {
    return T(lhs.Get() * rhs.Get());
  }

  friend constexpr T& operator*=(T& lhs, const T& rhs) {
    lhs.Get() *= rhs.Get();
    return lhs;
  }
};

template <typename T>
struct Divisible {
  friend constexpr T operator/(const T& lhs, const T& rhs) {
    return T(lhs.Get() / rhs.Get());
  }

  friend constexpr T& operator/=(T& lhs, const T& rhs) {
    lhs.Get() /= rhs.Get();
    return lhs;
  }
};

template <typename T>
struct ScalarMultiplicable {
  using Scalar = typename detail::StrongTypeTraits<T>::ValueType;

  friend constexpr T operator*(const T& lhs, const Scalar& s) {
    return T(lhs.Get() * s);
  }

  friend constexpr T operator*(const Scalar& s, const T& rhs) {
    return T(s * rhs.Get());
  }

  friend constexpr T& operator*=(T& lhs, const Scalar& s) {
    lhs.Get() *= s;
    return lhs;
  }
};

template <typename T>
struct ScalarDivisible {
  using Scalar = typename detail::StrongTypeTraits<T>::ValueType;

  friend constexpr T operator/(const T& lhs, const Scalar& s) {
    return T(lhs.Get() / s);
  }

  friend constexpr T& operator/=(T& lhs, const Scalar& s) {
    lhs.Get() /= s;
    return lhs;
  }
};

template <typename T>
struct Modulable {
  friend constexpr T operator%(const T& lhs, const T& rhs) {
    return T(lhs.Get() % rhs.Get());
  }

  friend constexpr T& operator%=(T& lhs, const T& rhs) {
    lhs.Get() %= rhs.Get();
    return lhs;
  }
};

template <typename T>
struct BitwiseAndable {
  friend constexpr T operator&(const T& lhs, const T& rhs) {
    return T(lhs.Get() & rhs.Get());
  }

  friend constexpr T& operator&=(T& lhs, const T& rhs) {
    lhs.Get() &= rhs.Get();
    return lhs;
  }
};

template <typename T>
struct BitwiseOrable {
  friend constexpr T operator|(const T& lhs, const T& rhs) {
    return T(lhs.Get() | rhs.Get());
  }

  friend constexpr T& operator|=(T& lhs, const T& rhs) {
    lhs.Get() |= rhs.Get();
    return lhs;
  }
};

template <typename T>
struct BitwiseXorable {
  friend constexpr T operator^(const T& lhs, const T& rhs) {
    return T(lhs.Get() ^ rhs.Get());
  }

  friend constexpr T& operator^=(T& lhs, const T& rhs) {
    lhs.Get() ^= rhs.Get();
    return lhs;
  }
};

template <typename T>
struct BitwiseComplementable {
  constexpr T operator~() const {
    return T(~static_cast<const T*>(this)->Get());
  }
};

template <typename T>
struct BitwiseShiftable {
  friend constexpr T operator<<(const T& lhs, const T& shift) {
    return T(lhs.Get() << shift.Get());
  }

  friend constexpr T operator>>(const T& lhs, const T& shift) {
    return T(lhs.Get() >> shift.Get());
  }

  friend constexpr T& operator<<=(T& lhs, const T& shift) {
    lhs.Get() <<= shift.Get();
    return lhs;
  }

  friend constexpr T& operator>>=(T& lhs, const T& shift) {
    lhs.Get() >>= shift.Get();
    return lhs;
  }
};

template <typename T>
struct Bitwise : BitwiseAndable<T>,
                 BitwiseOrable<T>,
                 BitwiseXorable<T>,
                 BitwiseComplementable<T>,
                 BitwiseShiftable<T> {};

template <typename T>
struct EqualityComparable {
  friend constexpr bool operator==(const T& lhs, const T& rhs) {
    return lhs.Get() == rhs.Get();
  }

  friend constexpr bool operator!=(const T& lhs, const T& rhs) {
    return !(lhs == rhs);
  }
};

template <typename T>
struct Orderable {
  friend constexpr bool operator<(const T& lhs, const T& rhs) {
    return lhs.Get() < rhs.Get();
  }

  friend constexpr bool operator>(const T& lhs, const T& rhs) {
    return rhs < lhs;
  }

  friend constexpr bool operator<=(const T& lhs, const T& rhs) {
    return !(rhs < lhs);
  }

  friend constexpr bool operator>=(const T& lhs, const T& rhs) {
    return !(lhs < rhs);
  }
};

template <typename T>
struct Comparable : EqualityComparable<T>, Orderable<T> {};

template <typename T>
struct Dereferenceable {
  using U = typename detail::StrongTypeTraits<T>::ValueType;

  constexpr U& operator*() & { return static_cast<T*>(this)->Get(); }

  constexpr U const& operator*() const& {
    return static_cast<const T*>(this)->Get();
  }
};

template <typename T>
struct ImplicitBool {
  constexpr explicit operator bool() const {
    return static_cast<bool>(static_cast<const T*>(this)->Get());
  }

  constexpr bool operator!() const {
    return !static_cast<bool>(*static_cast<const T*>(this));
  }
};

template <typename T>
struct AxioReprable {
  template <typename Output>
  friend void AxioRepr(Output& output, const T& value) {
    AxioRepr(output, value.Get());
  }
};

template <typename T>
struct Arithmetic : Addable<T>,
                    Subtractable<T>,
                    Divisible<T>,
                    Multiplicable<T>,
                    Modulable<T>,
                    Bitwise<T>,
                    Comparable<T>,
                    AxioReprable<T> {};

template <typename T>
struct PointerLike : Comparable<T>, ImplicitBool<T> {
  using U = typename detail::StrongTypeTraits<T>::ValueType;

  constexpr decltype(auto) operator*() & {
    return *static_cast<T*>(this)->Get();
  }

  constexpr decltype(auto) operator*() const& {
    return *static_cast<const T*>(this)->Get();
  }

  constexpr auto operator->() {
    return detail::GetPointer(static_cast<T*>(this)->Get());
  }

  constexpr const auto operator->() const {
    return detail::GetPointer(static_cast<const T*>(this)->Get());
  }
};

template <typename T>
struct Flag : Bitwise<T>, Comparable<T>, AxioReprable<T> {};
}  // namespace axio

#endif