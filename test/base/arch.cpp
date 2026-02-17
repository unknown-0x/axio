#include <simpletest/simpletest.hpp>

#include <axio/base/arch.hpp>

#if defined(AXIO_ARCH_X64)
TEST_CASE(Arch, IsX64) {
  CHECK_EQ(AXIO_ARCH_X64, 1);
  CHECK_STR_EQ(AXIO_ARCH_NAME, "x64");
}

#elif defined(AXIO_ARCH_X86)
TEST_CASE(Arch, IsX86) {
  CHECK_EQ(AXIO_ARCH_X86, 1);
  CHECK_STR_EQ(AXIO_ARCH_NAME, "x86");
}

#elif defined(AXIO_ARCH_ARM64)
TEST_CASE(Arch, IsARM64) {
  CHECK_EQ(AXIO_ARCH_ARM64, 1);
  CHECK_STR_EQ(AXIO_ARCH_NAME, "arm64");
}

#elif defined(AXIO_ARCH_ARM)
TEST_CASE(Arch, IsARM) {
  CHECK_EQ(AXIO_ARCH_ARM, 1);
  CHECK_STR_EQ(AXIO_ARCH_NAME, "arm");
}

#endif
