#ifndef AXIO_STRING_AXIO_REPR_HPP_
#define AXIO_STRING_AXIO_REPR_HPP_

#include "../utility/forward.hpp"

#include "internal/number_conversion.hpp"

#include <string_view>

namespace axio {
#define MAKE_AXIO_REPR_FOR_INTEGER(T)                               \
  template <typename Output>                                        \
  void AxioRepr(Output& out, T integer) {                           \
    char buffer[internal::kIntToStringBufferSize<T>];               \
    char* end = internal::WriteIntegerToBuffer<T>(buffer, integer); \
    out.Append(buffer, static_cast<SizeT>(end - buffer));           \
  }

MAKE_AXIO_REPR_FOR_INTEGER(int);
MAKE_AXIO_REPR_FOR_INTEGER(long);
MAKE_AXIO_REPR_FOR_INTEGER(long long);

MAKE_AXIO_REPR_FOR_INTEGER(unsigned);
MAKE_AXIO_REPR_FOR_INTEGER(unsigned long);
MAKE_AXIO_REPR_FOR_INTEGER(unsigned long long);

template <typename Output>
void AxioRepr(Output& out, Bool b) {
  b ? out.Append("true", 4) : out.Append("false", 5);
}

template <typename Output, SizeT N>
void AxioRepr(Output& out, const char (&s)[N]) {
  out.Append(s, N - 1);
}

template <typename Output>
void AxioRepr(Output& out, std::string_view sv) {
  out.Append(sv.data(), sv.size());
}

template <typename Output,
          typename T,
          axio::T<EnableIf<IsSame<T, const char*>::value, int>> = 0>
void AxioRepr(Output& out, T s) {
  out.Append(s, static_cast<SizeT>(std::strlen(s)));
}

template <typename Output,
          typename T,
          axio::T<EnableIf<IsSame<T, char*>::value, int>> = 0>
void AxioRepr(Output& out, T s) {
  out.Append(s, static_cast<SizeT>(std::strlen(s)));
}

template <typename Output>
void AxioRepr(Output& out, char c) {
  out.Append(1, c);
}

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

template <typename T>
using AxioReprOp = decltype(AxioRepr(std::declval<internal::DummyOutput&>(),
                                     std::declval<const T&>()));
template <typename T>
using HasAxioRepr = IsDetected<AxioReprOp, T>;

template <typename... Ts>
inline constexpr Bool kHasAxioReprPack = (HasAxioRepr<Ts>::value && ...);
}  // namespace axio

#endif