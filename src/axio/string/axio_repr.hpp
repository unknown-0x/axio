/**
 * @file axio_repr.hpp
 * @brief Defines the `AxioRepr` extension point and built-in overloads
 *        used to convert values into their textual representation.
 *
 * `AxioRepr(out, value)` writes the textual form of @p value into @p out
 * via `out.Append(const char*, SizeT)`. It is the single customization
 * point used by axio::StringCat(), axio::StringAppend() and the various
 * `StringJoin` helpers, so any type that provides a matching overload
 * (found via ADL or declared in `axio`) becomes serializable by those
 * utilities. Support for a type can be queried at compile time with
 * HasAxioRepr.
 */

#ifndef AXIO_STRING_AXIO_REPR_HPP_
#define AXIO_STRING_AXIO_REPR_HPP_

#include "../utility/forward.hpp"

#include "internal/number_conversion.hpp"
#include "internal/zmij.hpp"

#include <string_view>

namespace axio {
/**
 * @def MAKE_AXIO_REPR_FOR_INTEGER(T)
 * @brief Generates an `AxioRepr` overload that formats a built-in integer
 *        type @p T as decimal text.
 *
 * The generated function stack-allocates a buffer sized for the worst
 * case representation of @p T (sign + digits), converts @p value into it
 * via internal::WriteIntegerToBuffer, and appends the resulting span to
 * @p out. Instantiated below for `int`, `long`, `long long` and their
 * unsigned counterparts.
 *
 * @tparam Output Any type exposing `Append(const char*, SizeT)`.
 * @param out   Destination that receives the formatted digits.
 * @param value Integer value to format.
 */
#define MAKE_AXIO_REPR_FOR_INTEGER(T)                             \
  template <typename Output>                                      \
  void AxioRepr(Output& out, T value) {                           \
    char buffer[internal::kIntToStringBufferSize<T>];             \
    char* end = internal::WriteIntegerToBuffer<T>(buffer, value); \
    out.Append(buffer, static_cast<SizeT>(end - buffer));         \
  }

MAKE_AXIO_REPR_FOR_INTEGER(int);
MAKE_AXIO_REPR_FOR_INTEGER(long);
MAKE_AXIO_REPR_FOR_INTEGER(long long);

MAKE_AXIO_REPR_FOR_INTEGER(unsigned);
MAKE_AXIO_REPR_FOR_INTEGER(unsigned long);
MAKE_AXIO_REPR_FOR_INTEGER(unsigned long long);

#undef MAKE_AXIO_REPR_FOR_INTEGER

/**
 * @def MAKE_AXIO_REPR_FOR_FLOAT(T, buffer_size)
 * @brief Generates an `AxioRepr` overload that formats a built-in
 *        floating-point type @p T using the `zmij` float-to-string
 *        algorithm.
 *
 * Allocates a stack buffer of @p buffer_size + 1 bytes (the extra byte
 * guards against off-by-one requirements of `zmij::write`), formats
 * @p value into it, and appends the written span to @p out.
 *
 * @tparam Output Any type exposing `Append(const char*, SizeT)`.
 * @param out   Destination that receives the formatted text.
 * @param value Floating-point value to format.
 */
#define MAKE_AXIO_REPR_FOR_FLOAT(T, buffer_size)           \
  template <typename Output>                               \
  void AxioRepr(Output& out, T value) {                    \
    char buffer[buffer_size + 1];                          \
    auto end = zmij::write(buffer, sizeof(buffer), value); \
    out.Append(buffer, static_cast<SizeT>(end - buffer));  \
  }

MAKE_AXIO_REPR_FOR_FLOAT(float, zmij::float_buffer_size);
MAKE_AXIO_REPR_FOR_FLOAT(double, zmij::double_buffer_size);

#undef MAKE_AXIO_REPR_FOR_FLOAT

/**
 * @brief Formats a boolean as the literal text `"true"` or `"false"`.
 * @tparam Output Any type exposing `Append(const char*, SizeT)`.
 * @param out Destination that receives the formatted text.
 * @param b   Value to format.
 */
template <typename Output>
void AxioRepr(Output& out, Bool b) {
  b ? out.Append("true", 4) : out.Append("false", 5);
}

/**
 * @brief Appends a string literal (or any `const char[N]`) verbatim,
 *        excluding the trailing null terminator.
 * @tparam Output Any type exposing `Append(const char*, SizeT)`.
 * @tparam N      Array length, including the implicit null terminator.
 * @param out Destination that receives the text.
 * @param s   String literal to append.
 */
template <typename Output, SizeT N>
void AxioRepr(Output& out, const char (&s)[N]) {
  out.Append(s, N - 1);
}

/**
 * @brief Appends the contents of a `std::string_view` verbatim.
 * @tparam Output Any type exposing `Append(const char*, SizeT)`.
 * @param out Destination that receives the text.
 * @param sv  View whose contents are appended.
 */
template <typename Output>
void AxioRepr(Output& out, std::string_view sv) {
  out.Append(sv.data(), sv.size());
}

/**
 * @brief Appends a null-terminated `const char*` string.
 *
 * The length is computed with `std::strlen`, so @p s must be
 * null-terminated.
 *
 * @tparam Output Any type exposing `Append(const char*, SizeT)`.
 * @tparam T      Deduced as `const char*`; constrained via SFINAE.
 * @param out Destination that receives the text.
 * @param s   Null-terminated string to append.
 */
template <typename Output,
          typename T,
          EnableIf_T<IsSame_V<T, const char*>, int> = 0>
void AxioRepr(Output& out, T s) {
  out.Append(s, static_cast<SizeT>(std::strlen(s)));
}

/**
 * @brief Appends a null-terminated `char*` string.
 *
 * Mirrors the `const char*` overload for non-const pointers so that
 * mutable C strings bind without an extra qualification conversion.
 *
 * @tparam Output Any type exposing `Append(const char*, SizeT)`.
 * @tparam T      Deduced as `char*`; constrained via SFINAE.
 * @param out Destination that receives the text.
 * @param s   Null-terminated string to append.
 */
template <typename Output, typename T, EnableIf_T<IsSame_V<T, char*>, int> = 0>
void AxioRepr(Output& out, T s) {
  out.Append(s, static_cast<SizeT>(std::strlen(s)));
}

/**
 * @brief Appends a single character.
 * @tparam Output Any type exposing `Append(SizeT, char)`.
 * @param out Destination that receives the character.
 * @param c   Character to append.
 */
template <typename Output>
void AxioRepr(Output& out, char c) {
  out.Append(1, c);
}

/**
 * @brief Formats and appends an arbitrary list of values to @p output,
 *        in order, by invoking `AxioRepr` on each one.
 *
 * @tparam Output Any type accepted by the `AxioRepr` overloads used.
 * @tparam Ts     Types of the values to format; each must satisfy
 *                HasAxioRepr.
 * @param output Destination that accumulates the formatted text.
 * @param args   Values to format and append, in order.
 */
template <typename Output, typename... Ts>
void AppendToOutput(Output& output, Ts&&... args) {
  (AxioRepr(output, axio::Forward<Ts>(args)), ...);
}

namespace internal {
class DummyOutput {
 public:
  DummyOutput();
  void Append(const char*, SizeT);
};
}  // namespace internal

/**
 * @brief Detection-idiom expression: well-formed if and only if an
 *        `AxioRepr(internal::DummyOutput&, const T&)` overload is
 *        resolvable for @p T.
 * @tparam T Candidate type.
 */
template <typename T>
using AxioReprOp = decltype(AxioRepr(std::declval<internal::DummyOutput&>(),
                                     std::declval<const T&>()));

/**
 * @brief Compile-time trait reporting whether @p T can be formatted via
 *        `AxioRepr`.
 *
 * Inherits from `std::true_type` / `std::false_type` (via IsDetected)
 * depending on whether AxioReprOp<T> is well-formed.
 *
 * @tparam T Type to test.
 */
template <typename T>
struct HasAxioRepr : IsDetected<AxioReprOp, T> {};
template <typename T>
inline constexpr auto HasAxioRepr_V = HasAxioRepr<T>::value;
}  // namespace axio

#endif