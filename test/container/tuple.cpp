#include <simpletest/simpletest.hpp>

#include <axio/container/tuple.hpp>
#include <string_view>

using axio::ForwardAsTuple;
using axio::Get;
using axio::MakeTuple;
using axio::Tuple;

struct Empty {
  constexpr Empty() = default;
  constexpr int Foo() const { return 42; }
};

constexpr bool ConstexprConstructorTest() {
  bool ret = true;
  {
    // Tuple<> empty{};
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

using axio::TupleElement;
using axio::TupleSize;
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

  constexpr Foo(int f, int b) : foo(f), bar(b) {}
};

struct Ref {
  int& ref;
  constexpr Ref(int& r) : ref(r) {}
};
constexpr bool ConstexprMakeFromTuple() {
  bool ret = true;
  {
    auto t = axio::MakeTuple(1, 2);
    auto f = axio::MakeFromTuple<Foo>(t);
    ret = ret && f.foo == 1 && f.bar == 2;
  }
  {
    int val = 42;
    auto t = axio::ForwardAsTuple(val);
    auto obj = axio::MakeFromTuple<Ref>(t);
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

  return ret;
}
static_assert(ConstexprTupleComparison(), "");
