#include <simpletest/simpletest.hpp>

#include <axio/base/compiler.hpp>

#if defined(AXIO_COMPILER_CLANG)
TEST_CASE(Compiler, IsClang) {
  CHECK_EQ(AXIO_COMPILER_CLANG, 1);
#if !defined(AXIO_COMPILER_APPLE_CLANG) && !defined(AXIO_COMPILER_MSVC)
  CHECK_STR_EQ(AXIO_COMPILER_NAME, "clang");
#endif
}

#if defined(AXIO_COMPILER_APPLE_CLANG)
TEST_CASE(Compiler, IsAppleClang) {
  CHECK_EQ(AXIO_COMPILER_APPLE_CLANG, 1);
  CHECK_STR_EQ(AXIO_COMPILER_NAME, "apple-clang");
}
#elif defined(AXIO_COMPILER_MSVC)
TEST_CASE(Compiler, IsClangCl) {
  CHECK_EQ(AXIO_COMPILER_MSVC, 1);
  CHECK_STR_EQ(AXIO_COMPILER_NAME, "clang-cl");
}
#endif

#elif defined(AXIO_COMPILER_MSVC)
TEST_CASE(Compiler, IsMsvc) {
  CHECK_EQ(AXIO_COMPILER_MSVC, 1);
  CHECK_STR_EQ(AXIO_COMPILER_NAME, "msvc");
}

#elif defined(AXIO_COMPILER_GCC)
TEST_CASE(Compiler, IsGCC) {
  CHECK_EQ(AXIO_COMPILER_GCC, 1);
  CHECK_STR_EQ(AXIO_COMPILER_NAME, "gcc");
}
#endif
