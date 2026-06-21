/**
 * @file strong_type.hpp
 * @brief StrongType<T, Tag, Skills...> — a named wrapper around a primitive
 *        or compound type that prevents implicit mixing of logically distinct
 *        values sharing the same underlying type.
 *
 * Skills are CRTP mixin templates that opt the wrapper into operator sets.
 * Compose them freely:
 *
 * @code
 *   struct MetersTag {};
 *   using Meters = StrongType<double, MetersTag, Addable, Subtractable,
 *                                                ScalarMultiplicable,
 *                                                Comparable, AxioReprable>;
 *
 *   Meters a(1.0), b(2.0);
 *   Meters c = a + b;           // OK — same type
 *   // double d = a;            // error — no implicit conversion
 *   double raw = static_cast<double>(a);  // explicit cast allowed
 * @endcode
 */

#ifndef AXIO_UTILITY_STRONG_TYPE_HPP_
#define AXIO_UTILITY_STRONG_TYPE_HPP_

#include "move.hpp"

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"
#include "../string/axio_repr.hpp"

namespace axio {
/**
 * @brief Named wrapper around a value of type T.
 *
 * The Tag parameter makes each instantiation a distinct type even when T is
 * the same.  Extra operator sets are enabled by listing Skill templates.
 *
 * @tparam T      The underlying value type (must not be a reference).
 * @tparam Tag    A unique tag type used solely to distinguish instantiations.
 * @tparam Skills CRTP mixin templates that add operators (e.g. Addable).
 *
 * @code
 *   struct UserIdTag {};
 *   struct OrderIdTag {};
 *   using UserId  = StrongType<int, UserIdTag, EqualityComparable>;
 *   using OrderId = StrongType<int, OrderIdTag, EqualityComparable>;
 *
 *   UserId u(7);
 *   OrderId o(7);
 *   // u == o;       // error — different types, even though both wrap int
 *   u == UserId(7);  // OK — true
 * @endcode
 */
template <typename T, typename Tag, template <typename> class... Skills>
struct StrongType : public Skills<StrongType<T, Tag, Skills...>>... {
  static_assert(!IsReference<T>::value, "Reference types are not supported");

  using ValueType = T;

  /** @brief Value-initializes the underlying value (e.g. `0` for `int`). */
  constexpr StrongType() : value_{} {}

  constexpr StrongType(const StrongType&) = default;
  constexpr StrongType(StrongType&&) = default;
  constexpr StrongType& operator=(const StrongType&) = default;
  constexpr StrongType& operator=(StrongType&&) = default;

  /** @brief Explicit construction from a const lvalue. */
  explicit constexpr StrongType(const T& value) noexcept(
      IsNothrowCopyConstructible_V<T>)
      : value_(value) {}

  /** @brief Explicit construction from an rvalue. */
  explicit constexpr StrongType(T&& value) noexcept(
      IsNothrowMoveConstructible_V<T>)
      : value_(axio::Move(value)) {}

  /** @brief Returns a mutable reference to the underlying value. */
  AXIO_NODISCARD constexpr T& Get() noexcept { return value_; }

  /** @brief Returns a const reference to the underlying value. */
  AXIO_NODISCARD constexpr const T& Get() const noexcept { return value_; }

  /**
   * @brief Explicit conversion back to T.
   *
   * @code
   *   Meters m(3.0);
   *   double raw = static_cast<double>(m);  // 3.0
   * @endcode
   */
  AXIO_NODISCARD explicit constexpr operator T() const noexcept {
    return value_;
  }

 private:
  ValueType value_;
};

namespace detail {
/// @cond INTERNAL

template <typename T>
struct StrongTypeTraits;

/** @brief Extracts ValueType from a StrongType instantiation. */
template <typename T, typename Tag, template <typename> class... Skills>
struct StrongTypeTraits<StrongType<T, Tag, Skills...>> {
  using ValueType = T;
};

/**
 * @brief operator-> helper for pointer-like wrapped types.
 *
 * Used by PointerLike when the wrapped value itself has its own
 * `operator->` (e.g. a smart pointer), delegating to it directly.
 *
 * @tparam Ptr  Smart-pointer-like type providing `operator->()`.
 */
template <typename Ptr>
constexpr auto GetPointer(Ptr& p) -> decltype(p.operator->()) {
  return p.operator->();
}

/**
 * @brief operator-> helper for raw pointers.
 *
 * Used by PointerLike when the wrapped value is itself a raw pointer, which
 * has no `operator->()` member to call — the pointer is simply returned.
 *
 * @tparam Element  Pointee type.
 */
template <typename Element>
constexpr auto GetPointer(Element* p) -> Element* {
  return p;
}

/// @endcond
}  // namespace detail

/**
 * @brief Adds prefix/postfix `operator++`.
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   Count c(0);
 *   ++c;     // Count(1)
 *   c++;     // returns Count(1), c is now Count(2)
 * @endcode
 */
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

/**
 * @brief Adds prefix/postfix `operator--`.
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   Count c(2);
 *   --c;     // Count(1)
 * @endcode
 */
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

/**
 * @brief Adds `T + T` and `T += T`.
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Adds `T - T` and `T -= T`.
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Adds unary `+T`.
 * @tparam T  The StrongType being extended.
 */
template <typename T>
struct UnaryAddable {
  constexpr T operator+() const {
    return T(+static_cast<const T*>(this)->Get());
  }
};

/**
 * @brief Adds unary negation `-T`.
 * @tparam T  The StrongType being extended.
 */
template <typename T>
struct Negatable {
  constexpr T operator-() const {
    return T(-static_cast<const T*>(this)->Get());
  }
};

/**
 * @brief Combines BinaryAddable, UnaryAddable, and Incrementable.
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   using Meters = StrongType<double, struct MetersTag, Addable>;
 *   Meters a(1.0), b(2.0);
 *   Meters c = a + b;  // Meters(3.0)
 *   ++c;                // Meters(4.0)
 * @endcode
 */
template <typename T>
struct Addable : BinaryAddable<T>, UnaryAddable<T>, Incrementable<T> {};

/**
 * @brief Combines BinarySubtractable, Negatable, and Decrementable.
 * @tparam T  The StrongType being extended.
 */
template <typename T>
struct Subtractable : BinarySubtractable<T>, Negatable<T>, Decrementable<T> {};

/**
 * @brief Adds `T * T` and `T *= T`.
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Adds `T / T` and `T /= T`.
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Adds `T * Scalar`, `Scalar * T`, and `T *= Scalar`.
 *
 * Scalar is the underlying ValueType of T.
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   using Meters = StrongType<double, struct MetersTag, ScalarMultiplicable>;
 *   Meters m(2.0);
 *   Meters n = m * 3.0;  // Meters(6.0)
 *   Meters p = 3.0 * m;  // Meters(6.0)
 * @endcode
 */
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

/**
 * @brief Adds `T / Scalar` and `T /= Scalar`.
 *
 * Scalar is the underlying ValueType of T.
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Adds `T % T` and `T %= T`.
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Adds `T & T` and `T &= T`.
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Adds `T | T` and `T |= T`.
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Adds `T ^ T` and `T ^= T`.
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Adds bitwise complement `~T`.
 * @tparam T  The StrongType being extended.
 */
template <typename T>
struct BitwiseComplementable {
  constexpr T operator~() const {
    return T(~static_cast<const T*>(this)->Get());
  }
};

/**
 * @brief Adds left/right shift operators (`<<`, `>>`, `<<=`, `>>=`).
 *
 * Both operands are T — the shift amount is taken from another instance's
 * underlying value, not a plain integer.
 *
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Combines all bitwise operators:
 *        BitwiseAndable, BitwiseOrable, BitwiseXorable,
 *        BitwiseComplementable, BitwiseShiftable.
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   using Flags = StrongType<unsigned, struct FlagsTag, Bitwise>;
 *   Flags a(0b0011), b(0b0110);
 *   Flags c = a | b;   // Flags(0b0111)
 *   Flags d = ~a;      // bitwise complement of the underlying value
 * @endcode
 */
template <typename T>
struct Bitwise : BitwiseAndable<T>,
                 BitwiseOrable<T>,
                 BitwiseXorable<T>,
                 BitwiseComplementable<T>,
                 BitwiseShiftable<T> {};

/**
 * @brief Adds `==` and `!=`.
 * @tparam T  The StrongType being extended.
 */
template <typename T>
struct EqualityComparable {
  friend constexpr bool operator==(const T& lhs, const T& rhs) {
    return lhs.Get() == rhs.Get();
  }

  friend constexpr bool operator!=(const T& lhs, const T& rhs) {
    return !(lhs == rhs);
  }
};

/**
 * @brief Adds `<`, `>`, `<=`, `>=`.
 *
 * Only `operator<` is implemented directly; the other three are derived
 * from it, so T's underlying value only needs to support `<`.
 *
 * @tparam T  The StrongType being extended.
 */
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

/**
 * @brief Combines EqualityComparable and Orderable.
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   using Meters = StrongType<double, struct MetersTag, Comparable>;
 *   Meters a(1.0), b(2.0);
 *   a < b;   // true
 *   a == b;  // false
 * @endcode
 */
template <typename T>
struct Comparable : EqualityComparable<T>, Orderable<T> {};

/**
 * @brief Adds `*obj` returning a reference to the underlying value.
 *
 * Useful for iterator-like wrappers where Get() yields the element directly.
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   using Cell = StrongType<int, struct CellTag, Dereferenceable>;
 *   Cell c(5);
 *   int& v = *c;  // v == 5, refers into c
 * @endcode
 */
template <typename T>
struct Dereferenceable {
  using U = typename detail::StrongTypeTraits<T>::ValueType;

  constexpr U& operator*() & { return static_cast<T*>(this)->Get(); }

  constexpr U const& operator*() const& {
    return static_cast<const T*>(this)->Get();
  }
};

/**
 * @brief Adds an explicit `operator bool` and `operator!`.
 *
 * Converts to bool via `static_cast<bool>(Get())`.
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   using Flag = StrongType<int, struct FlagTag, ImplicitBool>;
 *   Flag f(1);
 *   if (f) { ... }   // true, since static_cast<bool>(1) == true
 *   !f;              // false
 * @endcode
 */
template <typename T>
struct ImplicitBool {
  constexpr explicit operator bool() const {
    return static_cast<bool>(static_cast<const T*>(this)->Get());
  }

  constexpr bool operator!() const {
    return !static_cast<bool>(*static_cast<const T*>(this));
  }
};

/**
 * @brief Adds AxioRepr support.
 *
 * Forwards directly to the underlying value's AxioRepr overload, so T's
 * formatted output is identical to formatting the wrapped value alone.
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   using Meters = StrongType<double, struct MetersTag, AxioReprable>;
 *   Meters m(3.5);
 *   AxioRepr(out, m);  // same output as AxioRepr(out, 3.5)
 * @endcode
 */
template <typename T>
struct AxioReprable {
  template <typename Output>
  friend void AxioRepr(Output& output, const T& value) {
    AxioRepr(output, value.Get());
  }
};

/**
 * @brief Full arithmetic bundle: Addable, Subtractable, Divisible,
 *        Multiplicable, Modulable, Bitwise, Comparable, AxioReprable.
 * @tparam T  The StrongType being extended.
 */
template <typename T>
struct Arithmetic : Addable<T>,
                    Subtractable<T>,
                    Divisible<T>,
                    Multiplicable<T>,
                    Modulable<T>,
                    Bitwise<T>,
                    Comparable<T>,
                    AxioReprable<T> {};

/**
 * @brief Skills for a type wrapping a smart-pointer or raw pointer:
 *        Comparable, ImplicitBool, unary `*`, and `->`.
 *
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   using OwnedFoo = StrongType<std::unique_ptr<Foo>, struct OwnedFooTag,
 *                                                      PointerLike>;
 *   OwnedFoo f(std::make_unique<Foo>());
 *   if (f) { f->DoSomething(); (*f).DoSomething(); }
 * @endcode
 */
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

/**
 * @brief Skills for a bitmask/flag type: Bitwise, Comparable, AxioReprable.
 *
 * Suitable for strongly-typed enum-flag wrappers where arithmetic does not
 * make sense but bitwise combination does.
 *
 * @tparam T  The StrongType being extended.
 *
 * @code
 *   using Permissions = StrongType<unsigned, struct PermissionsTag, Flag>;
 *   Permissions p = Permissions(kRead) | Permissions(kWrite);
 * @endcode
 */
template <typename T>
struct Flag : Bitwise<T>, Comparable<T>, AxioReprable<T> {};
}  // namespace axio

#endif