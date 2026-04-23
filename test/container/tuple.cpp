#include <simpletest/simpletest.hpp>

#include <axio/base/macros.hpp>
#include <axio/container/tuple.hpp>
#include <string_view>

using axio::ForwardAsTuple;
using axio::Get;
using axio::MakeFromTuple;
using axio::MakeTuple;
using axio::Tuple;
using axio::TupleElement;
using axio::TupleSize;

struct Empty {
  constexpr Empty() = default;
  constexpr int Foo() const { return 42; }
};

constexpr bool ConstexprConstructorTest() {
  bool ret = true;
  {
    Tuple<> empty1{};
    Tuple<> empty2{empty1};
    Tuple<> empty3{axio::Move(empty2)};
    static_assert(TupleSize<decltype(empty1)>::value == 0, "");

    Tuple<Tuple<>, int> t{{}, 2};
    static_assert(TupleSize<decltype(t)>::value == 2, "");
  }
  {
    Tuple<int, int, int> t{};
    static_assert(sizeof(t) == 12, "");
    ret = ret && (Get<0>(t) == 0 && Get<1>(t) == 0 && Get<2>(t) == 0);
  }
  {
    struct A {
      A() = delete;
    };
    static_assert(!axio::IsDefaultConstructible<Tuple<A>>::value, "");
    static_assert(axio::IsDefaultConstructible<Tuple<int, double>>::value, "");
    static_assert(
        axio::IsNothrowConstructible<Tuple<int, double>,
                                     const Tuple<int, double>&>::value,
        "");
  }
  {
    Tuple<int, int, int> t(1, 2, 3);
    ret = ret && (Get<0>(t) == 1 && Get<1>(t) == 2 && Get<2>(t) == 3);
  }
  {
    int x = 1, y = 2, z = 3;
    Tuple<int, int, int> t(x, y, z);
    x = 11;
    y = 22;
    z = 33;
    ret = ret && (Get<0>(t) == 1 && Get<1>(t) == 2 && Get<2>(t) == 3);
  }
  {
    int x = 1, y = 2, z = 3;
    Tuple<int, int, int> t(x, y, z);
    Tuple copy(t);
    Tuple moved(axio::Move(t));
    static_assert(axio::IsSame<decltype(copy), Tuple<int, int, int>>::value,
                  "");
    static_assert(axio::IsSame<decltype(moved), Tuple<int, int, int>>::value,
                  "");
    ret = ret && (Get<0>(t) == 1 && Get<1>(t) == 2 && Get<2>(t) == 3);
  }
  {
    int x = 1, y = 2, z = 3;
    Tuple<int&, int&, int&> t(x, y, z);
    x = 10;
    y = 20;
    z = 30;
    ret = ret && (Get<0>(t) == 10 && Get<1>(t) == 20 && Get<2>(t) == 30);
  }
  {
    int x = 1, y = 2, z = 3;
    Tuple<int&, int&, int&> t(x, y, z);
    Tuple<int&, int&, int&> copy(t);
    x = 10;
    y = 20;
    z = 30;
    ret = ret && (Get<0>(t) == 10 && Get<1>(t) == 20 && Get<2>(t) == 30);
    ret =
        ret && (Get<0>(copy) == 10 && Get<1>(copy) == 20 && Get<2>(copy) == 30);
  }
  {
    Tuple<const char*, int, char> t1("hello", 1, '0');
    Tuple<std::string_view, int, int> t2(t1);
    ret = ret && (Get<0>(t2) == "hello" && Get<1>(t2) == 1 && Get<2>(t2) == 48);
  }
  {
    Tuple<std::string_view, int, int> t2(
        Tuple<const char*, int, char>("hello", 1, '0'));
    ret = ret && (Get<0>(t2) == "hello" && Get<1>(t2) == 1 && Get<2>(t2) == 48);
  }
  {
    Tuple<Empty, int, int> t({}, 1, 1);
    static_assert(Get<0>(t).Foo() == 42, "");
    static_assert(sizeof(t) == 0 + sizeof(int) + sizeof(int), "");

    Tuple<Empty, Empty, Empty, int, Empty> t1({}, {}, {}, 1, {});
    static_assert(Get<0>(t1).Foo() == 42, "");
    static_assert(Get<1>(t1).Foo() == 42, "");
    static_assert(Get<2>(t1).Foo() == 42, "");
    static_assert(Get<4>(t1).Foo() == 42, "");
  }
  return ret;
}
static_assert(ConstexprConstructorTest(), "");

constexpr bool ConstexprAssignTest() {
  bool ret = true;
  {
    Tuple<> empty1{};
    Tuple<> empty2{};
    Tuple<> empty3{};
    empty2 = empty1;
    empty3 = axio::Move(empty1);
    AXIO_IGNORE(empty1);
    AXIO_IGNORE(empty2);
    AXIO_IGNORE(empty3);

    Tuple<Tuple<>, int> t1{{}, 1};
    Tuple<Tuple<>, int> t2{{}, 2};
    t1 = t2;
    ret = ret && Get<0>(t1) == Get<0>(t2);
    ret = ret && Get<1>(t1) == Get<1>(t2);
  }
  {
    Tuple<int, double> a(1, 2.0);
    Tuple<int, double> b(10, 20.0);
    b = a;
    ret = ret && (Get<0>(b) == 1);
    ret = ret && (Get<1>(b) == 2.0);
  }
  {
    Tuple<int, double> a(1, 2.0);
    Tuple<int, double> b(10, 20.0);
    b = axio::Move(a);
    ret = ret && (Get<0>(b) == 1);
    ret = ret && (Get<1>(b) == 2.0);
  }
  {
    Tuple<const char*, int, char> a("hello", 1, '0');
    Tuple<std::string_view, int, int> b;
    b = a;
    ret = ret && (Get<0>(b) == "hello");
    ret = ret && (Get<1>(b) == 1);
    ret = ret && (Get<2>(b) == 48);
  }
  {
    Tuple<std::string_view, int, int> b;
    b = Tuple<const char*, int, char>("hello", 1, '0');
    ret = ret && (Get<0>(b) == "hello");
    ret = ret && (Get<1>(b) == 1);
    ret = ret && (Get<2>(b) == 48);
  }
  {
    // Must not compile
    // struct A { A& operator=(const A&) = delete; };
    // Tuple<A> a, b;
    // a = b;
  }
  {
    Tuple<int, double> a(1, 2.0);
    a = a;
    ret = ret && (Get<0>(a) == 1 && Get<1>(a) == 2.0);
  }
  {
    Tuple<int, double> a(1, 2.0);
    Tuple<long, long double> b;
    b = a;
    ret = ret && (Get<0>(b) == 1);
    ret = ret && (Get<1>(b) == 2.0L);
  }
  return ret;
}
static_assert(ConstexprAssignTest(), "");

constexpr bool ConstexprGet() {
  bool ret = true;
  {
    // Must not compile
    // Tuple<> empty{};
    // Get<0>(empty);
  }
  {
    Tuple<int, double, char> t(1, 2.5, 'a');
    ret = ret && Get<0>(t) == 1 && Get<1>(t) == 2.5 && Get<2>(t) == 'a';
  }
  {
    int x = 10;
    double y = 2.5;
    Tuple<int&, double&> t(x, y);
    Get<0>(t) = 20;
    Get<1>(t) = 5.5;
    ret = ret && x == 20 && y == 5.5;
  }
  {
    const Tuple<int, double, char> t(1, 2.5, 'a');
    ret = ret && Get<0>(t) == 1 && Get<1>(t) == 2.5 && Get<2>(t) == 'a';
  }
  {
    Tuple<std::string_view> t("hello");
    auto&& val = Get<0>(axio::Move(t));
    ret = ret && val == "hello";
  }
  {
    const Tuple<int> t(42);
    const auto&& ref = Get<0>(axio::Move(t));
    ret = ret && ref == 42;
  }
  {
    Tuple<Empty, int> t1({}, 5);
    ret = ret && Get<0>(t1).Foo() == 42 && Get<1>(t1) == 5;

    const Tuple<Empty, int> t2({}, 5);
    ret = ret && Get<0>(t2).Foo() == 42 && Get<1>(t2) == 5;

    Tuple<Empty, int> t3({}, 5);
    ret = ret && Get<0>(axio::Move(t3)).Foo() == 42;

    const Tuple<Empty, int> t4({}, 5);
    ret = ret && Get<0>(axio::Move(t4)).Foo() == 42;
  }
  {
    Tuple<int, int, double> t(1, 2, 3.0);
    Get<0>(t) = 99;
    Get<1>(t) = 99;
    Get<2>(t) = 99.0;
    ret = ret && Get<0>(t) == 99 && Get<1>(t) == 99 && Get<2>(t) == 99.0;
  }
  {
    Tuple<const char*, int, char> t("hello", 1, '0');
    Tuple<std::string_view, int, int> t2(t);
    ret = ret && Get<0>(t2) == "hello" && Get<1>(t2) == 1 && Get<2>(t2) == 48;
  }
  {
    Tuple<int, int, int> t(1, 2, 3);
    Get<0>(t) += Get<1>(t);
    Get<1>(t) += Get<2>(t);
    Get<2>(t) += Get<0>(t);
    ret = ret && Get<0>(t) == 3 && Get<1>(t) == 5 && Get<2>(t) == 6;
  }
  {
    Tuple<int, double, char> t(1, 2.0, 'a');
    auto& a = Get<0>(t);
    auto& b = Get<1>(t);
    auto& c = Get<2>(t);
    a += 10;
    b *= 2;
    c = 'z';

    ret = ret && a == 11 && b == 4.0 && c == 'z';
  }
  return ret;
}
static_assert(ConstexprGet(), "");

constexpr bool ConstexprMakeTuple() {
  bool ret = true;
  {
    auto t = MakeTuple();
    static_assert(axio::IsSame<decltype(t), Tuple<>>::value, "");
  }
  {
    auto t = MakeTuple(1, 2.0, 'a');
    static_assert(axio::IsSame<decltype(t), Tuple<int, double, char>>::value,
                  "");
    ret = ret && Get<0>(t) == 1 && Get<1>(t) == 2.0 && Get<2>(t) == 'a';
  }
  {
    int x = 10;
    auto t = MakeTuple(x);
    static_assert(axio::IsSame<decltype(t), Tuple<int>>::value, "");
    x = 42;
    ret = ret && Get<0>(t) == 10;
    static_assert(axio::IsSame<decltype(MakeTuple(std::declval<int&>())),
                               Tuple<int>>::value,
                  "");
  }
  {
    const int x = 10;
    auto t = MakeTuple(x);
    static_assert(axio::IsSame<decltype(t), Tuple<int>>::value, "");
    ret = ret && Get<0>(t) == 10;
    static_assert(axio::IsSame<decltype(MakeTuple(std::declval<const int&>())),
                               Tuple<int>>::value,
                  "");
  }
  {
    std::string_view sv = "hello";
    int x = 10;
    auto t = MakeTuple(axio::Move(sv), x);
    static_assert(
        axio::IsSame<decltype(t), Tuple<std::string_view, int>>::value, "");
    ret = ret && Get<0>(t) == "hello" && Get<1>(t) == 10;
  }
  return ret;
}
static_assert(ConstexprMakeTuple(), "");

constexpr bool ConstexprForwardAsTuple() {
  bool ret = true;
  {
    auto t = ForwardAsTuple();
    static_assert(axio::IsSame<decltype(t), Tuple<>>::value, "");
  }
  {
    int x = 10;
    double y = 2.5;
    auto t = ForwardAsTuple(x, y);
    Get<0>(t) = 20;
    Get<1>(t) = 5.5;
    ret = ret && x == 20 && y == 5.5;
    static_assert(
        axio::IsSame<decltype(ForwardAsTuple(std::declval<int&>(),
                                             std::declval<double&>())),
                     Tuple<int&, double&>>::value,
        "");
    static_assert(
        axio::IsSame<decltype(ForwardAsTuple(std::declval<const int&>(),
                                             std::declval<const double&>())),
                     Tuple<const int&, const double&>>::value,
        "");
  }
  {
    int x = 1;
    int y = 2;
    auto t = ForwardAsTuple(x, y);
    Get<0>(t) += Get<1>(t);
    Get<1>(t) += Get<0>(t);
    ret = ret && x == 3 && y == 5;
  }
  {
    int x = 1;
    const int y = 2;
    auto t = ForwardAsTuple(x, y);
    Get<0>(t) = 10;
    ret = ret && x == 10 && Get<1>(t) == 2;
  }
  return ret;
}
static_assert(ConstexprForwardAsTuple(), "");

static_assert(TupleSize<Tuple<int, double, char>>::value == 3, "");
static_assert(TupleSize<Tuple<int, double>>::value == 2, "");
static_assert(TupleSize<Tuple<int>>::value == 1, "");
static_assert(TupleSize<Tuple<>>::value == 0, "");

static_assert(
    axio::IsSame<TupleElement<0, Tuple<int, double, char>>::type, int>::value,
    "");
static_assert(axio::IsSame<TupleElement<1, Tuple<int, double, char>>::type,
                           double>::value,
              "");
static_assert(
    axio::IsSame<TupleElement<2, Tuple<int, double, char>>::type, char>::value,
    "");

// won't compile =))
// using X = TupleElement<3, Tuple<int, double, char>>::type;

static_assert(axio::IsSame<TupleElement<0, const Tuple<int, double>>::type,
                           const int>::value,
              "");
static_assert(axio::IsSame<TupleElement<0, volatile Tuple<int, double>>::type,
                           volatile int>::value,
              "");
static_assert(
    axio::IsSame<TupleElement<0, const volatile Tuple<int, double>>::type,
                 const volatile int>::value,
    "");

constexpr bool ConstepxrStructuredBinding() {
  bool ret = true;
  {
    Tuple<int, double, char> t{1, 2.0, 'a'};
    auto [a, b, c] = t;
    ret = ret && a == 1 && b == 2.0 && c == 'a';
  }
  {
    int x = 10;
    Tuple<Empty, int&> t({}, x);
    auto [a, b] = t;
    b = 42;
    ret = ret && a.Foo() == 42;
    ret = ret && x == 42;
  }
  return ret;
}
static_assert(ConstepxrStructuredBinding(), "");

struct Foo {
  int foo;
  int bar;

  constexpr Foo() : foo(0), bar(0) {};
  constexpr Foo(int f, int b) : foo(f), bar(b) {}
};

struct Ref {
  int& ref;
  constexpr Ref(int& r) : ref(r) {}
};
constexpr bool ConstexprMakeFromTuple() {
  bool ret = true;
  {
    auto t = MakeTuple();
    Foo res = MakeFromTuple<Foo>(t);
  }
  {
    auto t = MakeTuple(1, 2);
    auto f = MakeFromTuple<Foo>(t);
    ret = ret && f.foo == 1 && f.bar == 2;
  }
  {
    int val = 42;
    auto t = ForwardAsTuple(val);
    auto obj = MakeFromTuple<Ref>(t);
    ret = ret && (&obj.ref == &val);
    val = 50;
    ret = ret && (obj.ref == 50);
  }
  return ret;
}
static_assert(ConstexprMakeFromTuple(), "");

using axio::TupleCat;
constexpr bool ConstexprTupleCat() {
  bool ret = true;
  {
    auto empty = TupleCat();
    static_assert(axio::IsSame<decltype(empty), Tuple<>>::value, "");

    auto t =
        TupleCat(empty, Tuple<int, int>{1, 2}, Tuple<std::string_view>{"foo"});
    static_assert(
        axio::IsSame<decltype(t), Tuple<int, int, std::string_view>>::value,
        "");
  }
  {
    axio::Tuple<int, char, char> a(1, 'a', 'b');
    axio::Tuple<double, int> b(2.5, 42);
    auto r = TupleCat(a, b);
    static_assert(
        axio::IsSame<decltype(r), Tuple<int, char, char, double, int>>::value,
        "");
    ret = ret && Get<0>(r) == 1 && Get<1>(r) == 'a' && Get<2>(r) == 'b';
    ret = ret && Get<3>(r) == 2.5 && Get<4>(r) == 42;
  }
  {
    int x = 1, y = 2, z = 3;
    double a = 2.5, b = 3.14, c = 1.0;
    std::string_view sv{"hello"};
    Tuple<int&, int&, double, double&> t1{x, y, a, b};
    Tuple<int&, int> t2{z, 3};
    Tuple<std::string_view, Empty> t3{sv, {}};
    Tuple<double> t4{c};
    auto t = TupleCat(t1, t2, t3, t4);
    static_assert(axio::IsSame<decltype(t),
                               Tuple<int&, int&, double, double&, int&, int,
                                     std::string_view, Empty, double>>::value,
                  "");
    x = 11;
    y = 22;
    z = 33;
    a = 4.5;
    b = 2.8;
    c = 9.0;
    ret = ret && Get<0>(t) == 11 && Get<1>(t) == 22 && Get<2>(t) == 2.5 &&
          Get<3>(t) == 2.8;
    ret = ret && Get<4>(t) == 33 && Get<5>(t) == 3;
    ret = ret && Get<6>(t) == "hello" && Get<7>(t).Foo() == 42;
    ret = ret && Get<8>(t) == 1.0;
  }
  {
    Tuple<Tuple<int, double>, char, int> t1{Tuple<int, double>{1, 3.0}, 'a',
                                            42};
    Tuple<double> t2{2.0};
    auto t = TupleCat(t1, t2);
    static_assert(
        axio::IsSame<decltype(t),
                     Tuple<Tuple<int, double>, char, int, double>>::value,
        "");
    ret = ret && Get<0>(Get<0>(t)) == 1 && Get<1>(Get<0>(t)) == 3.0;
    ret = ret && Get<1>(t) == 'a' && Get<2>(t) == 42 && Get<3>(t) == 2.0;
  }
  {
    Tuple<int, char> t1{1, 'a'};
    Tuple<double, const char*> t2{3.0, "bar"};
    auto t = TupleCat(Tuple<std::string_view>("foo"), t1, Move(t2));
    static_assert(axio::IsSame<decltype(t), Tuple<std::string_view, int, char,
                                                  double, const char*>>::value,
                  "");
    ret = ret && Get<0>(t) == "foo";
    ret = ret && Get<1>(t) == 1 && Get<2>(t) == 'a';
    ret = ret && Get<3>(t) == 3.0 && Get<4>(t) == std::string_view{"bar"};
  }
  static_assert(
      axio::IsSame<decltype(TupleCat(
                       std::declval<Tuple<int, int&, const int&>>(),
                       std::declval<Tuple<int, const int, const volatile int,
                                          volatile int>>())),
                   Tuple<int, int&, const int&, int, const int,
                         const volatile int, volatile int>>::value,
      "");
  return ret;
}
static_assert(ConstexprTupleCat(), "");

constexpr bool ConstexprTupleComparison() {
  bool ret = true;

  {
    Tuple<int, int, int> a{1, 2, 3};
    Tuple<int, int, int> b{1, 2, 4};
    Tuple<int, int, int> c{1, 2, 3};
    Tuple<int, int, int> d{1, 3, 0};
    Tuple<int, int, int> e{2, 0, 0};
    Tuple<int, int, int> f{1, 2, 0};
    Tuple<int, int, int> g{1, 1, 100};

    ret = ret && (a == c);
    ret = ret && (!(a == b));
    ret = ret && (!(a == f));

    ret = ret && (a != b);
    ret = ret && (!(a != c));

    ret = ret && (a < b);
    ret = ret && (a < d);
    ret = ret && (a < e);
    ret = ret && (f < a);
    ret = ret && (g < a);

    ret = ret && (!(b < a));
    ret = ret && (!(a < c));

    ret = ret && (a <= b);
    ret = ret && (a <= c);
    ret = ret && (f <= a);
    ret = ret && (!(e <= a));

    ret = ret && (e > a);
    ret = ret && (b > a);
    ret = ret && (d > a);
    ret = ret && (!(a > b));

    ret = ret && (e >= a);
    ret = ret && (a >= c);
    ret = ret && (a >= f);
    ret = ret && (!(a >= b));
  }
  {
    Tuple<> t1{};
    Tuple<> t2{};

    ret = ret && t1 == t2;
    ret = ret && t1 >= t2;
    ret = ret && t1 <= t2;
    ret = ret && !(t1 != t2);
    ret = ret && !(t1 > t2);
    ret = ret && !(t1 < t2);
  }

  return ret;
}
static_assert(ConstexprTupleComparison(), "");

#define CHECK_TUPLE_VALUE_EQ(t, idx, expected) CHECK_EQ(Get<idx>(t), expected);

struct Tracker {
  static inline int default_construct = 0;
  static inline int value_construct = 0;
  static inline int copy_construct = 0;
  static inline int move_construct = 0;
  static inline int copy_assign = 0;
  static inline int move_assign = 0;
  static inline int value_assign = 0;

  int value;

  Tracker() : value(0) { ++default_construct; }

  Tracker(int v) : value(v) { ++value_construct; }

  Tracker(const Tracker& other) : value(other.value) { ++copy_construct; }

  Tracker(Tracker&& other) noexcept : value(other.value) {
    ++move_construct;
    other.value = -1;
  }

  Tracker& operator=(const Tracker& other) {
    value = other.value;
    ++copy_assign;
    return *this;
  }

  Tracker& operator=(Tracker&& other) noexcept {
    value = other.value;
    other.value = -1;
    ++move_assign;
    return *this;
  }

  Tracker& operator=(int v) {
    value = v;
    ++value_assign;
    return *this;
  }

  friend bool operator==(const Tracker& a, const Tracker& b) {
    return a.value == b.value;
  }

  friend bool operator!=(const Tracker& a, const Tracker& b) {
    return !(a == b);
  }

  friend bool operator<(const Tracker& a, const Tracker& b) {
    return a.value < b.value;
  }

  friend bool operator>(const Tracker& a, const Tracker& b) { return b < a; }

  friend bool operator<=(const Tracker& a, const Tracker& b) {
    return !(b < a);
  }

  friend bool operator>=(const Tracker& a, const Tracker& b) {
    return !(a < b);
  }

  friend bool operator==(const Tracker& a, int b) { return a.value == b; }

  friend bool operator==(int a, const Tracker& b) { return a == b.value; }

  friend bool operator<(const Tracker& a, int b) { return a.value < b; }

  friend bool operator<(int a, const Tracker& b) { return a < b.value; }

  static void Reset() {
    default_construct = 0;
    value_construct = 0;
    copy_construct = 0;
    move_construct = 0;
    copy_assign = 0;
    move_assign = 0;
    value_assign = 0;
  }
};

#define CHECK_TRACKER(default_construct_, value_construct_, copy_construct_, \
                      move_construct_, copy_assign_, move_assign_,           \
                      value_assign_)                                         \
  CHECK_EQ(Tracker::default_construct, default_construct_);                  \
  CHECK_EQ(Tracker::value_construct, value_construct_);                      \
  CHECK_EQ(Tracker::copy_construct, copy_construct_);                        \
  CHECK_EQ(Tracker::move_construct, move_construct_);                        \
  CHECK_EQ(Tracker::copy_assign, copy_assign_);                              \
  CHECK_EQ(Tracker::move_assign, move_assign_);                              \
  CHECK_EQ(Tracker::value_assign, value_assign_);

TEST_CASE(Tuple, DefaultConstructor) {
  {
    Tuple<> t{};
    AXIO_IGNORE(t);
  }
  Tracker::Reset();
  Tuple<int, int, double, Tracker> t{};
  CHECK_TUPLE_VALUE_EQ(t, 0, 0);
  CHECK_TUPLE_VALUE_EQ(t, 1, 0);
  CHECK_TUPLE_VALUE_EQ(t, 2, 0.0);
  CHECK_TUPLE_VALUE_EQ(t, 3, 0);
  CHECK_TRACKER(1, 0, 0, 0, 0, 0, 0);
}

TEST_CASE(Tuple, CopyVariadicConstructor) {
  Tracker::Reset();
  const int a = 10;
  const double b = 3.14;
  const Tracker c{42};
  const Tracker d{10};
  CHECK_TRACKER(0, 2, 0, 0, 0, 0, 0);
  Tuple<int, double, Tracker, Tracker> t{a, b, c, d};
  CHECK_TUPLE_VALUE_EQ(t, 0, 10);
  CHECK_TUPLE_VALUE_EQ(t, 1, 3.14);
  CHECK_TUPLE_VALUE_EQ(t, 2, 42);
  CHECK_TUPLE_VALUE_EQ(t, 3, 10);
  CHECK_TRACKER(0, 2, 2, 0, 0, 0, 0);
}

TEST_CASE(Tuple, ForwardingVariadicConstructor) {
  {
    Tracker::Reset();
    Tuple<Tracker, Tracker, Tracker> t{1, 2, 3};
    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
    CHECK_TUPLE_VALUE_EQ(t, 2, 3);
    CHECK_TRACKER(0, 3, 0, 0, 0, 0, 0);
  }
  {
    Tracker::Reset();
    Tuple<Tracker, Tracker, Tracker> t{Tracker{1}, Tracker{2}, Tracker{3}};
    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
    CHECK_TUPLE_VALUE_EQ(t, 2, 3);
    CHECK_TRACKER(0, 3, 0, 3, 0, 0, 0);
  }
  {
    Tracker::Reset();
    Tracker a{1};
    CHECK_TRACKER(0, 1, 0, 0, 0, 0, 0);
    Tuple<Tracker, Tracker, Tracker, Tracker> t{a, 2, Tracker{3},
                                                axio::Move(a)};
    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
    CHECK_TUPLE_VALUE_EQ(t, 2, 3);
    CHECK_TUPLE_VALUE_EQ(t, 3, 1);
    CHECK_EQ(a.value, -1);
    CHECK_TRACKER(0, 3, 1, 2, 0, 0, 0);
  }
}

TEST_CASE(Tuple, CopyConstructor) {
  {
    Tracker::Reset();
    Tuple<Tracker, std::string, int> t{1, "hello", 3};
    CHECK_TRACKER(0, 1, 0, 0, 0, 0, 0);
    Tuple<Tracker, std::string, int> copy{t};
    CHECK_TRACKER(0, 1, 1, 0, 0, 0, 0);
    CHECK_TUPLE_VALUE_EQ(copy, 0, 1);
    CHECK_TUPLE_VALUE_EQ(copy, 1, "hello");
    CHECK_TUPLE_VALUE_EQ(copy, 2, 3);
  }
  {
    Tracker::Reset();
    Tuple<int, int, const char*> t1{1, 2, "foo"};
    Tuple<Tracker, Tracker, std::string> t{t1};
    CHECK_TRACKER(0, 2, 0, 0, 0, 0, 0);
    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
    CHECK_TUPLE_VALUE_EQ(t, 2, "foo");
  }
  {
    Tracker::Reset();
    Tuple<int, int, const char*, std::string> t1{1, 2, "foo", "bar"};
    Tuple<Tracker, Tracker, std::string, std::string> t{t1};
    CHECK_TRACKER(0, 2, 0, 0, 0, 0, 0);
    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
    CHECK_TUPLE_VALUE_EQ(t, 2, "foo");
    CHECK_TUPLE_VALUE_EQ(t, 3, "bar");
  }
}

TEST_CASE(Tuple, MoveConstructor) {
  {
    Tracker::Reset();
    Tuple<Tracker, std::string, int> t{2, "aa", 4};
    CHECK_TRACKER(0, 1, 0, 0, 0, 0, 0);
    Tuple<Tracker, std::string, int> move{axio::Move(t)};
    CHECK_TUPLE_VALUE_EQ(move, 0, 2);
    CHECK_TUPLE_VALUE_EQ(move, 1, "aa");
    CHECK_TUPLE_VALUE_EQ(move, 2, 4);
    CHECK_TUPLE_VALUE_EQ(t, 0, -1);
    CHECK_TRACKER(0, 1, 0, 1, 0, 0, 0);
  }
  {
    Tracker::Reset();
    Tuple<Tracker, std::string, int> t{2, "aa", 4};
    CHECK_TRACKER(0, 1, 0, 0, 0, 0, 0);
    Tuple<Tracker, std::string, Tracker> move{axio::Move(t)};
    CHECK_TUPLE_VALUE_EQ(move, 0, 2);
    CHECK_TUPLE_VALUE_EQ(move, 1, "aa");
    CHECK_TUPLE_VALUE_EQ(move, 2, 4);
    CHECK_TUPLE_VALUE_EQ(t, 0, -1);
    CHECK_TRACKER(0, 2, 0, 1, 0, 0, 0);
  }
}

TEST_CASE(Tuple, CopyAssign) {
  {
    Tracker::Reset();
    Tuple<Tracker, int, std::string> t1{100, 200, "300"};
    CHECK_TRACKER(0, 1, 0, 0, 0, 0, 0);
    Tuple<Tracker, int, std::string> t2{1, 2, "3"};
    CHECK_TRACKER(0, 2, 0, 0, 0, 0, 0);
    Tracker::Reset();
    t2 = t1;
    CHECK_TRACKER(0, 0, 0, 0, 1, 0, 0);
    CHECK_TUPLE_VALUE_EQ(t2, 0, 100);
    CHECK_TUPLE_VALUE_EQ(t2, 1, 200);
    CHECK_TUPLE_VALUE_EQ(t2, 2, "300");

    CHECK_TUPLE_VALUE_EQ(t1, 0, 100);
    CHECK_TUPLE_VALUE_EQ(t1, 1, 200);
    CHECK_TUPLE_VALUE_EQ(t1, 2, "300");
  }
  {
    Tracker::Reset();
    Tuple<int, int, const char*> t1{100, 200, "300"};
    Tuple<Tracker, Tracker, std::string> t2{1, 2, "3"};
    CHECK_TRACKER(0, 2, 0, 0, 0, 0, 0);
    Tracker::Reset();
    t2 = t1;
    CHECK_TRACKER(0, 0, 0, 0, 0, 0, 2);
    CHECK_TUPLE_VALUE_EQ(t2, 0, 100);
    CHECK_TUPLE_VALUE_EQ(t2, 1, 200);
    CHECK_TUPLE_VALUE_EQ(t2, 2, "300");
  }
}

TEST_CASE(Tuple, MoveAssign) {
  {
    Tracker::Reset();
    Tuple<Tracker, int, std::string> t1{100, 200, "300"};
    CHECK_TRACKER(0, 1, 0, 0, 0, 0, 0);
    Tuple<Tracker, int, std::string> t2{1, 2, "3"};
    CHECK_TRACKER(0, 2, 0, 0, 0, 0, 0);
    Tracker::Reset();
    t2 = axio::Move(t1);
    CHECK_TRACKER(0, 0, 0, 0, 0, 1, 0);
    CHECK_TUPLE_VALUE_EQ(t2, 0, 100);
    CHECK_TUPLE_VALUE_EQ(t2, 1, 200);
    CHECK_TUPLE_VALUE_EQ(t2, 2, "300");

    CHECK_TUPLE_VALUE_EQ(t1, 0, -1);
  }
  {
    Tracker::Reset();
    Tuple<Tracker, int, const char*> t1{100, 200, "300"};
    CHECK_TRACKER(0, 1, 0, 0, 0, 0, 0);
    Tuple<Tracker, Tracker, std::string> t2{1, 2, "3"};
    CHECK_TRACKER(0, 3, 0, 0, 0, 0, 0);
    Tracker::Reset();
    t2 = axio::Move(t1);
    CHECK_TRACKER(0, 0, 0, 0, 0, 1, 1);
    CHECK_TUPLE_VALUE_EQ(t2, 0, 100);
    CHECK_TUPLE_VALUE_EQ(t2, 1, 200);
    CHECK_TUPLE_VALUE_EQ(t2, 2, "300");

    CHECK_TUPLE_VALUE_EQ(t1, 0, -1);
  }
}

TEST_CASE(Tuple, LvalueReturnsLRef) {
  Tuple<int, std::string> t(1, "hello");

  static_assert(axio::IsLvalueReference<decltype(Get<0>(t))>::value, "");
  static_assert(axio::IsLvalueReference<decltype(Get<1>(t))>::value, "");

  CHECK_EQ(Get<0>(t), 1);
  CHECK_EQ(Get<1>(t), "hello");
}

TEST_CASE(Tuple, ConstLvalueReturnsConstRef) {
  const Tuple<int, std::string> t(1, "hello");

  static_assert(
      axio::IsConst<
          typename axio::RemoveReference<decltype(Get<0>(t))>::type>::value,
      "");
  static_assert(
      axio::IsConst<
          typename axio::RemoveReference<decltype(Get<1>(t))>::type>::value,
      "");
  static_assert(axio::IsLvalueReference<decltype(Get<0>(t))>::value, "");
  static_assert(axio::IsLvalueReference<decltype(Get<1>(t))>::value, "");

  CHECK_EQ(Get<0>(t), 1);
  CHECK_EQ(Get<1>(t), "hello");
}

static_assert(axio::IsRvalueReference<decltype(Get<0>(Tuple<int>(1)))>::value,
              "");

TEST_CASE(Tuple, MoveFromRvalueTuple) {
  Tracker::Reset();
  Tuple<Tracker> t(2);
  CHECK_TRACKER(0, 1, 0, 0, 0, 0, 0);
  auto&& ref = Get<0>(axio::Move(t));
  CHECK_EQ(ref, 2);
}

TEST_CASE(Tuple, ForwardingLvalueVsRvalue) {
  std::string s = "hello";
  Tuple<std::string> t(s);

  std::string& r1 = Get<0>(t);
  r1 = "changed";

  CHECK_EQ(Get<0>(t), "changed");

  std::string&& r2 = Get<0>(axio::Move(t));
  CHECK_EQ(r2, "changed");
}

static_assert(axio::IsSame<decltype(Get<0>(Tuple<int>(1))), int&&>::value, "");

TEST_CASE(Tuple, MakeTupleCopiesLvalues) {
  Tracker::Reset();
  Tracker a(10);
  Tracker b(20);
  CHECK_TRACKER(0, 2, 0, 0, 0, 0, 0);

  auto t = MakeTuple(a, b);
  CHECK_TRACKER(0, 2, 2, 0, 0, 0, 0);
  CHECK_TUPLE_VALUE_EQ(t, 0, 10);
  CHECK_TUPLE_VALUE_EQ(t, 1, 20);
}

TEST_CASE(Tuple, MakeTupleMovesRvalues) {
  Tracker::Reset();

  auto t = MakeTuple(Tracker(10), Tracker(20));
  CHECK_TRACKER(0, 2, 0, 2, 0, 0, 0);
  CHECK_TUPLE_VALUE_EQ(t, 0, 10);
  CHECK_TUPLE_VALUE_EQ(t, 1, 20);
}

TEST_CASE(Tuple, MakeTupleMixed) {
  Tracker::Reset();

  Tracker a(10);
  Tracker b(10);
  CHECK_TRACKER(0, 2, 0, 0, 0, 0, 0);

  auto t = MakeTuple(a, Tracker(20), axio::Move(b));
  CHECK_TRACKER(0, 3, 1, 2, 0, 0, 0);

  CHECK_EQ(b.value, -1);
  CHECK_TUPLE_VALUE_EQ(t, 0, 10);
  CHECK_TUPLE_VALUE_EQ(t, 1, 20);
  CHECK_TUPLE_VALUE_EQ(t, 2, 10);
}

TEST_CASE(Tuple, ForwardAsTuple) {
  int x = 10;
  std::string s = "hello";
  auto t = ForwardAsTuple(x, s);

  Get<0>(t) = 99;
  Get<1>(t) = "world";

  CHECK_EQ(x, 99);
  CHECK_EQ(s, "world");
}

TEST_CASE(Tuple, ForwardAsTuple_NoCopy) {
  Tracker::Reset();

  Tracker a(10);
  Tracker b(10);
  CHECK_TRACKER(0, 2, 0, 0, 0, 0, 0);
  auto t = ForwardAsTuple(a, b);
  a.value = 100;
  b.value = 200;
  CHECK_EQ(Get<0>(t), 100);
  CHECK_EQ(Get<1>(t), 200);
  CHECK_TRACKER(0, 2, 0, 0, 0, 0, 0);
}

TEST_CASE(Tuple, MakeFromTuple) {
  struct Bar {
    int x;
    double y;
    std::string z;
    Bar(int i, double d, const std::string& s) : x(i), y(d), z(s) {};
  };
  auto t = MakeTuple(1, 3.14, "foo");
  auto b = MakeFromTuple<Bar>(t);
  CHECK_EQ(b.x, 1);
  CHECK_EQ(b.y, 3.14);
  CHECK_EQ(b.z, "foo");
}

TEST_CASE(Tuple, MakeFromTuple_Lvalue) {
  Tracker::Reset();

  Tuple<Tracker, Tracker> src(Tracker(1), Tracker(2));
  CHECK_TRACKER(0, 2, 0, 2, 0, 0, 0);
  auto t = MakeFromTuple<Tuple<Tracker, Tracker>>(src);
  CHECK_TRACKER(0, 2, 2, 2, 0, 0, 0);

  CHECK_TUPLE_VALUE_EQ(src, 0, 1);
  CHECK_TUPLE_VALUE_EQ(src, 1, 2);

  CHECK_TUPLE_VALUE_EQ(t, 0, 1);
  CHECK_TUPLE_VALUE_EQ(t, 1, 2);
}

TEST_CASE(Tuple, MakeFromTuple_Rvalue) {
  {
    Tracker::Reset();
    auto t = MakeFromTuple<Tuple<Tracker, Tracker>>(
        Tuple<Tracker, Tracker>(Tracker(1), Tracker(2)));
    CHECK_TRACKER(0, 2, 0, 4, 0, 0, 0);
    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
  }
  {
    Tracker::Reset();
    Tuple<Tracker, Tracker> src(Tracker(1), Tracker(2));
    CHECK_TRACKER(0, 2, 0, 2, 0, 0, 0);
    auto t = MakeFromTuple<Tuple<Tracker, Tracker>>(axio::Move(src));
    CHECK_TRACKER(0, 2, 0, 4, 0, 0, 0);

    CHECK_TUPLE_VALUE_EQ(src, 0, -1);
    CHECK_TUPLE_VALUE_EQ(src, 1, -1);

    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
  }
}

TEST_CASE(Tuple, TupleCat_Basic) {
  {
    auto t1 = MakeTuple(1, 2);
    auto t2 = MakeTuple(3.14, "hello", 1);
    auto t = TupleCat(t1, t2);
    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
    CHECK_TUPLE_VALUE_EQ(t, 2, 3.14);
    CHECK_TUPLE_VALUE_EQ(t, 3, std::string_view("hello"));
    CHECK_TUPLE_VALUE_EQ(t, 4, 1);
  }
  {
    auto t1 = MakeTuple(1, 2);
    auto t2 = MakeTuple(3.14, "hello", 1);
    auto t3 = MakeTuple();

    int x = 10;
    int y = 10;
    auto t4 = MakeTuple(MakeTuple(1, std::ref(y), 3), std::ref(x));
    auto t = TupleCat(t1, t2, t3, t4);
    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
    CHECK_TUPLE_VALUE_EQ(t, 2, 3.14);
    CHECK_TUPLE_VALUE_EQ(t, 3, std::string_view("hello"));
    CHECK_TUPLE_VALUE_EQ(t, 4, 1);

    x = 42;
    y = 20;
    CHECK_EQ(Get<0>(Get<5>(t)), 1);
    CHECK_EQ(Get<1>(Get<5>(t)), 20);
    CHECK_EQ(Get<2>(Get<5>(t)), 3);
    CHECK_TUPLE_VALUE_EQ(t, 6, 42);
  }
}

TEST_CASE(Tuple, TupleCat_Lvalue) {
  Tracker::Reset();

  Tuple<Tracker> t1(Tracker(1));
  Tuple<Tracker> t2(Tracker(2));
  CHECK_TRACKER(0, 2, 0, 2, 0, 0, 0);
  auto t = TupleCat(t1, t2);
  CHECK_TRACKER(0, 2, 2, 2, 0, 0, 0);
  CHECK_TUPLE_VALUE_EQ(t, 0, 1);
  CHECK_TUPLE_VALUE_EQ(t, 1, 2);
}

TEST_CASE(Tuple, TupleCat_Rvalue) {
  {
    Tracker::Reset();
    auto t = TupleCat(Tuple<Tracker>(Tracker(1)), Tuple<Tracker>(Tracker(2)));
    CHECK_TRACKER(0, 2, 0, 4, 0, 0, 0);
    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
  }
  {
    Tracker::Reset();
    Tuple<Tracker, Tracker> t1(Tracker(1), Tracker(2));
    CHECK_TRACKER(0, 2, 0, 2, 0, 0, 0);
    Tuple<Tracker> t2(Tracker(3));
    CHECK_TRACKER(0, 3, 0, 3, 0, 0, 0);
    auto t = TupleCat(axio::Move(t1), axio::Move(t2));
    CHECK_TRACKER(0, 3, 0, 6, 0, 0, 0);

    CHECK_TUPLE_VALUE_EQ(t, 0, 1);
    CHECK_TUPLE_VALUE_EQ(t, 1, 2);
    CHECK_TUPLE_VALUE_EQ(t, 2, 3);

    CHECK_TUPLE_VALUE_EQ(t1, 0, -1);
    CHECK_TUPLE_VALUE_EQ(t1, 1, -1);

    CHECK_TUPLE_VALUE_EQ(t2, 0, -1);
  }
}

TEST_CASE(Tuple, TupleCat_MixedValueCategories) {
  Tracker::Reset();

  Tuple<Tracker> t1(Tracker(1));
  Tuple<Tracker> t2(Tracker(3));
  CHECK_TRACKER(0, 2, 0, 2, 0, 0, 0);
  auto t = TupleCat(t1, Tuple<Tracker>(Tracker(2)), axio::Move(t2));
  CHECK_TRACKER(0, 3, 1, 5, 0, 0, 0);
  CHECK_TUPLE_VALUE_EQ(t, 0, 1);
  CHECK_TUPLE_VALUE_EQ(t, 1, 2);
  CHECK_TUPLE_VALUE_EQ(t, 2, 3);

  CHECK_TUPLE_VALUE_EQ(t1, 0, 1);
  CHECK_TUPLE_VALUE_EQ(t2, 0, -1);
}

TEST_CASE(Tuple, TupleCat_ReferenceDecayCheck) {
  IGNORE_RESULT();
  int x = 10;
  auto t = TupleCat(ForwardAsTuple(x));
  static_assert(axio::IsLvalueReference<decltype(Get<0>(t))>::value, "");
}

TEST_CASE(Tuple, Comparison) {
  {
    Tuple<int, int, std::string> a(1, 2, "hello");
    Tuple<int, int, std::string> b(1, 2, "hello");
    Tuple<int, int, std::string> c(1, 2, "world");

    CHECK_TRUE(a == b);
    CHECK_FALSE(a != b);
    CHECK_TRUE(a != c);
    CHECK_FALSE(a == c);
  }
  {
    Tuple<int, int, std::string> a(1, 2, "a");
    Tuple<int, int, std::string> b(1, 2, "b");
    Tuple<int, int, std::string> c(1, 3, "a");

    CHECK_TRUE(a < b);
    CHECK_FALSE(b < a);
    CHECK_TRUE(a < c);
    CHECK_TRUE(c > a);
  }
  {
    Tuple<int, int, std::string> a(1, 2, "x");
    Tuple<int, int, std::string> b(1, 2, "x");
    Tuple<int, int, std::string> c(2, 0, "a");

    CHECK_TRUE(a <= b);
    CHECK_TRUE(a >= b);
    CHECK_TRUE(a <= c);
    CHECK_FALSE(a >= c);
  }
  {
    Tuple<int, int, std::string> a(1, 1, "a");
    Tuple<int, int, std::string> b(1, 2, "a");

    CHECK_TRUE(a < b);
    CHECK_TRUE(b > a);
    CHECK_FALSE(a > b);
    CHECK_FALSE(b < a);
    CHECK_TRUE((a < b) == !(a >= b));
    CHECK_TRUE((a == b) == !(a < b) && !(b < a));
  }
}