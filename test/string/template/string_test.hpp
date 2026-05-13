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

struct SubstringTestCase {
  const CHAR* text;
  const SizeType pos;
  const SizeType count;
  const CHAR* expected;
};

static constexpr SubstringTestCase kSubstringTestCases[]{
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
    {TEXT("abcdefghijklmnopqrstuvwxyz"), 1, 22, TEXT("bcdefghijklmnopqrstuvw")},
};
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
  for (const auto& test : kSubstringTestCases) {
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

#include <sstream>

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
  for (const auto& test : kSubstringTestCases) {
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

STRING_TEST_CASE(String, ForwardItAssignment) {
  String a;
  for (const auto& test : kAssignmentTestCases) {
    {
      String s(test.text, test.text + test.size);
      a.Assign(s.begin(), s.end());
      CHECK_EQ(a.Size(), test.size);
      CHECK_STR_EQ(a.CStr(), test.text);
    }
    {
      std::list<CHAR> l(test.text, test.text + test.size);
      a.Assign(l.begin(), l.end());
      CHECK_EQ(a.Size(), test.size);
      CHECK_STR_EQ(a.CStr(), test.text);
    }
    {
      std::vector<CHAR> v(test.text, test.text + test.size);
      a.Assign(v.begin(), v.end());
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

STRING_TEST_CASE(String, AssignCString) {
  String a1;
  String a2;
  for (const auto& test : kAssignmentTestCases) {
    {
      a1 = test.text;
      CHECK_EQ(a1.Size(), test.size);
      CHECK_STR_EQ(a1.CStr(), test.text);
    }
    {
      a2.Assign(test.text);
      CHECK_EQ(a2.Size(), test.size);
      CHECK_STR_EQ(a2.CStr(), test.text);
    }
  }
}

STRING_TEST_CASE(String, Assign_StringViewLike) {
  {
    String a1;
    String a2;
    for (const auto& test : kAssignmentTestCases) {
      std::basic_string<CHAR> s(test.text, test.size);
      std::basic_string_view<CHAR> sv(test.text, test.size);
      {
        a1 = s;
        CHECK_EQ(a1.Size(), test.size);
        CHECK_STR_EQ(a1.CStr(), test.text);
      }
      {
        a2 = sv;
        CHECK_EQ(a2.Size(), test.size);
        CHECK_STR_EQ(a2.CStr(), test.text);
      }
    }
  }
  {
    String a1;
    String a2;
    for (const auto& test : kAssignmentTestCases) {
      std::basic_string<CHAR> s(test.text, test.size);
      std::basic_string_view<CHAR> sv(test.text, test.size);
      {
        a1.Assign(s);
        CHECK_EQ(a1.Size(), test.size);
        CHECK_STR_EQ(a1.CStr(), test.text);
      }
      {
        a2.Assign(sv);
        CHECK_EQ(a2.Size(), test.size);
        CHECK_STR_EQ(a2.CStr(), test.text);
      }
    }
  }
}

STRING_TEST_CASE(String, Assign_Substr) {
  {
    String a1;
    String a2;
    String a3;

    for (const auto& test : kSubstringTestCases) {
      String ss(test.text);
      std::basic_string<CHAR> s(test.text);
      std::basic_string_view<CHAR> sv(test.text);

      a1.Assign(s, test.pos, test.count);
      a2.Assign(sv, test.pos, test.count);
      a3.Assign(ss, test.pos, test.count);

      const auto len = CharTraits::length(test.expected);

      CHECK_EQ(a1.Size(), len);
      CHECK_EQ(a2.Size(), len);
      CHECK_EQ(a3.Size(), len);
      CHECK_STR_EQ(a1.CStr(), test.expected);
      CHECK_STR_EQ(a2.CStr(), test.expected);
      CHECK_STR_EQ(a3.CStr(), test.expected);
    }
  }
}

STRING_TEST_CASE(String, AssignInitList) {
  {
    String s(TEXT("hello"));

    s.Assign({});

    CHECK_TRUE(s.IsEmpty());
    CHECK_EQ(s.Size(), 0);
    CHECK_STR_EQ(s.CStr(), TEXT(""));

    s.Assign({TEXT('a')});

    CHECK_EQ(s.Size(), 1);
    CHECK_STR_EQ(s.CStr(), TEXT("a"));

    s.Assign({
        TEXT('a'), TEXT('b'), TEXT('c'), TEXT('d'), TEXT('e'),
        TEXT('f'), TEXT('g'), TEXT('h'), TEXT('i'), TEXT('j'),
        TEXT('k'), TEXT('l'), TEXT('m'), TEXT('n'), TEXT('o'),
        TEXT('p'), TEXT('q'), TEXT('r'), TEXT('s'), TEXT('t'),
    });

    CHECK_EQ(s.Size(), 20);
    CHECK_STR_EQ(s.CStr(), TEXT("abcdefghijklmnopqrst"));

    s.Assign({
        TEXT('!'),
        TEXT('@'),
        TEXT('#'),
        TEXT('$'),
    });

    CHECK_EQ(s.Size(), 4);
    CHECK_STR_EQ(s.CStr(), TEXT("!@#$"));
  }

  {
    String s(TEXT("hello"));

    s = {};

    CHECK_TRUE(s.IsEmpty());
    CHECK_EQ(s.Size(), 0);
    CHECK_STR_EQ(s.CStr(), TEXT(""));

    s = {TEXT('a')};

    CHECK_EQ(s.Size(), 1);
    CHECK_STR_EQ(s.CStr(), TEXT("a"));

    s = {
        TEXT('a'), TEXT('b'), TEXT('c'), TEXT('d'), TEXT('e'),
        TEXT('f'), TEXT('g'), TEXT('h'), TEXT('i'), TEXT('j'),
        TEXT('k'), TEXT('l'), TEXT('m'), TEXT('n'), TEXT('o'),
        TEXT('p'), TEXT('q'), TEXT('r'), TEXT('s'), TEXT('t'),
    };

    CHECK_EQ(s.Size(), 20);
    CHECK_STR_EQ(s.CStr(), TEXT("abcdefghijklmnopqrst"));

    s = {
        TEXT('!'),
        TEXT('@'),
        TEXT('#'),
        TEXT('$'),
    };

    CHECK_EQ(s.Size(), 4);
    CHECK_STR_EQ(s.CStr(), TEXT("!@#$"));
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

STRING_TEST_CASE(String, Pop) {
  struct PopTestCase {
    const CHAR* original;
    const SizeType count;
    const CHAR* expected;
  };

  static constexpr PopTestCase test_cases[]{
      PopTestCase{TEXT(""), 0, TEXT("")},
      PopTestCase{TEXT("abcdef"), 6, TEXT("")},
      PopTestCase{TEXT("abcdef"), 3, TEXT("abc")},
      PopTestCase{TEXT("this is loooong string!!!!!!!!!!!!!!!!!!"), 10,
                  TEXT("this is loooong string!!!!!!!!")},
      PopTestCase{TEXT("!#$!(()%&][[:;*+]])"), 10, TEXT("!#$!(()%&")},
  };

  for (const auto& test : test_cases) {
    String s(test.original);
    for (SizeType i = 0; i < test.count; ++i) {
      s.Pop();
    }
    CHECK_EQ(s.Size(), CharTraits::length(test.expected));
    CHECK_STR_EQ(s.CStr(), test.expected);
  }
}

STRING_TEST_CASE(String, Remove_Index_Count) {
  struct RICTestCase {
    const CHAR* original;
    const SizeType index;
    const SizeType count;
    const CHAR* expected;
  };
  static constexpr RICTestCase test_cases[]{
      RICTestCase{TEXT(""), 0, 0, TEXT("")},
      RICTestCase{TEXT(""), 0, String::kNpos, TEXT("")},
      RICTestCase{TEXT("A"), 0, 0, TEXT("A")},
      RICTestCase{TEXT("A"), 0, 1, TEXT("")},
      RICTestCase{TEXT("A"), 0, 2, TEXT("")},
      RICTestCase{TEXT("A"), 0, String::kNpos, TEXT("")},
      RICTestCase{TEXT("ABCDE"), 0, 0, TEXT("ABCDE")},
      RICTestCase{TEXT("ABCDE"), 0, 1, TEXT("BCDE")},
      RICTestCase{TEXT("ABCDE"), 0, 2, TEXT("CDE")},
      RICTestCase{TEXT("ABCDE"), 0, 4, TEXT("E")},
      RICTestCase{TEXT("ABCDE"), 0, 5, TEXT("")},
      RICTestCase{TEXT("ABCDE"), 0, 6, TEXT("")},
      RICTestCase{TEXT("ABCDE"), 0, String::kNpos, TEXT("")},
      RICTestCase{TEXT("ABCDE"), 1, 0, TEXT("ABCDE")},
      RICTestCase{TEXT("ABCDE"), 1, 1, TEXT("ACDE")},
      RICTestCase{TEXT("ABCDE"), 1, 2, TEXT("ADE")},
      RICTestCase{TEXT("ABCDE"), 1, 3, TEXT("AE")},
      RICTestCase{TEXT("ABCDE"), 1, 4, TEXT("A")},
      RICTestCase{TEXT("ABCDE"), 1, 5, TEXT("A")},
      RICTestCase{TEXT("ABCDE"), 1, String::kNpos, TEXT("A")},
      RICTestCase{TEXT("ABCDE"), 2, 0, TEXT("ABCDE")},
      RICTestCase{TEXT("ABCDE"), 2, 1, TEXT("ABDE")},
      RICTestCase{TEXT("ABCDE"), 2, 2, TEXT("ABE")},
      RICTestCase{TEXT("ABCDE"), 2, 3, TEXT("AB")},
      RICTestCase{TEXT("ABCDE"), 2, 10, TEXT("AB")},
      RICTestCase{TEXT("ABCDE"), 2, String::kNpos, TEXT("AB")},
      RICTestCase{TEXT("ABCDE"), 4, 0, TEXT("ABCDE")},
      RICTestCase{TEXT("ABCDE"), 4, 1, TEXT("ABCD")},
      RICTestCase{TEXT("ABCDE"), 4, 2, TEXT("ABCD")},
      RICTestCase{TEXT("ABCDE"), 4, String::kNpos, TEXT("ABCD")},
      RICTestCase{TEXT("ABCDE"), 3, 2, TEXT("ABC")},
      RICTestCase{TEXT("ABCDE"), 3, 3, TEXT("ABC")},
      RICTestCase{TEXT("ABCDE"), 3, 10, TEXT("ABC")},
      RICTestCase{TEXT("ABCDE"), 5, 0, TEXT("ABCDE")},
      RICTestCase{TEXT("ABCDE"), 5, String::kNpos, TEXT("ABCDE")},
      RICTestCase{TEXT("AAAAA"), 1, 3, TEXT("AA")},
      RICTestCase{TEXT("AAAAA"), 0, 4, TEXT("A")},
      RICTestCase{TEXT("AAAAA"), 2, 2, TEXT("AAA")},
      RICTestCase{TEXT("A B C"), 1, 1, TEXT("AB C")},
      RICTestCase{TEXT("A-B-C"), 1, 3, TEXT("AC")},
      RICTestCase{TEXT("!!@@##"), 2, 2, TEXT("!!##")},
      RICTestCase{TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0, 10,
                  TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ")},
      RICTestCase{TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 10, 10,
                  TEXT("0123456789KLMNOPQRSTUVWXYZ")},
      RICTestCase{TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 20, 100,
                  TEXT("0123456789ABCDEFGHIJ")},
      RICTestCase{TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 35, 1,
                  TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXY")},
      RICTestCase{TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 36, 0,
                  TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ")},
      RICTestCase{TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0,
                  String::kNpos, TEXT("")},
  };

  for (const auto& test : test_cases) {
    String s(test.original);

    s.Remove(test.index, test.count);

    const auto size = CharTraits::length(test.expected);
    CHECK_EQ(s.Size(), size);
    CHECK_STR_EQ(s.CStr(), test.expected);
  }
}

STRING_TEST_CASE(String, Remove_Pos) {
  struct RITestCase {
    const CHAR* original;
    const SizeType index;
    const CHAR* expected;
  };
  {
    static constexpr RITestCase test_cases[]{
        {TEXT("A"), 0, TEXT("")},
        {TEXT("ABCDE"), 0, TEXT("BCDE")},
        {TEXT("ABCDE"), 1, TEXT("ACDE")},
        {TEXT("ABCDE"), 2, TEXT("ABDE")},
        {TEXT("ABCDE"), 3, TEXT("ABCE")},
        {TEXT("ABCDE"), 4, TEXT("ABCD")},
        {TEXT("AAAAA"), 2, TEXT("AAAA")},
        {TEXT("A B C"), 1, TEXT("AB C")},
        {TEXT("A-B-C"), 1, TEXT("AB-C")},
        {TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0,
         TEXT("123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ")},
        {TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 10,
         TEXT("0123456789BCDEFGHIJKLMNOPQRSTUVWXYZ")},
        {TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 35,
         TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXY")},
    };

    for (const auto& test : test_cases) {
      String str(test.original);

      const auto it = str.Remove(str.begin() + test.index);

      CHECK_EQ(it, str.begin() + test.index);
      CHECK_STR_EQ(str.CStr(), test.expected);
    }
  }

  {
    static constexpr RITestCase test_cases[]{
        {TEXT("A"), 0, TEXT("")},
        {TEXT("ABCDE"), 4, TEXT("ABCD")},
        {TEXT("AAAAA"), 4, TEXT("AAAA")},
        {TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 35,
         TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXY")},
    };

    for (const auto& test : test_cases) {
      String str(test.original);

      const auto it = str.Remove(str.begin() + test.index);

      CHECK_EQ(it, str.end());
      CHECK_STR_EQ(str.CStr(), test.expected);
    }
  }
}

STRING_TEST_CASE(String, Remove_Iterator_Range) {
  struct RIRTestCase {
    const CHAR* original;
    const SizeType first;
    const SizeType last;
    const CHAR* expected;
    const SizeType returned_index;
    const bool returns_end;
  };

  static constexpr RIRTestCase test_cases[]{
      {TEXT("ABCDE"), 0, 0, TEXT("ABCDE"), 0, false},
      {TEXT("ABCDE"), 2, 2, TEXT("ABCDE"), 2, false},
      {TEXT("ABCDE"), 5, 5, TEXT("ABCDE"), 5, true},
      {TEXT("ABCDE"), 0, 1, TEXT("BCDE"), 0, false},
      {TEXT("ABCDE"), 1, 2, TEXT("ACDE"), 1, false},
      {TEXT("ABCDE"), 4, 5, TEXT("ABCD"), 4, true},
      {TEXT("ABCDE"), 0, 2, TEXT("CDE"), 0, false},
      {TEXT("ABCDE"), 0, 5, TEXT(""), 0, true},
      {TEXT("ABCDE"), 1, 3, TEXT("ADE"), 1, false},
      {TEXT("ABCDE"), 1, 4, TEXT("AE"), 1, false},
      {TEXT("ABCDE"), 2, 4, TEXT("ABE"), 2, false},
      {TEXT("ABCDE"), 2, 5, TEXT("AB"), 2, true},
      {TEXT("ABCDE"), 3, 5, TEXT("ABC"), 3, true},
      {TEXT("AAAAA"), 1, 4, TEXT("AA"), 1, false},
      {TEXT("A B C"), 1, 4, TEXT("AC"), 1, false},
      {TEXT("!!@@##"), 2, 4, TEXT("!!##"), 2, false},
      {TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0, 10,
       TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0, false},
      {TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 10, 20,
       TEXT("0123456789KLMNOPQRSTUVWXYZ"), 10, false},
      {TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 20, 36,
       TEXT("0123456789ABCDEFGHIJ"), 20, true},
      {TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0, 36, TEXT(""), 0, true},
  };

  for (const auto& test : test_cases) {
    String s(test.original);
    const auto it = s.Remove(s.begin() + test.first, s.begin() + test.last);
    CHECK_STR_EQ(s.CStr(), test.expected);
    if (test.returns_end) {
      CHECK_EQ(it, s.end());
    } else {
      CHECK_EQ(it, s.begin() + test.returned_index);
    }
  }
}

namespace {
struct AppendTestCase {
  const CHAR* text;
  const CHAR* expected;
};

#if 0
static constexpr AppendTestCase kAppendTestCases[]{
    AppendTestCase{TEXT("AB"), TEXT("AB")},
    AppendTestCase{TEXT(""), TEXT("AB")},
    AppendTestCase{TEXT("CD"), TEXT("ABCD")},
    AppendTestCase{TEXT(""), TEXT("ABCD")},
    AppendTestCase{TEXT("EFG"), TEXT("ABCDEFG")},
    AppendTestCase{TEXT(" "), TEXT("ABCDEFG ")},
    AppendTestCase{TEXT("123"), TEXT("ABCDEFG 123")},
    AppendTestCase{TEXT("!@#"), TEXT("ABCDEFG 123!@#")},
    AppendTestCase{TEXT("aa"), TEXT("ABCDEFG 123!@#aa")},
    AppendTestCase{TEXT("bb"), TEXT("ABCDEFG 123!@#aabb")},
    AppendTestCase{TEXT("ccc"), TEXT("ABCDEFG 123!@#aabbccc")},
    AppendTestCase{TEXT("\n"), TEXT("ABCDEFG 123!@#aabbccc\n")},
    AppendTestCase{TEXT("\t"), TEXT("ABCDEFG 123!@#aabbccc\n\t")},
    AppendTestCase{TEXT("LONG_LONG_LONG_TEXT"),
                   TEXT("ABCDEFG 123!@#aabbccc\n\tLONG_LONG_LONG_TEXT")},
    AppendTestCase{
        TEXT("0123456789"),
        TEXT("ABCDEFG 123!@#aabbccc\n\tLONG_LONG_LONG_TEXT0123456789")},
    AppendTestCase{TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
                   TEXT("ABCDEFG 123!@#aabbccc\n\tLONG_LONG_LONG_TEXT0123456789"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ")},
    AppendTestCase{
        TEXT("abcdefghijklmnopqrstuvwxyz"),
        TEXT("ABCDEFG 123!@#aabbccc\n\tLONG_LONG_LONG_TEXT0123456789"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")},
    AppendTestCase{
        TEXT("___"),
        TEXT("ABCDEFG 123!@#aabbccc\n\tLONG_LONG_LONG_TEXT0123456789"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz___")},
    AppendTestCase{
        TEXT("END"),
        TEXT("ABCDEFG 123!@#aabbccc\n\tLONG_LONG_LONG_TEXT0123456789"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz___END")},
};
#else
static constexpr AppendTestCase kAppendTestCases[]{
    AppendTestCase{TEXT("AASDFASDFASDFASDFASDFASDB"),
                   TEXT("AASDFASDFASDFASDFASDFASDB")},
    AppendTestCase{TEXT(""), TEXT("AASDFASDFASDFASDFASDFASDB")},
    AppendTestCase{TEXT("CD"), TEXT("AASDFASDFASDFASDFASDFASDBCD")},
    AppendTestCase{TEXT(""), TEXT("AASDFASDFASDFASDFASDFASDBCD")},
    AppendTestCase{TEXT("EFG"), TEXT("AASDFASDFASDFASDFASDFASDBCDEFG")},
    AppendTestCase{TEXT(" "), TEXT("AASDFASDFASDFASDFASDFASDBCDEFG ")},
    AppendTestCase{TEXT("123"), TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123")},
    AppendTestCase{TEXT("!@#"), TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#")},
    AppendTestCase{TEXT("aa"), TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aa")},
    AppendTestCase{TEXT("bb"),
                   TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aabb")},
    AppendTestCase{TEXT("ccc"),
                   TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aabbccc")},
    AppendTestCase{TEXT("\n"),
                   TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aabbccc\n")},
    AppendTestCase{TEXT("\t"),
                   TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aabbccc\n\t")},
    AppendTestCase{TEXT("LONG_LONG_LONG_TEXT"),
                   TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aabbccc\n\t"
                        "LONG_LONG_LONG_TEXT")},
    AppendTestCase{TEXT("0123456789"),
                   TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aabbccc\n\t"
                        "LONG_LONG_LONG_TEXT0123456789")},
    AppendTestCase{TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
                   TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aabbccc\n\t"
                        "LONG_LONG_LONG_TEXT0123456789"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ")},
    AppendTestCase{
        TEXT("abcdefghijklmnopqrstuvwxyz"),
        TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aabbccc\n\t"
             "LONG_LONG_LONG_TEXT0123456789"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")},
    AppendTestCase{
        TEXT("___"),
        TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aabbccc\n\t"
             "LONG_LONG_LONG_TEXT0123456789"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz___")},
    AppendTestCase{
        TEXT("END"),
        TEXT("AASDFASDFASDFASDFASDFASDBCDEFG 123!@#aabbccc\n\t"
             "LONG_LONG_LONG_TEXT0123456789"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz___END")},
};
#endif
}  // namespace

STRING_TEST_CASE(String, Append) {
  String s1;
  String s2;
  String s3;
  String s4;
  String s5;
  for (const auto& test : kAppendTestCases) {
    const auto text_size = CharTraits::length(test.text);
    const auto expected_size = CharTraits::length(test.expected);
    {
      s1.Append(test.text, text_size);
      CHECK_EQ(s1.Size(), expected_size);
      CHECK_STR_EQ(s1.CStr(), test.expected);
    }
    {
      s2.Append(test.text);
      CHECK_EQ(s2.Size(), expected_size);
      CHECK_STR_EQ(s2.CStr(), test.expected);
    }
    {
      std::basic_string<CHAR> std_str(test.text, text_size);
      s3.Append(std_str);
      CHECK_EQ(s3.Size(), expected_size);
      CHECK_STR_EQ(s3.CStr(), test.expected);
    }
    {
      std::basic_string_view<CHAR> sv(test.text, text_size);
      s4.Append(sv);
      CHECK_EQ(s4.Size(), expected_size);
      CHECK_STR_EQ(s4.CStr(), test.expected);
    }
    {
      String other(test.text, text_size);
      s5.Append(other);
      CHECK_EQ(s5.Size(), expected_size);
      CHECK_STR_EQ(s5.CStr(), test.expected);
    }
  }

  {
    String s;
    s.Append({
        TEXT('a'), TEXT('b'), TEXT('c'), TEXT('d'), TEXT('e'),
        TEXT('f'), TEXT('g'), TEXT('h'), TEXT('i'), TEXT('j'),
        TEXT('k'), TEXT('l'), TEXT('m'), TEXT('n'), TEXT('o'),
        TEXT('p'), TEXT('q'), TEXT('r'), TEXT('s'), TEXT('t'),
    });
    CHECK_EQ(s.Size(), 20);
    CHECK_STR_EQ(s.CStr(), TEXT("abcdefghijklmnopqrst"));
    s.Append({
        TEXT('1'),
        TEXT('2'),
        TEXT('3'),
        TEXT('4'),
        TEXT('5'),
    });
    CHECK_EQ(s.Size(), 25);
    CHECK_STR_EQ(s.CStr(), TEXT("abcdefghijklmnopqrst12345"));
  }
}

STRING_TEST_CASE(String, AppendSelf) {
  String s;

  s.Append(s);
  CHECK_EQ(s.Size(), 0);
  CHECK_STR_EQ(s.CStr(), TEXT(""));

  s.Append(TEXT("ABCDEF")).Append(s);
  CHECK_EQ(s.Size(), 12);
  CHECK_STR_EQ(s.CStr(), TEXT("ABCDEFABCDEF"));

  s.Append(s);
  CHECK_EQ(s.Size(), 24);
  CHECK_STR_EQ(s.CStr(), TEXT("ABCDEFABCDEFABCDEFABCDEF"));

  s.Append(s);
  CHECK_EQ(s.Size(), 48);
  CHECK_STR_EQ(s.CStr(),
               TEXT("ABCDEFABCDEFABCDEFABCDEFABCDEFABCDEFABCDEFABCDEF"));
}

STRING_TEST_CASE(String, Append_Substr) {
  struct AppendSubstrTestCase {
    const CHAR* text;
    const SizeType pos;
    const SizeType count;
    const CHAR* expected;
  };

#if 1
  static constexpr AppendSubstrTestCase test_cases[]{
      {TEXT("ABCDEFG"), 0, 2, TEXT("AB")},
      {TEXT("ABCDEFG"), 2, 3, TEXT("ABCDE")},
      {TEXT("ABCDEFG"), 5, 2, TEXT("ABCDEFG")},
      {TEXT("ABCDEFG"), 7, 0, TEXT("ABCDEFG")},
      {TEXT("ABCDEFG"), 0, String::kNpos, TEXT("ABCDEFGABCDEFG")},
      {TEXT("ABCDEFG"), 2, String::kNpos, TEXT("ABCDEFGABCDEFGCDEFG")},
      {TEXT("123456789"), 3, 3, TEXT("ABCDEFGABCDEFGCDEFG456")},
      {TEXT("123456789"), 8, 1, TEXT("ABCDEFGABCDEFGCDEFG4569")},
      {TEXT("123456789"), 8, 100, TEXT("ABCDEFGABCDEFGCDEFG45699")},
      {TEXT("LONG_LONG_LONG_TEXT"), 5, 4, TEXT("ABCDEFGABCDEFGCDEFG45699LONG")},
      {TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 10, 10,
       TEXT("ABCDEFGABCDEFGCDEFG45699LONGKLMNOPQRST")},
  };
#else
  static constexpr AppendSubstrTestCase test_cases[]{
      {TEXT("AASDFASDFASDFASDFASDFASDB"), 0, 5, TEXT("AASDF")},
      {TEXT("ABCDEFG"), 2, 3, TEXT("AASDFCDE")},
      {TEXT("ABCDEFG"), 5, 2, TEXT("AASDFCDEFG")},
      {TEXT("ABCDEFG"), 7, 0, TEXT("AASDFCDEFG")},
      {TEXT("ABCDEFG"), 0, String::kNpos, TEXT("AASDFCDEFGABCDEFG")},
      {TEXT("ABCDEFG"), 2, String::kNpos, TEXT("AASDFCDEFGABCDEFGCDEFG")},
      {TEXT("123456789"), 3, 3, TEXT("AASDFCDEFGABCDEFGCDEFG456")},
      {TEXT("123456789"), 8, 1, TEXT("AASDFCDEFGABCDEFGCDEFG4569")},
      {TEXT("123456789"), 8, 100, TEXT("AASDFCDEFGABCDEFGCDEFG45699")},
      {TEXT("LONG_LONG_LONG_TEXT"), 5, 4,
       TEXT("AASDFCDEFGABCDEFGCDEFG45699LONG")},
      {TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 10, 10,
       TEXT("AASDFCDEFGABCDEFGCDEFG45699LONGKLMNOPQRST")},
  };
#endif

  String s1;
  String s2;
  String s3;

  for (const auto& test : test_cases) {
    const auto text_size = CharTraits::length(test.text);
    const auto expected_size = CharTraits::length(test.expected);
    {
      String other(test.text, text_size);
      s1.Append(other, test.pos, test.count);
      CHECK_EQ(s1.Size(), expected_size);
      CHECK_STR_EQ(s1.CStr(), test.expected);
    }
    {
      std::basic_string_view<CHAR> sv(test.text, text_size);
      s2.Append(sv, test.pos, test.count);
      CHECK_EQ(s2.Size(), expected_size);
      CHECK_STR_EQ(s2.CStr(), test.expected);
    }
    {
      std::basic_string<CHAR> std_str(test.text, text_size);
      s3.Append(std_str, test.pos, test.count);
      CHECK_EQ(s3.Size(), expected_size);
      CHECK_STR_EQ(s3.CStr(), test.expected);
    }
  }
}

STRING_TEST_CASE(String, Append_InputIt) {
  String s;

  for (const auto& test : kAppendTestCases) {
    const auto expected_size = CharTraits::length(test.expected);

    std::basic_istringstream<CHAR> stream(test.text);
    stream >> std::noskipws;

    std::istreambuf_iterator<CHAR> first(stream);
    std::istreambuf_iterator<CHAR> last;

    s.Append(first, last);

    CHECK_EQ(s.Size(), expected_size);
    CHECK_STR_EQ(s.CStr(), test.expected);
  }
}

STRING_TEST_CASE(String, Append_ForwardIt) {
  String s1;
  String s2;
  String s3;

  for (const auto& test : kAppendTestCases) {
    const auto text_size = CharTraits::length(test.text);
    const auto expected_size = CharTraits::length(test.expected);
    {
      String a(test.text, text_size);
      s1.Append(a.begin(), a.end());
      CHECK_EQ(s1.Size(), expected_size);
      CHECK_STR_EQ(s1.CStr(), test.expected);
    }
    {
      std::list<CHAR> l(test.text, test.text + text_size);
      s2.Append(l.begin(), l.end());
      CHECK_EQ(s2.Size(), expected_size);
      CHECK_STR_EQ(s2.CStr(), test.expected);
    }
    {
      std::vector<CHAR> v(test.text, test.text + text_size);
      s3.Append(v.begin(), v.end());
      CHECK_EQ(s3.Size(), expected_size);
      CHECK_STR_EQ(s3.CStr(), test.expected);
    }
  }
}

STRING_TEST_CASE(String, Append_Count_Char) {
  struct ACCTestCase {
    const SizeType count;
    const CHAR ch;
    const CHAR* expected;
  };

  static constexpr ACCTestCase test_cases[]{
      ACCTestCase{0, TEXT('A'), TEXT("")},
      ACCTestCase{0, TEXT('Z'), TEXT("")},
      ACCTestCase{1, TEXT('A'), TEXT("A")},
      ACCTestCase{1, TEXT('B'), TEXT("AB")},
      ACCTestCase{2, TEXT('C'), TEXT("ABCC")},
      ACCTestCase{3, TEXT('D'), TEXT("ABCCDDD")},
      ACCTestCase{1, TEXT(' '), TEXT("ABCCDDD ")},
      ACCTestCase{2, TEXT('!'), TEXT("ABCCDDD !!")},
      ACCTestCase{1, TEXT('\n'), TEXT("ABCCDDD !!\n")},
      ACCTestCase{1, TEXT('\t'), TEXT("ABCCDDD !!\n\t")},
      ACCTestCase{5, TEXT('X'), TEXT("ABCCDDD !!\n\tXXXXX")},
      ACCTestCase{1, TEXT('X'), TEXT("ABCCDDD !!\n\tXXXXXX")},
      ACCTestCase{4, TEXT('Y'), TEXT("ABCCDDD !!\n\tXXXXXXYYYY")},
      ACCTestCase{10, TEXT('0'), TEXT("ABCCDDD !!\n\tXXXXXXYYYY0000000000")},
      ACCTestCase{26, TEXT('Z'),
                  TEXT("ABCCDDD !!\n\tXXXXXXYYYY0000000000"
                       "ZZZZZZZZZZZZZZZZZZZZZZZZZZ")},
      ACCTestCase{64, TEXT('Q'),
                  TEXT("ABCCDDD !!\n\tXXXXXXYYYY0000000000"
                       "ZZZZZZZZZZZZZZZZZZZZZZZZZZ"
                       "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                       "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ")},
      ACCTestCase{3, TEXT('@'),
                  TEXT("ABCCDDD !!\n\tXXXXXXYYYY0000000000"
                       "ZZZZZZZZZZZZZZZZZZZZZZZZZZ"
                       "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                       "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                       "@@@")},
      ACCTestCase{8, TEXT('#'),
                  TEXT("ABCCDDD !!\n\tXXXXXXYYYY0000000000"
                       "ZZZZZZZZZZZZZZZZZZZZZZZZZZ"
                       "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                       "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                       "@@@########")},
      ACCTestCase{32, TEXT('W'),
                  TEXT("ABCCDDD !!\n\tXXXXXXYYYY0000000000"
                       "ZZZZZZZZZZZZZZZZZZZZZZZZZZ"
                       "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                       "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                       "@@@########"
                       "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW")},
  };

  String s;
  SizeType expected_size = 0;

  for (const auto& test : test_cases) {
    s.Append(test.count, test.ch);
    expected_size += test.count;
    CHECK_EQ(s.Size(), expected_size);
    CHECK_STR_EQ(s.CStr(), test.expected);
  }
}

namespace {
struct InsertTestCase {
  const SizeType insert_pos;
  const CHAR* insert_data;
  const CHAR* expected;
};

#if 1
static constexpr InsertTestCase kInsertTestCases[]{
    InsertTestCase{0, TEXT(""), TEXT("")},
    InsertTestCase{0, TEXT("A"), TEXT("A")},
    InsertTestCase{1, TEXT("B"), TEXT("AB")},
    InsertTestCase{1, TEXT("X"), TEXT("AXB")},
    InsertTestCase{0, TEXT("!"), TEXT("!AXB")},
    InsertTestCase{4, TEXT("?"), TEXT("!AXB?")},
    InsertTestCase{2, TEXT("123"), TEXT("!A123XB?")},
    InsertTestCase{0, TEXT("START"), TEXT("START!A123XB?")},
    InsertTestCase{13, TEXT("END"), TEXT("START!A123XB?END")},
    InsertTestCase{5, TEXT("---"), TEXT("START---!A123XB?END")},
    InsertTestCase{8, TEXT("MID"), TEXT("START---MID!A123XB?END")},
    InsertTestCase{22, TEXT(""), TEXT("START---MID!A123XB?END")},
    InsertTestCase{0, TEXT("++"), TEXT("++START---MID!A123XB?END")},
    InsertTestCase{24, TEXT("--"), TEXT("++START---MID!A123XB?END--")},
    InsertTestCase{10, TEXT("INSERT"),
                   TEXT("++START---INSERTMID!A123XB?END--")},
    InsertTestCase{3, TEXT("Q"), TEXT("++SQTART---INSERTMID!A123XB?END--")},
    InsertTestCase{33, TEXT("TAIL"),
                   TEXT("++SQTART---INSERTMID!A123XB?END--TAIL")},
    InsertTestCase{20, TEXT("CENTER"),
                   TEXT("++SQTART---INSERTMIDCENTER!A123XB?END--TAIL")},
};
#else
static constexpr InsertTestCase kInsertTestCases[]{
    InsertTestCase{0, TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"),
                   TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")},

    InsertTestCase{0, TEXT("START-"),
                   TEXT("START-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")},

    InsertTestCase{42, TEXT("-END"),
                   TEXT("START-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-END")},

    InsertTestCase{
        6, TEXT("INSERT-"),
        TEXT("START-INSERT-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-END")},

    InsertTestCase{
        0, TEXT("+++"),
        TEXT("+++START-INSERT-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-END")},

    InsertTestCase{
        56, TEXT("---"),
        TEXT("+++START-INSERT-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-END---")},

    InsertTestCase{
        20, TEXT("MID"),
        TEXT("+++START-INSERT-ABCDMIDEFGHIJKLMNOPQRSTUVWXYZ0123456789-END---")},

    InsertTestCase{10, TEXT("1234567890"),
                   TEXT("+++START-I1234567890NSERT-"
                        "ABCDMIDEFGHIJKLMNOPQRSTUVWXYZ0123456789-END---")},

    InsertTestCase{0, TEXT("LONG-LONG-LONG-"),
                   TEXT("LONG-LONG-LONG-+++START-I1234567890NSERT-"
                        "ABCDMIDEFGHIJKLMNOPQRSTUVWXYZ0123456789-END---")},

    InsertTestCase{87, TEXT("-TAIL"),
                   TEXT("LONG-LONG-LONG-+++START-I1234567890NSERT-"
                        "ABCDMIDEFGHIJKLMNOPQRSTUVWXYZ0123456789-END----TAIL")},

    InsertTestCase{50, TEXT("[CENTER]"),
                   TEXT("LONG-LONG-LONG-+++START-I1234567890NSERT-ABCDMIDEF["
                        "CENTER]GHIJKLMNOPQRSTUVWXYZ0123456789-END----TAIL")},
};

#endif
}  // namespace

STRING_TEST_CASE(String, Insert) {
  String s1;
  String s2;
  String s3;
  String s4;
  String s5;
  String s6;
  SizeType expected_size = 0;

  for (const auto& test : kInsertTestCases) {
    const auto insert_len = CharTraits::length(test.insert_data);
    expected_size += insert_len;
    {
      auto it = s1.Insert(s1.begin() + test.insert_pos, test.insert_data,
                          test.insert_data + insert_len);
      CHECK_EQ(it, s1.begin() + test.insert_pos);
      CHECK_EQ(*it, test.insert_data[0]);
      CHECK_EQ(s1.Size(), expected_size);
      CHECK_STR_EQ(s1.CStr(), test.expected);
    }
    {
      std::list<CHAR> l(test.insert_data, test.insert_data + insert_len);
      auto it = s2.Insert(s2.begin() + test.insert_pos, l.begin(), l.end());
      CHECK_EQ(it, s2.begin() + test.insert_pos);
      CHECK_EQ(*it, test.insert_data[0]);
      CHECK_EQ(s2.Size(), expected_size);
      CHECK_STR_EQ(s2.CStr(), test.expected);
    }
    {
      s3.Insert(test.insert_pos, test.insert_data);
      CHECK_EQ(s3.Size(), expected_size);
      CHECK_STR_EQ(s3.CStr(), test.expected);
    }
    {
      s4.Insert(test.insert_pos, test.insert_data, insert_len);
      CHECK_EQ(s4.Size(), expected_size);
      CHECK_STR_EQ(s4.CStr(), test.expected);
    }
    {
      String s(test.insert_data, insert_len);
      s5.Insert(test.insert_pos, s);
      CHECK_EQ(s5.Size(), expected_size);
      CHECK_STR_EQ(s5.CStr(), test.expected);
    }
    {
      std::basic_string_view<CHAR> sv(test.insert_data, insert_len);
      s6.Insert(test.insert_pos, sv);
      CHECK_EQ(s6.Size(), expected_size);
      CHECK_STR_EQ(s6.CStr(), test.expected);
    }
  }
}

STRING_TEST_CASE(String, InsertInputIt) {
  String s;

  for (const auto& test : kInsertTestCases) {
    const auto expected_size = CharTraits::length(test.expected);

    std::basic_istringstream<CHAR> stream(test.insert_data);
    stream >> std::noskipws;

    std::istreambuf_iterator<CHAR> first(stream);
    std::istreambuf_iterator<CHAR> last;

    auto it = s.Insert(s.begin() + test.insert_pos, first, last);

    CHECK_EQ(it, s.begin() + test.insert_pos);
    CHECK_EQ(*it, test.insert_data[0]);

    CHECK_EQ(s.Size(), expected_size);
    CHECK_STR_EQ(s.CStr(), test.expected);
  }
}

STRING_TEST_CASE(String, Insert_Char) {
  IGNORE_RESULT();

  String s;

  auto it = s.Insert(s.begin(), TEXT('S'));
  CHECK_EQ(it, s.begin());
  CHECK_EQ(*it, TEXT('S'));
  CHECK_STR_EQ(s.CStr(), TEXT("S"));

  const SizeType N = AXIO_ARRAY_SIZE(kChars);
  for (int i = 0; i < 3; ++i) {
    for (SizeType j = 0; j < N; ++j) {
      const SizeType pos = j / 2;
      it = s.Insert(s.begin() + pos, kChars[j % N]);
      CHECK_EQ(it, s.begin() + pos);
      CHECK_EQ(*it, kChars[j % N]);
    }
  }

  CHECK_EQ(s.Size(), 61);
  CHECK_STR_EQ(
      s.CStr(),
      TEXT("B#Q@Z!P$N&2v9k0m7x3aB#Q@Z!P$N&2v9k0m7x3aB#Q@Z!P$N&2v9k0m7x3aS"));
}

STRING_TEST_CASE(String, Insert_Count_Char) {
  struct ICCTestCase {
    const SizeType pos;
    const SizeType count;
    const CHAR value;
    const CHAR* expected;
  };

  static constexpr ICCTestCase test_cases[]{
      ICCTestCase{0, 0, TEXT('A'), TEXT("")},
      ICCTestCase{0, 1, TEXT('S'), TEXT("S")},
      ICCTestCase{0, 2, TEXT('a'), TEXT("aaS")},
      ICCTestCase{1, 3, TEXT('B'), TEXT("aBBBaS")},
      ICCTestCase{6, 2, TEXT('3'), TEXT("aBBBaS33")},
      ICCTestCase{2, 4, TEXT('#'), TEXT("aB####BBaS33")},
      ICCTestCase{0, 1, TEXT('x'), TEXT("xaB####BBaS33")},
      ICCTestCase{13, 5, TEXT('Q'), TEXT("xaB####BBaS33QQQQQ")},
      ICCTestCase{7, 3, TEXT('7'), TEXT("xaB####777BBaS33QQQQQ")},
      ICCTestCase{10, 2, TEXT('@'), TEXT("xaB####777@@BBaS33QQQQQ")},
      ICCTestCase{0, 6, TEXT('m'), TEXT("mmmmmmxaB####777@@BBaS33QQQQQ")},
      ICCTestCase{29, 1, TEXT('Z'), TEXT("mmmmmmxaB####777@@BBaS33QQQQQZ")},
      ICCTestCase{5, 8, TEXT('0'),
                  TEXT("mmmmm00000000mxaB####777@@BBaS33QQQQQZ")},
      ICCTestCase{20, 4, TEXT('!'),
                  TEXT("mmmmm00000000mxaB###!!!!#777@@BBaS33QQQQQZ")},
      ICCTestCase{0, 3, TEXT('k'),
                  TEXT("kkkmmmmm00000000mxaB###!!!!#777@@BBaS33QQQQQZ")},
      ICCTestCase{15, 2, TEXT('P'),
                  TEXT("kkkmmmmm0000000PP0mxaB###!!!!#777@@BBaS33QQQQQZ")},
  };

  String s;
  SizeType expected_size = 0;
  for (const auto& test : test_cases) {
    auto it = s.Insert(s.begin() + test.pos, test.count, test.value);
    CHECK_EQ(it, s.begin() + test.pos);
    expected_size += test.count;
    CHECK_EQ(s.Size(), expected_size);
    CHECK_STR_EQ(s.CStr(), test.expected);
  }
}

STRING_TEST_CASE(String, Insert_Pos_Count) {
  struct IPCTestCase {
    const CHAR* initial;
    const SizeType index;
    const CHAR* insert_str;
    const SizeType pos;
    const SizeType count;
    const CHAR* expected;
  };

  static constexpr IPCTestCase test_cases[]{
      IPCTestCase{TEXT(""), 0, TEXT("ABC"), 0, 0, TEXT("")},
      IPCTestCase{TEXT("HELLO"), 0, TEXT("ABC"), 0, 1, TEXT("AHELLO")},
      IPCTestCase{TEXT("HELLO"), 5, TEXT("ABC"), 1, 2, TEXT("HELLOBC")},
      IPCTestCase{TEXT("HELLO"), 2, TEXT("ABCDEFG"), 2, 3, TEXT("HECDELLO")},
      IPCTestCase{TEXT("12345"), 3, TEXT("ABCDE"), 0, 5, TEXT("123ABCDE45")},
      IPCTestCase{TEXT("STARTEND"), 5, TEXT("----"), 0, 4,
                  TEXT("START----END")},
      IPCTestCase{TEXT("ABCDE"), 1, TEXT("123456789"), 3, 2, TEXT("A45BCDE")},
      IPCTestCase{TEXT("XYZ"), 0, TEXT("HELLO"), 1, 3, TEXT("ELLXYZ")},
      IPCTestCase{TEXT("TAIL"), 4, TEXT("123456"), 2, kNpos, TEXT("TAIL3456")},
      IPCTestCase{TEXT("MID"), 1, TEXT("ABCDEFG"), 4, 100, TEXT("MEFGID")},
  };

  for (const auto& test : test_cases) {
    const auto expected_size = CharTraits::length(test.expected);
    {
      String s(test.initial);
      String insert_str(test.insert_str);
      s.Insert(test.index, insert_str, test.pos, test.count);
      CHECK_STR_EQ(s.CStr(), test.expected);
      CHECK_EQ(s.Size(), expected_size);
    }
    {
      String s(test.initial);
      std::basic_string_view<CHAR> insert_str(test.insert_str);
      s.Insert(test.index, insert_str, test.pos, test.count);
      CHECK_STR_EQ(s.CStr(), test.expected);
      CHECK_EQ(s.Size(), expected_size);
    }
  }
}

STRING_TEST_CASE(String, Comparison) {
  struct CompareTestCase {
    const CHAR* lhs;
    const CHAR* rhs;
    bool eq;
    bool ne;
    bool gt;
    bool lt;
    bool ge;
    bool le;
  };

  static constexpr CompareTestCase test_cases[] = {
      {TEXT(""), TEXT(""), true, false, false, false, true, true},
      {TEXT(""), TEXT("A"), false, true, false, true, false, true},
      {TEXT("A"), TEXT(""), false, true, true, false, true, false},

      {TEXT("A"), TEXT("A"), true, false, false, false, true, true},
      {TEXT("A"), TEXT("B"), false, true, false, true, false, true},
      {TEXT("B"), TEXT("A"), false, true, true, false, true, false},

      {TEXT("ABC"), TEXT("ABC"), true, false, false, false, true, true},
      {TEXT("ABC"), TEXT("ABD"), false, true, false, true, false, true},
      {TEXT("ABD"), TEXT("ABC"), false, true, true, false, true, false},

      {TEXT("ABC"), TEXT("AB"), false, true, true, false, true, false},
      {TEXT("AB"), TEXT("ABC"), false, true, false, true, false, true},

      {TEXT("HELLO"), TEXT("HELLO"), true, false, false, false, true, true},
      {TEXT("HELLO"), TEXT("HELL"), false, true, true, false, true, false},
      {TEXT("HELL"), TEXT("HELLO"), false, true, false, true, false, true},

      {TEXT("AAAA"), TEXT("ZZZZ"), false, true, false, true, false, true},
      {TEXT("ZZZZ"), TEXT("AAAA"), false, true, true, false, true, false},

      {TEXT("ABCDEF"), TEXT("ABCXYZ"), false, true, false, true, false, true},
      {TEXT("ABCXYZ"), TEXT("ABCDEF"), false, true, true, false, true, false},

      {TEXT("123"), TEXT("123"), true, false, false, false, true, true},
      {TEXT("123"), TEXT("124"), false, true, false, true, false, true},
      {TEXT("124"), TEXT("123"), false, true, true, false, true, false},

      {TEXT("abc"), TEXT("abc"), true, false, false, false, true, true},
      {TEXT("abc"), TEXT("abd"), false, true, false, true, false, true},
      {TEXT("abd"), TEXT("abc"), false, true, true, false, true, false},
      {TEXT("abc"), TEXT("ABC"), false, true, true, false, true, false},
      {TEXT("ABC"), TEXT("abc"), false, true, false, true, false, true},

      {TEXT("THIS_IS_A_LONG_STRING"), TEXT("THIS_IS_A_LONG_STRING"), true,
       false, false, false, true, true},
      {TEXT("THIS_IS_A_LONG_STRING"), TEXT("THIS_IS_A_LONG_STRINH"), false,
       true, false, true, false, true},
      {TEXT("THIS_IS_A_LONG_STRINH"), TEXT("THIS_IS_A_LONG_STRING"), false,
       true, true, false, true, false},

      {TEXT("!@#"), TEXT("!@#"), true, false, false, false, true, true},
      {TEXT("!@#"), TEXT("!@$"), false, true, false, true, false, true},
      {TEXT("!@$"), TEXT("!@#"), false, true, true, false, true, false},

      {TEXT(" "), TEXT(" "), true, false, false, false, true, true},
      {TEXT(" "), TEXT("  "), false, true, false, true, false, true},
      {TEXT("  "), TEXT(" "), false, true, true, false, true, false},

      {TEXT("\x01"), TEXT("\x02"), false, true, false, true, false, true},
      {TEXT("\x02"), TEXT("\x01"), false, true, true, false, true, false},

      {TEXT("ABCDE"), TEXT("ABCDF"), false, true, false, true, false, true},
      {TEXT("ABCDF"), TEXT("ABCDE"), false, true, true, false, true, false},

      {TEXT("!@#$%^&*()_+-=[]{}|;':,./<>?"),
       TEXT("!@#$%^&*()_+-=[]{}|;':,./<>?"), true, false, false, false, true,
       true},

      {TEXT("!@#$%^&*()_+-=[]{}|;':,./<>?"),
       TEXT("!@#$%^&*()_+-=[]{}|;':,./<>@"), false, true, false, true, false,
       true},
  };

  for (const auto& test : test_cases) {
    {
      String lhs(test.lhs);

      CHECK_EQ((lhs == test.rhs), test.eq);
      CHECK_EQ((lhs != test.rhs), test.ne);
      CHECK_EQ((lhs > test.rhs), test.gt);
      CHECK_EQ((lhs < test.rhs), test.lt);
      CHECK_EQ((lhs >= test.rhs), test.ge);
      CHECK_EQ((lhs <= test.rhs), test.le);
    }

    {
      String rhs(test.rhs);

      CHECK_EQ((test.lhs == rhs), test.eq);
      CHECK_EQ((test.lhs != rhs), test.ne);
      CHECK_EQ((test.lhs > rhs), test.gt);
      CHECK_EQ((test.lhs < rhs), test.lt);
      CHECK_EQ((test.lhs >= rhs), test.ge);
      CHECK_EQ((test.lhs <= rhs), test.le);
    }

    {
      String lhs(test.lhs);
      String rhs(test.rhs);

      CHECK_EQ((lhs == rhs.CStr()), test.eq);
      CHECK_EQ((lhs != rhs.CStr()), test.ne);
      CHECK_EQ((lhs > rhs.CStr()), test.gt);
      CHECK_EQ((lhs < rhs.CStr()), test.lt);
      CHECK_EQ((lhs >= rhs.CStr()), test.ge);
      CHECK_EQ((lhs <= rhs.CStr()), test.le);
    }
  }
}

STRING_TEST_CASE(String, Substr) {
  struct SubstrTestCase {
    const CHAR* input;
    SizeType pos;
    SizeType count;
    const CHAR* expected;
  };

  static constexpr SubstrTestCase test_cases[] = {
      {TEXT("HELLO"), 0, 5, TEXT("HELLO")},
      {TEXT("HELLO"), 0, 2, TEXT("HE")},
      {TEXT("HELLO"), 1, 3, TEXT("ELL")},
      {TEXT("HELLO"), 4, 1, TEXT("O")},
      {TEXT("HELLO"), 0, kNpos, TEXT("HELLO")},
      {TEXT("HELLO"), 2, kNpos, TEXT("LLO")},
      {TEXT("HELLO"), 2, 100, TEXT("LLO")},
      {TEXT("HELLO"), 4, 10, TEXT("O")},
      {TEXT("HELLO"), 5, 0, TEXT("")},
      {TEXT("HELLO"), 5, kNpos, TEXT("")},
      {TEXT(""), 0, 0, TEXT("")},
      {TEXT(""), 0, kNpos, TEXT("")},
      {TEXT("A"), 0, 1, TEXT("A")},
      {TEXT("A"), 0, kNpos, TEXT("A")},
      {TEXT("A"), 1, 0, TEXT("")},
      {TEXT("ABCDEFG"), 2, 3, TEXT("CDE")},
      {TEXT("ABCDEFG"), 3, 2, TEXT("DE")},
      {TEXT("ABCDEFG"), 4, 3, TEXT("EFG")},
      {TEXT("ABCDEFG"), 4, 0, TEXT("")},
      {TEXT("ABCDEFG"), 0, 0, TEXT("")},
      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"),
       10, 20, TEXT("AAAAAAAAAAAAAAAAAAAA")},
      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"),
       10, kNpos,
       TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")},
      {TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"), 25, 100,
       TEXT("ZABCDEFGHIJKLMNOPQRSTUVWXYZ")},
      {TEXT("ABABABABABABABABABAB"), 1, 6, TEXT("BABABA")},
      {TEXT("   HELLO WORLD   "), 3, 11, TEXT("HELLO WORLD")},
      {TEXT("!@#$%^&*()_+-="), 5, 4, TEXT("^&*(")},
      {TEXT("12345678901234567890"), 10, 5, TEXT("12345")},
      {TEXT("aAaAaAaAaAaA"), 1, 5, TEXT("AaAaA")},
  };

  for (const auto& test : test_cases) {
    String s(test.input);
    String sub = s.Substr(test.pos, test.count);

    CHECK_STR_EQ(sub.CStr(), test.expected);
    CHECK_EQ(sub.Size(), CharTraits::length(test.expected));
  }
}

STRING_TEST_CASE(String, Compare_Pos_Count_BasicString) {
  struct CPCBTestCase {
    const CHAR* lhs;
    SizeType pos1;
    SizeType count1;
    const CHAR* rhs;
    int expected_sign;
  };

  static constexpr CPCBTestCase test_cases[] = {
      {TEXT("HELLO"), 0, 5, TEXT("HELLO"), 0},
      {TEXT("HELLO"), 0, 4, TEXT("HELL"), 0},
      {TEXT("HELLO"), 1, 3, TEXT("ELL"), 0},
      {TEXT("HELLO"), 1, 3, TEXT("ELM"), -1},
      {TEXT("HELLO"), 1, 3, TEXT("ELD"), +1},

      {TEXT("HELLO"), 2, 100, TEXT("LLO"), 0},
      {TEXT("HELLO"), 2, 100, TEXT("LLP"), -1},

      {TEXT("HELLO"), 0, 3, TEXT("HELLO"), -1},
      {TEXT("HELLO"), 0, 5, TEXT("HEL"), +1},

      {TEXT("HELLO"), 5, 0, TEXT(""), 0},
      {TEXT("HELLO"), 5, 0, TEXT("A"), -1},
      {TEXT("HELLO"), 0, 0, TEXT(""), 0},
  };

  for (const auto& t : test_cases) {
    String lhs(t.lhs);
    String rhs(t.rhs);
    int res = lhs.Compare(t.pos1, t.count1, rhs);
    if (t.expected_sign == 0) {
      CHECK_EQ(res, 0);
    } else if (t.expected_sign < 0) {
      CHECK_LT(res, 0);
    } else {
      CHECK_GT(res, 0);
    }
  }
}

STRING_TEST_CASE(String, Compare_Pos_Pos_Count_BasicString) {
  struct CPPCBTestCase {
    const CHAR* lhs;
    SizeType pos1;
    SizeType count1;
    const CHAR* rhs;
    SizeType pos2;
    SizeType count2;
    int expected_sign;
  };

  static constexpr CPPCBTestCase test_cases[] = {
      {TEXT("ABCDEFG"), 2, 3, TEXT("XXCDEYY"), 2, 3, 0},
      {TEXT("ABCDEFG"), 2, 3, TEXT("XXCDFYY"), 2, 3, -1},
      {TEXT("ABCDEFG"), 2, 3, TEXT("XXCDDYY"), 2, 3, +1},
      {TEXT("ABCDEFG"), 2, 3, TEXT("XXCDEYY"), 2, kNpos, -1},
      {TEXT("ABCDEFG"), 2, 3, TEXT("XXCD"), 2, 10, +1},
      {TEXT("ABCDEFG"), 4, 100, TEXT("EFG"), 0, 3, 0},
      {TEXT("ABC"), 3, 0, TEXT("XYZ"), 3, 0, 0},
  };

  for (const auto& t : test_cases) {
    String lhs(t.lhs);

    String rhs(t.rhs);
    std::basic_string<CHAR> rhs_bs(t.rhs);
    std::basic_string_view<CHAR> rhs_bsv(t.rhs);

    {
      int res = lhs.Compare(t.pos1, t.count1, rhs, t.pos2, t.count2);
      if (t.expected_sign == 0) {
        CHECK_EQ(res, 0);
      } else if (t.expected_sign < 0) {
        CHECK_LT(res, 0);
      } else {
        CHECK_GT(res, 0);
      }
    }
    {
      int res = lhs.Compare(t.pos1, t.count1, rhs_bs, t.pos2, t.count2);
      if (t.expected_sign == 0) {
        CHECK_EQ(res, 0);
      } else if (t.expected_sign < 0) {
        CHECK_LT(res, 0);
      } else {
        CHECK_GT(res, 0);
      }
    }
    {
      int res = lhs.Compare(t.pos1, t.count1, rhs_bsv, t.pos2, t.count2);
      if (t.expected_sign == 0) {
        CHECK_EQ(res, 0);
      } else if (t.expected_sign < 0) {
        CHECK_LT(res, 0);
      } else {
        CHECK_GT(res, 0);
      }
    }
  }
}

STRING_TEST_CASE(String, Compare_Ptr_Count) {
  struct CPCTestCase {
    const CHAR* lhs;
    const CHAR* rhs;
    SizeType count;
    int expected_sign;
  };

  static constexpr CPCTestCase test_cases[] = {
      {TEXT("HELLO"), TEXT("HELLO"), 5, 0},
      {TEXT("HELLO"), TEXT("HELLOX"), 5, 0},
      {TEXT("HELLO"), TEXT("HELLX"), 5, -1},
      {TEXT("HELLO"), TEXT("HELL"), 5, +1},
      {TEXT("HELLO"), TEXT("HE"), 2, +1},
      {TEXT("HELLO"), TEXT("HF"), 2, -1},
      {TEXT("HELLO"), TEXT("XXXXX"), 0, +1},
      {TEXT(""), TEXT("XXXXX"), 0, 0},
  };

  for (const auto& t : test_cases) {
    String lhs(t.lhs);

    int res = lhs.Compare(t.rhs, t.count);
    if (t.expected_sign == 0) {
      CHECK_EQ(res, 0);
    } else if (t.expected_sign < 0) {
      CHECK_LT(res, 0);
    } else {
      CHECK_GT(res, 0);
    }
  }
}

STRING_TEST_CASE(String, Compare_StringViewLike) {
  struct CSVLTestCase {
    const CHAR* lhs;
    const CHAR* rhs;
    int expected_sign;
  };

  static constexpr CSVLTestCase test_cases[] = {
      {TEXT("HELLO"), TEXT("HELLO"), 0}, {TEXT("HELLO"), TEXT("HELL"), +1},
      {TEXT("HELL"), TEXT("HELLO"), -1}, {TEXT("ABC"), TEXT("ABD"), -1},
      {TEXT("ABD"), TEXT("ABC"), +1},
  };

  for (const auto& t : test_cases) {
    String lhs(t.lhs);
    std::basic_string_view<CHAR> view(t.rhs);

    int res1 = lhs.Compare(view);
    int res2 = lhs.Compare(0, kNpos, view);
    int res3 = lhs.Compare(0, kNpos, view, 0, kNpos);

    if (t.expected_sign == 0) {
      CHECK_EQ(res1, 0);
      CHECK_EQ(res2, 0);
      CHECK_EQ(res3, 0);
    } else if (t.expected_sign < 0) {
      CHECK_LT(res1, 0);
      CHECK_LT(res2, 0);
      CHECK_LT(res3, 0);
    } else {
      CHECK_GT(res1, 0);
      CHECK_GT(res2, 0);
      CHECK_GT(res3, 0);
    }
  }
}

STRING_TEST_CASE(String, StartsWith_EndsWith) {
  struct SETestCase {
    const CHAR* str;
    const CHAR* prefix;
    const CHAR* suffix;
    CHAR c_start;
    CHAR c_end;
    bool sw_str;
    bool ew_str;
    bool sw_char;
    bool ew_char;
  };

  static constexpr SETestCase test_cases[] = {
      {TEXT("HELLO"), TEXT("HE"), TEXT("LO"), TEXT('H'), TEXT('O'), true, true,
       true, true},
      {TEXT("HELLO"), TEXT("HELLO"), TEXT("HELLO"), TEXT('H'), TEXT('O'), true,
       true, true, true},
      {TEXT("HELLO"), TEXT("HELL"), TEXT("ELLO"), TEXT('H'), TEXT('O'), true,
       true, true, true},
      {TEXT("HELLO"), TEXT("HI"), TEXT("XO"), TEXT('X'), TEXT('X'), false,
       false, false, false},
      {TEXT("HELLO"), TEXT("HELLOO"), TEXT("HELLOO"), TEXT('H'), TEXT('O'),
       false, false, true, true},
      {TEXT(""), TEXT(""), TEXT(""), TEXT('A'), TEXT('A'), true, true, false,
       false},
      {TEXT(""), TEXT("A"), TEXT("A"), TEXT('A'), TEXT('A'), false, false,
       false, false},
      {TEXT("A"), TEXT("A"), TEXT("A"), TEXT('A'), TEXT('A'), true, true, true,
       true},
      {TEXT("A"), TEXT(""), TEXT(""), TEXT('A'), TEXT('A'), true, true, true,
       true},
      {TEXT("ABCDE"), TEXT("ABC"), TEXT("CDE"), TEXT('A'), TEXT('E'), true,
       true, true, true},
      {TEXT("ABCDE"), TEXT("BCD"), TEXT("BCD"), TEXT('B'), TEXT('D'), false,
       false, false, false},
      {TEXT("AAAAA"), TEXT("AAA"), TEXT("AAA"), TEXT('A'), TEXT('A'), true,
       true, true, true},
      {TEXT("   SPACE"), TEXT("   "), TEXT("ACE"), TEXT(' '), TEXT('E'), true,
       true, true, true},
      {TEXT("SPACE   "), TEXT("SPA"), TEXT("   "), TEXT('S'), TEXT(' '), true,
       true, true, true},
      {TEXT("LINE1\nLINE2"), TEXT("LINE1"), TEXT("LINE2"), TEXT('L'), TEXT('2'),
       true, true, true, true},
      {TEXT("!@#$%^"), TEXT("!@#"), TEXT("^"), TEXT('!'), TEXT('^'), true, true,
       true, true},
      {TEXT("ABC"), TEXT("ABC"), TEXT("BC"), TEXT('A'), TEXT('C'), true, true,
       true, true},
      {TEXT("ABC"), TEXT("ABCD"), TEXT("ABCD"), TEXT('A'), TEXT('C'), false,
       false, true, true},
      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"),
       TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAA"),
       TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAA"), TEXT('A'), TEXT('A'), true, true,
       true, true},
      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB"),
       TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAA"),
       TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAC"), TEXT('A'), TEXT('B'), true, false,
       true, true},
      {TEXT("ZAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"),
       TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAA"),
       TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAA"), TEXT('Z'), TEXT('A'), false, true,
       true, true},
      {TEXT("ABABABABABABABABABABABABABABABAB"), TEXT("ABABABABAB"),
       TEXT("ABABABABAB"), TEXT('A'), TEXT('B'), true, true, true, true},
      {TEXT("aAaAaAaAaAaAaAaAaA"), TEXT("aAaAaA"), TEXT("AaAaAa"), TEXT('a'),
       TEXT('A'), true, false, true, true},
      {TEXT("        MANY SPACES        "), TEXT("        "), TEXT(" "),
       TEXT(' '), TEXT(' '), true, true, true, true},
      {TEXT("\t\tLINE1\nLINE2\n"), TEXT("\t\t"), TEXT("LINE2\n"), TEXT('\t'),
       TEXT('\n'), true, true, true, true},
      {TEXT("!@#$%^&*()_+-=[]{}|;':,./<>?!@#$%^&*()"), TEXT("!@#$%^&*()"),
       TEXT("!@#$%^&*()"), TEXT('!'), TEXT(')'), true, true, true, true},
      {TEXT("LONGSTRINGTEST"), TEXT("LONGSTRINGTEST"), TEXT("TEST"), TEXT('L'),
       TEXT('T'), true, true, true, true},
      {TEXT("LONGSTRINGTEST"), TEXT("LONG"), TEXT("LONGSTRINGTEST"), TEXT('L'),
       TEXT('T'), true, true, true, true},
      {TEXT("SHORT"), TEXT("SHORTER"), TEXT("SHORTER"), TEXT('S'), TEXT('T'),
       false, false, true, true},
      {TEXT("SHORT"), TEXT("SH"), TEXT("LONGSHORT"), TEXT('S'), TEXT('T'), true,
       false, true, true},
      {TEXT("VERYVERYLONGSTRINGTHATKEEPSGOING"), TEXT(""), TEXT(""), TEXT('V'),
       TEXT('G'), true, true, true, true},
  };

  for (const auto& t : test_cases) {
    String s(t.str);

    String prefix(t.prefix);
    String suffix(t.suffix);

    std::basic_string_view<CHAR> prefix_view(t.prefix);
    std::basic_string_view<CHAR> suffix_view(t.suffix);

    CHECK_EQ(s.StartsWith(t.c_start), t.sw_char);
    CHECK_EQ(s.StartsWith(t.prefix), t.sw_str);
    CHECK_EQ(s.StartsWith(prefix), t.sw_str);
    CHECK_EQ(s.StartsWith(prefix_view), t.sw_str);

    CHECK_EQ(s.EndsWith(t.c_end), t.ew_char);
    CHECK_EQ(s.EndsWith(t.suffix), t.ew_str);
    CHECK_EQ(s.EndsWith(suffix), t.ew_str);
    CHECK_EQ(s.EndsWith(suffix_view), t.ew_str);
  }
}

STRING_TEST_CASE(String, Contains_AllOverloads) {
  struct TestCase {
    const CHAR* lhs;
    const CHAR* rhs;
    bool expected_char;
    bool expected_substr;
  };

  static constexpr TestCase test_cases[] = {
      {TEXT("HELLO"), TEXT("HELLO"), true, true},
      {TEXT("HELLO"), TEXT("HELL"), true, true},
      {TEXT("HELL"), TEXT("HELLO"), true, false},
      {TEXT("ABCDEF"), TEXT("BCD"), true, true},
      {TEXT("ABCDEF"), TEXT("ABX"), true, false},
      {TEXT(""), TEXT(""), false, true},
      {TEXT(""), TEXT("A"), false, false},
      {TEXT("A"), TEXT(""), false, true},
      {TEXT("ABCABC"), TEXT("ABC"), true, true},
      {TEXT("ABCDE"), TEXT("E"), true, true},
      {TEXT("ABCDE"), TEXT("Z"), false, false},
      {TEXT("AAAAAAAAAA"), TEXT("AAAAAAAAAA"), true, true},
      {TEXT("AAAAAAAAAA"), TEXT("AAAAAAAAAAA"), true, false},
      {TEXT("AAAAAAAAAA"), TEXT("AAAAAAAAAAB"), true, false},
      {TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"),
       TEXT("JKLMNOPQR"), true, true},
      {TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"),
       TEXT("QRSTUVWXYZABCD"), true, true},
      {TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"),
       TEXT("XYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"), true,
       false},
      {TEXT("THEQUICKBROWNFOXJUMPSOVERTHELAZYDOG"), TEXT("BROWNFOXJUMPS"), true,
       true},
      {TEXT("THEQUICKBROWNFOXJUMPSOVERTHELAZYDOG"), TEXT("JUMPSOVERTHESKY"),
       true, false},
      {TEXT("123456789012345678901234567890"), TEXT("5678901234"), true, true},
      {TEXT("123456789012345678901234567890"),
       TEXT("567890123456789012345678901234567890123456789012345"), true,
       false},
      {TEXT("AAAAAAAAAABBBBBBBBBBCCCCCCCCCC"), TEXT("BBBBBBBBBBCCCC"), true,
       true},
      {TEXT("AAAAAAAAAABBBBBBBBBBCCCCCCCCCC"),
       TEXT("FCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEF"), false, false},
  };

  for (const auto& t : test_cases) {
    String lhs(t.lhs);

    CHAR rhs_c = t.rhs[0];
    const CHAR* rhs_cc = t.rhs;
    String rhs_str(t.rhs);
    std::basic_string_view<CHAR> rhs_view(t.rhs);

    bool r1 = lhs.Contains(rhs_c);
    bool r2 = lhs.Contains(rhs_cc);
    bool r3 = lhs.Contains(rhs_str);
    bool r4 = lhs.Contains(rhs_view);

    CHECK_EQ(r1, t.expected_char);
    CHECK_EQ(r2, t.expected_substr);
    CHECK_EQ(r3, t.expected_substr);
    CHECK_EQ(r4, t.expected_substr);
  }
}

namespace {
static constexpr CHAR kBig1[] =
    TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");

static constexpr CHAR kBig2[] =
    TEXT("THEQUICKBROWNFOXJUMPSOVERTHELAZYDOGTHEQUICKBROWNFOX");

static constexpr CHAR kBig3[] =
    TEXT("12345678901234567890123456789012345678901234567890");
}  // namespace

STRING_TEST_CASE(String, Find_ValueType) {
  struct FVTestCase {
    const CHAR* lhs;
    CHAR ch;
    SizeType pos;
    SizeType expected;
  };

  static constexpr FVTestCase test_cases[] = {
      {TEXT("HELLO"), TEXT('H'), 0, 0},
      {TEXT("HELLO"), TEXT('E'), 0, 1},
      {TEXT("HELLO"), TEXT('L'), 0, 2},
      {TEXT("HELLO"), TEXT('L'), 3, 3},
      {TEXT("HELLO"), TEXT('O'), 0, 4},
      {TEXT("HELLO"), TEXT('Z'), 0, kNpos},
      {TEXT("HELLO"), TEXT('H'), 1, kNpos},
      {TEXT("AAAAAA"), TEXT('A'), 0, 0},
      {TEXT("AAAAAA"), TEXT('A'), 3, 3},
      {TEXT("AAAAAA"), TEXT('A'), 6, kNpos},
      {TEXT("ABABABAB"), TEXT('B'), 0, 1},
      {TEXT("ABABABAB"), TEXT('B'), 2, 3},
      {TEXT("ABABABAB"), TEXT('B'), 7, 7},
      {TEXT(""), TEXT('A'), 0, kNpos},
      {TEXT("A"), TEXT('A'), 0, 0},
      {TEXT("A"), TEXT('A'), 1, kNpos},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    CHECK_EQ(s.Find(t.ch, t.pos), t.expected);
  }
}

STRING_TEST_CASE(String, Find_Pointer_Count) {
  struct FPCTestCase {
    const CHAR* lhs;
    const CHAR* rhs;
    SizeType pos;
    SizeType count;
    SizeType expected;
  };

  static constexpr FPCTestCase test_cases[] = {
      {TEXT("HELLO"), TEXT("HE"), 0, 2, 0},
      {TEXT("HELLO"), TEXT("ELL"), 0, 3, 1},
      {TEXT("HELLO"), TEXT("LL"), 0, 2, 2},
      {TEXT("HELLO"), TEXT("LO"), 0, 2, 3},
      {TEXT("HELLO"), TEXT("HELLO"), 0, 5, 0},

      {TEXT("HELLOHELLO"), TEXT("HELLO"), 0, 5, 0},
      {TEXT("HELLOHELLO"), TEXT("HELLO"), 1, 5, 5},

      {TEXT("ABCDE"), TEXT("BC"), 0, 2, 1},
      {TEXT("ABCDE"), TEXT("BC"), 2, 2, kNpos},

      {TEXT("AAAAAA"), TEXT("AAA"), 0, 3, 0},
      {TEXT("AAAAAA"), TEXT("AAA"), 1, 3, 1},
      {TEXT("AAAAAA"), TEXT("AAA"), 4, 3, kNpos},

      {TEXT("ABABABAB"), TEXT("BAB"), 0, 3, 1},
      {TEXT("ABABABAB"), TEXT("BAB"), 2, 3, 3},

      {TEXT(""), TEXT(""), 0, 0, 0},
      {TEXT("A"), TEXT(""), 0, 0, 0},
      {TEXT("A"), TEXT("A"), 0, 1, 0},
      {TEXT("A"), TEXT("A"), 1, 1, kNpos},

      {TEXT("SHORT"), TEXT("LONGER"), 0, 6, kNpos},

      {kBig1, TEXT("JKLMNOPQR"), 0, 9, 9},
      {kBig1, TEXT("QRSTUVWXYZ"), 0, 10, 16},
      {kBig1, TEXT("XYZ"), 0, 3, 23},
      {kBig1, TEXT("ABC"), 1, 3, 26},

      {kBig2, TEXT("BROWNFOX"), 0, 8, 8},
      {kBig2, TEXT("JUMPSOVER"), 0, 9, 16},
      {kBig2, TEXT("LAZYDOG"), 0, 7, 28},
      {kBig2, TEXT("NOTFOUND"), 0, 8, kNpos},

      {kBig3, TEXT("5678901234"), 0, 10, 4},
      {kBig3, TEXT("89012345"), 0, 8, 7},
      {kBig3, TEXT("1234567890"), 10, 10, 10},
      {kBig3, TEXT("1234567890"), 11, 10, 20},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    CHECK_EQ(s.Find(t.rhs, t.pos, t.count), t.expected);
  }
}

STRING_TEST_CASE(String, RFind_ValueType) {
  struct RFVTestCase {
    const CHAR* lhs;
    CHAR ch;
    SizeType pos;
    SizeType expected;
  };

  static constexpr RFVTestCase test_cases[] = {
      {TEXT("HELLO"), TEXT('H'), kNpos, 0},
      {TEXT("HELLO"), TEXT('O'), kNpos, 4},
      {TEXT("HELLO"), TEXT('L'), kNpos, 3},
      {TEXT("HELLO"), TEXT('L'), 2, 2},
      {TEXT("HELLO"), TEXT('L'), 1, kNpos},
      {TEXT("HELLO"), TEXT('Z'), kNpos, kNpos},

      {TEXT("AAAAAA"), TEXT('A'), kNpos, 5},
      {TEXT("AAAAAA"), TEXT('A'), 3, 3},
      {TEXT("AAAAAA"), TEXT('A'), 2, 2},
      {TEXT("AAAAAA"), TEXT('A'), 0, 0},

      {TEXT("ABABABAB"), TEXT('B'), kNpos, 7},
      {TEXT("ABABABAB"), TEXT('B'), 6, 5},
      {TEXT("ABABABAB"), TEXT('B'), 1, 1},

      {TEXT("A"), TEXT('A'), kNpos, 0},
      {TEXT("A"), TEXT('A'), 0, 0},
      {TEXT("A"), TEXT('A'), 1, 0},

      {TEXT(""), TEXT('A'), kNpos, kNpos},
      {TEXT(""), TEXT('A'), 0, kNpos},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    CHECK_EQ(s.RFind(t.ch, t.pos), t.expected);
  }
}

STRING_TEST_CASE(String, RFind_Pointer_Count) {
  struct RFPCTestCase {
    const CHAR* lhs;
    const CHAR* rhs;
    SizeType pos;
    SizeType count;
    SizeType expected;
  };

  static constexpr RFPCTestCase test_cases[] = {
      {TEXT("HELLO"), TEXT("HE"), kNpos, 2, 0},
      {TEXT("HELLO"), TEXT("EL"), kNpos, 2, 1},
      {TEXT("HELLO"), TEXT("LL"), kNpos, 2, 2},
      {TEXT("HELLO"), TEXT("LO"), kNpos, 2, 3},
      {TEXT("HELLOHELLO"), TEXT("HELLO"), kNpos, 5, 5},
      {TEXT("HELLOHELLO"), TEXT("HELLO"), 6, 5, 5},
      {TEXT("HELLOHELLO"), TEXT("HELLO"), 4, 5, 0},
      {TEXT("AAAAAA"), TEXT("AAA"), kNpos, 3, 3},
      {TEXT("AAAAAA"), TEXT("AAA"), 5, 3, 3},
      {TEXT("AAAAAA"), TEXT("AAA"), 2, 3, 2},
      {TEXT("AAAAAA"), TEXT("AAA"), 1, 3, 1},
      {TEXT("ABABABAB"), TEXT("BAB"), kNpos, 3, 5},
      {TEXT("ABABABAB"), TEXT("BAB"), 6, 3, 5},
      {TEXT("ABABABAB"), TEXT("BAB"), 4, 3, 3},
      {TEXT("ABCDEF"), TEXT("BCD"), kNpos, 3, 1},
      {TEXT("ABCDEF"), TEXT("BCD"), 0, 3, kNpos},
      {TEXT(""), TEXT(""), 0, 0, 0},
      {TEXT("A"), TEXT(""), 0, 0, 0},
      {TEXT("A"), TEXT("A"), 0, 1, 0},
      {TEXT("A"), TEXT("A"), 1, 1, 0},
      {TEXT("SHORT"), TEXT("LONGER"), kNpos, 6, kNpos},
      {kBig1, TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), kNpos, 26, 26},
      {kBig1, TEXT("XYZ"), kNpos, 3, 49},
      {kBig1, TEXT("ABC"), kNpos, 3, 26},

      {kBig2, TEXT("BROWNFOX"), kNpos, 8, 43},
      {kBig2, TEXT("BROWNFOX"), 40, 8, 8},
      {kBig2, TEXT("LAZYDOG"), kNpos, 7, 28},
      {kBig2, TEXT("FOX"), kNpos, 3, 48},

      {kBig3, TEXT("5678901234"), kNpos, 10, 34},
      {kBig3, TEXT("1234567890"), kNpos, 10, 40},
      {kBig3, TEXT("1234567890"), 30, 10, 30},

      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"), TEXT("AAAAA"),
       kNpos, 5, 39},
      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"), TEXT("AAAAA"), 30,
       5, 30},
      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"), TEXT("AAAAA"), 10,
       5, 10},

      {TEXT("ABABABABABABABABAB"), TEXT("BAB"), kNpos, 3, 15},
      {TEXT("ABABABABABABABABAB"), TEXT("BAB"), 10, 3, 9},

      {TEXT("EDGECASEEDGECASEEDGECASE"), TEXT("EDGE"), kNpos, 4, 16},
      {TEXT("EDGECASEEDGECASEEDGECASE"), TEXT("CASE"), kNpos, 4, 20},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    CHECK_EQ(s.RFind(t.rhs, t.pos, t.count), t.expected);
  }
}

STRING_TEST_CASE(String, FindFirstOf_ValueType) {
  struct FFOVTestCase {
    const CHAR* lhs;
    CHAR ch;
    SizeType pos;
    SizeType expected;
  };

  static constexpr FFOVTestCase test_cases[] = {
      {TEXT("HELLO"), TEXT('H'), 0, 0},
      {TEXT("HELLO"), TEXT('E'), 0, 1},
      {TEXT("HELLO"), TEXT('L'), 0, 2},
      {TEXT("HELLO"), TEXT('L'), 3, 3},
      {TEXT("HELLO"), TEXT('O'), 0, 4},
      {TEXT("HELLO"), TEXT('Z'), 0, kNpos},
      {TEXT("HELLO"), TEXT('H'), 1, kNpos},

      {TEXT("AAAAAA"), TEXT('A'), 0, 0},
      {TEXT("AAAAAA"), TEXT('A'), 4, 4},
      {TEXT("AAAAAA"), TEXT('A'), 6, kNpos},

      {TEXT("ABABABAB"), TEXT('B'), 0, 1},
      {TEXT("ABABABAB"), TEXT('B'), 2, 3},
      {TEXT("ABABABAB"), TEXT('B'), 7, 7},

      {TEXT(""), TEXT('A'), 0, kNpos},
      {TEXT("A"), TEXT('A'), 0, 0},
      {TEXT("A"), TEXT('A'), 1, kNpos},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    const auto result = s.FindFirstOf(t.ch, t.pos);
    CHECK_EQ(result, t.expected);
  }
}

STRING_TEST_CASE(String, FindFirstOf_Pointer_Count) {
  struct FFOPCTestCase {
    const CHAR* lhs;
    const CHAR* set;
    SizeType pos;
    SizeType count;
    SizeType expected;
  };

  static constexpr FFOPCTestCase test_cases[] = {
      {TEXT("HELLO"), TEXT("AEIOU"), 0, 5, 1},
      {TEXT("HELLO"), TEXT("XYZLO"), 0, 5, 2},
      {TEXT("HELLO"), TEXT("O"), 0, 1, 4},

      {TEXT("ABCDE"), TEXT("XYZ"), 0, 3, kNpos},
      {TEXT("ABCDE"), TEXT("CDE"), 0, 3, 2},

      {TEXT("ABABABAB"), TEXT("B"), 0, 1, 1},
      {TEXT("ABABABAB"), TEXT("BA"), 0, 2, 0},
      {TEXT("ABABABAB"), TEXT("CBA"), 0, 3, 0},

      {TEXT("AAAAAA"), TEXT("BCA"), 0, 3, 0},
      {TEXT("AAAAAA"), TEXT("B"), 0, 1, kNpos},

      {TEXT(""), TEXT("A"), 0, 1, kNpos},
      {TEXT("A"), TEXT(""), 0, 0, kNpos},
      {TEXT("A"), TEXT("A"), 0, 1, 0},

      {TEXT("HELLOHELLO"), TEXT("LO"), 0, 2, 2},
      {TEXT("HELLOHELLO"), TEXT("LO"), 4, 2, 4},

      {TEXT("EDGECASE"), TEXT("CASE"), 0, 4, 0},

      {kBig1, TEXT("XYZ"), 0, 3, 23},
      {kBig1, TEXT("ABC"), 1, 3, 1},

      {kBig2, TEXT("QWERTYUIOP"), 0, 10, 0},
      {kBig2, TEXT("FOX"), 0, 3, 10},
      {kBig2, TEXT("DOG"), 0, 3, 10},
      {kBig2, TEXT("XYZ"), 0, 3, 15},

      {kBig3, TEXT("567890"), 0, 6, 4},
      {kBig3, TEXT("0123"), 0, 4, 0},

      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"), TEXT("BCDA"), 0, 4, 0},
      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"), TEXT("BCDA"), 10, 4, 10},

      {TEXT("ABABABABABABABABAB"), TEXT("B"), 0, 1, 1},
      {TEXT("ABABABABABABABABAB"), TEXT("B"), 5, 1, 5},

      {TEXT("EDGEEDGEEDGEEDGE"), TEXT("GDE"), 0, 3, 0},

      {TEXT(""), TEXT("ABC"), 0, 3, kNpos},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    const auto result = s.FindFirstOf(t.set, t.pos, t.count);
    CHECK_EQ(result, t.expected);
  }
}

STRING_TEST_CASE(String, FindFirstNotOf_ValueType) {
  struct FFNVTestCase {
    const CHAR* lhs;
    CHAR ch;
    SizeType pos;
    SizeType expected;
  };

  static constexpr FFNVTestCase test_cases[] = {
      {TEXT("AAAAAA"), TEXT('A'), 0, kNpos},
      {TEXT("AAAAAB"), TEXT('A'), 0, 5},
      {TEXT("BAAAAA"), TEXT('A'), 0, 0},
      {TEXT("BAAAAA"), TEXT('A'), 1, kNpos},

      {TEXT("HELLO"), TEXT('H'), 0, 1},
      {TEXT("HELLO"), TEXT('E'), 0, 0},
      {TEXT("HELLO"), TEXT('L'), 0, 0},
      {TEXT("HELLO"), TEXT('L'), 3, 4},
      {TEXT("HELLO"), TEXT('O'), 0, 0},

      {TEXT("ABABABAB"), TEXT('A'), 0, 1},
      {TEXT("ABABABAB"), TEXT('B'), 0, 0},

      {TEXT("A"), TEXT('A'), 0, kNpos},
      {TEXT("A"), TEXT('A'), 1, kNpos},
      {TEXT("A"), TEXT('B'), 0, 0},

      {TEXT(""), TEXT('A'), 0, kNpos},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    const auto result = s.FindFirstNotOf(t.ch, t.pos);
    CHECK_EQ(result, t.expected);
  }
}

STRING_TEST_CASE(String, FindFirstNotOf_Pointer_Count) {
  struct FFNPCTestCase {
    const CHAR* lhs;
    const CHAR* set;
    SizeType pos;
    SizeType count;
    SizeType expected;
  };

  static constexpr FFNPCTestCase test_cases[] = {
      {TEXT("AAAAAA"), TEXT("A"), 0, 1, kNpos},
      {TEXT("AAAAB"), TEXT("A"), 0, 1, 4},
      {TEXT("BAAAAA"), TEXT("A"), 0, 1, 0},
      {TEXT("BAAAAA"), TEXT("A"), 1, 1, kNpos},

      {TEXT("HELLO"), TEXT("HELLO"), 0, 5, kNpos},
      {TEXT("HELLO"), TEXT("HEL"), 0, 3, 4},
      {TEXT("HELLO"), TEXT("ELL"), 0, 3, 0},
      {TEXT("HELLO"), TEXT("LO"), 0, 2, 0},

      {TEXT("ABCDEF"), TEXT("ABC"), 0, 3, 3},
      {TEXT("ABCDEF"), TEXT("DEF"), 0, 3, 0},

      {TEXT("ABABABAB"), TEXT("AB"), 0, 2, kNpos},
      {TEXT("ABABABAB"), TEXT("A"), 0, 1, 1},

      {TEXT("A"), TEXT("A"), 0, 1, kNpos},
      {TEXT("A"), TEXT("B"), 0, 1, 0},

      {TEXT(""), TEXT("A"), 0, 1, kNpos},

      {kBig1, TEXT("A"), 0, 1, 1},
      {kBig1, TEXT("A"), 10, 1, 10},

      {kBig2, TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0, 26, kNpos},
      {kBig2, TEXT("ABCDEF"), 0, 6, 0},
      {kBig2, TEXT("XYZ"), 0, 3, 0},
      {kBig2, TEXT("XYZ"), 23, 3, 23},

      {kBig3, TEXT("AEIOU"), 0, 5, 0},
      {kBig3, TEXT("QWERTYUIOPASDFGHJKLZXCVBNM"), 0, 26, 0},
      {kBig3, TEXT("BCDFG"), 0, 5, 0},
      {kBig3, TEXT("BCDFG"), 10, 5, 10},

      {TEXT("EDGECASEEDGECASEEDGECASE"), TEXT("EDGE"), 0, 4, 4},
      {TEXT("EDGECASEEDGECASEEDGECASE"), TEXT("CASE"), 0, 4, 1},

      {TEXT(""), TEXT("ABC"), 0, 3, kNpos},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    const auto result = s.FindFirstNotOf(t.set, t.pos, t.count);
    CHECK_EQ(result, t.expected);
  }
}

STRING_TEST_CASE(String, FindLastOf_ValueType) {
  struct FLVTestCase {
    const CHAR* lhs;
    CHAR ch;
    SizeType pos;
    SizeType expected;
  };

  static constexpr FLVTestCase test_cases[] = {
      {TEXT("HELLO"), TEXT('L'), kNpos, 3},
      {TEXT("HELLO"), TEXT('H'), kNpos, 0},
      {TEXT("HELLO"), TEXT('O'), kNpos, 4},
      {TEXT("HELLO"), TEXT('E'), kNpos, 1},
      {TEXT("HELLO"), TEXT('Z'), kNpos, kNpos},

      {TEXT("HELLO"), TEXT('L'), 2, 2},
      {TEXT("HELLO"), TEXT('L'), 1, kNpos},

      {TEXT("AAAAAA"), TEXT('A'), kNpos, 5},
      {TEXT("AAAAAA"), TEXT('A'), 3, 3},
      {TEXT("AAAAAA"), TEXT('A'), 0, 0},

      {TEXT("ABABABAB"), TEXT('B'), kNpos, 7},
      {TEXT("ABABABAB"), TEXT('B'), 5, 5},
      {TEXT("ABABABAB"), TEXT('B'), 0, kNpos},

      {TEXT("A"), TEXT('A'), kNpos, 0},
      {TEXT("A"), TEXT('A'), 0, 0},
      {TEXT("A"), TEXT('A'), 1, 0},

      {TEXT(""), TEXT('A'), kNpos, kNpos},
  };

  int i = 0;
  for (const auto& t : test_cases) {
    String s(t.lhs);
    const auto result = s.FindLastOf(t.ch, t.pos);
    CHECK_EQ(result, t.expected);
    if (result != t.expected) {
      printf("%d\n", i);
    }
    ++i;
  }
}

STRING_TEST_CASE(String, FindLastOf_Pointer_Count) {
  struct FLPCTestCase {
    const CHAR* lhs;
    const CHAR* set;
    SizeType pos;
    SizeType count;
    SizeType expected;
  };

  static constexpr FLPCTestCase test_cases[] = {
      {TEXT("HELLO"), TEXT("LO"), kNpos, 2, 4},
      {TEXT("HELLO"), TEXT("LO"), 3, 2, 3},
      {TEXT("HELLO"), TEXT("LO"), 2, 2, 2},

      {TEXT("HELLO"), TEXT("HEL"), kNpos, 3, 3},
      {TEXT("HELLO"), TEXT("HEL"), 2, 3, 2},
      {TEXT("HELLO"), TEXT("HEL"), 0, 3, 0},

      {TEXT("ABCDE"), TEXT("ABC"), kNpos, 3, 2},
      {TEXT("ABCDE"), TEXT("ABC"), 1, 3, 1},
      {TEXT("ABCDE"), TEXT("ABC"), 0, 3, 0},

      {TEXT("ABABABAB"), TEXT("AB"), kNpos, 2, 7},
      {TEXT("ABABABAB"), TEXT("AB"), 5, 2, 5},
      {TEXT("ABABABAB"), TEXT("B"), kNpos, 1, 7},

      {TEXT("A"), TEXT("A"), kNpos, 1, 0},
      {TEXT("A"), TEXT("B"), kNpos, 1, kNpos},

      {TEXT(""), TEXT("A"), kNpos, 1, kNpos},
      {TEXT("HELLO"), TEXT(""), kNpos, 0, kNpos},

      {kBig1, TEXT("XYZ"), kNpos, 3, 51},
      {kBig1, TEXT("ABC"), kNpos, 3, 28},

      {kBig2, TEXT("FOX"), kNpos, 3, 50},
      {kBig2, TEXT("FOX"), 40, 3, 33},
      {kBig2, TEXT("FOX"), 10, 3, 10},

      {kBig2, TEXT("DOG"), kNpos, 3, 49},
      {kBig2, TEXT("DOG"), 30, 3, 21},

      {kBig3, TEXT("567890"), kNpos, 6, 49},
      {kBig3, TEXT("123456"), kNpos, 6, 45},

      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAB"), TEXT("AB"), kNpos, 2, 29},

      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAB"), TEXT("AB"), 20, 2, 20},

      {TEXT("ABABABABABABABABAB"), TEXT("B"), kNpos, 1, 17},
      {TEXT("ABABABABABABABABAB"), TEXT("A"), kNpos, 1, 16},

      {TEXT("EDGEEDGEEDGEEDGE"), TEXT("EDGE"), kNpos, 4, 15},
      {TEXT("EDGEEDGEEDGEEDGE"), TEXT("E"), kNpos, 1, 15},

      {TEXT(""), TEXT("ABC"), kNpos, 3, kNpos},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    const auto result = s.FindLastOf(t.set, t.pos, t.count);
    CHECK_EQ(result, t.expected);
  }
}

STRING_TEST_CASE(String, FindLastNotOf_ValueType) {
  struct FLNVTestCase {
    const CHAR* lhs;
    CHAR ch;
    SizeType pos;
    SizeType expected;
  };

  static constexpr FLNVTestCase test_cases[] = {
      {TEXT("AAAAAA"), TEXT('A'), kNpos, kNpos},
      {TEXT("AAAAAB"), TEXT('A'), kNpos, 5},
      {TEXT("BAAAAA"), TEXT('A'), kNpos, 0},
      {TEXT("BAAAAA"), TEXT('A'), 0, 0},
      {TEXT("BAAAAA"), TEXT('B'), kNpos, 5},

      {TEXT("HELLO"), TEXT('H'), kNpos, 4},
      {TEXT("HELLO"), TEXT('E'), kNpos, 4},
      {TEXT("HELLO"), TEXT('L'), kNpos, 4},
      {TEXT("HELLO"), TEXT('O'), kNpos, 3},

      {TEXT("ABABABAB"), TEXT('A'), kNpos, 7},
      {TEXT("ABABABAB"), TEXT('B'), kNpos, 6},

      {TEXT("A"), TEXT('A'), kNpos, kNpos},
      {TEXT("A"), TEXT('B'), kNpos, 0},

      {TEXT(""), TEXT('A'), kNpos, kNpos},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    const auto result = s.FindLastNotOf(t.ch, t.pos);
    CHECK_EQ(result, t.expected);
  }
}

STRING_TEST_CASE(String, FindLastNotOf_Pointer_Count) {
  struct FLNPCTestCase {
    const CHAR* lhs;
    const CHAR* set;
    SizeType pos;
    SizeType count;
    SizeType expected;
  };

  static constexpr FLNPCTestCase test_cases[] = {
      {TEXT("AAAAAA"), TEXT("A"), kNpos, 1, kNpos},
      {TEXT("AAAABB"), TEXT("A"), kNpos, 1, 5},
      {TEXT("BAAAAA"), TEXT("A"), kNpos, 1, 0},
      {TEXT("BAAAAA"), TEXT("A"), 3, 1, 0},

      {TEXT("HELLO"), TEXT("HELLO"), kNpos, 5, kNpos},
      {TEXT("HELLO"), TEXT("HELL"), kNpos, 4, 4},
      {TEXT("HELLO"), TEXT("LO"), kNpos, 2, 1},
      {TEXT("HELLO"), TEXT("LO"), 2, 2, 1},

      {TEXT("ABCDE"), TEXT("ABC"), kNpos, 3, 4},
      {TEXT("ABCDE"), TEXT("DEF"), kNpos, 3, 2},

      {TEXT("ABABABAB"), TEXT("AB"), kNpos, 2, kNpos},
      {TEXT("ABABABAB"), TEXT("A"), kNpos, 1, 7},

      {TEXT("A"), TEXT("A"), kNpos, 1, kNpos},
      {TEXT("A"), TEXT("B"), kNpos, 1, 0},

      {TEXT(""), TEXT("A"), kNpos, 1, kNpos},

      {kBig1, TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), kNpos, 26, kNpos},
      {kBig1, TEXT("ABC"), kNpos, 3, 51},
      {kBig1, TEXT("XYZ"), kNpos, 3, 48},

      {kBig2, TEXT("AEIOU"), kNpos, 5, 50},
      {kBig2, TEXT("BCDFG"), kNpos, 5, 50},
      {kBig2, TEXT("XYZ"), kNpos, 3, 49},

      {kBig3, TEXT("0123456789"), kNpos, 10, kNpos},

      {kBig3, TEXT("567890"), kNpos, 6, 43},

      {kBig3, TEXT("123"), kNpos, 3, 49},

      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAB"), TEXT("A"), kNpos, 1, 29},
      {TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAB"), TEXT("B"), kNpos, 1, 28},

      {TEXT("ABABABABABABABABAB"), TEXT("AB"), kNpos, 2, kNpos},
      {TEXT("ABABABABABABABABAB"), TEXT("A"), kNpos, 1, 17},

      {TEXT("EDGEEDGEEDGEEDGE"), TEXT("EDGE"), kNpos, 4, kNpos},
  };

  for (const auto& t : test_cases) {
    String s(t.lhs);
    const auto result = s.FindLastNotOf(t.set, t.pos, t.count);
    CHECK_EQ(result, t.expected);
  }
}