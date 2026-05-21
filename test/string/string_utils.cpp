#include <simpletest/simpletest.hpp>

#include <axio/container/array.hpp>
#include <axio/container/tuple.hpp>
#include <axio/container/vector.hpp>
#include <axio/string/string_utils.hpp>

#include <list>
#include <map>

struct Point {
  Point() = default;
  Point(int xv, int yv) : x(xv), y(yv) {}

  int x;
  int y;
};

template <typename Output>
void AxioRepr(Output& output, const Point& p) {
  axio::AppendToOutput(output, "(x=", p.x, ", y=", p.y, ')');
}

struct PointFriend {
  PointFriend(int xv, int yv) : x(xv), y(yv) {}

  template <typename Output>
  friend void AxioRepr(Output& output, const PointFriend& p) {
    axio::AppendToOutput(output, "(x=", p.x, ", y=", p.y, ')');
  }

 private:
  int x;
  int y;
};

namespace foo {
struct Foo {
  template <typename Output>
  friend void AxioRepr(Output& output, const Foo&) {
    output.Append("Foo");
  }
};
}  // namespace foo

TEST_CASE(StringUtils, StringCat) {
  {
    auto s = axio::StringCat();
    CHECK_EQ(s, "");
  }
  {
    auto s = axio::StringCat("hello world");
    CHECK_EQ(s, "hello world");
  }
  {
    auto s = axio::StringCat(false);
    CHECK_EQ(s, "false");
  }
  {
    auto s = axio::StringCat(-1);
    CHECK_EQ(s, "-1");
  }
  {
    auto s = axio::StringCat(-1, false);
    CHECK_EQ(s, "-1false");
  }
  {
    char mutable_array[] = "world";
    char* mutable_ptr = mutable_array;
    auto s = axio::StringCat(mutable_ptr);
    CHECK_EQ(s, "world");
  }
  {
    auto s = axio::StringCat(-1, false, " foo");
    CHECK_EQ(s, "-1false foo");
  }
  {
    std::string_view sv("string view");
    auto s = axio::StringCat("x=", 2, ", this is my ", sv, '.');
    CHECK_EQ(s, "x=2, this is my string view.");
  }
  {
    const char* cc = "const char*";
    bool b = true;
    axio::Int8 i8 = -10;
    axio::Int16 i16 = -1000;
    axio::Int32 i32 = -100000;
    axio::Int64 i64 = -10000000;
    axio::UInt8 ui8 = 10;
    axio::UInt16 ui16 = 1000;
    axio::UInt32 ui32 = 100000;
    axio::UInt64 ui64 = 10000000;
    std::string ss("foo");

    auto s = axio::StringCat("cc=", cc, ", b=", b, ", i8=", i8, ", i16=", i16,
                             ", i32=", i32, ", i64=", i64, ", ui8=", ui8,
                             ", ui16=", ui16, ", ui32=", ui32, ", ui64=", ui64,
                             ", ss=", ss);
    CHECK_EQ(s,
             "cc=const char*, b=true, i8=-10, i16=-1000, i32=-100000, "
             "i64=-10000000, "
             "ui8=10, ui16=1000, "
             "ui32=100000, ui64=10000000, ss=foo");
  }
  {
    const auto p1 = Point(1, 2);
    const auto p2 = PointFriend(42, 10);
    const auto f = foo::Foo();
    auto s = axio::StringCat(p1, ' ', p2, ' ', f);
    CHECK_EQ(s, "(x=1, y=2) (x=42, y=10) Foo");
  }
  {
    axio::String str = "hello";
    auto s = axio::StringCat(str, str, str);
    CHECK_EQ(s, "hellohellohello");
  }
  {
    axio::Vector<int> v;
    auto s = axio::StringCat(v);
    CHECK_EQ(s, "[]");

    v.Push(1);
    s = axio::StringCat(v);
    CHECK_EQ(s, "[1]");

    v.Push(2);
    s = axio::StringCat(v);
    CHECK_EQ(s, "[1, 2]");

    v.Push(3);
    s = axio::StringCat(v);
    CHECK_EQ(s, "[1, 2, 3]");

    v.Push(4);
    s = axio::StringCat(v);
    CHECK_EQ(s, "[1, 2, 3, 4]");
  }

  {
    axio::Vector<axio::String> v;
    auto s = axio::StringCat(v);
    CHECK_EQ(s, "[]");

    v.Push("hello");
    s = axio::StringCat(v);
    CHECK_EQ(s, "[hello]");

    v.Push("world");
    s = axio::StringCat(v);
    CHECK_EQ(s, "[hello, world]");
    v.Push("foo");
    s = axio::StringCat(v);
    CHECK_EQ(s, "[hello, world, foo]");
    v.Push("bar");
    s = axio::StringCat(v);
    CHECK_EQ(s, "[hello, world, foo, bar]");
  }
  {
    axio::Tuple<> t1;
    axio::Tuple<int> t2{42};
    axio::Tuple<int, axio::String> t3{2, "hello"};
    axio::Tuple<int, axio::String, foo::Foo> t4{2, "hello", {}};

    auto s = axio::StringCat(t1);
    CHECK_EQ(s, "()");
    s = axio::StringCat(t2, t3);
    CHECK_EQ(s, "(42)(2, hello)");
    s = axio::StringCat(t1, t3, t4);
    CHECK_EQ(s, "()(2, hello)(2, hello, Foo)");
  }
}

TEST_CASE(StringUtils, StringAppend) {
  {
    axio::String s;
    axio::StringAppend(s);
    CHECK_EQ(s, "");
  }
  {
    axio::String s;
    Point p{1, 2};
    axio::StringAppend(s, 100, " hello world ", p);
    CHECK_EQ(s, "100 hello world (x=1, y=2)");

    foo::Foo f{};
    axio::StringAppend(s, ' ', true, ' ', f);
    CHECK_EQ(s, "100 hello world (x=1, y=2) true Foo");

    axio::StringAppend(s);
    CHECK_EQ(s, "100 hello world (x=1, y=2) true Foo");
  }
  {
    axio::Vector<Point> vp{{1, 2}, {3, 4}, {5, 6}, {7, 8}};
    axio::Vector<foo::Foo> vf{{}, {}, {}};
    axio::Vector<int> vi{1, 2, 3, 4};

    axio::String s;
    axio::StringAppend(s, vp, ' ', vi);
    CHECK_EQ(s,
             "[(x=1, y=2), (x=3, y=4), (x=5, y=6), (x=7, y=8)] [1, 2, 3, 4]");
    axio::StringAppend(s, ' ', vf);
    CHECK_EQ(s,
             "[(x=1, y=2), (x=3, y=4), (x=5, y=6), (x=7, y=8)] [1, 2, 3, 4] "
             "[Foo, Foo, Foo]");
  }
  {
    axio::Array<int, 0> a0;
    axio::Array<int, 3> a1{1, 2, 3};
    axio::Array<axio::String, 3> a2{"aaa", "bbb", "ccc"};
    axio::Array<foo::Foo, 1> a3{{}};

    axio::String s;

    axio::StringAppend(s, a0, a1);
    CHECK_EQ(s, "[][1, 2, 3]");

    axio::StringAppend(s, a2, a3);
    CHECK_EQ(s, "[][1, 2, 3][aaa, bbb, ccc][Foo]");
  }
}

TEST_CASE(StringUtils, StringJoin) {
  axio::Vector<int> empty = {};
  axio::Vector<int> one = {1};
  axio::Vector<int> nums = {1, 2, 3, 4, 5};

  auto s1 = axio::StringJoin(empty, " | ");
  auto s2 = axio::StringJoin(one, " | ");
  auto s3 = axio::StringJoin(nums, " | ");
  CHECK_EQ(s1, "");
  CHECK_EQ(s2, "1");
  CHECK_EQ(s3, "1 | 2 | 3 | 4 | 5");

  axio::Tuple<> t1;
  axio::Tuple<int> t2{1};
  axio::Tuple<int, axio::String, axio::Vector<int>> t3{
      1, "string", {1, 2, 3, 4}};

  s1 = axio::StringJoin(t1, " ! ");
  s2 = axio::StringJoin(t2, " $ ");
  s3 = axio::StringJoin(t3, " & ");
  CHECK_EQ(s1, "");
  CHECK_EQ(s2, "1");
  CHECK_EQ(s3, "1 & string & [1, 2, 3, 4]");

  std::list<foo::Foo> l{{}, {}, {}, {}};
  s1 = axio::StringJoin(l, "/");
  CHECK_EQ(s1, "Foo/Foo/Foo/Foo");

  std::vector<Point> points{{1, 2}, {3, 4}, {5, 6}};
  s1 = axio::StringJoin(points, " | ",
                        [](axio::Buffer<>& buffer, const Point& p) {
                          axio::AppendToOutput(buffer, '{', p.x, ',', p.y, '}');
                        });
  CHECK_EQ(s1, "{1,2} | {3,4} | {5,6}");

  std::map<int, Point> point_map;
  point_map[100] = Point{1, 0};
  point_map[110] = Point{1, 1};
  point_map[120] = Point{1, 2};
  point_map[200] = Point{2, 0};
  point_map[230] = Point{2, 3};

  s1 = axio::StringJoin(
      point_map, " @ ",
      [](axio::Buffer<>& buffer, const std::pair<int, Point>& p) {
        axio::AppendToOutput(buffer, p.first, '{', p.second.x, ',', p.second.y,
                             '}');
      });
  CHECK_EQ(s1, "100{1,0} @ 110{1,1} @ 120{1,2} @ 200{2,0} @ 230{2,3}");
}

TEST_CASE(StringUtils, StringJoinValues) {
  axio::Vector<foo::Foo> empty{};
  axio::Vector<foo::Foo> v{{}, {}, {}};
  axio::Tuple<int, axio::String, Point> t{1, "hello", {1, 2}};

  auto s1 = axio::StringJoinValues(",");
  auto s2 = axio::StringJoinValues(",", 1);
  auto s3 = axio::StringJoinValues(",", empty, t);
  CHECK_EQ(s1, "");
  CHECK_EQ(s2, "1");
  CHECK_EQ(s3, "[],(1, hello, (x=1, y=2))");

  auto s4 = axio::StringJoinValues(" ~ ", s3, v, 1, 2, 3);
  CHECK_EQ(s4, "[],(1, hello, (x=1, y=2)) ~ [Foo, Foo, Foo] ~ 1 ~ 2 ~ 3");
}

namespace {
template <typename T, typename A, typename U>
axio::Bool Expect(const axio::Vector<T, A>& v, std::initializer_list<U> l) {
  if (v.Size() != l.size()) {
    return false;
  }
  auto vb = v.begin();
  auto ve = v.end();
  auto lb = l.begin();
  for (; vb != ve; ++vb, ++lb) {
    if (!(*vb == *lb)) {
      return false;
    }
  }
  return true;
}
}  // namespace

#define CHECK_VECTOR(v, ...) CHECK_TRUE(Expect(v, __VA_ARGS__))

TEST_CASE(StringUtils, SplitByChar) {
  using ViewVector = axio::Vector<std::string_view>;

  auto s1 = axio::Split("", ',');
  auto s2 = axio::Split("11", ',');
  auto s3 = axio::Split("11,", ',');
  auto s4 = axio::Split("aa,bb,cc", ',');
  auto s5 = axio::Split("aa,,bb,cc,dd,ee, ,ff,", ',');
  auto s6 = axio::Split(",", ',');
  auto s7 = axio::Split(",,", ',');

  ViewVector v1(s1.begin(), s1.end());
  ViewVector v2(s2.begin(), s2.end());
  ViewVector v3(s3.begin(), s3.end());
  ViewVector v4(s4.begin(), s4.end());
  ViewVector v5(s5.begin(), s5.end());
  ViewVector v6(s6.begin(), s6.end());
  ViewVector v7(s7.begin(), s7.end());

  CHECK_VECTOR(v1, {""});
  CHECK_VECTOR(v2, {"11"});
  CHECK_VECTOR(v3, {"11", ""});
  CHECK_VECTOR(v4, {"aa", "bb", "cc"});
  CHECK_VECTOR(v5, {"aa", "", "bb", "cc", "dd", "ee", " ", "ff", ""});
  CHECK_VECTOR(v6, {"", ""});
  CHECK_VECTOR(v7, {"", "", ""});
}

TEST_CASE(StringUtils, SplitByString) {
  using ViewVector = axio::Vector<std::string_view>;

  auto s1 = axio::Split("", "@@@");
  auto s2 = axio::Split("11", "abc");
  auto s3 = axio::Split("abc@@@", "@@@");
  auto s4 = axio::Split("aa@@@bb@@@cc", "@@@");
  auto s5 = axio::Split("aa@@@@@@bb@@@cc@@@dd@@@ee@@@ @@@ff@@@", "@@@");
  auto s6 = axio::Split("@@@", "@@@");
  auto s7 = axio::Split("abcabcabc", "abc");

  ViewVector v1(s1.begin(), s1.end());
  ViewVector v2(s2.begin(), s2.end());
  ViewVector v3(s3.begin(), s3.end());
  ViewVector v4(s4.begin(), s4.end());
  ViewVector v5(s5.begin(), s5.end());
  ViewVector v6(s6.begin(), s6.end());
  ViewVector v7(s7.begin(), s7.end());

  CHECK_VECTOR(v1, {""});
  CHECK_VECTOR(v2, {"11"});
  CHECK_VECTOR(v3, {"abc", ""});
  CHECK_VECTOR(v4, {"aa", "bb", "cc"});
  CHECK_VECTOR(v5, {"aa", "", "bb", "cc", "dd", "ee", " ", "ff", ""});
  CHECK_VECTOR(v6, {"", ""});
  CHECK_VECTOR(v7, {"", "", "", ""});

  const char* arr[]{"", "aa", "bb", "cc", "", "dd", ""};
  axio::SizeT i = 0;
  for (auto x : axio::Split(",aa,bb,cc,,dd,", ',')) {
    CHECK_EQ(x, arr[i++]);
  }
}

TEST_CASE(StringUtils, SplitByAnyChar) {
  using ViewVector = axio::Vector<std::string_view>;

  axio::AnyCharDelimiter any_delim{",;|"};
  axio::AnyCharDelimiter dup_delim{",,"};

  auto t1 = axio::Split("", any_delim);
  auto t2 = axio::Split("abc", any_delim);
  auto t3 = axio::Split("aa,bb;cc|dd", any_delim);
  auto t4 = axio::Split(",,;;||", any_delim);
  auto t5 = axio::Split("aa,;bb", any_delim);
  auto t6 = axio::Split(",aa;", any_delim);
  auto t7 = axio::Split("aa,bb", dup_delim);

  ViewVector v1(t1.begin(), t1.end());
  ViewVector v2(t2.begin(), t2.end());
  ViewVector v3(t3.begin(), t3.end());
  ViewVector v4(t4.begin(), t4.end());
  ViewVector v5(t5.begin(), t5.end());
  ViewVector v6(t6.begin(), t6.end());
  ViewVector v7(t7.begin(), t7.end());

  CHECK_VECTOR(v1, {""});
  CHECK_VECTOR(v2, {"abc"});
  CHECK_VECTOR(v3, {"aa", "bb", "cc", "dd"});
  CHECK_VECTOR(v4, {"", "", "", "", "", "", ""});
  CHECK_VECTOR(v5, {"aa", "", "bb"});
  CHECK_VECTOR(v6, {"", "aa", ""});
  CHECK_VECTOR(v7, {"aa", "bb"});

  std::list<std::string_view> l(t3.begin(), t3.end());
  auto b = l.begin();
  for (axio::SizeT i = 0; i < l.size(); ++i) {
    CHECK_EQ(v3[i], *b);
    ++b;
  }
}

TEST_CASE(StringUtils, Split) {
  {
    auto view1 = axio::Split("a  ,   b,   , c,  d ,", ',') | axio::Trim;
    auto view2 = axio::Split("a  ,   b,   , c,  d ,", ',') | axio::Trim;
    auto view3 = axio::Split("a  ,   b,   , c,  d ,", ',') | axio::Trim;

    axio::Vector<std::string_view> expected{"a", "b", "", "c", "d", ""};

    axio::Vector<std::string_view> v1(view1.begin(), view1.end());
    std::vector<std::string_view> v2(view2.begin(), view2.end());
    std::list<std::string_view> v3(view3.begin(), view3.end());

    CHECK_EQ(v1.Size(), expected.Size());
    CHECK_EQ(v2.size(), expected.Size());
    CHECK_EQ(v3.size(), expected.Size());

    auto b1 = v1.begin();
    auto b2 = v2.begin();
    auto b3 = v3.begin();
    for (axio::SizeT i = 0; i < expected.Size(); ++i) {
      CHECK_EQ(*b1, expected[i]);
      CHECK_EQ(*b2, expected[i]);
      CHECK_EQ(*b3, expected[i]);
      ++b1, ++b2, ++b3;
    }
  }
  {
    auto result = axio::Split("a  ,   b,   , c,  d ,", ',') | axio::Trim |
                  axio::SkipEmpty;
    axio::Vector<std::string> v(result.begin(), result.end());
    // trim: {"a", "b", "", "c", "d", ""}
    // skip empty: {"a", "b", "c", "d"}
    CHECK_VECTOR(v, {"a", "b", "c", "d"});
  }
  {
    struct StringViewHolder {
      std::string_view sv;
      explicit StringViewHolder(std::string_view s) : sv(s) {}
    };

    auto v = axio::Split("a  ,   b,   , c,  d ,", ',') | axio::Trim |
             axio::SkipEmpty | axio::To<axio::Vector<std::string>>();
    CHECK_VECTOR(v, {"a", "b", "c", "d"});

    auto l = axio::Split("a  ,   b,   , c,  d ,", ',') | axio::Trim |
             axio::SkipEmpty | axio::To<std::list<StringViewHolder>>();
    CHECK_EQ(l.size(), v.Size());
    auto b = l.begin();
    for (axio::SizeT i = 0; i < v.Size(); ++i) {
      CHECK_EQ(v[i], (*b).sv);
      ++b;
    }
  }
}