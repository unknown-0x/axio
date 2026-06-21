/**
 * @file compiler.hpp
 * @brief Compile-time compiler detection.
 *
 * @par Defined macros
 * - AXIO_COMPILER_CLANG
 * - AXIO_COMPILER_APPLE_CLANG
 * - AXIO_COMPILER_GCC
 * - AXIO_COMPILER_MSVC
 * - AXIO_COMPILER_NAME (Human-readable name of the detected compiler)
 * - AXIO_COMPILER_VERSION
 *
 * @note Clang-cl defines both `__clang__` and `_MSC_VER`.
 *
 * @warning Compilation fails on unsupported compilers.
 */

#ifndef AXIO_BASE_COMPILER_HPP_
#define AXIO_BASE_COMPILER_HPP_

#if defined(__clang__)
/// @def AXIO_COMPILER_CLANG
/// @brief Defined (as 1) when compiling with any Clang-based compiler.
#define AXIO_COMPILER_CLANG 1
#if defined(__apple_build_version__)
/// @def AXIO_COMPILER_APPLE_CLANG
/// @brief Defined (as 1) when compiling with Apple's Clang distribution.
#define AXIO_COMPILER_APPLE_CLANG 1
#define AXIO_COMPILER_NAME "apple-clang"
#elif defined(_MSC_VER)
/// @def AXIO_COMPILER_MSVC
/// @brief Defined (as 1) when compiling with MSVC or an MSVC-compatible
/// front end (e.g. clang-cl).
#define AXIO_COMPILER_MSVC 1
#define AXIO_COMPILER_NAME "clang-cl"
#else
#define AXIO_COMPILER_NAME "clang"
#endif
#ifndef __clang_patchlevel__
#define __clang_patchlevel__ 0
#endif
#define AXIO_COMPILER_VERSION \
  (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)

#elif defined(_MSC_VER)
#define AXIO_COMPILER_MSVC 1
#define AXIO_COMPILER_NAME "msvc"
#define AXIO_COMPILER_VERSION _MSC_VER

#elif defined(__GNUC__)
/// @def AXIO_COMPILER_GCC
/// @brief Defined (as 1) when compiling with GCC.
#define AXIO_COMPILER_GCC 1
#define AXIO_COMPILER_NAME "gcc"
#if defined(__GNUC_PATCHLEVEL__)
#define AXIO_COMPILER_VERSION \
  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#define AXIO_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100)
#endif

#else
#error "Unsupported compiler"
#endif

#endif