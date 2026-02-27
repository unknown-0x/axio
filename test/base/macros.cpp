#include <simpletest/simpletest.hpp>

#include <axio/base/macros.hpp>

TEST_CASE(Macros, ArraySize) {
  IGNORE_RESULT();

  {
    int arr[5];
    static_assert(AXIO_ARRAY_SIZE(arr) == 5, "Array size should be 5");
  }

  {
    char arr[1];
    static_assert(AXIO_ARRAY_SIZE(arr) == 1, "Array size should be 1");
  }

  {
    const int arr[7] = {};
    static_assert(AXIO_ARRAY_SIZE(arr) == 7, "Const array size should be 7");
  }

  {
    int arr2d[3][4];
    static_assert(AXIO_ARRAY_SIZE(arr2d) == 3, "Outer dimension should be 3");
  }

  {
    constexpr int arr[6] = {};
    static_assert(AXIO_ARRAY_SIZE(arr) == 6,
                  "AXIO_ARRAY_SIZE must be constexpr usable");
  }
}

#define TEST_VALUE 123
#define TEST_TOKEN abc

TEST_CASE(Macros, Stringify) {
  IGNORE_RESULT();

  static_assert(AXIO_STRINGIFY(abc)[0] == 'a', "Should stringify tokens");
  static_assert(AXIO_STRINGIFY(abc)[1] == 'b', "Should stringify tokens");
  static_assert(AXIO_STRINGIFY(abc)[2] == 'c', "Should stringify tokens");

  static_assert(AXIO_STRINGIFY(123)[0] == '1', "Should stringify numbers");
  static_assert(AXIO_STRINGIFY(123)[1] == '2', "Should stringify numbers");
  static_assert(AXIO_STRINGIFY(123)[2] == '3', "Should stringify numbers");

  static_assert(AXIO_STRINGIFY(TEST_VALUE)[0] == '1',
                "Should expand macro then stringify");
  static_assert(AXIO_STRINGIFY(TEST_VALUE)[1] == '2',
                "Should expand macro then stringify");
  static_assert(AXIO_STRINGIFY(TEST_VALUE)[2] == '3',
                "Should expand macro then stringify");
  static_assert(AXIO_STRINGIFY(TEST_VALUE)[3] == '\0',
                "Must be null-terminated");

  static_assert(AXIO_STRINGIFY(TEST_TOKEN)[0] == 'a',
                "Should expand macro token");
  static_assert(AXIO_STRINGIFY(TEST_TOKEN)[1] == 'b',
                "Should expand macro token");
  static_assert(AXIO_STRINGIFY(TEST_TOKEN)[2] == 'c',
                "Should expand macro token");
}

#define AXIO_TEST_A Foo
#define AXIO_TEST_B Bar
#define AXIO_TEST_NUM 7

TEST_CASE(Macros, Concat) {
  IGNORE_RESULT();

  struct FooBar {};
  static_assert(std::is_same_v<AXIO_CONCAT(Foo, Bar), FooBar>,
                "AXIO_CONCAT should concatenate tokens into a type");
  static_assert(std::is_same_v<AXIO_CONCAT(AXIO_TEST_A, AXIO_TEST_B), FooBar>,
                "AXIO_CONCAT should concatenate tokens into a type");

  struct HelloWorld {};
#define HELLO Hello
#define WORLD World
  static_assert(std::is_same_v<AXIO_CONCAT(HELLO, WORLD), HelloWorld>,
                "AXIO_CONCAT should expand macros before concatenation");

  constexpr int foobar = 42;
  static_assert(AXIO_CONCAT(foo, bar) == 42,
                "AXIO_CONCAT should form valid identifiers");
}

TEST_CASE(Macros, Bit) {
  IGNORE_RESULT();

  static_assert(AXIO_BIT(0) == 1ULL, "Bit 0 should be 1");
  static_assert(AXIO_BIT(1) == 2ULL, "Bit 1 should be 2");
  static_assert(AXIO_BIT(3) == 8ULL, "Bit 3 should be 8");
  static_assert(AXIO_BIT(7) == 128ULL, "Bit 7 should be 128");

  static_assert(std::is_same_v<decltype(AXIO_BIT(5)), unsigned long long>,
                "AXIO_BIT should produce ULL-based expression");

  constexpr std::uint64_t mask = AXIO_BIT(10);
  static_assert(mask == 1024ULL, "AXIO_BIT must be constexpr usable");
}

TEST_CASE(Macros, MinMax) {
  IGNORE_RESULT();

  static_assert(AXIO_MIN(1, 2) == 1, "");
  static_assert(AXIO_MAX(1, 2) == 2, "");
}

constexpr int Foo(int a) {
  AXIO_ASSERT(a != 0);
  return a;
}

static_assert(Foo(5) == 5, "Should be evaluated");
// static_assert(Foo(0) == 0, "Should not be evaluated");