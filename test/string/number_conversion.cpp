#include <simpletest/simpletest.hpp>

#include <axio/string/internal/number_conversion.hpp>

#include <string_view>

template <typename Integer>
bool Test(Integer value, std::string_view expected) {
  char buffer[32];
  char* end = axio::internal::WriteIntegerToBuffer<Integer>(buffer, value);
  *end = '\0';
  const auto size = static_cast<std::size_t>(end - buffer);
  return expected.size() == size && expected == buffer;
}

template <typename Integer>
bool Test(Integer value) {
  char buffer[32];
  char* end = axio::internal::WriteIntegerToBuffer<Integer>(buffer, value);
  *end = '\0';
  const auto size = static_cast<std::size_t>(end - buffer);

  const auto expected = std::to_string(value);
  return expected.size() == size && expected == buffer;
}

template <typename Integer>
bool GenerateTest() {
  for (int b : {2, 10}) {
    for (axio::UInt64 u = 1, p = u; u >= p;
         p = u, u *= static_cast<unsigned int>(b)) {
      if (!Test<Integer>(static_cast<Integer>(u))) {
        return false;
      }
      if (!Test<Integer>(static_cast<Integer>(u - 1))) {
        return false;
      }
      if (!Test<Integer>(static_cast<Integer>(u + 1))) {
        return false;
      }
      if (!Test<Integer>(static_cast<Integer>(u + 5))) {
        return false;
      }
    }
  }
  return true;
}

#define CHECK_NC(T, value, expected) CHECK_TRUE(Test<T>(value, expected))

TEST_CASE(NumberConversion, Int8) {
  CHECK_NC(axio::Int8, 1, "1");
  CHECK_NC(axio::Int8, 10, "10");
  CHECK_NC(axio::Int8, 100, "100");
  CHECK_NC(axio::Int8, 127, "127");
  CHECK_NC(axio::Int8, -1, "-1");
  CHECK_NC(axio::Int8, -10, "-10");
  CHECK_NC(axio::Int8, -100, "-100");
  CHECK_NC(axio::Int8, -128, "-128");

  CHECK_NC(axio::Int8, 0, "0");
  CHECK_NC(axio::Int8, 42, "42");
  CHECK_NC(axio::Int8, 115, "115");
  CHECK_NC(axio::Int8, -54, "-54");
  CHECK_NC(axio::Int8, -115, "-115");
  CHECK_TRUE(GenerateTest<axio::Int8>());
}

TEST_CASE(NumberConversion, Int16) {
  CHECK_NC(axio::Int16, 0, "0");
  CHECK_NC(axio::Int16, 1, "1");
  CHECK_NC(axio::Int16, 12, "12");
  CHECK_NC(axio::Int16, 123, "123");
  CHECK_NC(axio::Int16, 1234, "1234");
  CHECK_NC(axio::Int16, 12345, "12345");
  CHECK_NC(axio::Int16, 32767, "32767");
  CHECK_NC(axio::Int16, -1, "-1");
  CHECK_NC(axio::Int16, -12, "-12");
  CHECK_NC(axio::Int16, -123, "-123");
  CHECK_NC(axio::Int16, -1234, "-1234");
  CHECK_NC(axio::Int16, -12345, "-12345");
  CHECK_NC(axio::Int16, -32767, "-32767");
  CHECK_NC(axio::Int16, -32768, "-32768");

  CHECK_NC(axio::Int16, 10, "10");
  CHECK_NC(axio::Int16, 100, "100");
  CHECK_NC(axio::Int16, 1000, "1000");
  CHECK_NC(axio::Int16, 10000, "10000");
  CHECK_NC(axio::Int16, -10, "-10");
  CHECK_NC(axio::Int16, -100, "-100");
  CHECK_NC(axio::Int16, -1000, "-1000");
  CHECK_NC(axio::Int16, -10000, "-10000");
  CHECK_TRUE(GenerateTest<axio::Int16>());
}

TEST_CASE(NumberConversion, Int32) {
  CHECK_NC(axio::Int32, 0, "0");
  CHECK_NC(axio::Int32, 1, "1");
  CHECK_NC(axio::Int32, 12, "12");
  CHECK_NC(axio::Int32, 123, "123");
  CHECK_NC(axio::Int32, 1234, "1234");
  CHECK_NC(axio::Int32, 12345, "12345");
  CHECK_NC(axio::Int32, 123456, "123456");
  CHECK_NC(axio::Int32, 1234567, "1234567");
  CHECK_NC(axio::Int32, 12345678, "12345678");
  CHECK_NC(axio::Int32, 123456789, "123456789");
  CHECK_NC(axio::Int32, 1234567890, "1234567890");
  CHECK_NC(axio::Int32, 2147483647, "2147483647");
  CHECK_NC(axio::Int32, -1, "-1");
  CHECK_NC(axio::Int32, -12, "-12");
  CHECK_NC(axio::Int32, -123, "-123");
  CHECK_NC(axio::Int32, -1234, "-1234");
  CHECK_NC(axio::Int32, -12345, "-12345");
  CHECK_NC(axio::Int32, -123456, "-123456");
  CHECK_NC(axio::Int32, -1234567, "-1234567");
  CHECK_NC(axio::Int32, -12345678, "-12345678");
  CHECK_NC(axio::Int32, -123456789, "-123456789");
  CHECK_NC(axio::Int32, -1234567890, "-1234567890");
  CHECK_NC(axio::Int32, -2147483648, "-2147483648");

  CHECK_NC(axio::Int32, 1 << 31, "-2147483648");
  CHECK_NC(axio::Int32, 0x7fffffff, "2147483647");
  CHECK_NC(axio::Int32, -0x7fffffff - 1, "-2147483648");
  CHECK_TRUE(GenerateTest<axio::Int32>());
}

TEST_CASE(NumberConversion, Int64) {
  CHECK_NC(axio::Int64, 0, "0");
  CHECK_NC(axio::Int64, 1, "1");
  CHECK_NC(axio::Int64, 12, "12");
  CHECK_NC(axio::Int64, 123, "123");
  CHECK_NC(axio::Int64, 1234, "1234");
  CHECK_NC(axio::Int64, 12345, "12345");
  CHECK_NC(axio::Int64, 123456, "123456");
  CHECK_NC(axio::Int64, 1234567, "1234567");
  CHECK_NC(axio::Int64, 12345678, "12345678");
  CHECK_NC(axio::Int64, 123456789, "123456789");
  CHECK_NC(axio::Int64, 1234567890, "1234567890");
  CHECK_NC(axio::Int64, 12345678901, "12345678901");
  CHECK_NC(axio::Int64, 123456789012, "123456789012");
  CHECK_NC(axio::Int64, 1234567890123, "1234567890123");
  CHECK_NC(axio::Int64, 12345678901234, "12345678901234");
  CHECK_NC(axio::Int64, 123456789012345, "123456789012345");
  CHECK_NC(axio::Int64, 1234567890123456, "1234567890123456");
  CHECK_NC(axio::Int64, 12345678901234567, "12345678901234567");
  CHECK_NC(axio::Int64, 123456789012345678, "123456789012345678");
  CHECK_NC(axio::Int64, 1234567890123456789, "1234567890123456789");
  CHECK_NC(axio::Int64, 9223372036854775807, "9223372036854775807");
  CHECK_NC(axio::Int64, -1, "-1");
  CHECK_NC(axio::Int64, -12, "-12");
  CHECK_NC(axio::Int64, -123, "-123");
  CHECK_NC(axio::Int64, -1234, "-1234");
  CHECK_NC(axio::Int64, -12345, "-12345");
  CHECK_NC(axio::Int64, -123456, "-123456");
  CHECK_NC(axio::Int64, -1234567, "-1234567");
  CHECK_NC(axio::Int64, -12345678, "-12345678");
  CHECK_NC(axio::Int64, -123456789, "-123456789");
  CHECK_NC(axio::Int64, -1234567890, "-1234567890");
  CHECK_NC(axio::Int64, -12345678901, "-12345678901");
  CHECK_NC(axio::Int64, -123456789012, "-123456789012");
  CHECK_NC(axio::Int64, -1234567890123, "-1234567890123");
  CHECK_NC(axio::Int64, -12345678901234, "-12345678901234");
  CHECK_NC(axio::Int64, -123456789012345, "-123456789012345");
  CHECK_NC(axio::Int64, -1234567890123456, "-1234567890123456");
  CHECK_NC(axio::Int64, -12345678901234567, "-12345678901234567");
  CHECK_NC(axio::Int64, -123456789012345678, "-123456789012345678");
  CHECK_NC(axio::Int64, -1234567890123456789, "-1234567890123456789");
  CHECK_NC(axio::Int64, -9223372036854775807, "-9223372036854775807");

  CHECK_NC(axio::Int64, 5999999999999999999LL, "5999999999999999999");
  CHECK_NC(axio::Int64, -5999999999999999999LL, "-5999999999999999999");
  CHECK_NC(axio::Int64, 99909000009LL, "99909000009");
  CHECK_NC(axio::Int64, -99909000009LL, "-99909000009");
  CHECK_TRUE(GenerateTest<axio::Int64>());
}

TEST_CASE(NumberConversion, UInt8) {
  CHECK_NC(axio::UInt8, 0, "0");
  CHECK_NC(axio::UInt8, 1, "1");
  CHECK_NC(axio::UInt8, 255, "255");
  CHECK_NC(axio::UInt8, 100, "100");
  CHECK_NC(axio::UInt8, 42, "42");
  CHECK_TRUE(GenerateTest<axio::UInt8>());
}

TEST_CASE(NumberConversion, UInt16) {
  CHECK_NC(axio::UInt16, 0, "0");
  CHECK_NC(axio::UInt16, 1, "1");
  CHECK_NC(axio::UInt16, 65535, "65535");
  CHECK_NC(axio::UInt16, 5000, "5000");
  CHECK_NC(axio::UInt16, 100, "100");
  CHECK_TRUE(GenerateTest<axio::UInt16>());
}

TEST_CASE(NumberConversion, UInt32) {
  CHECK_NC(axio::UInt32, 0, "0");
  CHECK_NC(axio::UInt32, 1, "1");
  CHECK_NC(axio::UInt32, 4294967295u, "4294967295");
  CHECK_NC(axio::UInt32, 1234567890u, "1234567890");
  CHECK_TRUE(GenerateTest<axio::UInt32>());
}

TEST_CASE(NumberConversion, UInt64) {
  CHECK_NC(axio::UInt64, 0, "0");
  CHECK_NC(axio::UInt64, 1, "1");
  CHECK_NC(axio::UInt64, 18446744073709551615ull, "18446744073709551615");
  CHECK_NC(axio::UInt64, 1234567890123456789ull, "1234567890123456789");
  CHECK_NC(axio::UInt64, 514515435345134513ull, "514515435345134513");

  CHECK_NC(axio::UInt64, 1000000000900000000ULL, "1000000000900000000");
  CHECK_NC(axio::UInt64, 1000000000800000001ULL, "1000000000800000001");
  CHECK_NC(axio::UInt64, 1000000000700000002ULL, "1000000000700000002");
  CHECK_NC(axio::UInt64, 1000000000600000003ULL, "1000000000600000003");
  CHECK_NC(axio::UInt64, 1000000000500000004ULL, "1000000000500000004");
  CHECK_NC(axio::UInt64, 1000000000400000005ULL, "1000000000400000005");
  CHECK_NC(axio::UInt64, 1000000000300000006ULL, "1000000000300000006");
  CHECK_NC(axio::UInt64, 1000000000200000007ULL, "1000000000200000007");
  CHECK_NC(axio::UInt64, 1000000000100000008ULL, "1000000000100000008");
  CHECK_NC(axio::UInt64, 1000000000000000009ULL, "1000000000000000009");

  CHECK_NC(axio::UInt64, 17999999999999999999ULL, "17999999999999999999");
  CHECK_NC(axio::UInt64, 999010423400ULL, "999010423400");
  CHECK_NC(axio::UInt64, 999014001231ULL, "999014001231");
  CHECK_NC(axio::UInt64, 999010863409ULL, "999010863409");
  CHECK_NC(axio::UInt64, 99909000000ULL, "99909000000");
  CHECK_NC(axio::UInt64, 99909000001ULL, "99909000001");
  CHECK_NC(axio::UInt64, 99909000009ULL, "99909000009");
  CHECK_TRUE(GenerateTest<axio::UInt64>());
}

TEST_CASE(NumberConversion, SizeT) {
  CHECK_NC(axio::SizeT, 0, "0");
  CHECK_NC(axio::SizeT, 1, "1");
  CHECK_NC(axio::SizeT, 123456789, "123456789");
  CHECK_NC(axio::SizeT, static_cast<axio::SizeT>(-1),
           std::to_string(static_cast<axio::SizeT>(-1)).c_str());
  CHECK_NC(axio::SizeT, static_cast<axio::SizeT>(-2),
           std::to_string(static_cast<axio::SizeT>(-2)).c_str());
  CHECK_NC(axio::SizeT, static_cast<axio::SizeT>(-10),
           std::to_string(static_cast<axio::SizeT>(-10)).c_str());
  CHECK_TRUE(GenerateTest<axio::SizeT>());
}
