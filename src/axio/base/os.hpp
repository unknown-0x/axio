/**
 * @file os.hpp
 * @brief Operating system detection macros.
 *
 * @par Defined macros
 * - AXIO_OS_WINDOWS
 * - AXIO_OS_WINDOWS_X86
 * - AXIO_OS_WINDOWS_X64
 * - AXIO_OS_LINUX
 * - AXIO_OS_APPLE
 * - AXIO_OS_MACOS
 * - AXIO_OS_IOS
 * - AXIO_OS_IOS_SIMULATOR
 * - AXIO_OS_POSIX
 * - AXIO_OS_NAME (Human-readable name of the detected operating system)
 *
 * AXIO_OS_NAME is a human-readable operating system name, one of:
 * `"windows-x86"`, `"windows-x64"`, `"linux"`, `"macos"`, `"ios"`,
 * `"ios-simulator"`, `"posix"`.
 *
 * Example:
 * ```cpp
 * std::cout << AXIO_OS_NAME << '\n';
 * ```
 *
 * @warning Compilation fails on unsupported operating systems.
 */

#ifndef AXIO_BASE_OS_HPP_
#define AXIO_BASE_OS_HPP_

#if defined(_WIN32) || defined(_WIN64)
// Windows
/// @def AXIO_OS_WINDOWS
/// @brief Defined (as 1) when targeting Windows, on either architecture.
#define AXIO_OS_WINDOWS 1
#if defined(_WIN64)
// 64-bit Windows
/// @def AXIO_OS_WINDOWS_X64
/// @brief Defined (as 1) when targeting 64-bit Windows.
#define AXIO_OS_WINDOWS_X64 1
#define AXIO_OS_NAME "windows-x64"
#else
// 32-bit Windows
/// @def AXIO_OS_WINDOWS_X86
/// @brief Defined (as 1) when targeting 32-bit Windows.
#define AXIO_OS_WINDOWS_X86 1
#define AXIO_OS_WINDOWS 1
#define AXIO_OS_NAME "windows-x86"
#endif

#elif defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

// Apple platforms (macOS, iOS, ...)
/// @def AXIO_OS_APPLE
/// @brief Defined (as 1) when targeting any Apple platform.
#define AXIO_OS_APPLE 1
/// @def AXIO_OS_POSIX
/// @brief Defined (as 1) when targeting a POSIX-compatible platform.
#define AXIO_OS_POSIX 1

#if TARGET_OS_IOS && TARGET_OS_SIMULATOR
// iOS simulator
/// @def AXIO_OS_IOS_SIMULATOR
/// @brief Defined (as 1) when targeting the iOS simulator.
#define AXIO_OS_IOS_SIMULATOR 1
#define AXIO_OS_NAME "ios-simulator"
#elif TARGET_OS_IOS
// iOS device
/// @def AXIO_OS_IOS
/// @brief Defined (as 1) when targeting a physical iOS device.
#define AXIO_OS_IOS 1
#define AXIO_OS_NAME "ios"
#elif TARGET_OS_MAC
// macOS platform
/// @def AXIO_OS_MACOS
/// @brief Defined (as 1) when targeting macOS.
#define AXIO_OS_MACOS 1
#define AXIO_OS_NAME "macos"
#else
#define AXIO_OS_NAME "apple"
#endif

#elif defined(__linux__)
// Linux platform
/// @def AXIO_OS_LINUX
/// @brief Defined (as 1) when targeting Linux.
#define AXIO_OS_LINUX 1
#define AXIO_OS_POSIX 1
#define AXIO_OS_NAME "linux"

#elif defined(__unix__) || defined(__unix) || defined(_POSIX_VERSION)
// POSIX-compatible platform
#define AXIO_OS_POSIX 1
#define AXIO_OS_NAME "posix"

#else
#error "Unsupported operating system"
#endif

#endif