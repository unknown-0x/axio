/**
 * @file print.hpp
 * @brief Formatted printing of values implementing the AxioRepr protocol.
 */

#ifndef AXIO_UTILITY_PRINT_HPP_
#define AXIO_UTILITY_PRINT_HPP_

#include <cstdio>
#include <system_error>

#include "../string/axio_repr.hpp"
#include "../string/buffer.hpp"

namespace axio {
namespace internal {
/**
 * @brief Writes a raw buffer to a stream, throwing on short writes.
 *
 * @param stream Output stream to write to.
 * @param buffer Pointer to the bytes to write.
 * @param size Number of bytes to write.
 *
 * @throws std::system_error if fewer than @p size bytes were written.
 */
AXIO_INLINE void FWrite(std::FILE* stream,
                        const void* buffer,
                        const SizeT size) {
  if (std::fwrite(buffer, 1, size, stream) != size) {
    throw std::system_error(errno, std::generic_category(),
                            "Failed to write to file");
  }
}
}  // namespace internal

/**
 * @brief Prints arguments to stdout without a trailing newline.
 *
 * @tparam Ts Types satisfying HasAxioRepr.
 * @param args Values to print.
 *
 * @throws std::system_error if the write to stdout fails.
 */
template <typename... Ts, EnableIf_T<(HasAxioRepr_V<Ts> && ...), int> = 0>
void Print(Ts&&... args) {
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  internal::FWrite(stdout, buffer.Data(), buffer.Size());
}

/**
 * @brief Prints arguments to a given stream without a trailing newline.
 *
 * @tparam Ts Types satisfying HasAxioRepr.
 * @param stream Output stream.
 * @param args Values to print.
 *
 * @throws std::system_error if the write to @p stream fails.
 */
template <typename... Ts, EnableIf_T<(HasAxioRepr_V<Ts> && ...), int> = 0>
void Print(std::FILE* stream, Ts&&... args) {
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  internal::FWrite(stream, buffer.Data(), buffer.Size());
}

/**
 * @brief Prints arguments to stdout, followed by a newline.
 *
 * @tparam Ts Types satisfying HasAxioRepr.
 * @param args Values to print.
 *
 * @throws std::system_error if the write to stdout fails.
 */
template <typename... Ts, EnableIf_T<(HasAxioRepr_V<Ts> && ...), int> = 0>
void PrintLn(Ts&&... args) {
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  buffer.Append(1, '\n');
  internal::FWrite(stdout, buffer.Data(), buffer.Size());
}

/**
 * @brief Prints arguments to a given stream, followed by a newline.
 *
 * @tparam Ts Types satisfying HasAxioRepr.
 * @param stream Output stream.
 * @param args Values to print.
 *
 * @throws std::system_error if the write to @p stream fails.
 */
template <typename... Ts, EnableIf_T<(HasAxioRepr_V<Ts> && ...), int> = 0>
void PrintLn(std::FILE* stream, Ts&&... args) {
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  buffer.Append(1, '\n');
  internal::FWrite(stream, buffer.Data(), buffer.Size());
}
}  // namespace axio

#endif