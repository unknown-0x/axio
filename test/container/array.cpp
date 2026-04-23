#include <simpletest/simpletest.hpp>

#include <axio/container/array.hpp>

#include <string>

using axio::Array;
using axio::Get;

constexpr bool ConstexprBasicTest() {
  Array<int, 3> arr{1, 2, 3};
  const auto test1 = !arr.IsEmpty();
  const auto test2 = arr.Size() == 3;
  const auto test3 = arr.MaxSize() == 3;
  const auto test4 = arr[0] == 1 && arr[1] == 2 && arr[2] == 3;
  const auto test5 = arr.At(0) == 1 && arr.At(1) == 2 && arr.At(2) == 3;
  const auto test6 = arr.Front() == 1 && arr.Back() == 3;
  const auto test7 = *arr.Data() == 1;
  return test1 && test2 && test3 && test4 && test5 && test6 && test7;
}
static_assert(ConstexprBasicTest(), "");

constexpr bool ConstexprGetTest() {
  Array<int, 3> arr{3, 2, 5};
  return Get<0>(arr) == 3 && Get<1>(arr) == 2 && Get<2>(arr) == 5;
}
static_assert(ConstexprGetTest(), "");

TEST_CASE(Array, BasicUsage) {
  Array<int, 4> arr{1, 2, 3, 4};

  CHECK_FALSE(arr.IsEmpty());
  CHECK_EQ(arr.Size(), 4);
  CHECK_EQ(arr.MaxSize(), 4);
  CHECK_NE(arr.Data(), nullptr);
}

TEST_CASE(Array, ElementAccess) {
  Array<int, 3> arr{1, 2, 3};

  CHECK_EQ(arr[0], 1);
  CHECK_EQ(arr[1], 2);
  CHECK_EQ(arr[2], 3);

  CHECK_EQ(arr.At(0), 1);
  CHECK_EQ(arr.At(1), 2);
  CHECK_EQ(arr.At(2), 3);

  CHECK_EQ(arr.Front(), 1);
  CHECK_EQ(arr.Back(), 3);

  arr[1] = 42;
  CHECK_EQ(arr[1], 42);

  arr.Front() = 4;
  CHECK_EQ(arr[0], 4);
}

TEST_CASE(Array, ConstElementAccess) {
  const Array<int, 3> arr{7, 8, 9};

  CHECK_EQ(arr[0], 7);
  CHECK_EQ(arr.At(1), 8);
  CHECK_EQ(arr.Front(), 7);
  CHECK_EQ(arr.Back(), 9);
}

TEST_CASE(Array, IteratorsForward) {
  {
    Array<int, 5> arr{1, 2, 3, 4, 5};
    int sum = 0;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
      sum += *it;
    }
    CHECK_EQ(sum, 15);
  }
  {
    const Array<int, 5> arr{1, 2, 3, 4, 5};
    int sum = 0;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
      sum += *it;
    }
    CHECK_EQ(sum, 15);
  }
}

TEST_CASE(Array, IteratorsReverse) {
  {
    Array<int, 4> arr{1, 2, 3, 4};

    int expected = 4;
    for (auto it = arr.rbegin(); it != arr.rend(); ++it) {
      CHECK_EQ(*it, expected--);
    }
  }
  {
    const Array<int, 4> arr{1, 2, 3, 4};

    int expected = 4;
    for (auto it = arr.rbegin(); it != arr.rend(); ++it) {
      CHECK_EQ(*it, expected--);
    }
  }
}

TEST_CASE(Array, Comparison) {
  Array<int, 3> a{1, 2, 3};
  Array<int, 3> b{1, 2, 3};
  Array<int, 3> c{1, 2, 4};
  Array<int, 3> d{0, 9, 9};

  CHECK_TRUE(a == b);
  CHECK_FALSE(a != b);

  CHECK_TRUE(a != c);
  CHECK_TRUE(a < c);
  CHECK_TRUE(c > a);

  CHECK_TRUE(d < a);
  CHECK_TRUE(a >= b);
  CHECK_TRUE(a <= b);
}

TEST_CASE(Array, TupleInterface) {
  Array<int, 3> arr{11, 22, 33};

  static_assert(axio::TupleSize<Array<int, 3>>::value == 3, "");
  static_assert(axio::TupleSize<Array<int, 0>>::value == 0, "");
  static_assert(
      axio::IsSame<axio::TupleElement<1, Array<int, 3>>::type, int>::value, "");

  CHECK_EQ(Get<0>(arr), 11);
  CHECK_EQ(Get<1>(arr), 22);
  CHECK_EQ(Get<2>(arr), 33);

  Get<1>(arr) = 99;
  CHECK_EQ(arr[1], 99);
}

TEST_CASE(Array, MoveGet) {
  Array<std::string, 2> arr{"abcd", "hello"};
  std::string value = Get<0>(axio::Move(arr));
  CHECK_EQ(value, "abcd");

  CHECK_EQ(arr[0], "");
  CHECK_EQ(arr[1], "hello");
}

constexpr bool ConstexprBasicTest_ZeroLengthArray() {
  Array<int, 0> arr;
  const auto test1 = arr.IsEmpty();
  const auto test2 = arr.Size() == 0;
  const auto test3 = arr.MaxSize() == 0;
  const auto test4 = arr.Data() == nullptr;
  const auto test5 = arr.begin() == nullptr;
  const auto test6 = arr.end() == nullptr;
  return test1 && test2 && test3 && test4 && test5 && test6;
}
static_assert(ConstexprBasicTest_ZeroLengthArray(), "");

TEST_CASE(ZeroLengthArray, BasicProperties) {
  Array<int, 0> arr;

  CHECK_TRUE(arr.IsEmpty());
  CHECK_EQ(arr.Size(), 0u);
  CHECK_EQ(arr.MaxSize(), 0u);
  CHECK_EQ(arr.Data(), nullptr);
  CHECK_EQ(arr.begin(), nullptr);
  CHECK_EQ(arr.end(), nullptr);
}

TEST_CASE(ZeroLengthArray, Comparison) {
  Array<int, 0> a;
  Array<int, 0> b;

  CHECK_TRUE(a == b);
  CHECK_FALSE(a != b);
  CHECK_FALSE(a < b);
  CHECK_FALSE(a > b);
  CHECK_TRUE(a <= b);
  CHECK_TRUE(a >= b);
}