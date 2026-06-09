#ifndef AXIO_UTILITY_PRINT_HPP_
#define AXIO_UTILITY_PRINT_HPP_

#include <cstdio>
#include <system_error>

#include "../string/axio_repr.hpp"
#include "../string/buffer.hpp"

namespace axio {
namespace internal {
AXIO_INLINE void FWrite(std::FILE* stream,
                        const void* buffer,
                        const SizeT size) {
  if (std::fwrite(buffer, 1, size, stream) != size) {
    throw std::system_error(errno, std::generic_category(),
                            "Failed to write to file");
  }
}
}  // namespace internal

template <typename... Ts, EnableIf_T<(HasAxioRepr_V<Ts> && ...), int> = 0>
void Print(Ts&&... args) {
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  internal::FWrite(stdout, buffer.Data(), buffer.Size());
}

template <typename... Ts, EnableIf_T<(HasAxioRepr_V<Ts> && ...), int> = 0>
void Print(std::FILE* stream, Ts&&... args) {
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  internal::FWrite(stream, buffer.Data(), buffer.Size());
}

template <typename... Ts, EnableIf_T<(HasAxioRepr_V<Ts> && ...), int> = 0>
void PrintLn(Ts&&... args) {
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  buffer.Append(1, '\n');
  internal::FWrite(stdout, buffer.Data(), buffer.Size());
}

template <typename... Ts, EnableIf_T<(HasAxioRepr_V<Ts> && ...), int> = 0>
void PrintLn(std::FILE* stream, Ts&&... args) {
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  buffer.Append(1, '\n');
  internal::FWrite(stream, buffer.Data(), buffer.Size());
}
}  // namespace axio

#endif