/**
 * @file arch.hpp
 * @brief Compile-time CPU architecture detection.
 *
 * @par Defined macros
 * - AXIO_ARCH_X64
 * - AXIO_ARCH_X86
 * - AXIO_ARCH_ARM64
 * - AXIO_ARCH_ARM
 * - AXIO_ARCH_NAME (Human-readable name of the detected architecture)
 *
 * @warning Compilation fails on unsupported architectures.
 */

#ifndef AXIO_BASE_ARCH_HPP_
#define AXIO_BASE_ARCH_HPP_

#if defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || \
    defined(_M_AMD64)
/// @def AXIO_ARCH_X64
/// @brief Defined (as 1) when targeting x86-64 (AMD64).
#define AXIO_ARCH_X64 1
#define AXIO_ARCH_NAME "x64"

#elif defined(__i386) || defined(_M_IX86)
/// @def AXIO_ARCH_X86
/// @brief Defined (as 1) when targeting 32-bit x86.
#define AXIO_ARCH_X86 1
#define AXIO_ARCH_NAME "x86"

#elif defined(__aarch64__) || defined(_M_ARM64)
/// @def AXIO_ARCH_ARM64
/// @brief Defined (as 1) when targeting ARM64 (AArch64).
#define AXIO_ARCH_ARM64 1
#define AXIO_ARCH_NAME "arm64"

#elif defined(__arm__) || defined(_M_ARM)
/// @def AXIO_ARCH_ARM
/// @brief Defined (as 1) when targeting 32-bit ARM.
#define AXIO_ARCH_ARM 1
#define AXIO_ARCH_NAME "arm"

#else
#error "Unsupported architecture"
#endif

#endif