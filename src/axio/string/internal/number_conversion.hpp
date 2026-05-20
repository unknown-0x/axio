#ifndef AXIO_STRING_INTERNAL_NUMBER_CONVERSION_HPP_
#define AXIO_STRING_INTERNAL_NUMBER_CONVERSION_HPP_

#include "../../base/macros.hpp"
#include "../../base/type_traits.hpp"
#include "../../base/types.hpp"

#include <cstring>

// Reference:
// https://github.com/jeaiii/itoa/blob/main/include/itoa/jeaiii_to_text.h

namespace axio {
namespace internal {
template <typename T>
static constexpr SizeT kIntToStringBufferSize =
    (sizeof(T) <= sizeof(UInt16))   ? 8
    : (sizeof(T) <= sizeof(UInt32)) ? 16
                                    : 24;

struct CharPair {
  char v[2];
  constexpr CharPair(char c) : v{c, '\0'} {}
  constexpr CharPair(int n) : v{"0123456789"[n / 10], "0123456789"[n % 10]} {}
};

constexpr struct {
  CharPair dd[100]{
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
      17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
      34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
      51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
      68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
      85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
  };
  CharPair fd[100]{
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 10, 11, 12, 13, 14,
      15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 26, 27, 28, 29,
      30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40, 41, 42, 43, 44,
      45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55, 56, 57, 58, 59,
      60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70, 71, 72, 73, 74,
      75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85, 86, 87, 88, 89,
      90,  91,  92,  93,  94,  95,  96,  97,  98,  99,
  };
} kDigits;

constexpr UInt64 kMask24 = (UInt64(1) << 24) - 1;
constexpr UInt64 kMask32 = (UInt64(1) << 32) - 1;
constexpr UInt64 kMask57 = (UInt64(1) << 57) - 1;

template <typename I, I a, I b>
inline constexpr I kScaledConstant = I(10 * (I(1) << a) / b + 1);

#define WRITE_2(dst, src) std::memcpy(dst, &src, 2)

#define WRITE_2_MASKED(v, mask, dst, src)   \
  v = (v & AXIO_CONCAT(kMask, mask)) * 100; \
  WRITE_2(dst, src[v >> mask])

#define WRITE_2_MASKED_3(v, mask, dst, src) \
  WRITE_2_MASKED(v, mask, dst, src);        \
  WRITE_2_MASKED(v, mask, dst + 2, src);    \
  WRITE_2_MASKED(v, mask, dst + 4, src)

#define WRITE_2_MASKED_4(v, mask, dst, src) \
  WRITE_2_MASKED_3(v, mask, dst, src);      \
  WRITE_2_MASKED(v, mask, dst + 6, src)

template <typename Integer>
char* WriteIntegerToBuffer(char* out, Integer value) {
  using UInt = axio::T<MakeUnsigned<Integer>>;
  // convert bool to int before test with unary + to silence warning if T
  // happens to be bool
  const auto n =
      static_cast<UInt>(value < 0 ? (*out++ = '-', 0 - value) : value);

  if (n < 100) {
    WRITE_2(out, kDigits.fd[n]);
    return n < 10 ? out + 1 : out + 2;
  }
  if (n < 1'000'000) {
    if (n < 10'000) {
      auto v = kScaledConstant<UInt32, 24, 1000> * n;
      WRITE_2(out, kDigits.fd[v >> 24]);
      out += 2 - (n < 1000);
      WRITE_2_MASKED(v, 24, out, kDigits.dd);
      return out + 2;
    }
    auto v = kScaledConstant<UInt64, 32ull, 100000ull> * n;
    WRITE_2(out, kDigits.fd[v >> 32]);
    out += 2 - (n < 100000);
    WRITE_2_MASKED(v, 32, out, kDigits.dd);
    WRITE_2_MASKED(v, 32, out + 2, kDigits.dd);
    return out + 4;
  }

  static constexpr UInt64 kMul_48_1e6 = (1ull << 48ull) / 1'000'000 + 1;
  static constexpr UInt64 kPow2_32 = UInt64(1) << UInt64(32);
  if (n < kPow2_32) {
    if (n < 100'000'000) {
      auto v = kScaledConstant<UInt64, 48ull, 10'000'000> * n >> 16;
      WRITE_2(out, kDigits.fd[v >> 32]);
      out += 2 - (n < 10'000'000);
      WRITE_2_MASKED_3(v, 32, out, kDigits.dd);
      return out + 6;
    }
    auto v = kScaledConstant<UInt64, 57ull, 1'000'000'000> * n;
    WRITE_2(out, kDigits.fd[v >> 57]);
    out += 2 - (n < 1'000'000'000);
    WRITE_2_MASKED_4(v, 57, out, kDigits.dd);
    return out + 8;
  }

  auto z = static_cast<UInt32>(n % 100'000'000);
  auto u = static_cast<UInt64>(n / 100'000'000);

  if (u < 100) {
    WRITE_2(out, kDigits.dd[u]);
    out += 2;
  } else if (u < 1000000) {
    if (u < 10000) {
      auto v = kScaledConstant<UInt32, 24, 1000> * u;
      WRITE_2(out, kDigits.fd[v >> 24]);
      out += 2 - (u < 1000);
      WRITE_2_MASKED(v, 24, out, kDigits.dd);
      out += 2;
    } else {
      auto v = kScaledConstant<UInt64, 32ull, 100'000> * u;
      WRITE_2(out, kDigits.fd[v >> 32]);
      out += 2 - (u < 100'000);
      WRITE_2_MASKED(v, 32, out, kDigits.dd);
      WRITE_2_MASKED(v, 32, out + 2, kDigits.dd);
      out += 4;
    }
  } else if (u < 100'000'000) {
    auto v = kScaledConstant<UInt64, 48ull, 10'000'000> * u >> 16;
    WRITE_2(out, kDigits.fd[v >> 32]);
    out += 2 - (u < 10'000'000);
    WRITE_2_MASKED_3(v, 32, out, kDigits.dd);
    out += 6;
  } else if (u < kPow2_32) {
    auto v = kScaledConstant<UInt64, 57ull, 1'000'000'000> * u;
    WRITE_2(out, kDigits.fd[v >> 57]);
    out += 2 - (u < 1'000'000'000);
    WRITE_2_MASKED_4(v, 57, out, kDigits.dd);
    out += 8;
  } else {
    UInt32 y = static_cast<UInt32>(u % 100'000'000);
    u /= 100'000'000;

    // u is 2, 3, or 4 digits (if u < 10 it would have been handled above)
    if (u < 100) {
      WRITE_2(out, kDigits.dd[u]);
      out += 2;
    } else {
      auto v = kScaledConstant<UInt32, 24, 1'000> * u;
      WRITE_2(out, kDigits.fd[v >> 24]);
      out += 2 - (u < 1'000);
      WRITE_2_MASKED(v, 24, out, kDigits.dd);
      out += 2;
    }

    // do 8 digits
    auto v = (kMul_48_1e6 * y >> 16) + 1;
    WRITE_2(out, kDigits.dd[v >> 32]);
    WRITE_2_MASKED_3(v, 32, out + 2, kDigits.dd);
    out += 8;
  }
  // do 8 digits
  auto v = (kMul_48_1e6 * z >> 16) + 1;
  WRITE_2(out, kDigits.dd[v >> 32]);
  WRITE_2_MASKED_3(v, 32, out + 2, kDigits.dd);
  return out + 8;
}

#undef WRITE_2
}  // namespace internal
}  // namespace axio

#endif