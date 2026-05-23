#include <simpletest/simpletest.hpp>

#include <axio/functional/fixed_function.hpp>

void Nothing() {}

int Add(int x, int y) {
  return x + y;
}

struct Functor {
  int i = 0;

  int operator()(int x) { return i + x; }
  int operator()(int x, int y) { return i + x + y; }
  int operator()(int x, int y, int z) { return i + x + y + z; }
};

TEST_CASE(FixedFunction, FreeFunction) {
  axio::FixedFunction<int(int, int)> f1 = axio::Bind(Add);
  CHECK_EQ(f1(1, 2), 3);
  CHECK_EQ(f1(5, 4), 9);

  axio::FixedFunction<void()> f2 = axio::Bind(Nothing);
  f2();

  axio::FixedFunction<int(int, int)> f3 = axio::Bind<&Add>();
  CHECK_EQ(f3(3, 2), 5);
}

TEST_CASE(FixedFunction, Functor) {
  Functor functor{5};
  axio::FixedFunction<int(int)> f1 = axio::Bind(&functor);
  axio::FixedFunction<int(int, int)> f2 = axio::Bind(&functor);
  axio::FixedFunction<int(int, int, int)> f3 = axio::Bind(&functor);

  CHECK_EQ(f1(1), 6);
  CHECK_EQ(f2(1, 2), 8);
  CHECK_EQ(f3(1, 2, 3), 11);
}

TEST_CASE(FixedFunction, Lambda) {
  axio::FixedFunction<int(int)> f1 =
      axio::Bind([](int x) -> int { return x + 1; });
  CHECK_EQ(f1(1), 2);
  CHECK_EQ(f1(2), 3);

  int x = 42;
  axio::FixedFunction<int(int)> f2 =
      axio::Bind([x](int y) -> int { return x + y + 1; });
  CHECK_EQ(f2(1), 44);
  CHECK_EQ(f2(2), 45);

  int x1 = 1;
  int x2 = 1;
  int x3 = 1;
  int x4 = 1;
  int x5 = 1;
  int x6 = 1;
  int x7 = 1;
  int x8 = 1;
  const auto l = [x1, x2, x3, x4, x5, x6, x7, x8] {
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
  };
  // the storage size is smaller than sizeof(l) -> must not compile
  // axio::FixedFunction<int(), 8> f3 = axio::Bind(l);

  // ok! storage size >= sizoef(l)
  axio::FixedFunction<int(), sizeof(l)> f3 = axio::Bind(l);
  CHECK_EQ(f3(), 8);

  struct Large {
    int value[16];
  } large{};

  axio::FixedFunction<void(int)> f4 =
      axio::Bind([&large](int idx) { large.value[idx] = idx; });
  static_assert(decltype(f4)::kStorageSize >= sizeof(&large), "");

  f4(4);
  f4(2);
  CHECK_EQ(large.value[4], 4);
  CHECK_EQ(large.value[2], 2);
}

#include <axio/string/string.hpp>

TEST_CASE(FixedFunction, MemberFunc) {
  axio::String s = "hello world";
  using SizeType = typename axio::String::SizeType;

  axio::FixedFunction<SizeType()> f1 = axio::Bind<&axio::String::Size>(&s);
  CHECK_EQ(f1(), 11);
  s = "short";
  CHECK_EQ(f1(), 5);

  axio::FixedFunction<SizeType(const axio::String&)> f2 =
      axio::Bind<&axio::String::Size>();
  s = "this is a string";
  CHECK_EQ(f2(s), 16);
}

struct Base {
  virtual int GetInt() const = 0;
  virtual int GetValue() const { return 1; }
  virtual ~Base() = default;
};

struct Derived : public Base {
  int GetInt() const override { return 42; };
  int GetValue() const override { return 2; }
};

TEST_CASE(FixedFunction, VirtualFunc) {
  Derived d{};
  Base& b = d;

  axio::FixedFunction<int()> f1 = axio::Bind<&Base::GetValue>(&b);
  CHECK_EQ(f1(), 2);

  axio::FixedFunction<int()> f2 = axio::Bind<&Base::GetInt>(&b);
  CHECK_EQ(f2(), 42);
}

TEST_CASE(FixedFunction, OperatorBool) {
  axio::FixedFunction<void()> f;
  CHECK_FALSE(static_cast<bool>(f));

  axio::FixedFunction<void()> f1 = axio::Bind([] {});
  CHECK_TRUE(static_cast<bool>(f1));
}

TEST_CASE(FixedFunction, Assign) {
  const auto l1 = [](int a, int b) { return a + b + 1; };
  const auto l2 = [](int a, int b) { return a + b + 2; };

  axio::FixedFunction<int(int, int)> f1;
  axio::FixedFunction<int(int, int)> f2 = axio::Bind(l2);
  axio::FixedFunction<int(int, int)> f3 = axio::Bind(l1);

  f1 = f2;
  CHECK_EQ(f1(1, 1), 4);

  f1 = f3;
  CHECK_EQ(f1(1, 1), 3);
}