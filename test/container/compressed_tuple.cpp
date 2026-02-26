#include <simpletest/simpletest.hpp>

#include <axio/container/compressed_tuple.hpp>

#include <string_view>

using axio::CompressedPair;
using axio::CompressedTuple;
using axio::Get;

constexpr bool ConstexprBasicTest() {
  CompressedTuple<int, int, char> t(1, 2, 'c');
  static_assert(sizeof(t) == 12, "");
  return t.Get<0>() == 1 && t.Get<1>() == 2 && t.Get<2>() == 'c';
}
static_assert(ConstexprBasicTest(), "");

constexpr bool ConstexprEqualityTest() {
  CompressedTuple<int, int, char> t1{1, 2, 'c'};
  CompressedTuple<int, int, char> t2{1, 2, 'c'};
  return t1 == t2;
}
static_assert(ConstexprEqualityTest(), "");

constexpr bool ConstexprInequalityTest() {
  CompressedTuple<int, int, char> t1{1, 2, 'c'};
  CompressedTuple<int, int, char> t2{2, 0, 'd'};
  return t1 != t2;
}
static_assert(ConstexprInequalityTest(), "");

struct Empty1 {};
struct Empty2 {};
struct Empty3 {};
constexpr bool ConstexprEBOTest_1() {
  using T = CompressedTuple<Empty1, int>;
  return sizeof(T) == sizeof(int);
}
static_assert(ConstexprEBOTest_1(), "");

constexpr bool ConstexprEBOTest_2() {
  using T1 = CompressedTuple<Empty1, Empty2, int, Empty3>;
  using T2 = CompressedTuple<Empty1, Empty2, Empty3, int>;
  return (sizeof(T1) == sizeof(int)) && (sizeof(T2) == sizeof(int));
}
static_assert(ConstexprEBOTest_2(), "");

TEST_CASE(CompressedTuple, BasicTest) {
  CompressedTuple<int, std::string> t{42, "hello"};
  CHECK_EQ(Get<0>(t), 42);
  CHECK_EQ(Get<1>(t), "hello");
}

constexpr bool CopyConstructorsTest() {
  CompressedTuple<int, const char*> t1{42, "abcd"};
  CompressedTuple<std::size_t, std::string_view> t2{t1};
  return Get<0>(t2) == 42 && Get<1>(t2) == "abcd";
}
static_assert(CopyConstructorsTest(), "");

TEST_CASE(CompressedTuple, CopyConstructors) {
  {
    CompressedTuple<int, std::string> t1{42, "abcd"};
    auto t2{t1};
    CHECK_EQ(Get<0>(t1), 42);
    CHECK_EQ(Get<1>(t1), "abcd");
    CHECK_EQ(Get<0>(t2), 42);
    CHECK_EQ(Get<1>(t2), "abcd");
  }
  {
    CompressedTuple<int, const char*> t1{42, "abcd"};
    CompressedTuple<std::size_t, std::string> t2{t1};
    CHECK_EQ(Get<0>(t2), 42);
    CHECK_EQ(Get<1>(t2), "abcd");
  }
}

constexpr bool MoveConstructorsTest() {
  axio::CompressedTuple<int, int> t1{42, 100};
  axio::CompressedTuple<int, int> t2{axio::Move(t1)};
  return axio::Get<0>(t2) == 42 && axio::Get<1>(t2) == 100;
}
static_assert(MoveConstructorsTest(), "");

TEST_CASE(CompressedTuple, MoveConstructors) {
  {
    CompressedTuple<int, std::string> t1{42, "abcd"};
    auto t2{axio::Move(t1)};
    CHECK_EQ(Get<0>(t1), 42);
    CHECK_EQ(Get<1>(t1), "");
    CHECK_EQ(Get<0>(t2), 42);
    CHECK_EQ(Get<1>(t2), "abcd");
  }

  {
    CompressedTuple<int, const char*> t1{42, "abcd"};
    CompressedTuple<std::size_t, std::string> t2{axio::Move(t1)};
    CHECK_EQ(Get<0>(t2), 42);
    CHECK_EQ(Get<1>(t2), "abcd");
  }
}

constexpr bool CopyAssignTest() {
  CompressedTuple<int, int> t1{42, 20};
  CompressedTuple<std::size_t, axio::Int64> t2{0, 4};
  t2 = t1;
  return Get<0>(t2) == 42 && Get<1>(t2) == 20;
}
static_assert(CopyAssignTest(), "");

TEST_CASE(CompressedTuple, CopyAssign) {
  {
    CompressedTuple<int, std::string> t1{42, "abcd"};
    CompressedTuple<int, std::string> t2{0, "a"};
    t2 = t1;
    CHECK_EQ(Get<0>(t1), 42);
    CHECK_EQ(Get<1>(t1), "abcd");
    CHECK_EQ(Get<0>(t2), 42);
    CHECK_EQ(Get<1>(t2), "abcd");
  }
  {
    CompressedTuple<int, const char*> t1{42, "abcd"};
    CompressedTuple<std::size_t, std::string> t2{0, "asd"};
    t2 = t1;
    CHECK_EQ(Get<0>(t2), 42);
    CHECK_EQ(Get<1>(t2), "abcd");
  }
}

constexpr bool MoveAssignTest() {
  CompressedTuple<int, int> t1{42, 20};
  CompressedTuple<std::size_t, axio::Int64> t2{0, 4};
  t2 = axio::Move(t1);
  return Get<0>(t2) == 42 && Get<1>(t2) == 20;
}
static_assert(MoveAssignTest(), "");

TEST_CASE(CompressedTuple, MoveAssign) {
  {
    CompressedTuple<int, std::string> t1{42, "abcd"};
    CompressedTuple<int, std::string> t2{0, "a"};
    t2 = axio::Move(t1);
    CHECK_EQ(Get<0>(t1), 42);
    CHECK_EQ(Get<1>(t1), "");
    CHECK_EQ(Get<0>(t2), 42);
    CHECK_EQ(Get<1>(t2), "abcd");
  }
  {
    CompressedTuple<int, const char*> t1{42, "abcd"};
    CompressedTuple<std::size_t, std::string> t2{4, "asd"};
    t2 = axio::Move(t1);
    CHECK_EQ(Get<0>(t2), 42);
    CHECK_EQ(Get<1>(t2), "abcd");
  }
}

TEST_CASE(CompressedTuple, LvalueGet) {
  CompressedTuple<int, std::string> t(10, "abcd");
  Get<0>(t) = 20;
  Get<1>(t).push_back('e');
  CHECK_EQ(Get<0>(t), 20);
  CHECK_EQ(Get<1>(t), "abcde");
}

TEST_CASE(CompressedTuple, RvalueGet) {
  CompressedTuple<int, std::string> t(10, "abcd");
  std::string s = Get<1>(axio::Move(t));
  CHECK_EQ(Get<0>(t), 10);
  CHECK_EQ(Get<1>(t), "");
  CHECK_EQ(s, "abcd");
}

TEST_CASE(CompressedTuple, ConstAccess) {
  const CompressedTuple<int, int> t(7, 8);
  CHECK_EQ(Get<0>(t), 7);
  CHECK_EQ(Get<1>(t), 8);
}

TEST_CASE(CompressedTuple, LargePackAccess) {
  CompressedTuple<int, int, int, int, int, int, int, int> t(1, 2, 3, 4, 5, 6, 7,
                                                            8);

  CHECK_EQ(Get<3>(t), 4);
  CHECK_EQ(Get<7>(t), 8);
}

struct NonEmpty {
  int value;
  constexpr NonEmpty(int v = 0) : value(v) {}
  constexpr bool operator==(const NonEmpty& other) const {
    return value == other.value;
  }
};
TEST_CASE(CompressedTuple, EqualityOperator) {
  CompressedTuple<int, NonEmpty> a(1, NonEmpty{2});
  CompressedTuple<int, NonEmpty> b(1, NonEmpty{2});
  CompressedTuple<int, NonEmpty> c(1, NonEmpty{3});

  CHECK_EQ(a, b);
  CHECK_NE(a, c);
}

TEST_CASE(CompressedTuple, EBOTests) {
  using T1 = CompressedTuple<Empty1>;
  using T2 = CompressedTuple<Empty1, Empty1>;
  using T3 = CompressedTuple<Empty1, int>;

  CHECK_EQ(sizeof(T1), sizeof(Empty1));
  CHECK_EQ(sizeof(T3), sizeof(int));
  CHECK_LE(sizeof(T2), sizeof(Empty1) * 2);  // no duplication penalty
}

struct MoveOnly {
  int value;
  MoveOnly(int v) : value(v) {}
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;
  MoveOnly(MoveOnly&&) noexcept = default;
  MoveOnly& operator=(MoveOnly&&) noexcept = default;

  bool operator==(const MoveOnly& other) const { return value == other.value; }
};
TEST_CASE(CompressedTuple, MoveOnlyType) {
  CompressedTuple<MoveOnly> t(MoveOnly{5});
  CHECK_EQ(Get<0>(t).value, 5);
}

TEST_CASE(CompressedTuple, ReferenceType) {
  int x = 100;
  CompressedTuple<int&> t(x);
  Get<0>(t) = 200;
  CHECK_EQ(x, 200);
}

TEST_CASE(CompressedTuple, TupleTraits) {
  IGNORE_RESULT();
  using T = CompressedTuple<int, double>;

  static_assert(axio::TupleSize<T>::value == 2, "");
  static_assert(axio::IsSame<axio::TupleElement<0, T>::type, int>::value, "");
  static_assert(axio::IsSame<axio::TupleElement<1, T>::type, double>::value,
                "");
}

TEST_CASE(CompressedTuple, SingleElementSpecialCase) {
  CompressedTuple<int> t(123);
  CHECK_EQ(Get<0>(t), 123);
}

TEST_CASE(CompressedTuple, NestedCompressedTuple) {
  CompressedTuple<int, CompressedTuple<int, int>> t(
      1, CompressedTuple<int, int>(2, 3));
  CHECK_EQ(Get<0>(t), 1);
  CHECK_EQ(Get<0>(Get<1>(t)), 2);
  CHECK_EQ(Get<1>(Get<1>(t)), 3);

  CompressedTuple<int> inner(5);
  CompressedTuple<CompressedTuple<int>> outer(inner);
  CHECK_EQ(Get<0>(Get<0>(outer)), 5);
}

TEST_CASE(CompressedTuple, EmptyTuple) {
  IGNORE_RESULT();
  CompressedTuple<> t;
  (void)t;
}

struct TrivialType {
  int x;
};

TEST_CASE(CompressedTuple, TriviallyDestructible) {
  IGNORE_RESULT();
  static_assert(std::is_trivially_destructible_v<
                CompressedTuple<TrivialType, int, double>>);
}

TEST_CASE(CompressedTuple, CompressedPairSize) {
  using P = axio::CompressedPair<Empty1, int>;
  CHECK_EQ(sizeof(P), sizeof(int));
}