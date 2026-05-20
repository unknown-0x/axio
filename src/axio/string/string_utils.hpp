#ifndef AXIO_STRING_STRING_UTILS_HPP_
#define AXIO_STRING_STRING_UTILS_HPP_

#include "axio_repr.hpp"
#include "buffer.hpp"
#include "string.hpp"

#include "../container/tuple.hpp"

namespace axio {
String StringCat() {
  return String();
}

template <typename... Ts>
String StringCat(Ts&&... args) {
  static_assert(kHasAxioReprPack<Ts...>,
                "All arguments to StringCat must support AxioRepr");

  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  return String(buffer.Data(), buffer.Size());
}

void StringAppend(String&) {}

template <typename... Ts>
void StringAppend(String& s, Ts&&... args) {
  static_assert(kHasAxioReprPack<Ts...>,
                "All arguments to StringAppend must support AxioRepr");
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  s.Append(buffer.Data(), buffer.Size());
}

namespace detail {
template <typename InputIt>
void StringJoin(String& output,
                InputIt first,
                InputIt last,
                std::string_view separator) {
  if (first == last) {
    return;
  }

  const char* const sep = separator.data();
  const auto size = separator.size();

  Buffer<> buffer{};
  AxioRepr(buffer, *first++);
  while (first != last) {
    buffer.Append(sep, size);
    AxioRepr(buffer, *first++);
  }
  output.Append(buffer.Data(), buffer.Size());
}

template <typename InputIt, typename Formatter>
void StringJoin(String& output,
                InputIt first,
                InputIt last,
                std::string_view separator,
                Formatter&& formatter) {
  if (first == last) {
    return;
  }

  const char* const sep = separator.data();
  const auto size = separator.size();

  Buffer<> buffer{};
  formatter(buffer, *first++);
  while (first != last) {
    buffer.Append(sep, size);
    formatter(buffer, *first++);
  }
  output.Append(buffer.Data(), buffer.Size());
}

template <typename... Ts, SizeT... Is>
void JoinRestOfTuple(Buffer<>& buffer,
                     const Tuple<Ts...>& tuple,
                     std::string_view separator,
                     std::index_sequence<Is...>) {
  const char* const sep = separator.data();
  const auto size = separator.size();

  AXIO_IGNORE(sep);
  AXIO_IGNORE(size);

  ((buffer.Append(sep, size), AxioRepr(buffer, Get<Is + 1>(tuple))), ...);
}
}  // namespace detail

template <typename Container>
String StringJoin(const Container& container, std::string_view separator) {
  String result;
  detail::StringJoin(result, std::begin(container), std::end(container),
                     separator);
  return result;
}

template <typename Container, typename Formatter>
String StringJoin(const Container& container,
                  std::string_view separator,
                  Formatter&& formatter) {
  String result;
  detail::StringJoin(result, std::begin(container), std::end(container),
                     separator, Forward<Formatter>(formatter));
  return result;
}

String StringJoin(Tuple<>, std::string_view) {
  return String();
}

template <typename... Ts>
String StringJoin(const Tuple<Ts...>& tuple, std::string_view separator) {
  Buffer<> buffer{};
  AxioRepr(buffer, Get<0>(tuple));
  detail::JoinRestOfTuple(buffer, tuple, separator,
                          std::make_index_sequence<sizeof...(Ts) - 1>{});
  return String(buffer.Data(), buffer.Size());
}

String StringJoinValues(std::string_view) {
  return String();
}

template <typename... Values>
String StringJoinValues(std::string_view separator, Values&&... values) {
  const char* const sep = separator.data();
  const auto size = separator.size();

  Buffer<> buffer{};
  SizeT index = 0;
  (((index++ > 0 ? (void)(buffer.Append(sep, size)) : (void)0),
    AxioRepr(buffer, Forward<Values>(values))),
   ...);

  return String(buffer.Data(), buffer.Size());
}
}  // namespace axio

#endif