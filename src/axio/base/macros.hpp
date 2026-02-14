#ifndef AXIO_BASE_MACROS_HPP_
#define AXIO_BASE_MACROS_HPP_

#include "compiler.hpp"
#include "types.hpp"

namespace axio {
namespace internal {
template <typename T, SizeT N>
auto ArraySizeImpl(const T (&arr)[N]) -> char (&)[N];
}
}  // namespace axio

#define AXIO_ARRAY_SIZE(arr) (sizeof(::axio::internal::ArraySizeImpl(arr)))

#define AXIO_STRINGIFY_IMPL(x) #x
#define AXIO_STRINGIFY(x) AXIO_STRINGIFY_IMPL(x)

#define AXIO_CONCAT_IMPL(a, b) a##b
#define AXIO_CONCAT(a, b) AXIO_CONCAT_IMPL(a, b)

#define AXIO_BIT(x) (1ULL << (x))

#define AXIO_LINE __LINE__
#define AXIO_FILE __FILE__

#if defined(__has_cpp_attribute)
#define AXIO_HAS_CPP_ATTRIBUTE(attr) __has_cpp_attribute(attr)
#else
#define AXIO_HAS_CPP_ATTRIBUTE(attr) 0
#endif

#if AXIO_HAS_CPP_ATTRIBUTE(nodiscard)
#define AXIO_NODISCARD [[nodiscard]]
#else
#if AXIO_COMPILER_GCC || AXIO_COMPILER_CLANG
#define AXIO_NODISCARD __attribute__((warn_unused_result))
#elif AXIO_COMPILER_MSVC
#define AXIO_NODISCARD _Check_return_
#else
#define AXIO_NODISCARD
#endif
#endif

#endif