#include <simpletest/simpletest.hpp>

#include <axio/base/type_traits.hpp>
#include <axio/utility/strong_type.hpp>

#include <memory>

using ::axio::StrongType;

struct Foo {
  int value = 0;

  constexpr Foo() = default;

  constexpr Foo(int v) : value(v) {}
  constexpr Foo(const Foo& other) : value(other.value) {}
  constexpr Foo(Foo&& other) noexcept : value(other.value) { other.value = 0; }

  constexpr Foo& operator=(const Foo& other) {
    value = other.value;
    return *this;
  }
  constexpr Foo& operator=(Foo&& other) noexcept {
    value = other.value;
    other.value = 0;
    return *this;
  }
};

using FooType = StrongType<Foo, struct FooTag>;
using TagType = StrongType<int, struct Tag>;
using Meter = StrongType<int, struct MeterTag>;

static_assert(axio::IsDefaultConstructible_V<TagType>);
static_assert(axio::IsCopyConstructible_V<TagType>);
static_assert(axio::IsMoveConstructible_V<TagType>);
static_assert(axio::IsCopyAssignable_V<TagType>);
static_assert(axio::IsMoveAssignable_V<TagType>);

static_assert(!axio::IsConvertible_V<int, TagType>);
static_assert(!axio::IsConvertible_V<TagType, int>);

static_assert(axio::IsConstructible_V<TagType, int>);

static_assert(!axio::IsSame_V<Meter, TagType>);
static_assert(!axio::IsConstructible_V<Meter, TagType>);
static_assert(!axio::IsAssignable_V<Meter&, TagType>);

static_assert(axio::IsSame_V<Meter::ValueType, int>);
static_assert(axio::IsSame_V<FooType::ValueType, Foo>);

constexpr bool Constructor_Constexpr() {
  bool ret = true;

  TagType t1{};
  ret = ret && (t1.Get() == 0);

  TagType t2{42};
  TagType t3{t2};
  ret = ret && (t2.Get() == 42);
  ret = ret && (t3.Get() == 42);

  int value = static_cast<int>(t2);
  ret = ret && (value == 42);

  FooType a{Foo{3}};
  FooType b{axio::Move(a)};

  ret = ret && (b.Get().value == 3);
  ret = ret && (a.Get().value == 0);

  return ret;
}
static_assert(Constructor_Constexpr(), "");

constexpr bool Constexpr_Assign() {
  TagType t1{42};
  TagType t2{10};
  t1 = t2;

  bool ret = true;
  ret = ret && (t1.Get() == t2.Get());

  FooType a{Foo{3}};
  FooType b{Foo{5}};

  b = axio::Move(a);

  ret = ret && (b.Get().value == 3);
  ret = ret && (a.Get().value == 0);

  return ret;
}
static_assert(Constexpr_Assign(), "");

TEST_CASE(StrongType, Constructors) {
  TagType t1;
  CHECK_EQ(t1.Get(), 0);

  int value = 100;
  TagType t2(value);
  CHECK_EQ(t2.Get(), 100);

  TagType t3{420};
  CHECK_EQ(t3.Get(), 420);

  FooType f1(Foo{10});
  FooType f2(f1);
  CHECK_EQ(f2.Get().value, 10);

  FooType f3(Foo{10});
  FooType f4(axio::Move(f3));
  CHECK_EQ(f3.Get().value, 0);
  CHECK_EQ(f4.Get().value, 10);
}

TEST_CASE(StrongType, Assign) {
  FooType f1(Foo{2});
  FooType f2(Foo{4});
  f2 = f1;
  CHECK_EQ(f2.Get().value, 2);

  FooType f3(Foo{200});
  FooType f4(Foo{100});
  f3 = axio::Move(f4);
  CHECK_EQ(f3.Get().value, 100);
}

TEST_CASE(StrongType, ExplicitTypeCasting) {
  TagType t(250);
  int value = static_cast<int>(t);
  CHECK_EQ(value, 250);

  // int implicit_value = t; // must not compile
}

TEST_CASE(StrongType, Get) {
  TagType tag{100};
  const TagType ctag{100};

  tag.Get() += 100;
  CHECK_EQ(tag.Get(), 200);

  // ctag.Get() = 200; // must not compile
  CHECK_EQ(ctag.Get(), 100);
}

TEST_CASE(StrongType, MoveOnlyType) {
  using MoveOnly = StrongType<std::unique_ptr<int>, struct UniqueTag>;

  auto p = std::make_unique<int>(99);
  // MoveOnly mo(p); // must not compile
  MoveOnly mo(axio::Move(p));
  CHECK_EQ(*mo.Get(), 99);

  // MoveOnly copy_mo(mo); // must not compile
  MoveOnly moved_mo(axio::Move(mo));
  CHECK_EQ(*moved_mo.Get(), 99);
  CHECK_EQ(mo.Get(), nullptr);
}