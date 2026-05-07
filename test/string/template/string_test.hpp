#include <axio/string/string.hpp>
#include <simpletest/simpletest.hpp>

#ifndef TEXT
#error "TEXT is not defined"
#endif

#ifndef CHAR
#error "CHAR is not defined"
#endif

using String = axio::BasicString<CHAR>;

#if defined(USE_CHAR)
#define STRING_TEST_CASE(name, test_case_name) \
  TEST_CASE(name##_char, test_case_name)
#elif defined(USE_WCHAR)
#define STRING_TEST_CASE(name, test_case_name) \
  TEST_CASE(name##_wchar, test_case_name)
#elif defined(USE_CHAR8)
#define STRING_TEST_CASE(name, test_case_name) \
  TEST_CASE(name##_char8, test_case_name)
#elif defined(USE_CHAR16)
#define STRING_TEST_CASE(name, test_case_name) \
  TEST_CASE(name##_char16, test_case_name)
#elif defined(USE_CHAR32)
#define STRING_TEST_CASE(name, test_case_name) \
  TEST_CASE(name##_char32, test_case_name)
#endif

STRING_TEST_CASE(String, DefaultConstruct) {
  String s;
  CHECK_EQ(s.Size(), 0);
  CHECK_NE(s.Data(), nullptr);
  CHECK_EQ(s.CStr()[0], String::kNullTerminator);
}

STRING_TEST_CASE(String, FromCString) {
  const CHAR* text = TEXT("hello");
  String s(text);
  CHECK_EQ(s.Size(), 5);
  CHECK_STR_EQ(s.CStr(), text);
}

STRING_TEST_CASE(String, FillConstructor) {
  String s(5, TEXT('a'));

  CHECK_EQ(s.Size(), 5);
  for (size_t i = 0; i < s.Size(); ++i) {
    CHECK_EQ(s.CStr()[i], TEXT('a'));
  }
  CHECK_EQ(s.CStr()[5], String::kNullTerminator);
}

STRING_TEST_CASE(String, SSO_SmallString) {
  String s(TEXT("abc"));

  CHECK_EQ(s.Size(), 3);
  CHECK_STR_EQ(s.CStr(), TEXT("abc"));
  CHECK_TRUE(s.Capacity() >= s.Size());
}

STRING_TEST_CASE(String, Heap_LargeString) {
  std::basic_string<CHAR> big(200, TEXT('a'));

  String s(big.c_str());

  CHECK_EQ(s.Size(), 200);
  CHECK_STR_EQ(s.CStr(), big.c_str());
  CHECK_TRUE(s.Capacity() >= 200);
}