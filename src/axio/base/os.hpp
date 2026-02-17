#ifndef AXIO_BASE_OS_HPP_
#define AXIO_BASE_OS_HPP_

#if defined(_WIN32) || defined(_WIN64)
#define AXIO_OS_WINDOWS 1
#if defined(_WIN64)
#define AXIO_OS_WINDOWS_X64 1
#define AXIO_OS_NAME "windows-x64"
#else
#define AXIO_OS_WINDOWS_X86 1
#define AXIO_OS_WINDOWS 1
#define AXIO_OS_NAME "windows-x86"
#endif

#elif defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#define AXIO_OS_APPLE 1
#define AXIO_OS_POSIX 1

#if TARGET_OS_IOS && TARGET_OS_SIMULATOR
#define AXIO_OS_IOS_SIMULATOR 1
#define AXIO_OS_NAME "ios-simulator"
#elif TARGET_OS_IOS
#define AXIO_OS_IOS 1
#define AXIO_OS_NAME "ios"
#elif TARGET_OS_MAC
#define AXIO_OS_MACOS 1
#define AXIO_OS_NAME "macos"
#else
#define AXIO_OS_NAME "apple"
#endif

#elif defined(__linux__)
#define AXIO_OS_LINUX 1
#define AXIO_OS_POSIX 1
#define AXIO_OS_NAME "linux"

#elif defined(__unix__) || defined(__unix) || defined(_POSIX_VERSION)
#define AXIO_OS_POSIX 1
#define AXIO_OS_NAME "posix"

#else
#error "Unsupported operating system"
#endif

#endif