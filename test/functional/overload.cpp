#include <simpletest/simpletest.hpp>

#include <axio/functional/overload.hpp>
#include <axio/string/string.hpp>

#include <variant>

using axio::Overload;
using axio::String;

struct Foo {};

struct Multiplier {
  float mult_;
  Multiplier(float m) : mult_{m} {}

  template <typename T>
  T operator()(T v) const noexcept {
    return static_cast<T>(v * mult_);
  }
};

struct Adder {
  template <typename T>
  T operator()(T lhs, T rhs) const noexcept {
    return lhs + rhs;
  }
};

TEST_CASE(Overload, Basic) {
  auto overloaded = Overload{
      [](int) { return "int"; },
      [](double) { return "double"; },
      [](const char*) { return "const char*"; },
      [](auto) { return "unknown"; },
  };

  CHECK_STR_EQ(overloaded(42), "int");
  CHECK_STR_EQ(overloaded(4.2), "double");
  CHECK_STR_EQ(overloaded("420"), "const char*");
  CHECK_STR_EQ(overloaded(Foo{}), "unknown");
}

TEST_CASE(Overload, ArgumentsCount) {
  auto overloaded = Overload{
      [](int, int) { return "int int"; },
      [](int, Foo, int) { return "int Foo int"; },
      []() { return ""; },
      [](int) { return "int"; },
  };

  CHECK_STR_EQ(overloaded(1, 1), "int int");
  CHECK_STR_EQ(overloaded(1, Foo{}, 1), "int Foo int");
  CHECK_STR_EQ(overloaded(), "");
  CHECK_STR_EQ(overloaded(5), "int");
}

TEST_CASE(Overload, CapturingLambda) {
  int value = 10;

  auto overloaded = axio::Overload{
      [value](int x) { return value + x; },
      [](Foo) { return "Foo"; },
  };

  CHECK_EQ(overloaded(5), 15);
  CHECK_STR_EQ(overloaded(Foo{}), "Foo");
}

TEST_CASE(Overload, Constexpr) {
  constexpr auto overloaded = axio::Overload{
      [](int x) { return x + 1; },
      [](double x) { return x * 2; },
  };

  static_assert(overloaded(3) == 4);
  static_assert(overloaded(2.0) == 4.0);
}

TEST_CASE(Overload, DifferentReturnTypes) {
  auto overloaded = axio::Overload{
      [](int) -> int { return 10; },
      [](double) -> String { return "double"; },
  };

  static_assert(std::is_same_v<decltype(overloaded(1)), int>);
  static_assert(std::is_same_v<decltype(overloaded(1.0)), String>);

  CHECK_EQ(overloaded(1), 10);
  CHECK_EQ(overloaded(1.0), "double");
}

TEST_CASE(Overload, Functor) {
  auto overloaded = axio::Overload{
      []() { return 42; },
      Multiplier{5},
      Adder{},
  };

  CHECK_EQ(overloaded(), 42);
  CHECK_EQ(overloaded(4), 20);
  CHECK_EQ(overloaded(5.0), 25.0);

  CHECK_EQ(overloaded(4, 4), 8);
  CHECK_EQ(overloaded(4.0, 4.0), 8.0);
}

TEST_CASE(Overload, VariantVisit) {
  std::variant<String, int32_t, int64_t> v(int32_t{1});
  auto overloaded = axio::Overload{
      [](const String& s) { return s.Size(); },
      [](const auto& s) { return sizeof(s); },
  };

  CHECK_EQ(4, std::visit(overloaded, v));
  v = int64_t{1};
  CHECK_EQ(8, std::visit(overloaded, v));
  v = String("hello");
  CHECK_EQ(5, std::visit(overloaded, v));
}