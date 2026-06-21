/**
 * @file macros.hpp
 * @brief Common utility macros.
 */

#ifndef AXIO_BASE_MACROS_HPP_
#define AXIO_BASE_MACROS_HPP_

#include "compiler.hpp"
#include "types.hpp"

#include <cassert>

namespace axio {
namespace internal {
template <typename T, SizeT N>
auto ArraySizeImpl(const T (&arr)[N]) -> char (&)[N];
}
}  // namespace axio

/**
 * @brief Returns the number of elements in a C-style array.
 *
 * @param arr Array whose element count is computed.
 * @return Number of elements in @p arr, as a compile-time constant.
 *
 * @warning Only works with actual arrays, not pointers.
 *
 * Example:
 * ```cpp
 * int values[16];
 * auto count = AXIO_ARRAY_SIZE(values); // 16
 * ```
 */
#define AXIO_ARRAY_SIZE(arr) (sizeof(::axio::internal::ArraySizeImpl(arr)))

/**
 * @brief Implementation detail of AXIO_STRINGIFY.
 * @param x Token to stringify.
 */
#define AXIO_STRINGIFY_IMPL(x) #x

/**
 * @brief Converts a token into a string literal.
 *
 * @param x Token to convert.
 * @return String literal representation of @p x.
 *
 * Example:
 * ```cpp
 * AXIO_STRINGIFY(123) // "123"
 * ```
 */
#define AXIO_STRINGIFY(x) AXIO_STRINGIFY_IMPL(x)

/**
 * @brief Implementation detail of AXIO_CONCAT.
 * @param a First token.
 * @param b Second token.
 */
#define AXIO_CONCAT_IMPL(a, b) a##b

/**
 * @brief Concatenates two tokens.
 *
 * @param a First token.
 * @param b Second token.
 * @return Single token formed by joining @p a and @p b.
 *
 * Example:
 * ```cpp
 * AXIO_CONCAT(foo, bar) // foobar
 * ```
 */
#define AXIO_CONCAT(a, b) AXIO_CONCAT_IMPL(a, b)

/**
 * @brief Creates a bit mask with bit @p x set.
 *
 * @param x Zero-based bit index to set.
 * @return Unsigned integer mask with only bit @p x set.
 *
 * Example:
 * ```cpp
 * AXIO_BIT(3) // 0b1000
 * ```
 */
#define AXIO_BIT(x) (1ULL << (x))

/**
 * @brief Current source line.
 *
 * Expands to `__LINE__` at the point of use.
 */
#define AXIO_LINE __LINE__

/**
 * @brief Current source file.
 *
 * Expands to `__FILE__` at the point of use.
 */
#define AXIO_FILE __FILE__

#if defined(__has_cpp_attribute)
/**
 * @brief Checks whether a C++ attribute is supported.
 *
 * @param attr Attribute name to test (unquoted).
 * @return Nonzero if the compiler supports @p attr, otherwise 0.
 */
#define AXIO_HAS_CPP_ATTRIBUTE(attr) __has_cpp_attribute(attr)
#else
#define AXIO_HAS_CPP_ATTRIBUTE(attr) 0
#endif

/**
 * @brief Marks a function return value as required.
 *
 * Generates a compiler warning if the returned value is ignored.
 *
 * @note Expands to `[[nodiscard]]` when supported, otherwise falls back to
 * a compiler-specific attribute, or nothing on unsupported compilers.
 */
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

#ifdef __has_builtin
/**
 * @brief Checks whether a compiler builtin is available.
 *
 * @param name Builtin name to test (unquoted).
 * @return Nonzero if @p name is available, otherwise 0.
 */
#define AXIO_HAS_BUILTIN(name) __has_builtin(name)
#else
#define AXIO_HAS_BUILTIN(name) 0
#endif

/**
 * @brief Branch prediction hint for likely conditions.
 *
 * @param ... Boolean expression expected to evaluate to true.
 * @return The evaluated expression, annotated as likely true when the
 * compiler supports `__builtin_expect`.
 */
#if AXIO_HAS_BUILTIN(__builtin_expect) || defined(AXIO_COMPILER_GCC)
#define AXIO_LIKELY(...) (__builtin_expect(!!(__VA_ARGS__), true))
/**
 * @brief Branch prediction hint for unlikely conditions.
 *
 * @param ... Boolean expression expected to evaluate to false.
 * @return The evaluated expression, annotated as likely false when the
 * compiler supports `__builtin_expect`.
 */
#define AXIO_UNLIKELY(...) (__builtin_expect(!!(__VA_ARGS__), false))
#else
#define AXIO_LIKELY(...) (__VA_ARGS__)
#define AXIO_UNLIKELY(...) (__VA_ARGS__)
#endif

/**
 * @brief Runtime assertion.
 *
 * @param ... Boolean expression that must hold true.
 *
 * @note Disabled in release builds (when `NDEBUG` is defined), in which
 * case the expression is unevaluated for side effects but still checked
 * for validity.
 */
#ifdef NDEBUG
#define AXIO_ASSERT(...) ((void)(false && (__VA_ARGS__)))
#else
#define AXIO_ASSERT(...)                \
  (AXIO_LIKELY((__VA_ARGS__)) ? (void)0 \
                              : [] { assert(false && #__VA_ARGS__); }())
#endif

/**
 * @brief Returns the larger value between @p a and @p b.
 *
 * @param a First value.
 * @param b Second value.
 * @return @p a if greater than @p b, otherwise @p b.
 *
 * @note Arguments may be evaluated more than once.
 * Avoid passing expressions with side effects.
 *
 * Example:
 * ```cpp
 * AXIO_MAX(10, 20); // 20
 * ```
 */
#define AXIO_MAX(a, b) (a > b ? a : b)

/**
 * @brief Returns the smaller value between @p a and @p b.
 *
 * @param a First value.
 * @param b Second value.
 * @return @p a if smaller than @p b, otherwise @p b.
 *
 * @note Arguments may be evaluated more than once.
 * Avoid passing expressions with side effects.
 *
 * Example:
 * ```cpp
 * AXIO_MIN(10, 20); // 10
 * ```
 */
#define AXIO_MIN(a, b) (a < b ? a : b)

/**
 * @brief Marks a variable or expression as intentionally unused.
 *
 * @param x Variable or expression to suppress unused-warnings for.
 *
 * Example:
 * ```cpp
 * void Foo(int v) {
 *  AXIO_IGNORE(v);
 * }
 * ```
 */
#define AXIO_IGNORE(x) ((void)x)

/**
 * @brief Compiler-specific force inline directive.
 *
 * Expands to the strongest available "always inline" hint for the
 * detected compiler, falling back to plain `inline` otherwise.
 */
#if AXIO_COMPILER_MSVC
#define AXIO_INLINE __forceinline
#elif AXIO_COMPILER_GCC || AXIO_COMPILER_CLANG
#define AXIO_INLINE inline __attribute__((always_inline))
#else
#define AXIO_INLINE inline
#endif

/**
 * @brief C++ language standard version.
 *
 * Expands to `_MSVC_LANG` on MSVC and `__cplusplus` on other compilers.
 */
#if defined(_MSVC_LANG)
#define AXIO_CXX_STANDARD _MSVC_LANG
#else
#define AXIO_CXX_STANDARD __cplusplus
#endif

#endif