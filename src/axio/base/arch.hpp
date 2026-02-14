#ifndef AXIO_BASE_ARCH_HPP_
#define AXIO_BASE_ARCH_HPP_

#if defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || \
    defined(_M_AMD64)
#define AXIO_ARCH_X64 1
#define AXIO_ARCH_NAME "x64"

#elif defined(__i386) || defined(_M_IX86)
#define AXIO_ARCH_X86 1
#define AXIO_ARCH_NAME "x86"

#elif defined(__aarch64__) || defined(_M_ARM64)
#define AXIO_ARCH_ARM64 1
#define AXIO_ARCH_NAME "arm64"

#elif defined(__arm__) || defined(_M_ARM)
#define AXIO_ARCH_ARM 1
#define AXIO_ARCH_NAME "arm"

#else
#error "Unsupported architecture"
#endif

#endif