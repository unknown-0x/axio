#ifndef AXIO_BASE_ENDIANNESS_HPP_
#define AXIO_BASE_ENDIANNESS_HPP_

#include <bit>

namespace axio {
#define AXIO_LITTLE_ENDIAN 1234
#define AXIO_BIG_ENDIAN 4321

#if (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
     __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define AXIO_ENDIAN AXIO_LITTLE_ENDIAN
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define AXIO_ENDIAN AXIO_BIG_ENDIAN
#elif defined(_WIN32)
#define AXIO_ENDIAN AXIO_LITTLE_ENDIAN
#else
#error "Could not detect endianness!"
#endif

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