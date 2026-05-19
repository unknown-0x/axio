#ifndef AXIO_STRING_STRING_UTILS_HPP_
#define AXIO_STRING_STRING_UTILS_HPP_

#include "internal/buffer.hpp"

#include "axio_repr.hpp"
#include "string.hpp"

namespace axio {
String StringCat() {
  return String();
}

template <typename... Ts>
String StringCat(Ts&&... args) {
  static_assert(kHasAxioReprPack<Ts...>,
                "All arguments to StringCat must support AxioRepr");

  internal::BasicBuffer<char> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  return String(buffer.Data(), buffer.Size());
}

void StringAppend(String&) {}

template <typename... Ts>
void StringAppend(String& s, Ts&&... args) {
  static_assert(kHasAxioReprPack<Ts...>,
                "All arguments to StringAppend must support AxioRepr");
  internal::BasicBuffer<char> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  s.Append(buffer.Data(), buffer.Size());
}
}  // namespace axio

#endif