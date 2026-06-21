/**
 * @file endianness.hpp
 * @brief Compile-time endianness detection.
 *
 * @par Defined macros
 * - AXIO_LITTLE_ENDIAN
 * - AXIO_BIG_ENDIAN
 * - AXIO_ENDIAN
 *
 * @par Types
 * - axio::Endian
 *
 * @note
 * When C++20 `<bit>` and `std::endian` are available, axio::Endian is
 * mapped directly to the standard library implementation.
 *
 * @warning
 * If endianness cannot be determined, AXIO_ENDIAN falls back to
 * AXIO_LITTLE_ENDIAN and emits a compiler warning.
 */

#ifndef AXIO_BASE_ENDIANNESS_HPP_
#define AXIO_BASE_ENDIANNESS_HPP_

#include "macros.hpp"

#if AXIO_CXX_STANDARD >= 202002L
#include <bit>
#endif

/// @def AXIO_LITTLE_ENDIAN
/// @brief Identifier value representing little-endian byte order.
#define AXIO_LITTLE_ENDIAN 1234
/// @def AXIO_BIG_ENDIAN
/// @brief Identifier value representing big-endian byte order.
#define AXIO_BIG_ENDIAN 4321

#if (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
     __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
/// @def AXIO_ENDIAN
/// @brief Native byte order of the target platform, either
/// AXIO_LITTLE_ENDIAN or AXIO_BIG_ENDIAN.
#define AXIO_ENDIAN AXIO_LITTLE_ENDIAN
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define AXIO_ENDIAN AXIO_BIG_ENDIAN
#elif defined(_WIN32)
#define AXIO_ENDIAN AXIO_LITTLE_ENDIAN
#else
#warning "Could not detect endianness! Falling back to little endian"
#define AXIO_ENDIAN AXIO_LITTLE_ENDIAN
#endif

namespace axio {
/**
 * @brief Native byte-order representation.
 *
 * - `kLittle` : Little-endian
 * - `kBig`    : Big-endian
 * - `kNative` : Platform-native byte order
 */
#if defined(__cpp_lib_endian) && __cpp_lib_endian >= 201907L
enum class Endian {
  kLittle = static_cast<int>(std::endian::little),
  kBig = static_cast<int>(std::endian::big),
  kNative = static_cast<int>(std::endian::native),
};
#else
enum class Endian {
  kLittle = AXIO_LITTLE_ENDIAN,
  kBig = AXIO_BIG_ENDIAN,
  kNative = AXIO_ENDIAN,
};
#endif
}  // namespace axio

#endif