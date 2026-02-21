#include <simpletest/simpletest.hpp>

#include <axio/base/type_traits.hpp>

#include <tuple>
#include <vector>

TEST_CASE(TypeTraits, TraitType) {
  IGNORE_RESULT();

  struct NormalTrait {
    using type = int;
  };
  static_assert(axio::IsSame<axio::T<NormalTrait>, int>::value, "");

  struct ComplexTrait {
    using type = std::pair<int, float>;
  };
  static_assert(
      axio::IsSame<axio::T<ComplexTrait>, std::pair<int, float>>::value, "");

  struct ConstTypeTrait {
    using type = const int;
  };
  static_assert(axio::IsSame<axio::T<ConstTypeTrait>, const int>::value, "");

  struct ReferenceTypeTrait {
    using type = int&;
  };
  static_assert(axio::IsSame<axio::T<ReferenceTypeTrait>, int&>::value, "");

  struct VolatileTypeTrait {
    using type = volatile int;
  };
  static_assert(axio::IsSame<axio::T<VolatileTypeTrait>, volatile int>::value,
                "");

  static_assert(axio::IsSame<axio::T<axio::RemoveConst<const int>>, int>::value,
                "");
}

enum class Flag { A = 1, B = 2 };

struct EnumValueTrait {
  using type = int;
  static constexpr Flag value = Flag::B;
};

TEST_CASE(TypeTraits, TraitValue) {
  IGNORE_RESULT();

  static_assert(!axio::V<axio::IsClass<int>>, "");
  static_assert(axio::V<axio::IsIntegral<int>>, "");
  static_assert(axio::V<axio::Rank<int>> == 0, "");
  static_assert(axio::V<axio::Rank<int[1][1][1]>> == 3, "");

  static_assert(axio::V<EnumValueTrait> == Flag::B, "");
}

TEST_CASE(TypeTraits, IsBoundedArray) {
  IGNORE_RESULT();
  struct Type {};
  static_assert(!axio::V<axio::IsBoundedArray<Type>>, "");
  static_assert(!axio::V<axio::IsBoundedArray<Type[]>>, "");
  static_assert(axio::V<axio::IsBoundedArray<Type[3]>>, "");

  static_assert(!axio::V<axio::IsBoundedArray<int>>, "");
  static_assert(!axio::V<axio::IsBoundedArray<float>>, "");
  static_assert(!axio::V<axio::IsBoundedArray<int[]>>, "");
  static_assert(axio::V<axio::IsBoundedArray<int[3]>>, "");
  static_assert(axio::V<axio::IsBoundedArray<int[3][4]>>, "");
  static_assert(axio::V<axio::IsBoundedArray<int[3][4][5]>>, "");
  static_assert(axio::V<axio::IsBoundedArray<const int[3]>>, "");
  static_assert(axio::V<axio::IsBoundedArray<volatile int[3]>>, "");
  static_assert(axio::V<axio::IsBoundedArray<const volatile int[3]>>, "");

  static_assert(!axio::V<axio::IsBoundedArray<int*>>, "");
  static_assert(!axio::V<axio::IsBoundedArray<int&>>, "");
  static_assert(!axio::V<axio::IsBoundedArray<int&&>>, "");
}

TEST_CASE(TypeTraits, IsUnboundedArray) {
  IGNORE_RESULT();
  struct Type {};
  static_assert(!axio::V<axio::IsUnboundedArray<Type>>, "");
  static_assert(!axio::V<axio::IsUnboundedArray<Type[3]>>, "");
  static_assert(axio::V<axio::IsUnboundedArray<Type[]>>, "");

  static_assert(!axio::V<axio::IsUnboundedArray<int>>, "");
  static_assert(!axio::V<axio::IsUnboundedArray<float>>, "");
  static_assert(!axio::V<axio::IsUnboundedArray<int[3]>>, "");
  static_assert(!axio::V<axio::IsUnboundedArray<int[3][4]>>, "");
  static_assert(!axio::V<axio::IsUnboundedArray<int[3][4][5]>>, "");
  static_assert(axio::V<axio::IsUnboundedArray<int[]>>, "");
  static_assert(axio::V<axio::IsUnboundedArray<const int[]>>, "");
  static_assert(axio::V<axio::IsUnboundedArray<volatile int[]>>, "");
  static_assert(axio::V<axio::IsUnboundedArray<const volatile int[]>>, "");

  static_assert(!axio::V<axio::IsUnboundedArray<int*>>, "");
  static_assert(!axio::V<axio::IsUnboundedArray<int&>>, "");
  static_assert(!axio::V<axio::IsUnboundedArray<int&&>>, "");
}

template <typename T, typename U>
struct Pair {
  T first;
  U second;
};

template <typename T>
constexpr axio::T<axio::EnableIf<std::is_integral_v<T>, int>> SFINAETest(T) {
  return 1;
}

// Disabled overload (non-integral)
template <typename T>
constexpr axio::T<axio::EnableIf<!std::is_integral_v<T>, int>> SFINAETest(T) {
  return 2;
}

TEST_CASE(TypeTraits, EnableIf) {
  IGNORE_RESULT();
  static_assert(axio::IsSame<axio::T<axio::EnableIf<true, int>>, int>::value,
                "");
  static_assert(
      axio::IsSame<axio::T<axio::EnableIf<true, double>>, double>::value, "");
  static_assert(axio::IsSame<axio::T<axio::EnableIf<true>>, void>::value, "");
  static_assert(axio::IsSame<axio::T<axio::EnableIf<true, Pair<int, float>>>,
                             Pair<int, float>>::value,
                "");
  static_assert(
      axio::IsSame<axio::T<axio::EnableIf<true, const int>>, const int>::value,
      "");
  static_assert(axio::IsSame<axio::T<axio::EnableIf<true, volatile int>>,
                             volatile int>::value,
                "");
  static_assert(axio::IsSame<axio::T<axio::EnableIf<true, int&>>, int&>::value,
                "");
  static_assert(
      axio::IsSame<axio::T<axio::EnableIf<true, int&&>>, int&&>::value, "");
  static_assert(axio::IsSame<axio::T<axio::EnableIf<true, int*>>, int*>::value,
                "");
  static_assert(
      axio::IsSame<axio::T<axio::EnableIf<true, int[5]>>, int[5]>::value, "");
  static_assert(SFINAETest(10) == 1, "");
  static_assert(SFINAETest(3.14) == 2, "");
}

TEST_CASE(TypeTraits, TypeIdentity) {
  IGNORE_RESULT();
  static_assert(axio::IsSame<axio::T<axio::TypeIdentity<int>>, int>::value, "");
  static_assert(
      axio::IsSame<axio::T<axio::TypeIdentity<double>>, double>::value, "");
  static_assert(axio::IsSame<axio::T<axio::TypeIdentity<void>>, void>::value,
                "");
  static_assert(
      axio::IsSame<axio::T<axio::TypeIdentity<const int>>, const int>::value,
      "");
  static_assert(axio::IsSame<axio::T<axio::TypeIdentity<volatile int>>,
                             volatile int>::value,
                "");
  static_assert(axio::IsSame<axio::T<axio::TypeIdentity<const volatile int>>,
                             const volatile int>::value,
                "");

  static_assert(axio::IsSame<axio::T<axio::TypeIdentity<int&>>, int&>::value,
                "");
  static_assert(axio::IsSame<axio::T<axio::TypeIdentity<int&&>>, int&&>::value,
                "");
  static_assert(
      axio::IsSame<axio::T<axio::TypeIdentity<const int&>>, const int&>::value,
      "");

  static_assert(axio::IsSame<axio::T<axio::TypeIdentity<int*>>, int*>::value,
                "");
  static_assert(
      axio::IsSame<axio::T<axio::TypeIdentity<const int*>>, const int*>::value,
      "");
  static_assert(
      axio::IsSame<axio::T<axio::TypeIdentity<int* const>>, int* const>::value,
      "");

  static_assert(
      axio::IsSame<axio::T<axio::TypeIdentity<int[5]>>, int[5]>::value, "");
  static_assert(axio::IsSame<axio::T<axio::TypeIdentity<const int[3]>>,
                             const int[3]>::value,
                "");
  static_assert(axio::IsSame<axio::T<axio::TypeIdentity<int[]>>, int[]>::value,
                "");
}

template <typename... Ts>
struct MyTemplate {};

TEST_CASE(TypeTraits, IsSpecializationOf) {
  IGNORE_RESULT();

  static_assert(axio::IsSpecializationOf<std::vector<int>, std::vector>::value,
                "");
  static_assert(
      axio::IsSpecializationOf<std::vector<double>, std::vector>::value, "");
  static_assert(axio::IsSpecializationOf<std::tuple<int>, std::tuple>::value,
                "");
  static_assert(
      axio::IsSpecializationOf<std::tuple<int, float, char>, std::tuple>::value,
      "");

  static_assert(axio::IsSpecializationOf<MyTemplate<int>, MyTemplate>::value,
                "");
  static_assert(
      axio::IsSpecializationOf<MyTemplate<int, double>, MyTemplate>::value, "");
  static_assert(axio::IsSpecializationOf<MyTemplate<>, MyTemplate>::value, "");

  static_assert(!axio::IsSpecializationOf<int, std::vector>::value, "");
  static_assert(!axio::IsSpecializationOf<double, std::tuple>::value, "");
  static_assert(!axio::IsSpecializationOf<std::vector<int>, std::tuple>::value,
                "");
  static_assert(!axio::IsSpecializationOf<std::tuple<int>, std::vector>::value,
                "");

  static_assert(
      !axio::IsSpecializationOf<const std::vector<int>, std::vector>::value,
      "");
  static_assert(
      !axio::IsSpecializationOf<volatile std::vector<int>, std::vector>::value,
      "");
  static_assert(!axio::IsSpecializationOf<const volatile std::vector<int>,
                                          std::vector>::value,
                "");
}

struct Incomplete;
struct Complete {};
struct LateComplete;
struct LateComplete {};
struct Abstract {
  virtual void Foo() = 0;
};
TEST_CASE(TypeTraits, IsComplete) {
  IGNORE_RESULT();
  static_assert(axio::IsComplete<int>::value, "");
  static_assert(axio::IsComplete<double>::value, "");
  static_assert(axio::IsComplete<char>::value, "");
  static_assert(axio::IsComplete<Complete>::value, "");
  static_assert(axio::IsComplete<Abstract>::value, "");
  static_assert(axio::IsComplete<LateComplete>::value, "");

  static_assert(!axio::IsComplete<Incomplete>::value, "");
  static_assert(axio::IsComplete<Incomplete*>::value, "");
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
  IGNORE_RESULT();
  static_assert(axio::IsDetected<TypeMemberOp, HasType>::value, "");
  static_assert(!axio::IsDetected<TypeMemberOp, NoType>::value, "");

  static_assert(axio::IsDetected<PushBackOp, std::vector<int>>::value, "");
  static_assert(!axio::IsDetected<PushBackOp, int>::value, "");

  static_assert(!axio::IsDetected<ConstPushBackOp, std::vector<int>>::value,
                "");

  static_assert(axio::IsDetected<PlusOp, int, int>::value, "");
  static_assert(axio::IsDetected<PlusOp, double, float>::value, "");
  static_assert(
      !axio::IsDetected<PlusOp, std::vector<int>, std::vector<int>>::value, "");
}

TEST_CASE(TypeTraits, IsEqualityComparable) {
  IGNORE_RESULT();

  struct Foo {
    bool operator==(const Foo&) { return true; }
  };

  static_assert(axio::IsEqualityComparable<int>::value, "");
  static_assert(axio::IsEqualityComparable<Foo>::value, "");
  static_assert(axio::IsEqualityComparable<int, int>::value, "");
  static_assert(axio::IsEqualityComparable<double>::value, "");
  static_assert(axio::IsEqualityComparable<int, double>::value, "");
  static_assert(axio::IsEqualityComparable<char, int>::value, "");

  static_assert(axio::IsEqualityComparable<std::string>::value, "");
  static_assert(axio::IsEqualityComparable<std::string, const char*>::value,
                "");
  static_assert(axio::IsEqualityComparable<const char*, std::string>::value,
                "");
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
  IGNORE_RESULT();
  static_assert(axio::IsLessThanComparable<int>::value, "");
  static_assert(axio::IsLessThanComparable<int, double>::value, "");
  static_assert(axio::IsLessThanComparable<char, int>::value, "");

  static_assert(!axio::IsLessThanComparable<void>::value, "");
  static_assert(!axio::IsLessThanComparable<int, void>::value, "");
  static_assert(!axio::IsLessThanComparable<NoCompare>::value, "");
  static_assert(axio::IsLessThanComparable<FullCompare>::value, "");
}

TEST_CASE(TypeTraits, IsGreaterThanComparable) {
  IGNORE_RESULT();
  static_assert(axio::IsGreaterThanComparable<int>::value, "");
  static_assert(axio::IsGreaterThanComparable<double, int>::value, "");
  static_assert(axio::IsGreaterThanComparable<long, short>::value, "");

  static_assert(!axio::IsGreaterThanComparable<void>::value, "");
  static_assert(!axio::IsGreaterThanComparable<void, int>::value, "");
  static_assert(!axio::IsGreaterThanComparable<NoCompare>::value, "");
  static_assert(axio::IsGreaterThanComparable<FullCompare>::value, "");
}