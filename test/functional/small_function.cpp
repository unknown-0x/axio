#include <simpletest/simpletest.hpp>

#include <axio/functional/small_function.hpp>
#include <axio/string/string.hpp>

void Nothing() {}

int Add(int x, int y) {
  return x + y;
}

int& Modify(int& x, int value) {
  x = value;
  return x;
}

struct Functor {
  int i = 0;

  int operator()(int x) const { return i + x; }
  int operator()(int x, int y) const { return i + x + y; }
  int operator()(int x, int y, int z) const { return i + x + y + z; }
};

struct LargeFunctor {
  int arr[10];

  LargeFunctor(int value) {
    for (int& x : arr) {
      x = value;
    }
  }

  int operator()(int extra) const {
    for (int x : arr) {
      extra += x;
    }
    return extra;
  }
};

struct Base {
  virtual int GetInt() const = 0;
  virtual int GetValue() const { return 1; }
  virtual ~Base() = default;
};

struct Derived : public Base {
  int GetInt() const override { return 42; };
  int GetValue() const override { return 2; }
};

TEST_CASE(SmallFunction, DefaultAndNullConstructor) {
  axio::SmallFunction<void()> f1;
  CHECK_FALSE(static_cast<bool>(f1));

  axio::SmallFunction<void()> f2 = nullptr;
  CHECK_FALSE(static_cast<bool>(f2));
}

TEST_CASE(SmallFunction, FreeFunction) {
  axio::SmallFunction<void()> f1 = Nothing;
  f1();

  axio::SmallFunction<int(int, int)> f2 = Add;
  CHECK_EQ(f2(1, 2), 3);
  CHECK_EQ(f2(4, 4), 8);

  int x = 10;
  axio::SmallFunction<int&(int&, int)> f3 = Modify;
  int& ref_x = f3(x, 42);
  CHECK_EQ(&x, &ref_x);
  CHECK_EQ(x, 42);
}

TEST_CASE(SmallFunction, Functor) {
  axio::SmallFunction<int(int)> f1 = Functor{1};
  axio::SmallFunction<int(int, int)> f2 = Functor{2};
  axio::SmallFunction<int(int, int, int)> f3 = Functor{3};

  CHECK_EQ(f1(1), 2);
  CHECK_EQ(f2(1, 2), 5);
  CHECK_EQ(f3(1, 2, 3), 9);

  axio::SmallFunction<int(int)> f4 = LargeFunctor(1);
  CHECK_EQ(f4(5), 15);
}

TEST_CASE(SmallFunction, Lambda) {
  axio::SmallFunction<int(int, int)> multiplier = [](int x, int y) {
    return x * y;
  };
  CHECK_EQ(multiplier(2, 3), 6);
  CHECK_EQ(multiplier(4, 2), 8);

  int x = 10;
  axio::SmallFunction<int(int)> adder = [x](int y) { return x + y; };
  CHECK_EQ(adder(5), 15);

  axio::SmallFunction<int(int)> adder_ref = [&x](int y) { return x + y; };
  CHECK_EQ(adder_ref(5), 15);
  x = 15;
  CHECK_EQ(adder_ref(5), 20);

  LargeFunctor large{2};
  axio::SmallFunction<int(int, int)> f = [large](int a, int b) {
    return a + b + large(2);
  };

  CHECK_EQ(f(1, 2), 25);
  CHECK_EQ(f(1, 5), 28);
}

TEST_CASE(SmallFunction, MemberFunc) {
  using SizeType = typename axio::String::SizeType;

  axio::SmallFunction<SizeType(const axio::String&)> size = &axio::String::Size;

  axio::String s = "hello world";
  CHECK_EQ(size(s), 11);
  s = "short";
  CHECK_EQ(size(s), 5);
  s = "this is a string";
  CHECK_EQ(size(s), 16);

  const axio::String cs = "foo bar baz";
  CHECK_EQ(size(cs), 11);
}

TEST_CASE(SmallFunction, VirtualFunc) {
  Derived d{};
  Base& b = d;

  axio::SmallFunction<int(const Base&)> f1 = &Base::GetValue;
  CHECK_EQ(f1(b), 2);

  axio::SmallFunction<int(const Base&)> f2 = &Base::GetInt;
  CHECK_EQ(f2(b), 42);
}

TEST_CASE(SmallFunction, CopyAssign) {
  axio::SmallFunction<int(int, int)> f1 = Add;
  axio::SmallFunction<int(int, int)> f2;
  f2 = f1;
  CHECK_EQ(f2(1, 1), 2);
  CHECK_EQ(f1(1, 1), 2);

  int value = 42;
  axio::SmallFunction<int(int, int)> f3 = [value](int x, int y) {
    return value + x + y;
  };
  axio::SmallFunction<int(int, int)> f4 = Add;
  f4 = f3;
  CHECK_EQ(f4(3, 3), 48);
  CHECK_EQ(f3(3, 3), 48);
}

TEST_CASE(SmallFunction, MoveAssign) {
  using SizeType = typename axio::String::SizeType;

  axio::SmallFunction<int(int, int)> f1 = Add;
  axio::SmallFunction<int(int, int)> f2 = [](int x, int y) { return x + y; };
  axio::SmallFunction<int(int)> f3 = LargeFunctor(2);
  axio::SmallFunction<SizeType(const axio::String&)> f4 = &axio::String::Size;

  axio::SmallFunction<int(int, int)> moved_f1 = [](int x, int y) {
    return x + y;
  };
  axio::SmallFunction<int(int, int)> moved_f2 = Add;
  axio::SmallFunction<int(int)> moved_f3 = LargeFunctor(1);
  axio::SmallFunction<SizeType(const axio::String&)> moved_f4;

  moved_f1 = axio::Move(f1);
  moved_f2 = axio::Move(f2);
  moved_f3 = axio::Move(f3);
  moved_f4 = axio::Move(f4);

  CHECK_FALSE(bool(f1));
  CHECK_FALSE(bool(f2));
  CHECK_FALSE(bool(f3));
  CHECK_FALSE(bool(f4));

  axio::String s = "hello world";

  CHECK_EQ(moved_f1(1, 1), 2);
  CHECK_EQ(moved_f2(1, 1), 2);
  CHECK_EQ(moved_f3(1), 21);
  CHECK_EQ(moved_f4(s), 11);
}

int Add2(int x, int y) {
  return x + y + x;
}

TEST_CASE(SmallFunction, AssignCallable) {
  axio::SmallFunction<int(int, int)> adder = Add;
  adder = Add2;
  CHECK_EQ(adder(1, 1), 3);

  LargeFunctor large{1};
  adder = [large](int x, int y) { return large(2) + x + y; };
  CHECK_EQ(adder(1, 2), 15);

  axio::SmallFunction<int(int)> f = LargeFunctor{2};
  CHECK_EQ(f(1), 21);
  f = LargeFunctor{3};
  CHECK_EQ(f(1), 31);
}

TEST_CASE(SmallFunction, AssignNullptr) {
  axio::SmallFunction<int()> f = []() { return 42; };
  f = nullptr;
  CHECK_FALSE(bool(f));
}

TEST_CASE(SmallFunction, Stack) {
  int x1, x2, x3, x4;
  const auto l = [&x1, &x2, &x3, &x4](int value) {
    x1 = value;
    x2 = value;
    x3 = value;
    x4 = value;
    return value;
  };
  static constexpr auto kMaxAlign =
      AXIO_MAX(alignof(LargeFunctor), alignof(decltype(l)));

  axio::SmallFunction<int(int), sizeof(LargeFunctor), kMaxAlign> f =
      LargeFunctor{1};
  CHECK_EQ(f(1), 11);

  f = l;

  CHECK_EQ(f(1), 1);
  CHECK_EQ(x1, 1);
  CHECK_EQ(x2, 1);
  CHECK_EQ(x3, 1);
  CHECK_EQ(x4, 1);
}

TEST_CASE(SmallFunction, CompareToNullptr) {
  axio::SmallFunction<void()> f1;
  axio::SmallFunction<void()> f2 = nullptr;
  axio::SmallFunction<void()> f3 = [] {};

  CHECK_EQ(f1, nullptr);
  CHECK_EQ(f2, nullptr);
  CHECK_EQ(nullptr, f1);
  CHECK_EQ(nullptr, f2);

  CHECK_NE(f3, nullptr);
  CHECK_NE(nullptr, f3);
}

int G(int, int) {
  return 1;
}
int F(int, int) {
  return 2;
}

TEST_CASE(SmallFunction, Target) {
  axio::SmallFunction<int(int, int)> f1 = std::plus<int>();
  axio::SmallFunction<int(int, int)> f2 = std::minus<int>();
  axio::SmallFunction<int(int, int)> f3 = G;
  axio::SmallFunction<int(int, int)> f4 = F;

  CHECK_TRUE(f1.Target<std::plus<int>>());
  CHECK_TRUE(f2.Target<std::minus<int>>());

  using Fx = int (*)(int, int);

  int (*const* ptr3)(int, int) = f3.Target<Fx>();
  int (*const* ptr4)(int, int) = f4.Target<Fx>();
  CHECK_EQ(*ptr3, G);
  CHECK_EQ(*ptr4, F);
}