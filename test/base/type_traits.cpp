#include <simpletest/simpletest.hpp>

#include <axio/base/type_traits.hpp>

#include <tuple>
#include <vector>

enum class Flag { A = 1, B = 2 };

struct EnumValueTrait {
  using type = int;
  static constexpr Flag value = Flag::B;
};

TEST_CASE(TypeTraits, IsBoundedArray) {
  struct Type {};
  static_assert(!axio::IsBoundedArray_V<Type>, "");
  static_assert(!axio::IsBoundedArray_V<Type[]>, "");
  static_assert(axio::IsBoundedArray_V<Type[3]>, "");

  static_assert(!axio::IsBoundedArray_V<int>, "");
  static_assert(!axio::IsBoundedArray_V<float>, "");
  static_assert(!axio::IsBoundedArray_V<int[]>, "");
  static_assert(axio::IsBoundedArray_V<int[3]>, "");
  static_assert(axio::IsBoundedArray_V<int[3][4]>, "");
  static_assert(axio::IsBoundedArray_V<int[3][4][5]>, "");
  static_assert(axio::IsBoundedArray_V<const int[3]>, "");
  static_assert(axio::IsBoundedArray_V<volatile int[3]>, "");
  static_assert(axio::IsBoundedArray_V<const volatile int[3]>, "");

  static_assert(!axio::IsBoundedArray_V<int*>, "");
  static_assert(!axio::IsBoundedArray_V<int&>, "");
  static_assert(!axio::IsBoundedArray_V<int&&>, "");
}

TEST_CASE(TypeTraits, IsUnboundedArray) {
  struct Type {};
  static_assert(!axio::IsUnboundedArray_V<Type>, "");
  static_assert(!axio::IsUnboundedArray_V<Type[3]>, "");
  static_assert(axio::IsUnboundedArray_V<Type[]>, "");

  static_assert(!axio::IsUnboundedArray_V<int>, "");
  static_assert(!axio::IsUnboundedArray_V<float>, "");
  static_assert(!axio::IsUnboundedArray_V<int[3]>, "");
  static_assert(!axio::IsUnboundedArray_V<int[3][4]>, "");
  static_assert(!axio::IsUnboundedArray_V<int[3][4][5]>, "");
  static_assert(axio::IsUnboundedArray_V<int[]>, "");
  static_assert(axio::IsUnboundedArray_V<const int[]>, "");
  static_assert(axio::IsUnboundedArray_V<volatile int[]>, "");
  static_assert(axio::IsUnboundedArray_V<const volatile int[]>, "");

  static_assert(!axio::IsUnboundedArray_V<int*>, "");
  static_assert(!axio::IsUnboundedArray_V<int&>, "");
  static_assert(!axio::IsUnboundedArray_V<int&&>, "");
}

template <typename T, typename U>
struct Pair {
  T first;
  U second;
};

template <typename T>
constexpr axio::EnableIf_T<std::is_integral_v<T>, int> SFINAETest(T) {
  return 1;
}

// Disabled overload (non-integral)
template <typename T>
constexpr axio::EnableIf_T<!std::is_integral_v<T>, int> SFINAETest(T) {
  return 2;
}

TEST_CASE(TypeTraits, EnableIf) {
  static_assert(axio::IsSame_V<axio::EnableIf_T<true, int>, int>, "");
  static_assert(axio::IsSame_V<axio::EnableIf_T<true, double>, double>, "");
  static_assert(axio::IsSame_V<axio::EnableIf_T<true>, void>, "");
  static_assert(axio::IsSame_V<axio::EnableIf_T<true, Pair<int, float>>,
                               Pair<int, float>>,
                "");
  static_assert(axio::IsSame_V<axio::EnableIf_T<true, const int>, const int>,
                "");
  static_assert(
      axio::IsSame_V<axio::EnableIf_T<true, volatile int>, volatile int>, "");
  static_assert(axio::IsSame_V<axio::EnableIf_T<true, int&>, int&>, "");
  static_assert(axio::IsSame_V<axio::EnableIf_T<true, int&&>, int&&>, "");
  static_assert(axio::IsSame_V<axio::EnableIf_T<true, int*>, int*>, "");
  static_assert(axio::IsSame_V<axio::EnableIf_T<true, int[5]>, int[5]>, "");
  static_assert(SFINAETest(10) == 1, "");
  static_assert(SFINAETest(3.14) == 2, "");
}

TEST_CASE(TypeTraits, TypeIdentity) {
  static_assert(axio::IsSame_V<axio::TypeIdentity_T<int>, int>, "");
  static_assert(axio::IsSame_V<axio::TypeIdentity_T<double>, double>, "");
  static_assert(axio::IsSame_V<axio::TypeIdentity_T<void>, void>, "");
  static_assert(axio::IsSame_V<axio::TypeIdentity_T<const int>, const int>, "");
  static_assert(
      axio::IsSame_V<axio::TypeIdentity_T<volatile int>, volatile int>, "");
  static_assert(axio::IsSame_V<axio::TypeIdentity_T<const volatile int>,
                               const volatile int>,
                "");

  static_assert(axio::IsSame_V<axio::TypeIdentity_T<int&>, int&>, "");
  static_assert(axio::IsSame_V<axio::TypeIdentity_T<int&&>, int&&>, "");
  static_assert(axio::IsSame_V<axio::TypeIdentity_T<const int&>, const int&>,
                "");

  static_assert(axio::IsSame_V<axio::TypeIdentity_T<int*>, int*>, "");
  static_assert(axio::IsSame_V<axio::TypeIdentity_T<const int*>, const int*>,
                "");
  static_assert(axio::IsSame_V<axio::TypeIdentity_T<int* const>, int* const>,
                "");

  static_assert(axio::IsSame_V<axio::TypeIdentity_T<int[5]>, int[5]>, "");
  static_assert(
      axio::IsSame_V<axio::TypeIdentity_T<const int[3]>, const int[3]>, "");
  static_assert(axio::IsSame_V<axio::TypeIdentity_T<int[]>, int[]>, "");
}

template <typename... Ts>
struct MyTemplate {};

TEST_CASE(TypeTraits, IsSpecializationOf) {
  static_assert(axio::IsSpecializationOf_V<std::vector<int>, std::vector>, "");
  static_assert(axio::IsSpecializationOf_V<std::vector<double>, std::vector>,
                "");
  static_assert(axio::IsSpecializationOf_V<std::tuple<int>, std::tuple>, "");
  static_assert(
      axio::IsSpecializationOf_V<std::tuple<int, float, char>, std::tuple>, "");

  static_assert(axio::IsSpecializationOf_V<MyTemplate<int>, MyTemplate>, "");
  static_assert(axio::IsSpecializationOf_V<MyTemplate<int, double>, MyTemplate>,
                "");
  static_assert(axio::IsSpecializationOf_V<MyTemplate<>, MyTemplate>, "");

  static_assert(!axio::IsSpecializationOf_V<int, std::vector>, "");
  static_assert(!axio::IsSpecializationOf_V<double, std::tuple>, "");
  static_assert(!axio::IsSpecializationOf_V<std::vector<int>, std::tuple>, "");
  static_assert(!axio::IsSpecializationOf_V<std::tuple<int>, std::vector>, "");

  static_assert(
      !axio::IsSpecializationOf_V<const std::vector<int>, std::vector>, "");
  static_assert(
      !axio::IsSpecializationOf_V<volatile std::vector<int>, std::vector>, "");
  static_assert(
      !axio::IsSpecializationOf_V<const volatile std::vector<int>, std::vector>,
      "");
}

struct Incomplete;
struct Complete {};
struct LateComplete;
struct LateComplete {};
struct Abstract {
  virtual ~Abstract() = default;
  virtual void Foo() = 0;
};
TEST_CASE(TypeTraits, IsComplete) {
  static_assert(axio::IsComplete_V<int>, "");
  static_assert(axio::IsComplete_V<double>, "");
  static_assert(axio::IsComplete_V<char>, "");
  static_assert(axio::IsComplete_V<Complete>, "");
  static_assert(axio::IsComplete_V<Abstract>, "");
  static_assert(axio::IsComplete_V<LateComplete>, "");

  static_assert(!axio::IsComplete_V<Incomplete>, "");
  static_assert(axio::IsComplete_V<Incomplete*>, "");
}

struct HasType {
  using type = int;
};
struct NoType {};

template <typename T>
using TypeMemberOp = typename T::type;
template <typename T>
using PushBackOp = decltype(std::declval<T&>().push_back(0));
template <typename T>
using ConstPushBackOp = decltype(std::declval<const T&>().push_back(0));
template <typename T, typename U>
using PlusOp = decltype(std::declval<T>() + std::declval<U>());

TEST_CASE(TypeTraits, IsDetected) {
  static_assert(axio::IsDetected_V<TypeMemberOp, HasType>, "");
  static_assert(!axio::IsDetected_V<TypeMemberOp, NoType>, "");

  static_assert(axio::IsDetected_V<PushBackOp, std::vector<int>>, "");
  static_assert(!axio::IsDetected_V<PushBackOp, int>, "");

  static_assert(!axio::IsDetected_V<ConstPushBackOp, std::vector<int>>, "");

  static_assert(axio::IsDetected_V<PlusOp, int, int>, "");
  static_assert(axio::IsDetected_V<PlusOp, double, float>, "");
  static_assert(!axio::IsDetected_V<PlusOp, std::vector<int>, std::vector<int>>,
                "");
}

TEST_CASE(TypeTraits, IsEqualityComparable) {
  struct Foo {
    bool operator==(const Foo&) const { return true; }
  };

  static_assert(axio::IsEqualityComparable_V<int>, "");
  static_assert(axio::IsEqualityComparable_V<Foo>, "");
  static_assert(axio::IsEqualityComparable_V<int, int>, "");
  static_assert(axio::IsEqualityComparable_V<double>, "");
  static_assert(axio::IsEqualityComparable_V<int, double>, "");
  static_assert(axio::IsEqualityComparable_V<char, int>, "");

  static_assert(axio::IsEqualityComparable_V<std::string>, "");
  static_assert(axio::IsEqualityComparable_V<std::string, const char*>, "");
  static_assert(axio::IsEqualityComparable_V<const char*, std::string>, "");
}

struct NoCompare {};
struct FullCompare {
  int v;
};

inline bool operator<(const FullCompare& a, const FullCompare& b) {
  return a.v < b.v;
}

inline bool operator>(const FullCompare& a, const FullCompare& b) {
  return a.v > b.v;
}

TEST_CASE(TypeTraits, IsLessThanComparable) {
  static_assert(axio::IsLessThanComparable_V<int>, "");
  static_assert(axio::IsLessThanComparable_V<int, double>, "");
  static_assert(axio::IsLessThanComparable_V<char, int>, "");

  static_assert(!axio::IsLessThanComparable_V<void>, "");
  static_assert(!axio::IsLessThanComparable_V<int, void>, "");
  static_assert(!axio::IsLessThanComparable_V<NoCompare>, "");
  static_assert(axio::IsLessThanComparable_V<FullCompare>, "");
}

TEST_CASE(TypeTraits, IsGreaterThanComparable) {
  static_assert(axio::IsGreaterThanComparable_V<int>, "");
  static_assert(axio::IsGreaterThanComparable_V<double, int>, "");
  static_assert(axio::IsGreaterThanComparable_V<long, short>, "");

  static_assert(!axio::IsGreaterThanComparable_V<void>, "");
  static_assert(!axio::IsGreaterThanComparable_V<void, int>, "");
  static_assert(!axio::IsGreaterThanComparable_V<NoCompare>, "");
  static_assert(axio::IsGreaterThanComparable_V<FullCompare>, "");
}

TEST_CASE(TypeTraits, ShouldUseEBO) {
  struct Empty {};
  struct FinalEmpty final {};
  struct NonEmpty {
    int x;
  };

  static_assert(axio::ShouldUseEBO_V<Empty>, "");

  static_assert(!axio::ShouldUseEBO_V<FinalEmpty>, "");
  static_assert(!axio::ShouldUseEBO_V<NonEmpty>, "");
  static_assert(!axio::ShouldUseEBO_V<int>, "");
}