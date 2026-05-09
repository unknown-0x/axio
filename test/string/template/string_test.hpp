#include <axio/string/string.hpp>
#include <simpletest/simpletest.hpp>

#ifndef TEXT
#error "TEXT is not defined"
#endif

#ifndef CHAR
#error "CHAR is not defined"
#endif

using String = axio::BasicString<CHAR>;
using SizeType = typename String::SizeType;
using CharTraits = typename String::TraitsType;

static constexpr SizeType kNpos = String::kNpos;

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
  CHECK_EQ(s.CStr()[0], String::kNullTerminator);
  CHECK_EQ(s.Size(), 0);
  CHECK_TRUE(s.IsEmpty());
  CHECK_STR_EQ(s.CStr(), TEXT(""));
}

STRING_TEST_CASE(String, AllocatorConstructor) {
  axio::Allocator<CHAR> alloc;
  String s(alloc);
  CHECK_EQ(s.CStr()[0], String::kNullTerminator);
  CHECK_EQ(s.Size(), 0);
  CHECK_TRUE(s.IsEmpty());
  CHECK_STR_EQ(s.CStr(), TEXT(""));
}

namespace {
struct TestCase {
  const CHAR* text;
  const SizeType size;
};

static constexpr TestCase kTestCases[]{
    TestCase{TEXT(""), 0},
    TestCase{TEXT("a"), 1},
    TestCase{TEXT("aaa"), 3},
    TestCase{TEXT("hello"), 5},
    TestCase{TEXT("hello hello"), 11},
    TestCase{TEXT("this is a long string!"), 22},
    TestCase{TEXT("this is a loong string!"), 23},
    TestCase{TEXT("this is a looong string!"), 24},
    TestCase{TEXT("this is a loooong string!"), 25},
    TestCase{TEXT("this is a looooong string!"), 26},
    TestCase{TEXT("this is a loooooooooooong string!"), 33},
    TestCase{TEXT("this is a very loooooooooooong string!"), 38}};
}  // namespace

STRING_TEST_CASE(String, Constructor_CString) {
  for (const auto& tc : kTestCases) {
    String s(tc.text);
    CHECK_EQ(s.Size(), tc.size);
    CHECK_STR_EQ(s.CStr(), tc.text);
  }

  for (const auto& tc : kTestCases) {
    String s(tc.text, tc.size);
    CHECK_EQ(s.Size(), tc.size);
    CHECK_STR_EQ(s.CStr(), tc.text);
  }
}

STRING_TEST_CASE(String, FillConstructor) {
  const auto character = TEXT('A');
  const SizeType lengths[]{0, 1, 5, 22, 23, 24, 50};

  for (auto l : lengths) {
    String s(l, character);
    CHECK_EQ(s.Size(), l);
    for (size_t i = 0; i < s.Size(); ++i) {
      CHECK_EQ(s.CStr()[i], character);
    }
    CHECK_EQ(s.CStr()[l], String::kNullTerminator);
  }
}

STRING_TEST_CASE(String, CopyConstructor) {
  for (const auto& test : kTestCases) {
    String s(test.text);
    String copy(s);
    CHECK_EQ(copy.Size(), s.Size());
    CHECK_EQ(copy.Size(), test.size);
    CHECK_STR_EQ(copy.CStr(), test.text);
    CHECK_STR_EQ(s.CStr(), test.text);
  }
}

STRING_TEST_CASE(String, MoveConstructor) {
  for (const auto& test : kTestCases) {
    String s(test.text);
    String moved(axio::Move(s));

    CHECK_EQ(moved.Size(), test.size);
    CHECK_STR_EQ(moved.CStr(), test.text);

    CHECK_EQ(s.Size(), 0);
    CHECK_STR_EQ(s.CStr(), TEXT(""));
  }
}

STRING_TEST_CASE(String, SubstringConstructor) {
  struct SubstringTestCase {
    const CHAR* text;
    const SizeType pos;
    const SizeType count;
    const CHAR* expected;
  };
  static constexpr SubstringTestCase test_cases[]{
      {TEXT(""), 0, 0, TEXT("")},
      {TEXT(""), 0, kNpos, TEXT("")},
      {TEXT("hello"), 0, 5, TEXT("hello")},
      {TEXT("hello"), 0, 1, TEXT("h")},
      {TEXT("hello"), 1, 2, TEXT("el")},
      {TEXT("hello"), 4, 1, TEXT("o")},
      {TEXT("hello"), 5, 0, TEXT("")},
      {TEXT("hello world"), 0, 5, TEXT("hello")},
      {TEXT("hello world"), 6, 5, TEXT("world")},
      {TEXT("hello world"), 3, 4, TEXT("lo w")},
      {TEXT("hello world"), 6, kNpos, TEXT("world")},
      {TEXT("this is a very loooooooooooong string!"), 0, 4, TEXT("this")},
      {TEXT("this is a very loooooooooooong string!"), 10, 4, TEXT("very")},
      {TEXT("this is a very loooooooooooong string!"), 15, kNpos,
       TEXT("loooooooooooong string!")},
      {TEXT("this is a very loooooooooooong string!"), 0, kNpos,
       TEXT("this is a very loooooooooooong string!")},
      {TEXT("abcdef"), 2, 100, TEXT("cdef")},
      {TEXT("abcdef"), 6, 0, TEXT("")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 0, 24,
       TEXT("abcdefghijklmnopqrstuvwx")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 1, 23,
       TEXT("bcdefghijklmnopqrstuvwx")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 1, 22,
       TEXT("bcdefghijklmnopqrstuvw")},
  };

  for (const auto& test : test_cases) {
    String original(test.text);
    String s(original, test.pos, test.count);

    CHECK_STR_EQ(s.CStr(), test.expected);
    CHECK_STR_EQ(original.CStr(), test.text);
    CHECK_EQ(s.Size(), CharTraits::length(test.expected));
  }
}

STRING_TEST_CASE(String, ExactSSOBoundary) {
  CHAR text[String::kSSOCapacity + 1];
  for (SizeType i = 0; i < String::kSSOCapacity; ++i) {
    text[i] = TEXT('a');
  }
  text[String::kSSOCapacity] = CHAR();

  String s(text);
  CHECK_EQ(s.Size(), String::kSSOCapacity);
}

STRING_TEST_CASE(String, HeapBoundaryJustAboveSSO) {
  CHAR text[String::kSSOCapacity + 2];
  for (SizeType i = 0; i < String::kSSOCapacity + 1; ++i) {
    text[i] = TEXT('a');
  }
  text[String::kSSOCapacity + 1] = CHAR();

  String s(text);
  CHECK_EQ(s.Size(), String::kSSOCapacity + 1);
}

STRING_TEST_CASE(String, InputItConstructor) {
  for (const auto& test : kTestCases) {
    std::basic_istringstream<CHAR> stream(test.text);

    std::istreambuf_iterator<CHAR> first(stream);
    std::istreambuf_iterator<CHAR> last;

    String s(first, last);

    CHECK_EQ(s.Size(), test.size);
    CHECK_STR_EQ(s.CStr(), test.text);
  }
}

#include <list>
#include <vector>

STRING_TEST_CASE(String, ForwardItConstructor) {
  for (const auto& test : kTestCases) {
    {
      String s(test.text, test.text + test.size);
      CHECK_EQ(s.Size(), test.size);
      CHECK_STR_EQ(s.CStr(), test.text);
    }
    {
      std::list<CHAR> l(test.text, test.text + test.size);
      String s(l.begin(), l.end());
      CHECK_EQ(s.Size(), test.size);
      CHECK_STR_EQ(s.CStr(), test.text);
    }
    {
      std::vector<CHAR> v(test.text, test.text + test.size);
      String s(v.begin(), v.end());
      CHECK_EQ(s.Size(), test.size);
      CHECK_STR_EQ(s.CStr(), test.text);
    }
  }
}

STRING_TEST_CASE(String, StringViewConstructor) {
  for (const auto& test : kTestCases) {
    {
      std::basic_string<CHAR> s1(test.text);
      String s(s1);
      CHECK_EQ(s.Size(), test.size);
      CHECK_STR_EQ(s.CStr(), test.text);
    }
    {
      std::basic_string_view<CHAR> s1(test.text);
      String s(s1);
      CHECK_EQ(s.Size(), test.size);
      CHECK_STR_EQ(s.CStr(), test.text);
    }
  }
}

STRING_TEST_CASE(String, StringViewConstruct_Substr) {
  struct SubstringTestCase {
    const CHAR* text;
    const SizeType pos;
    const SizeType count;
    const CHAR* expected;
  };
  static constexpr SubstringTestCase test_cases[]{
      {TEXT(""), 0, 0, TEXT("")},
      {TEXT(""), 0, kNpos, TEXT("")},
      {TEXT("hello"), 0, 5, TEXT("hello")},
      {TEXT("hello"), 0, 1, TEXT("h")},
      {TEXT("hello"), 1, 2, TEXT("el")},
      {TEXT("hello"), 4, 1, TEXT("o")},
      {TEXT("hello"), 5, 0, TEXT("")},
      {TEXT("hello world"), 0, 5, TEXT("hello")},
      {TEXT("hello world"), 6, 5, TEXT("world")},
      {TEXT("hello world"), 3, 4, TEXT("lo w")},
      {TEXT("hello world"), 6, kNpos, TEXT("world")},
      {TEXT("this is a very loooooooooooong string!"), 0, 4, TEXT("this")},
      {TEXT("this is a very loooooooooooong string!"), 10, 4, TEXT("very")},
      {TEXT("this is a very loooooooooooong string!"), 15, kNpos,
       TEXT("loooooooooooong string!")},
      {TEXT("this is a very loooooooooooong string!"), 0, kNpos,
       TEXT("this is a very loooooooooooong string!")},
      {TEXT("abcdef"), 2, 100, TEXT("cdef")},
      {TEXT("abcdef"), 6, 0, TEXT("")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 0, 24,
       TEXT("abcdefghijklmnopqrstuvwx")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 1, 23,
       TEXT("bcdefghijklmnopqrstuvwx")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 1, 22,
       TEXT("bcdefghijklmnopqrstuvw")},
  };

  for (const auto& test : test_cases) {
    {
      std::basic_string<CHAR> s1(test.text);
      String s(s1, test.pos, test.count);
      CHECK_EQ(s.Size(), CharTraits::length(test.expected));
      CHECK_STR_EQ(s.CStr(), test.expected);
    }
    {
      std::basic_string_view<CHAR> s1(test.text);
      String s(s1, test.pos, test.count);
      CHECK_EQ(s.Size(), CharTraits::length(test.expected));
      CHECK_STR_EQ(s.CStr(), test.expected);
    }
  }
}

STRING_TEST_CASE(String, InitListConstructor) {
  struct InitListTestCase {
    InitListTestCase(std::initializer_list<CHAR> values,
                     const CHAR* txt,
                     const SizeType sz)
        : s(values), text(txt), size(sz) {}
    String s;
    const CHAR* text;
    const SizeType size;
  };

  const InitListTestCase test_cases[] = {
      InitListTestCase{{}, TEXT(""), 0},

      InitListTestCase{{TEXT('A')}, TEXT("A"), 1},

      InitListTestCase{{TEXT('H'), TEXT('e'), TEXT('l'), TEXT('l'), TEXT('o')},
                       TEXT("Hello"),
                       5},

      InitListTestCase{{TEXT('1'), TEXT('2'), TEXT('3'), TEXT('4'), TEXT('5'),
                        TEXT('6'), TEXT('7'), TEXT('8'), TEXT('9'), TEXT('0')},
                       TEXT("1234567890"),
                       10},

      InitListTestCase{
          {TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a'),
           TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a'),
           TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a'),
           TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a'), TEXT('a')},
          TEXT("aaaaaaaaaaaaaaaaaaaaaaa"),
          23},

      InitListTestCase{
          {TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'),
           TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'),
           TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'),
           TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b'), TEXT('b')},
          TEXT("bbbbbbbbbbbbbbbbbbbbbbbb"),
          24},

      InitListTestCase{
          {TEXT('t'), TEXT('h'), TEXT('i'), TEXT('s'), TEXT(' '), TEXT('i'),
           TEXT('s'), TEXT(' '), TEXT('l'), TEXT('o'), TEXT('n'), TEXT('g')},
          TEXT("this is long"),
          12}};

  for (const auto& test : test_cases) {
    CHECK_EQ(test.s.Size(), test.size);
    CHECK_STR_EQ(test.s.CStr(), test.text);
  }
}

STRING_TEST_CASE(String, OperatorStringView) {
  for (const auto& test : kTestCases) {
    String s(test.text);
    CHECK_EQ(s.Size(), test.size);
    CHECK_STR_EQ(s.CStr(), test.text);

    std::basic_string_view<CHAR> view = s;
    CHECK_EQ(view.size(), s.Size());
    CHECK_STR_EQ(s.CStr(), s.Data());

    if (!s.IsEmpty()) {
      s[0] = TEXT('H');
      CHECK_EQ(view.size(), s.Size());
      CHECK_STR_EQ(s.CStr(), s.Data());
    }
  }
}

namespace {
static constexpr CHAR kChars[]{
    TEXT('a'), TEXT('B'), TEXT('3'), TEXT('#'), TEXT('x'), TEXT('Q'), TEXT('7'),
    TEXT('@'), TEXT('m'), TEXT('Z'), TEXT('0'), TEXT('!'), TEXT('k'), TEXT('P'),
    TEXT('9'), TEXT('$'), TEXT('v'), TEXT('N'), TEXT('2'), TEXT('&'),
};

static constexpr TestCase kAssignmentTestCases[]{
    TestCase{TEXT(""), 0},
    TestCase{TEXT("a"), 1},
    TestCase{TEXT("aaa"), 3},
    TestCase{TEXT("hello"), 5},
    TestCase{TEXT("hello hello"), 11},
    TestCase{TEXT("this is a long string!"), 22},
    TestCase{TEXT("this is a loong string!"), 23},
    TestCase{TEXT("this is a looong string!"), 24},
    TestCase{TEXT("this is a loooong string!"), 25},
    TestCase{TEXT("this is a looooong string!"), 26},
    TestCase{TEXT("this is a loooooooooooong string!"), 33},
    TestCase{TEXT("this is a very loooooooooooong string!"), 38},
    TestCase{TEXT("hello"), 5},
    TestCase{TEXT("aaa"), 3},
    TestCase{TEXT(""), 0},
    TestCase{TEXT("foo foo foo bar bar"), 19},
    TestCase{
        TEXT("this is a very looooooooooooooooooooooooooooooooooooooooooong "
             "striiiiiiing!"),
        75},
    TestCase{TEXT("hello"), 5},
    TestCase{TEXT("foo foo foo bar bar bar"), 23},
    TestCase{TEXT(""), 0},
};
}  // namespace

STRING_TEST_CASE(String, CopyAssignment) {
  {
    String a(TEXT("Hello"));
    a = a;
    CHECK_EQ(a.Size(), 5);
    CHECK_STR_EQ(a.CStr(), TEXT("Hello"));
  }
  {
    String a;
    String b;
    a = b;
    CHECK_EQ(a.Size(), 0);
    CHECK_EQ(b.Size(), 0);
    CHECK_TRUE(a.IsEmpty());
    CHECK_TRUE(b.IsEmpty());
    CHECK_STR_EQ(a.CStr(), TEXT(""));
    CHECK_STR_EQ(b.CStr(), TEXT(""));
  }
  {
    String a1;
    String a2;
    for (const auto& test : kAssignmentTestCases) {
      String b(test.text);

      a1 = b;
      a2.Assign(b);

      CHECK_EQ(a1.Size(), test.size);
      CHECK_EQ(a2.Size(), test.size);
      CHECK_STR_EQ(a1.CStr(), test.text);
      CHECK_STR_EQ(a2.CStr(), test.text);
    }
  }
}

STRING_TEST_CASE(String, MoveAssignment) {
  {
    String a(TEXT("Hello"));
    a = axio::Move(a);
    CHECK_EQ(a.Size(), 5);
    CHECK_STR_EQ(a.CStr(), TEXT("Hello"));
  }
  {
    String a;
    String b;
    a = axio::Move(b);
    CHECK_EQ(a.Size(), 0);
    CHECK_EQ(b.Size(), 0);
    CHECK_TRUE(a.IsEmpty());
    CHECK_TRUE(b.IsEmpty());
    CHECK_STR_EQ(a.CStr(), TEXT(""));
    CHECK_STR_EQ(b.CStr(), TEXT(""));
  }
  {
    String a1;
    String a2;
    for (const auto& test : kAssignmentTestCases) {
      {
        String b(test.text);
        a1 = axio::Move(b);
        CHECK_EQ(a1.Size(), test.size);
        CHECK_STR_EQ(a1.CStr(), test.text);

        if (axio::IsSpecializationOf<typename String::AllocatorType,
                                     axio::Allocator>::value) {
          CHECK_EQ(b.Size(), 0);
          CHECK_TRUE(b.IsEmpty());
          CHECK_STR_EQ(b.CStr(), TEXT(""));
        }
      }
      {
        String b(test.text);
        a2.Assign(axio::Move(b));
        CHECK_EQ(a2.Size(), test.size);
        CHECK_STR_EQ(a2.CStr(), test.text);

        if (axio::IsSpecializationOf<typename String::AllocatorType,
                                     axio::Allocator>::value) {
          CHECK_EQ(b.Size(), 0);
          CHECK_TRUE(b.IsEmpty());
          CHECK_STR_EQ(b.CStr(), TEXT(""));
        }
      }
    }
  }
}

STRING_TEST_CASE(String, InputItAssignment) {
  String a;
  for (const auto& test : kAssignmentTestCases) {
    {
      std::basic_istringstream<CHAR> stream(test.text);

      std::istreambuf_iterator<CHAR> first(stream);
      std::istreambuf_iterator<CHAR> last;

      a.Assign(first, last);
      CHECK_EQ(a.Size(), test.size);
      CHECK_STR_EQ(a.CStr(), test.text);
    }
  }
}

STRING_TEST_CASE(String, CountCharAssignment) {
  struct CCATestCase {
    CHAR value;
    SizeType count;
    const CHAR* expected;
  };
  static constexpr CCATestCase test_cases[]{
      {TEXT('a'), 0, TEXT("")},
      {TEXT('x'), 1, TEXT("x")},
      {TEXT('a'), 2, TEXT("aa")},
      {TEXT('b'), 3, TEXT("bbb")},
      {TEXT('c'), 4, TEXT("cccc")},
      {TEXT('d'), 7, TEXT("ddddddd")},
      {TEXT('e'), 8, TEXT("eeeeeeee")},
      {TEXT('f'), 15, TEXT("fffffffffffffff")},
      {TEXT('g'), 16, TEXT("gggggggggggggggg")},
      {TEXT('h'), 17, TEXT("hhhhhhhhhhhhhhhhh")},
      {TEXT('i'), 31, TEXT("iiiiiiiiiiiiiiiiiiiiiiiiiiiiiii")},
      {TEXT('j'), 32, TEXT("jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj")},
      {TEXT('k'), 33, TEXT("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk")},
      {TEXT('$'), 78,
       TEXT("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"
            "$$$$$$$$$$$$")},
      {TEXT(' '), 0, TEXT("")},
      {TEXT('m'), 63,
       TEXT("mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
            "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm")},
      {TEXT('n'), 64,
       TEXT("nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"
            "nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn")},
      {TEXT('0'), 10, TEXT("0000000000")},
      {TEXT('#'), 5, TEXT("#####")},
      {TEXT(' '), 6, TEXT("      ")},
      {TEXT('['), 42, TEXT("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[")},
  };

  String a;
  for (const auto& test : test_cases) {
    a.Assign(test.count, test.value);
    CHECK_EQ(a.Size(), test.count);
    CHECK_STR_EQ(a.CStr(), test.expected);
  }
}

STRING_TEST_CASE(String, AssignOperator_Char) {
  struct AOCTestCase {
    const CHAR* original;
    CHAR value;
  };

  static constexpr AOCTestCase test_cases[]{
      AOCTestCase{TEXT(""), TEXT('A')},
      AOCTestCase{TEXT("a"), TEXT('$')},
      AOCTestCase{TEXT("aa"), TEXT('$')},
      AOCTestCase{TEXT("aaa"), TEXT('$')},
      AOCTestCase{TEXT("aaaa"), TEXT('$')},
      AOCTestCase{TEXT("aaaaa"), TEXT('$')},
      AOCTestCase{TEXT("aaaaaa"), TEXT('$')},
      AOCTestCase{TEXT("aaaaaaa"), TEXT('$')},
      AOCTestCase{TEXT("aaaaaaaa"), TEXT('$')},
      AOCTestCase{TEXT("aaaaaaaaa"), TEXT('$')},
      AOCTestCase{TEXT("hello world"), TEXT('x')},
      AOCTestCase{TEXT("this is loong string!"), TEXT('x')},
      AOCTestCase{TEXT("this is looong string!"), TEXT('y')},
      AOCTestCase{TEXT("this is loooong string!"), TEXT('z')},
      AOCTestCase{TEXT("this is looooong string!"), TEXT('^')},
      AOCTestCase{TEXT("this is loooooong string!"), TEXT('@')},
      AOCTestCase{TEXT("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), TEXT('5')},
      AOCTestCase{TEXT("!!&^^%&*%^&!@#$!@#"), TEXT(' ')},
      AOCTestCase{TEXT("                  "), TEXT(' ')},
  };

  SizeType n = 0;
  for (const auto& test : test_cases) {
    String s(test.original);

    s = test.value;

    CHECK_EQ(s.Size(), 1);
    CHECK_EQ(s[0], test.value);

    for (int i = 0; i < 5; ++i) {
      const auto ch = kChars[n++ % AXIO_ARRAY_SIZE(kChars)];
      s = ch;
      CHECK_EQ(s.Size(), 1);
      CHECK_EQ(s[0], ch);
    }
  }
  {
    String s;

    s = TEXT('a');

    CHECK_EQ(s.Size(), 1);
    CHECK_EQ(s[0], TEXT('a'));
    CHECK_STR_EQ(s.CStr(), TEXT("a"));
  }
}

STRING_TEST_CASE(String, Clear) {
  for (const auto& test : kTestCases) {
    String s(test.text);

    CHECK_EQ(s.Size(), test.size);
    CHECK_STR_EQ(s.CStr(), test.text);
    const auto old_capacity = s.Capacity();
    const auto* old_ptr = s.Data();

    s.Clear();

    CHECK_STR_EQ(s.CStr(), TEXT(""));
    CHECK_TRUE(s.IsEmpty());
    CHECK_EQ(s.Length(), 0u);
    CHECK_EQ(s.Size(), 0u);
    CHECK_EQ(s.Data(), old_ptr);
    CHECK_EQ(s.Capacity(), old_capacity);
  }

  {
    String s(TEXT("hello"));
    s.Clear();
    CHECK_TRUE(s.IsEmpty());
    s.Clear();

    CHECK_TRUE(s.IsEmpty());
    CHECK_EQ(s.Size(), 0u);
    CHECK_STR_EQ(s.CStr(), TEXT(""));
  }
  {
    String s(TEXT("abc"));
    s.Resize(100, TEXT('x'));

    s.Clear();

    CHECK_EQ(s.Size(), 0u);
    CHECK_STR_EQ(s.CStr(), TEXT(""));
  }
  {
    String s(TEXT("hello"));
    s.Reserve(200);
    const auto capacity = s.Capacity();
    s.Clear();

    CHECK_TRUE(s.IsEmpty());
    CHECK_EQ(s.Size(), 0);
    CHECK_EQ(s.Capacity(), capacity);
  }
}

STRING_TEST_CASE(String, Resize) {
  struct ResizeTestCase {
    const CHAR* text;
    const SizeType size;
    const SizeType new_size;
    const CHAR ch;
    const CHAR* result;
  };
  static constexpr ResizeTestCase test_cases[]{
      {TEXT(""), 0, 0, TEXT('x'), TEXT("")},
      {TEXT(""), 0, 1, TEXT('a'), TEXT("a")},
      {TEXT(""), 0, 5, TEXT('z'), TEXT("zzzzz")},
      {TEXT("a"), 1, 2, TEXT('b'), TEXT("ab")},
      {TEXT("abc"), 3, 5, TEXT('x'), TEXT("abcxx")},
      {TEXT("hello"), 5, 10, TEXT('!'), TEXT("hello!!!!!")},
      {TEXT("hello"), 5, 4, TEXT('x'), TEXT("hell")},
      {TEXT("hello"), 5, 1, TEXT('x'), TEXT("h")},
      {TEXT("hello"), 5, 0, TEXT('x'), TEXT("")},
      {TEXT("abcdefghijklmnopqrstuvw"), 23, 23, TEXT('x'),
       TEXT("abcdefghijklmnopqrstuvw")},
      {TEXT("abcdefghijklmnopqrstuvw"), 23, 24, TEXT('x'),
       TEXT("abcdefghijklmnopqrstuvwx")},
      {TEXT("abcdefghijklmnopqrstuvw"), 23, 30, TEXT('a'),
       TEXT("abcdefghijklmnopqrstuvwaaaaaaa")},
      {TEXT("abcdefghijklmnopqrstuvwx"), 24, 25, TEXT('y'),
       TEXT("abcdefghijklmnopqrstuvwxy")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 30, TEXT('!'),
       TEXT("abcdefghijklmnopqrstuvwxyz!!!!")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 25, TEXT('x'),
       TEXT("abcdefghijklmnopqrstuvwxy")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 10, TEXT('x'),
       TEXT("abcdefghij")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 1, TEXT('x'), TEXT("a")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 0, TEXT('x'), TEXT("")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 23, TEXT('x'),
       TEXT("abcdefghijklmnopqrstuvw")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 5, TEXT('x'), TEXT("abcde")},
      {TEXT("aaaaa"), 5, 10, TEXT('a'), TEXT("aaaaaaaaaa")},
      {TEXT("bbbbb"), 5, 8, TEXT('c'), TEXT("bbbbbccc")},
      {TEXT("abc"), 3, 50, TEXT('z'),
       TEXT("abczzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz")},
      {TEXT("abc"), 3, 3, TEXT('x'), TEXT("abc")},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 26, TEXT('x'),
       TEXT("abcdefghijklmnopqrstuvwxyz")},
      {TEXT("this is a very loooooooooooong string!"), 38, 4, TEXT('x'),
       TEXT("this")},
      {TEXT("this is a very loooooooooooong string!"), 38, 0, TEXT('x'),
       TEXT("")},
  };

  for (const auto& test : test_cases) {
    String s(test.text);
    CHECK_EQ(s.Size(), test.size);

    s.Resize(test.new_size, test.ch);
    CHECK_EQ(s.Size(), test.new_size);
    CHECK_STR_EQ(s.CStr(), test.result);
  }

  {
    const auto* text = TEXT("abcdefghijklmnopqrstuvw");
    String s(text);

    CHECK_EQ(s.Size(), 23);

    s.Resize(String::kSSOCapacity - 1);

    CHECK_EQ(s.Size(), String::kSSOCapacity - 1);
    CHECK_EQ(s.CStr()[String::kSSOCapacity - 1], TEXT('\0'));

    for (SizeType i = 0; i < s.Size(); ++i) {
      CHECK_EQ(s[i], text[i]);
    }
  }
}

STRING_TEST_CASE(String, Reserve_Shrink) {
  IGNORE_RESULT();

  struct RSTestCase {
    const CHAR* text;
    const SizeType size;
    const SizeType new_capacity;
  };
  static constexpr RSTestCase test_cases[]{
      {TEXT(""), 0, 0},
      {TEXT(""), 0, 1},
      {TEXT(""), 0, String::kSSOCapacity},
      {TEXT(""), 0, String::kSSOCapacity + 1},
      {TEXT(""), 0, 100},

      {TEXT("a"), 1, 0},
      {TEXT("a"), 1, 1},
      {TEXT("a"), 1, 5},
      {TEXT("hello"), 5, 10},
      {TEXT("hello"), 5, String::kSSOCapacity},
      {TEXT("hello"), 5, String::kSSOCapacity + 1},
      {TEXT("hello"), 5, 100},

      {TEXT("abcdefghijklmnopqrs"), 19, 23},
      {TEXT("abcdefghijklmnopqrs"), 19, 10},

      {TEXT("abcdefghijklmnopqrstuvw"), 23, 23},
      {TEXT("abcdefghijklmnopqrstuvw"), 23, 24},
      {TEXT("abcdefghijklmnopqrstuvw"), 23, 100},

      {TEXT("abcdefghijklmnopqrstuvwx"), 24, 24},
      {TEXT("abcdefghijklmnopqrstuvwx"), 24, 25},
      {TEXT("abcdefghijklmnopqrstuvwx"), 24, 100},

      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 26},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 27},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 64},
      {TEXT("abcdefghijklmnopqrstuvwxyz"), 26, 256},

      {TEXT("this is a very loooooooooooong string!"), 38, 38},
      {TEXT("this is a very loooooooooooong string!"), 38, 64},
      {TEXT("this is a very loooooooooooong string!"), 38, 512},

      {TEXT("this is a very loooooooooooong string!"), 38, 24},
  };

  for (const auto& test : test_cases) {
    String s(test.text, test.size);
    CHECK_EQ(s.Size(), test.size);
    CHECK_STR_EQ(s.CStr(), test.text);
    auto old_cap = s.Capacity();

    s.Reserve(test.new_capacity);
    CHECK_STR_EQ(s.CStr(), test.text);
    CHECK_GE(s.Capacity(), old_cap);
    CHECK_EQ(s.Size(), test.size);

    old_cap = s.Capacity();
    s.Shrink();
    CHECK_EQ(s.Size(), test.size);
    CHECK_STR_EQ(s.CStr(), test.text);

    const auto capacity =
        (test.size <= String::kSSOCapacity) ? String::kSSOCapacity : s.Size();
    CHECK_EQ(s.Capacity(), capacity);
    CHECK_LE(s.Capacity(), old_cap);
  }
}

STRING_TEST_CASE(String, Push) {
  IGNORE_RESULT();

  using string = std::basic_string<CHAR>;
  {
    string s1;
    String s2;
    for (SizeType i = 0; i < 100; ++i) {
      const auto c = kChars[i % AXIO_ARRAY_SIZE(kChars)];
      s1.push_back(c);
      auto& added = s2.Push(c);

      CHECK_EQ(added, c);
      CHECK_EQ(s2.Size(), s1.size());
      CHECK_STR_EQ(s2.CStr(), s1.c_str());
    }
  }

  {
    string s1;
    String s2;
    for (SizeType i = 0; i < 100; ++i) {
      const auto c = kChars[i % AXIO_ARRAY_SIZE(kChars)];
      s1.push_back(c);
      auto& added = s2.Push(TEXT('A'));
      added = c;

      CHECK_EQ(added, c);
      CHECK_EQ(s2.Size(), s1.size());
      CHECK_STR_EQ(s2.CStr(), s1.c_str());
    }
  }
}

STRING_TEST_CASE(String, ElementAccess) {
  static constexpr TestCase test_cases[]{
      TestCase{TEXT("a"), 1},
      TestCase{TEXT("aaa"), 3},
      TestCase{TEXT("hello"), 5},
      TestCase{TEXT("hello hello"), 11},
      TestCase{TEXT("this is a long string!"), 22},
      TestCase{TEXT("this is a loong string!"), 23},
      TestCase{TEXT("this is a looong string!"), 24},
      TestCase{TEXT("this is a loooong string!"), 25},
      TestCase{TEXT("this is a looooong string!"), 26},
      TestCase{TEXT("this is a loooooooooooong string!"), 33},
      TestCase{TEXT("this is a very loooooooooooong string!"), 38}};

  {
    SizeType idx = 0;
    for (const auto& test : test_cases) {
      String s(test.text);
      CHECK_FALSE(s.IsEmpty());
      CHECK_EQ(s.Size(), test.size);
      CHECK_EQ(s.Size(), s.Length());
      CHECK_EQ(s.Front(), test.text[0]);
      CHECK_EQ(s.Back(), test.text[test.size - 1]);
      for (SizeType i = 0; i < test.size; ++i) {
        CHECK_EQ(s[i], test.text[i]);
        CHECK_EQ(s.At(i), test.text[i]);
      }

      const auto c1 = kChars[idx++ % AXIO_ARRAY_SIZE(kChars)];
      s.Front() = c1;
      s.Back() = c1;
      CHECK_EQ(s.Front(), c1);
      CHECK_EQ(s.Back(), c1);
    }
  }
  {
    for (const auto& test : test_cases) {
      const String s(test.text);
      CHECK_FALSE(s.IsEmpty());
      CHECK_EQ(s.Size(), test.size);
      CHECK_EQ(s.Size(), s.Length());
      CHECK_EQ(s.Front(), test.text[0]);
      CHECK_EQ(s.Back(), test.text[test.size - 1]);
      for (SizeType i = 0; i < test.size; ++i) {
        CHECK_EQ(s[i], test.text[i]);
        CHECK_EQ(s.At(i), test.text[i]);
      }
    }
  }
}

STRING_TEST_CASE(String, Iterator) {
  {
    for (const auto& test : kTestCases) {
      String s(test.text);
      SizeType i = 0;
      for (auto& ch : s) {
        ch = kChars[i++ % AXIO_ARRAY_SIZE(kChars)];
      }
      i = 0;
      for (auto ch : s) {
        CHECK_EQ(ch, kChars[i++ % AXIO_ARRAY_SIZE(kChars)]);
      }
    }
  }
  {
    for (const auto& test : kTestCases) {
      const String s(test.text);
      SizeType i = 0;
      for (auto ch : s) {
        CHECK_EQ(ch, test.text[i++]);
      }
    }
  }
}

STRING_TEST_CASE(String, ReverseIterator) {
  {
    String s(TEXT("0123456789"));
    for (auto it = s.rbegin(); it != s.rend(); ++it) {
      *it = TEXT('0') + TEXT('9') - *it;
    }

    CHECK_STR_EQ(s.CStr(), TEXT("9876543210"));
  }
  {
    const String s(TEXT("0123456789"));
    CHAR i = 9;
    for (auto it = s.rbegin(); it != s.rend(); ++it) {
      CHECK_EQ(*it, TEXT('0') + i--);
    }
  }
}