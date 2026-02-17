#include <simpletest/simpletest.hpp>

#include <axio/base/types.hpp>
#include <type_traits>

TEST_CASE(Base, Types) {
  IGNORE_RESULT();

  static_assert(std::is_same_v<axio::Byte, std::uint8_t>,
                "Byte should be uint8_t");

  static_assert(std::is_same_v<axio::Int8, std::int8_t>,
                "Int8 should be int8_t");
  static_assert(std::is_same_v<axio::Int16, std::int16_t>,
                "Int16 should be int16_t");
  static_assert(std::is_same_v<axio::Int32, std::int32_t>,
                "Int32 should be int32_t");
  static_assert(std::is_same_v<axio::Int64, std::int64_t>,
                "Int64 should be int64_t");

  static_assert(std::is_same_v<axio::UInt8, std::uint8_t>,
                "UInt8 should be uint8_t");
  static_assert(std::is_same_v<axio::UInt16, std::uint16_t>,
                "UInt16 should be uint16_t");
  static_assert(std::is_same_v<axio::UInt32, std::uint32_t>,
                "UInt32 should be uint32_t");
  static_assert(std::is_same_v<axio::UInt64, std::uint64_t>,
                "UInt64 should be uint64_t");

  static_assert(std::is_same_v<axio::Bool, bool>, "Bool should be bool");

  static_assert(std::is_same_v<axio::Char, char>, "Char should be char");
  static_assert(std::is_same_v<axio::WChar, wchar_t>,
                "WChar should be wchar_t");

#ifdef __cpp_char8_t
  static_assert(std::is_same_v<axio::Char8, char8_t>,
                "Char8 should be char8_t");
#endif

  static_assert(std::is_same_v<axio::Char16, char16_t>,
                "Char16 should be char16_t");
  static_assert(std::is_same_v<axio::Char32, char32_t>,
                "Char32 should be char32_t");

  static_assert(std::is_same_v<axio::Float, float>, "Float should be float");
  static_assert(std::is_same_v<axio::Double, double>,
                "Double should be double");

  static_assert(std::is_same_v<axio::SizeT, std::size_t>,
                "SizeT should be size_t");
  static_assert(std::is_same_v<axio::NullPtrT, std::nullptr_t>,
                "NullPtrT should be nullptr_t");
  static_assert(std::is_same_v<axio::PtrDiffT, std::ptrdiff_t>,
                "PtrDiffT should be ptrdiff_t");

  static_assert(sizeof(axio::Byte) == 1, "Byte must be 1 byte");
  static_assert(sizeof(axio::Bool) == sizeof(bool), "Bool size mismatch");
  static_assert(sizeof(axio::Float) == sizeof(float), "Float size mismatch");
  static_assert(sizeof(axio::Double) == sizeof(double), "Double size mismatch");
}