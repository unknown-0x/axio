#ifndef AXIO_BASE_TYPES_HPP_
#define AXIO_BASE_TYPES_HPP_

#include <cstddef>
#include <cstdint>

namespace axio {
using Byte = std::uint8_t;

using Int8 = std::int8_t;
using Int16 = std::int16_t;
using Int32 = std::int32_t;
using Int64 = std::int64_t;

using UInt8 = std::uint8_t;
using UInt16 = std::uint16_t;
using UInt32 = std::uint32_t;
using UInt64 = std::uint64_t;

using Bool = bool;

using Char = char;
using WChar = wchar_t;
#ifdef __cpp_char8_t
using Char8 = char8_t;
#endif
using Char16 = char16_t;
using Char32 = char32_t;

using Float = float;
using Double = double;

using SizeT = std::size_t;
using NullPtrT = std::nullptr_t;
using PtrDiffT = std::ptrdiff_t;
}  // namespace axio

#endif