#include <simpletest/simpletest.hpp>

#include <axio/base/type_traits.hpp>
#include <axio/string/string_utils.hpp>
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

// Skills

using Pointer =
    axio::StrongType<std::unique_ptr<int>, struct PtrTag, axio::PointerLike>;
using Integer = axio::StrongType<int, struct IntegerTag, axio::Arithmetic>;
using Temperature = axio::StrongType<float,
                                     struct TemperatureTag,
                                     axio::Addable,
                                     axio::Subtractable,
                                     axio::ScalarMultiplicable,
                                     axio::ScalarDivisible,
                                     axio::Comparable>;

namespace test {
constexpr Integer a(10), b(2), c(2);
static_assert((a + b) == Integer(12), "");
static_assert((a - b) == Integer(8), "");
static_assert((a * b) == Integer(20), "");
static_assert((a % b) == Integer(0), "");
static_assert((a / b) == Integer(5), "");

static_assert(a != b, "");
static_assert(b == c, "");
static_assert(a > b, "");
static_assert(a >= b, "");
static_assert(b >= c, "");
static_assert(b < a, "");
static_assert(b <= a, "");
static_assert(b <= c, "");

constexpr Integer negative_a(-a);
static_assert(negative_a.Get() == -a.Get(), "");

constexpr int value_a = static_cast<int>(a);
static_assert(value_a == a.Get(), "");

constexpr bool Test() {
  bool ret = true;

  Integer x(10), y(2), z(3);
  z += x;
  ret = ret && (z == Integer(13));
  z -= y;
  ret = ret && (z == Integer(11));

  z *= y;
  ret = ret && (z == Integer(22));

  z /= y;
  ret = ret && (z == Integer(11));

  z %= y;
  ret = ret && (z == Integer(1));

  ret = ret && (++z == Integer(2));
  ret = ret && (z++ == Integer(2));
  ret = ret && (z == Integer(3));
  ret = ret && (--z == Integer(2));
  ret = ret && (z-- == Integer(2));
  ret = ret && (z == Integer(1));

  return ret;
}
static_assert(Test(), "");
}  // namespace test

TEST_CASE(StrongType_Skills, Integer) {
  Integer a(10), b(2);
  CHECK_EQ(a + b, Integer(12));
  CHECK_EQ(a - b, Integer(8));
  CHECK_EQ(a * b, Integer(20));
  CHECK_EQ(a % b, Integer(0));
  CHECK_EQ(a / b, Integer(5));

  Integer c(3);
  c += a;
  CHECK_EQ(c, Integer(13));
  c -= b;
  CHECK_EQ(c, Integer(11));
  c *= b;
  CHECK_EQ(c, Integer(22));
  c /= b;
  CHECK_EQ(c, Integer(11));
  c %= b;
  CHECK_EQ(c, Integer(1));

  CHECK_EQ(++c, Integer(2));
  CHECK_EQ(c++, Integer(2));
  CHECK_EQ(c, Integer(3));
  CHECK_EQ(--c, Integer(2));
  CHECK_EQ(c--, Integer(2));
  CHECK_EQ(c, Integer(1));

  a = Integer(12);
  b = Integer(4);
  c = Integer(4);
  CHECK_TRUE(a != b);
  CHECK_TRUE(b == c);
  CHECK_TRUE(a > b);
  CHECK_TRUE(a >= b);
  CHECK_TRUE(b >= c);
  CHECK_TRUE(b < a);
  CHECK_TRUE(b <= a);
  CHECK_TRUE(b <= c);

  // AxioReprable
  auto str = axio::StringJoinValues(", ", a, b, c);
  CHECK_EQ(str, "12, 4, 4");

  a = -a;
  b = +b;
  CHECK_EQ(a, Integer(-12));
  CHECK_EQ(b, Integer(4));
}

namespace test_bitwise {
constexpr Integer a(0b1100);
constexpr Integer b(0b1010);

static_assert((a & b) == Integer(0b1000), "");
static_assert((a | b) == Integer(0b1110), "");
static_assert((a ^ b) == Integer(0b0110), "");
static_assert(~a == Integer(~0b1100), "");

static_assert((Integer(1) << Integer(3)) == Integer(8), "");
static_assert((Integer(16) >> Integer(2)) == Integer(4), "");
}  // namespace test_bitwise

TEST_CASE(StrongType_Skills, Integer_Bitwise) {
  Integer a(0b1100);
  Integer b(0b1010);

  CHECK_EQ((a & b), Integer(0b1000));
  CHECK_EQ((a | b), Integer(0b1110));
  CHECK_EQ((a ^ b), Integer(0b0110));

  CHECK_EQ(~a, Integer(~0b1100));

  Integer c(a);
  c &= b;
  CHECK_EQ(c, Integer(0b1000));

  c = a;
  c |= b;
  CHECK_EQ(c, Integer(0b1110));

  c = a;
  c ^= b;
  CHECK_EQ(c, Integer(0b0110));

  a = Integer(1);
  CHECK_EQ((a << Integer(3)), Integer(8));
  b = Integer(16);
  CHECK_EQ((b >> Integer(2)), Integer(4));

  a <<= Integer(3);
  b >>= Integer(2);
  CHECK_EQ(a, Integer(8));
  CHECK_EQ(b, Integer(4));
}

namespace test_scalar_muldiv {
constexpr Temperature c(100.0f);
constexpr Temperature f = c * 1.8f + Temperature(32.0f);
static_assert(f == Temperature(212.f), "");

constexpr bool Test() {
  Temperature t(f);
  t /= 2.0f;
  return t.Get() == 106.0f;
}
static_assert(Test(), "");
}  // namespace test_scalar_muldiv

TEST_CASE(StrongType_Skills, Scalar_MulDiv) {
  Temperature c(100.0f);
  Temperature f = c * 1.8f + Temperature(32.0f);
  CHECK_NEAR(f.Get(), 212.f, 0.00001f);

  f /= 2.0f;
  CHECK_NEAR(f.Get(), 106.f, 0.00001f);
}

TEST_CASE(StrongType_Skills, Dereferenceable) {
  using StrongInt = StrongType<int, struct StrongIntTag, axio::Dereferenceable>;

  StrongInt si{1};
  const StrongInt csi{1};

  int& value = *si;
  // int& value = *csi; // must not compile
  const int& cvalue = *csi;

  CHECK_EQ(value, 1);
  CHECK_EQ(cvalue, 1);

  value = 10;
  CHECK_EQ(si.Get(), 10);

  static constexpr StrongInt constexpr_si(10);
  static_assert(*constexpr_si == 10, "");
}

TEST_CASE(StrongType_Skills, PointerLike) {
  {
    Pointer p(std::make_unique<int>(42));
    CHECK_EQ(*p, 42)
    CHECK_TRUE(bool(p));  // ImplicitBool

    Pointer moved(axio::Move(p));
    CHECK_EQ(*moved, 42);
    CHECK_TRUE(bool(moved));
    CHECK_FALSE(bool(p));
  }
  {
    using StringPtr = axio::StrongType<std::unique_ptr<std::string>,
                                       struct StringPtrTag, axio::PointerLike>;
    StringPtr ptr(std::make_unique<std::string>("hello"));
    CHECK_EQ(ptr->size(), 5u);
    CHECK_EQ(ptr->at(0), 'h');

    (*ptr).append(" world");
    CHECK_EQ(*ptr, "hello world");
  }
  {
    Pointer valid(std::make_unique<int>(1));
    Pointer empty(nullptr);

    CHECK_FALSE(!valid);
    CHECK_TRUE(!empty);
  }
  {
    using FooPtr = StrongType<Foo*, struct FooPtrTag, axio::PointerLike>;
    using CFooPtr =
        StrongType<const Foo*, struct CFooPtrTag, axio::PointerLike>;

    Foo f(100);
    const Foo cf(200);

    FooPtr fp(&f);
    // FooPtr fp(&cf); // must not compile
    CFooPtr cfp(&cf);

    CHECK_EQ(fp->value, 100);
    CHECK_EQ(cfp->value, 200);

    fp->value = 50;
    CHECK_EQ(fp->value, 50);

    CHECK_EQ((*cfp).value, 200);
    // cfp->value = 100; // must not compile

    Foo& fref = *fp;
    fref.value = 123;
    CHECK_EQ(fp->value, 123);

    static constexpr Foo ce_f(100);
    static constexpr CFooPtr ce_cfp(&ce_f);
    static_assert(ce_cfp->value == 100, "");
  }
}

enum class FilePermission : std::uint8_t {
  None = 0,
  Read = 1 << 0,
  Write = 1 << 1,
  Execute = 1 << 2
};

AXIO_DEFINE_ENUM_BITWISE(FilePermission);

template <typename Output>
void AxioRepr(Output& output, FilePermission fp) {
  const char* result = "<unknown>";
  switch (fp) {
    case FilePermission::None:
      result = "FP::None";
      break;
    case FilePermission::Read:
      result = "FP::Read";
      break;
    case FilePermission::Write:
      result = "FP::Write";
      break;
    case FilePermission::Execute:
      result = "FP::Execute";
      break;
  }
  output.Append(result);
}

using Permissions =
    StrongType<FilePermission, struct FilePermissionsTag, axio::Flag>;

namespace test_flag {
constexpr bool BitwiseOr() {
  Permissions read(FilePermission::Read);
  Permissions write(FilePermission::Write);
  return (read | write).Get() == (FilePermission::Read | FilePermission::Write);
}

constexpr bool BitwiseAnd() {
  Permissions value(FilePermission::Read | FilePermission::Write);
  Permissions read(FilePermission::Read);
  return (value & read).Get() == (FilePermission::Read);
}

constexpr bool BitwiseXor() {
  Permissions value(FilePermission::Read | FilePermission::Write);
  Permissions write(FilePermission::Write);
  return (value ^ write).Get() == (FilePermission::Read);
}

static_assert(BitwiseOr(), "");
static_assert(BitwiseAnd(), "");
static_assert(BitwiseXor(), "");
static_assert((~Permissions(FilePermission::Read)).Get() ==
                  ~FilePermission::Read,
              "");
}  // namespace test_flag

TEST_CASE(StrongType_FlagTest, BitwiseOr) {
  Permissions read(FilePermission::Read);
  Permissions write(FilePermission::Write);

  auto result = read | write;
  CHECK_EQ(result.Get(), (FilePermission::Read | FilePermission::Write));
}

TEST_CASE(StrongType_FlagTest, BitwiseAnd) {
  Permissions value(FilePermission::Read | FilePermission::Write);
  Permissions read(FilePermission::Read);

  auto result = value & read;
  CHECK_EQ(result.Get(), FilePermission::Read);
}

TEST_CASE(StrongType_FlagTest, BitwiseXor) {
  Permissions value(FilePermission::Read | FilePermission::Write);
  Permissions write(FilePermission::Write);

  auto result = value ^ write;
  CHECK_EQ(result.Get(), FilePermission::Read);
}

TEST_CASE(StrongType_FlagTest, BitwiseNot) {
  Permissions read(FilePermission::Read);
  auto result = ~read;
  CHECK_EQ(result.Get(), ~FilePermission::Read);
}

TEST_CASE(StrongType_FlagTest, Comparable) {
  Permissions a(FilePermission::Read);
  Permissions b(FilePermission::Read);
  Permissions c(FilePermission::Write);

  CHECK_EQ(a, b);
  CHECK_NE(a, c);
}

TEST_CASE(StrongType_FlagTest, Repr) {
  Permissions a(FilePermission::Read);
  Permissions b(FilePermission::Write);
  Permissions c(FilePermission::Execute);
  auto str = axio::StringJoinValues(" | ", a, b, c);
  CHECK_EQ(str, "FP::Read | FP::Write | FP::Execute");
}