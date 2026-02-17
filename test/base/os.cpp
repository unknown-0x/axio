#include <simpletest/simpletest.hpp>

#include <axio/base/os.hpp>

#if defined(AXIO_OS_WINDOWS)
TEST_CASE(OS, IsWindows) {
  IGNORE_RESULT();
  CHECK_EQ(AXIO_OS_WINDOWS, 1);
}

TEST_CASE(OS, IsNotPosix) {
  IGNORE_RESULT();
#if defined(AXIO_OS_POSIX)
  static_assert(false, "Windows should not define AXIO_OS_POSIX");
#endif
}

#if defined(AXIO_OS_WINDOWS_X64)
TEST_CASE(OS, IsWindowsX64) {
  CHECK_EQ(AXIO_OS_WINDOWS_X64, 1);
  CHECK_STR_EQ(AXIO_OS_NAME, "windows-x64");
}
#elif defined(AXIO_OS_WINDOWS_X86)
TEST_CASE(OS, IsWindowsX86) {
  CHECK_EQ(AXIO_OS_WINDOWS_X86, 1);
  CHECK_STR_EQ(AXIO_OS_NAME, "windows-x86");
}
#endif

#elif defined(AXIO_OS_APPLE)
TEST_CASE(OS, IsApple) {
  IGNORE_RESULT();

  CHECK_EQ(AXIO_OS_APPLE, 1);
  CHECK_EQ(AXIO_OS_POSIX, 1);

#if defined(AXIO_OS_IOS_SIMULATOR)
  CHECK_EQ(AXIO_OS_IOS_SIMULATOR, 1);
  CHECK_STR_EQ(AXIO_OS_NAME, "ios-simulator");

#elif defined(AXIO_OS_IOS)
  CHECK_EQ(AXIO_OS_IOS, 1);
  CHECK_STR_EQ(AXIO_OS_NAME, "ios");

#elif defined(AXIO_OS_MACOS)
  CHECK_EQ(AXIO_OS_MACOS, 1);
  CHECK_STR_EQ(AXIO_OS_NAME, "macos");

#else
  CHECK_STR_EQ(AXIO_OS_NAME, "apple");
#endif
}

#elif defined(AXIO_OS_LINUX)
TEST_CASE(OS, IsLinux) {
  IGNORE_RESULT();
  CHECK_EQ(AXIO_OS_LINUX, 1);
  CHECK_EQ(AXIO_OS_POSIX, 1);
  CHECK_STR_EQ(AXIO_OS_NAME, "linux");
}

#elif defined(AXIO_OS_POSIX)
TEST_CASE(OS, IsPosix) {
  IGNORE_RESULT();
  CHECK_EQ(AXIO_OS_POSIX, 1);
  CHECK_STR_EQ(AXIO_OS_NAME, "posix");
}
#endif
