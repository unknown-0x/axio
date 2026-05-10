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
  String s1;
  String s2;

  for (const auto& test : kAppendTestCases) {
    const auto text_size = CharTraits::length(test.text);
    const auto expected_size = CharTraits::length(test.expected);

    {
      std::basic_istringstream<CHAR> iss(test.text);
      iss >> std::noskipws;
      s1.Append(std::istream_iterator<CHAR, CHAR>(iss),
                std::istream_iterator<CHAR, CHAR>());

      CHECK_EQ(s1.Size(), expected_size);
      CHECK_STR_EQ(s1.CStr(), test.expected);
    }

    {
      std::basic_string<CHAR> tmp(test.text, text_size);
      std::basic_stringstream<CHAR> ss;
      ss >> std::noskipws;
      for (auto ch : tmp) {
        ss << ch;
      }
      std::istream_iterator<CHAR, CHAR> first(ss);
      std::istream_iterator<CHAR, CHAR> last;

      s2.Append(first, last);

      CHECK_EQ(s2.Size(), expected_size);
      CHECK_STR_EQ(s2.CStr(), test.expected);
    }
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