#include <simpletest/simpletest.hpp>

#include <axio/container/vector.hpp>
#include <axio/string/string.hpp>
#include <axio/utility/result.hpp>

#include <string>
#include <utility>
#include <variant>
#include <vector>

#if defined(AXIO_COMPILER_MSVC)
#define PRAGMA_PUSH() __pragma(warning(push))
#define PRAGMA_POP() __pragma(warning(pop))
#define DISABLE_SELF_ASSIGN() __pragma(warning(disable : 4522))
#define DISABLE_SELF_MOVE() __pragma(warning(disable : 26800))
#elif defined(AXIO_COMPILER_CLANG)
#define PRAGMA_PUSH() _Pragma("clang diagnostic push")
#define PRAGMA_POP() _Pragma("clang diagnostic pop")
#define DISABLE_SELF_ASSIGN() \
  _Pragma("clang diagnostic ignored \"-Wself-assign-overloaded\"")
#define DISABLE_SELF_MOVE() _Pragma("clang diagnostic ignored \"-Wself-move\"")
#elif defined(AXIO_COMPILER_GCC)
#define PRAGMA_PUSH() _Pragma("GCC diagnostic push")
#define PRAGMA_POP() _Pragma("GCC diagnostic pop")
#define DISABLE_SELF_ASSIGN() \
  _Pragma("GCC diagnostic ignored \"-Wself-assign-overloaded\"")
#define DISABLE_SELF_MOVE() _Pragma("GCC diagnostic ignored \"-Wself-move\"")
#else
#define PRAGMA_PUSH()
#define PRAGMA_POP()
#define DISABLE_SELF_ASSIGN()
#define DISABLE_SELF_MOVE()
#endif

using ::axio::Error;
using ::axio::Ok;
using ::axio::Result;
using ::axio::String;

struct Trivial {
  int x{};
  constexpr Trivial() = default;
  constexpr explicit Trivial(int v) noexcept : x(v) {}
  constexpr bool operator==(Trivial o) const noexcept { return x == o.x; }
};
static_assert(axio::IsTriviallyDestructible_V<Result<Trivial, Trivial>>, "");
static_assert(Result<Trivial, int>(std::in_place, 7).GetValue().x == 7,
              "in_place");
static_assert(Result<Trivial, int>(Ok(42))
                      .Then([](Trivial t) -> Result<Trivial, int> {
                        return Ok(t.x + 10);
                      })
                      .GetValue()
                      .x == 52,
              "in_place");

struct TrivialAssignOnly {
  int x{};
  TrivialAssignOnly() = default;
  TrivialAssignOnly(const TrivialAssignOnly& o) noexcept : x(o.x) {}
  TrivialAssignOnly& operator=(const TrivialAssignOnly&) = default;
  ~TrivialAssignOnly() = default;
};
static_assert(!axio::IsTriviallyCopyConstructible_V<TrivialAssignOnly>, "");
static_assert(axio::IsTriviallyCopyAssignable_V<TrivialAssignOnly>, "");
static_assert(axio::IsTriviallyCopyAssignable_V<
                  Result<TrivialAssignOnly, TrivialAssignOnly>>,
              "");

struct Point {
  int x, y;
};

namespace constexpr_tests {
constexpr Result<int, int> CTOk() {
  return Ok(42);
}
static_assert(CTOk().HasValue(), "");
static_assert(CTOk().GetValue() == 42, "");

constexpr Result<int, int> CTErr() {
  return Error(-1);
}
static_assert(!CTErr().HasValue(), "");
static_assert(CTErr().GetError() == -1, "");

static_assert(bool(CTOk()), "");
static_assert(!bool(CTErr()), "");

static_assert(CTOk().ValueOr(0) == 42, "");
static_assert(CTOk().ErrorOr(-9) == -9, "");
static_assert(CTErr().ValueOr(99) == 99, "");
static_assert(CTErr().ErrorOr(0) == -1, "");

static_assert(*CTOk() == 42, "");

static_assert(!(CTOk() == CTErr()), "");
static_assert((CTOk() == Ok<int>(42)), "");
static_assert((CTErr() == Error<int>(-1)), "");
static_assert((CTOk() == CTOk()), "");
static_assert((CTErr() == CTErr()), "");

static_assert(CTOk().Map([](int v) { return v * 2; }).GetValue() == 84, "");
static_assert(!CTErr().Map([](int v) { return v * 2; }).HasValue(), "");
static_assert(CTErr().MapError([](int e) { return e - 1; }).GetError() == -2,
              "");

static_assert(CTOk().Then([](int v) -> Result<int, int> {
                      return Ok<int>(v + 10);
                    }).GetValue() == 52,
              "");
static_assert(!CTErr()
                   .Then([](int v) -> Result<int, int> {
                     return Ok<int>(v + 10);
                   })
                   .HasValue(),
              "");

static_assert(CTOk().OrElse([](int e) -> Result<int, int> {
                      return Error<int>(e - 1);
                    }).GetValue() == 42,
              "");
static_assert(CTErr().OrElse([](int e) -> Result<int, int> {
                       return Error<int>(e - 1);
                     })
                      .GetError() == -2,
              "");

static_assert(CTOk().Map([](int v) { return v + 8; })
                      .Then([](int v) -> Result<int, int> {
                        return Ok<int>(v * 2);
                      })
                      .Map([](int v) { return v - 1; })
                      .GetValue() == 99,
              "");

constexpr auto ct_point = Result<Point, int>(Ok(Point{1, 2}));
static_assert(ct_point->x == 1, "");
static_assert(ct_point->y == 2, "");

constexpr Result<int, int> ok_lv = Ok(1);
constexpr Result<int, int> err_lv = Error(1);

static_assert(noexcept(*ok_lv), "");
static_assert(noexcept(ok_lv.HasValue()), "");
static_assert(noexcept(ok_lv.GetValue()), "");
static_assert(noexcept(ok_lv.ValueOr(0)), "");
static_assert(noexcept(err_lv.GetError()), "");
}  // namespace constexpr_tests

namespace {
struct Tracker {
  static int ctor, copy_ctor, move_ctor, copy_assign, move_assign, dtor;
  static void Reset() {
    ctor = copy_ctor = move_ctor = copy_assign = move_assign = dtor = 0;
  }

  int id;
  explicit Tracker(int i = 0) : id(i) { ++ctor; }
  Tracker(const Tracker& o) : id(o.id) { ++copy_ctor; }
  Tracker(Tracker&& o) noexcept : id(o.id) {
    o.id = -1;
    ++move_ctor;
  }
  Tracker& operator=(const Tracker& o) {
    id = o.id;
    ++copy_assign;
    return *this;
  }
  Tracker& operator=(Tracker&& o) noexcept {
    id = o.id;
    o.id = -1;
    ++move_assign;
    return *this;
  }
  ~Tracker() { ++dtor; }
  bool operator==(const Tracker& o) const { return id == o.id; }
};
int Tracker::ctor = 0, Tracker::copy_ctor = 0, Tracker::move_ctor = 0;
int Tracker::copy_assign = 0, Tracker::move_assign = 0, Tracker::dtor = 0;

struct ThrowOnCopy {
  int v;
  explicit ThrowOnCopy(int x = 0) : v(x) {}
  ThrowOnCopy(const ThrowOnCopy&) { throw std::runtime_error("copy!"); }
  ThrowOnCopy(ThrowOnCopy&&) noexcept = default;
  ThrowOnCopy& operator=(const ThrowOnCopy&) {
    throw std::runtime_error("copy!");
  }
  ThrowOnCopy& operator=(ThrowOnCopy&&) noexcept = default;
  bool operator==(const ThrowOnCopy& o) const { return v == o.v; }
};

struct NTD {
  int* counter;
  int v;
  NTD(int* c, int x) : counter(c), v(x) {}
  NTD(const NTD& o) : counter(o.counter), v(o.v) {}
  NTD(NTD&& o) noexcept : counter(o.counter), v(o.v) { o.counter = nullptr; }
  NTD& operator=(const NTD& o) {
    counter = o.counter;
    v = o.v;
    return *this;
  }
  NTD& operator=(NTD&& o) noexcept {
    counter = o.counter;
    v = o.v;
    o.counter = nullptr;
    return *this;
  }
  ~NTD() {
    if (counter) {
      ++(*counter);
    }
  }
  bool operator==(const NTD& o) const { return v == o.v; }
};

using Res = Result<int, String>;
using ResT = Result<Tracker, String>;
using ResN = Result<NTD, NTD>;

}  // namespace

TEST_CASE(ResultConstructor, FromOkRvalue) {
  Res r(Ok<int>(42));
  CHECK_TRUE(r.HasValue());
  CHECK_EQ(r.GetValue(), 42);
}

TEST_CASE(ResultConstructor, FromOkLvalue) {
  Ok<int> ok(42);
  Res r(ok);
  CHECK_TRUE(r.HasValue());
  CHECK_EQ(r.GetValue(), 42);
}

TEST_CASE(ResultConstructor, FromErrorRvalue) {
  Res r(Error<String>("oops"));
  CHECK_FALSE(r.HasValue());
  CHECK_EQ(r.GetError(), "oops");
}

TEST_CASE(ResultConstructor, FromErrorLvalue) {
  Error<String> e("oops");
  Res r(e);
  CHECK_FALSE(r.HasValue());
  CHECK_EQ(r.GetError(), "oops");
}

TEST_CASE(ResultConstructor, InPlaceSingleArg) {
  Res r(std::in_place, 7);
  CHECK_TRUE(r.HasValue());
  CHECK_EQ(r.GetValue(), 7);
}

TEST_CASE(ResultConstructor, InPlaceInitializerList) {
  Result<axio::Vector<int>, int> r(std::in_place, {1, 2, 3});
  CHECK_TRUE(r.HasValue());
  CHECK_EQ(r.GetValue().Size(), 3u);
}

TEST_CASE(ResultConstructor, OkDeductionGuide) {
  auto ok = Ok(99);
  static_assert(axio::IsSame_V<decltype(ok), Ok<int>>);
  Res r(axio::Move(ok));
  CHECK_EQ(r.GetValue(), 99);
}

TEST_CASE(ResultConstructor, ErrorDeductionGuide) {
  auto err = Error(String("e"));
  static_assert(std::is_same_v<decltype(err), Error<String>>);
  Res r(axio::Move(err));
  CHECK_EQ(r.GetError(), "e");
}

TEST_CASE(ResultConstructor, CopyConstructOk) {
  Res a(Ok<int>(1));
  Res b(a);
  CHECK_TRUE(b.HasValue());
  CHECK_EQ(b.GetValue(), 1);
  CHECK_TRUE(a.HasValue());
}

TEST_CASE(ResultConstructor, CopyConstructError) {
  Res a(Error<String>("x"));
  Res b(a);
  CHECK_FALSE(b.HasValue());
  CHECK_EQ(b.GetError(), "x");
}

TEST_CASE(ResultConstructor, MoveConstructOk) {
  Tracker::Reset();
  ResT a(Ok<Tracker>(Tracker(5)));
  ResT b(axio::Move(a));
  CHECK_TRUE(b.HasValue());
  CHECK_EQ(b.GetValue().id, 5);
  CHECK_GT(Tracker::move_ctor, 0);
}

TEST_CASE(ResultConstructor, MoveConstructError) {
  Res a(Error<String>("hello"));
  Res b(axio::Move(a));
  CHECK_FALSE(b.HasValue());
  CHECK_EQ(b.GetError(), "hello");
}

TEST_CASE(ResultAssign, CopyAssignOkToOk) {
  Res a(Ok<int>(1)), b(Ok<int>(2));
  b = a;
  CHECK_EQ(b.GetValue(), 1);
}

TEST_CASE(ResultAssign, CopyAssignErrorToError) {
  Res a(Error<String>("a")), b(Error<String>("b"));
  b = a;
  CHECK_EQ(b.GetError(), "a");
}

TEST_CASE(ResultAssign, CopyAssignOkToError) {
  Res ok(Ok<int>(5)), err(Error<String>("e"));
  err = ok;
  CHECK_TRUE(err.HasValue());
  CHECK_EQ(err.GetValue(), 5);
}

TEST_CASE(ResultAssign, CopyAssignErrorToOk) {
  Res ok(Ok<int>(5)), err(Error<String>("e"));
  ok = err;
  CHECK_FALSE(ok.HasValue());
  CHECK_EQ(ok.GetError(), "e");
}

TEST_CASE(ResultAssign, MoveAssignOkToOk) {
  Tracker::Reset();
  ResT a(Ok<Tracker>(Tracker(3))), b(Ok<Tracker>(Tracker(4)));
  Tracker::Reset();
  b = axio::Move(a);
  CHECK_EQ(b.GetValue().id, 3);
  CHECK_GT(Tracker::move_assign, 0);
}

TEST_CASE(ResultAssign, MoveAssignErrorToOk) {
  ResT a(Error<String>("err")), b(Ok<Tracker>(Tracker(1)));
  b = axio::Move(a);
  CHECK_FALSE(b.HasValue());
  CHECK_EQ(b.GetError(), "err");
}

TEST_CASE(ResultAssign, MoveAssignOkToError) {
  ResT a(Ok<Tracker>(Tracker(7))), b(Error<String>("e"));
  b = axio::Move(a);
  CHECK_TRUE(b.HasValue());
  CHECK_EQ(b.GetValue().id, 7);
}

TEST_CASE(ResultAssign, AssignOkWrapper) {
  Res r(Error<String>("e"));
  r = Ok<int>(42);
  CHECK_TRUE(r.HasValue());
  CHECK_EQ(r.GetValue(), 42);
}

TEST_CASE(ResultAssign, AssignErrorWrapper) {
  Res r(Ok<int>(1));
  r = Error<String>("new");
  CHECK_FALSE(r.HasValue());
  CHECK_EQ(r.GetError(), "new");
}

TEST_CASE(ResultAssign, AssignOkWrapperAlreadyOk) {
  Res r(Ok<int>(1));
  r = Ok<int>(99);
  CHECK_EQ(r.GetValue(), 99);
}

TEST_CASE(ResultAssign, AssignErrorWrapperAlreadyError) {
  Res r(Error<String>("old"));
  r = Error<String>("new");
  CHECK_EQ(r.GetError(), "new");
}

TEST_CASE(ResultAssign, SelfCopyAssign) {
  Res r(Ok<int>(7));

  PRAGMA_PUSH();
  DISABLE_SELF_ASSIGN();
  r = r;
  PRAGMA_POP();

  CHECK_EQ(r.GetValue(), 7);
}

TEST_CASE(ResultAssign, SelfMoveAssign) {
  Res r(Ok<int>(7));

  PRAGMA_PUSH();
  DISABLE_SELF_MOVE();
  r = axio::Move(r);
  PRAGMA_POP();

  CHECK_EQ(r.GetValue(), 7);
}

TEST_CASE(ResultAccessors, GetValueLvalue) {
  Res r(Ok<int>(10));
  int& v = r.GetValue();
  v = 20;
  CHECK_EQ(r.GetValue(), 20);
}

TEST_CASE(ResultAccessors, GetValueConstLvalue) {
  const Res r(Ok<int>(10));
  const int& v = r.GetValue();
  CHECK_EQ(v, 10);
}

TEST_CASE(ResultAccessors, GetValueRvalue) {
  Tracker::Reset();
  ResT r(Ok<Tracker>(Tracker(5)));
  Tracker t = axio::Move(r).GetValue();
  CHECK_EQ(t.id, 5);
  CHECK_GT(Tracker::move_ctor, 0);
}

TEST_CASE(ResultAccessors, GetValueConstRvalue) {
  const ResT r(Ok<Tracker>(Tracker(5)));
  Tracker t = axio::Move(r).GetValue();
  CHECK_EQ(t.id, 5);
  CHECK_GT(Tracker::copy_ctor, 0);
}

TEST_CASE(ResultAccessors, GetErrorLvalue) {
  Res r(Error<String>("e"));
  String& e = r.GetError();
  e = "modified";
  CHECK_EQ(r.GetError(), "modified");
}

TEST_CASE(ResultAccessors, GetErrorConstLvalue) {
  const Res r(Error<String>("e"));
  const String& e = r.GetError();
  CHECK_EQ(e, "e");
}

TEST_CASE(ResultAccessors, GetErrorRvalue) {
  Res r(Error<String>("e"));
  String e = axio::Move(r).GetError();
  CHECK_EQ(e, "e");
}

TEST_CASE(ResultAccessors, GetErrorConstRvalue) {
  const Res r(Error<String>("e"));
  String e = axio::Move(r).GetError();
  CHECK_EQ(e, "e");
}

TEST_CASE(ResultAccessors, Dereference) {
  Res r(Ok<int>(3));
  CHECK_EQ(*r, 3);
  *r = 9;
  CHECK_EQ(*r, 9);
}

TEST_CASE(ResultAccessors, DereferenceConst) {
  const Res r(Ok<int>(3));
  CHECK_EQ(*r, 3);
}

TEST_CASE(ResultAccessors, DereferenceRvalue) {
  Tracker::Reset();
  ResT r(Ok<Tracker>(Tracker(5)));
  Tracker t = *axio::Move(r);
  CHECK_EQ(t.id, 5);
}

TEST_CASE(ResultAccessors, Arrow) {
  Result<String, int> r(Ok<String>("hello"));
  r->Push('!');
  CHECK_EQ(r->Size(), 6u);
  CHECK_EQ(*r, "hello!");
}

TEST_CASE(ResultAccessors, ArrowConst) {
  const Result<String, int> r(Ok<String>("hello"));
  CHECK_EQ(r->Size(), 5u);
}

TEST_CASE(ResultAccessors, BoolConversion) {
  Res ok(Ok<int>(1));
  Res err(Error<String>("e"));
  CHECK_TRUE(static_cast<bool>(ok));
  CHECK_FALSE(static_cast<bool>(err));
}

TEST_CASE(ResultValueOrErrorOr, ValueOrOnOk) {
  Res r(Ok<int>(5));
  CHECK_EQ(r.ValueOr(0), 5);
}

TEST_CASE(ResultValueOrErrorOr, ValueOrOnError) {
  Res r(Error<String>("e"));
  CHECK_EQ(r.ValueOr(-1), -1);
}

TEST_CASE(ResultValueOrErrorOr, ValueOrRvalueOnOk) {
  CHECK_EQ(Res(Ok<int>(5)).ValueOr(0), 5);
}

TEST_CASE(ResultValueOrErrorOr, ValueOrRvalueOnError) {
  CHECK_EQ(Res(Error<String>("e")).ValueOr(-1), -1);
}

TEST_CASE(ResultValueOrErrorOr, ErrorOrOnError) {
  Res r(Error<String>("err"));
  CHECK_EQ(r.ErrorOr("fb"), "err");
}

TEST_CASE(ResultValueOrErrorOr, ErrorOrOnOk) {
  Res r(Ok<int>(1));
  CHECK_EQ(r.ErrorOr("fb"), "fb");
}

TEST_CASE(ResultValueOrErrorOr, ErrorOrRvalueOnError) {
  CHECK_EQ(Res(Error<String>("err")).ErrorOr("fb"), "err");
}

TEST_CASE(ResultValueOrErrorOr, ErrorOrRvalueOnOk) {
  CHECK_EQ(Res(Ok<int>(1)).ErrorOr("fb"), "fb");
}

TEST_CASE(ResultThen, LvalueOnOk) {
  Res r(Ok<int>(3));
  auto r2 = r.Then([](int& v) -> Res { return Ok<int>(v * 2); });
  CHECK_TRUE(r2.HasValue());
  CHECK_EQ(r2.GetValue(), 6);
}

TEST_CASE(ResultThen, LvalueOnError) {
  Res r(Error<String>("e"));
  auto r2 = r.Then([](int&) -> Res { return Ok<int>(0); });
  CHECK_FALSE(r2.HasValue());
  CHECK_EQ(r2.GetError(), "e");
}

TEST_CASE(ResultThen, ConstLvalueOnOk) {
  const Res r(Ok<int>(4));
  auto r2 = r.Then([](const int& v) -> Res { return Ok<int>(v + 1); });
  CHECK_EQ(r2.GetValue(), 5);
}

TEST_CASE(ResultThen, ConstLvalueOnError) {
  const Res r(Error<String>("e"));
  auto r2 = r.Then([](const int&) -> Res { return Ok<int>(0); });
  CHECK_FALSE(r2.HasValue());
}

TEST_CASE(ResultThen, RvalueOnOk) {
  auto r2 = Res(Ok<int>(3)).Then([](int&& v) -> Res { return Ok<int>(v * 3); });
  CHECK_EQ(r2.GetValue(), 9);
}

TEST_CASE(ResultThen, RvalueOnError) {
  auto r2 =
      Res(Error<String>("err")).Then([](int&&) -> Res { return Ok<int>(0); });
  CHECK_FALSE(r2.HasValue());
  CHECK_EQ(r2.GetError(), "err");
}

TEST_CASE(ResultThen, ConstRvalueOnOk) {
  const Res r(Ok<int>(2));
  auto r2 =
      axio::Move(r).Then([](const int&& v) -> Res { return Ok<int>(v * 4); });
  CHECK_EQ(r2.GetValue(), 8);
}

TEST_CASE(ResultThen, ConstRvalueOnError) {
  const Res r(Error<String>("err"));
  auto r2 = axio::Move(r).Then([](const int&&) -> Res { return Ok<int>(0); });
  CHECK_FALSE(r2.HasValue());
}

TEST_CASE(ResultThen, ChainMultiple) {
  auto result = Res(Ok<int>(1))
                    .Then([](int v) -> Res { return Ok<int>(v + 1); })
                    .Then([](int v) -> Res { return Ok<int>(v * 10); });
  CHECK_EQ(result.GetValue(), 20);
}

TEST_CASE(ResultThen, ChainShortCircuitsOnError) {
  bool called = false;
  auto result = Res(Error<String>("fail")).Then([&](int) -> Res {
    called = true;
    return Ok<int>(0);
  });
  CHECK_FALSE(called);
  CHECK_EQ(result.GetError(), "fail");
}

TEST_CASE(ResultOrElse, LvalueOnError) {
  Res r(Error<String>("e"));
  auto r2 = r.OrElse([](String& e) -> Res { return Ok<int>(int(e.Size())); });
  CHECK_TRUE(r2.HasValue());
  CHECK_EQ(r2.GetValue(), 1);
}

TEST_CASE(ResultOrElse, LvalueOnOk) {
  Res r(Ok<int>(5));
  auto r2 = r.OrElse([](String&) -> Res { return Ok<int>(0); });
  CHECK_EQ(r2.GetValue(), 5);
}

TEST_CASE(ResultOrElse, ConstLvalueOnError) {
  const Res r(Error<String>("ee"));
  auto r2 =
      r.OrElse([](const String& e) -> Res { return Ok<int>(int(e.Size())); });
  CHECK_EQ(r2.GetValue(), 2);
}

TEST_CASE(ResultOrElse, ConstLvalueOnOk) {
  const Res r(Ok<int>(5));
  auto r2 = r.OrElse([](const String&) -> Res { return Ok<int>(0); });
  CHECK_EQ(r2.GetValue(), 5);
}

TEST_CASE(ResultOrElse, RvalueOnError) {
  auto r2 = Res(Error<String>("eee")).OrElse([](String&& e) -> Res {
    return Ok<int>(int(e.Size()));
  });
  CHECK_EQ(r2.GetValue(), 3);
}

TEST_CASE(ResultOrElse, RvalueOnOk) {
  auto r2 = Res(Ok<int>(7)).OrElse([](String&&) -> Res { return Ok<int>(0); });
  CHECK_EQ(r2.GetValue(), 7);
}

TEST_CASE(ResultOrElse, ConstRvalueOnError) {
  const Res r(Error<String>("e"));
  auto r2 =
      axio::Move(r).OrElse([](const String&&) -> Res { return Ok<int>(-1); });
  CHECK_EQ(r2.GetValue(), -1);
}

TEST_CASE(ResultOrElse, ConstRvalueOnOk) {
  const Res r(Ok<int>(3));
  auto r2 =
      axio::Move(r).OrElse([](const String&&) -> Res { return Ok<int>(0); });
  CHECK_EQ(r2.GetValue(), 3);
}

TEST_CASE(ResultMap, LvalueOnOk) {
  Res r(Ok<int>(3));
  auto r2 = r.Map([](int& v) { return std::to_string(v); });
  CHECK_EQ(r2.GetValue(), "3");
}

TEST_CASE(ResultMap, LvalueOnError) {
  Res r(Error<String>("e"));
  auto r2 = r.Map([](int& v) { return std::to_string(v); });
  CHECK_FALSE(r2.HasValue());
  CHECK_EQ(r2.GetError(), "e");
}

TEST_CASE(ResultMap, ConstLvalueOnOk) {
  const Res r(Ok<int>(4));
  auto r2 = r.Map([](const int& v) { return v * 2.0; });
  CHECK_NEAR(r2.GetValue(), 8.0, 0.00001);
}

TEST_CASE(ResultMap, ConstLvalueOnError) {
  const Res r(Error<String>("e"));
  auto r2 = r.Map([](const int&) { return 0.0; });
  CHECK_FALSE(r2.HasValue());
}

TEST_CASE(ResultMap, RvalueOnOk) {
  Tracker::Reset();
  ResT r(Ok<Tracker>(Tracker(5)));
  auto r2 = axio::Move(r).Map([](Tracker&& t) { return t.id; });
  CHECK_EQ(r2.GetValue(), 5);
}

TEST_CASE(ResultMap, RvalueOnError) {
  auto r2 = Res(Error<String>("e")).Map([](int&&) { return 0; });
  CHECK_FALSE(r2.HasValue());
}

TEST_CASE(ResultMap, ConstRvalueOnOk) {
  const Res r(Ok<int>(9));
  auto r2 = axio::Move(r).Map([](const int&& v) { return v + 1; });
  CHECK_EQ(r2.GetValue(), 10);
}

TEST_CASE(ResultMap, ConstRvalueOnError) {
  const Res r(Error<String>("e"));
  auto r2 = axio::Move(r).Map([](const int&&) { return 0; });
  CHECK_FALSE(r2.HasValue());
}

TEST_CASE(ResultMap, TypeTransformation) {
  Result<int, std::string> r(Ok<int>(42));
  Result<std::string, std::string> r2 =
      r.Map([](int& v) { return std::to_string(v); });
  CHECK_EQ(r2.GetValue(), "42");
}

TEST_CASE(ResultMapError, LvalueOnError) {
  Res r(Error<String>("err"));
  auto r2 = r.MapError([](String& e) { return int(e.Size()); });
  CHECK_EQ(r2.GetError(), 3);
}

TEST_CASE(ResultMapError, LvalueOnOk) {
  Res r(Ok<int>(1));
  auto r2 = r.MapError([](String&) { return 0; });
  CHECK_TRUE(r2.HasValue());
  CHECK_EQ(r2.GetValue(), 1);
}

TEST_CASE(ResultMapError, ConstLvalueOnError) {
  const Res r(Error<String>("err"));
  auto r2 = r.MapError([](const String& e) { return int(e.Size()); });
  CHECK_EQ(r2.GetError(), 3);
}

TEST_CASE(ResultMapError, ConstLvalueOnOk) {
  const Res r(Ok<int>(2));
  auto r2 = r.MapError([](const String&) { return 0; });
  CHECK_TRUE(r2.HasValue());
}

TEST_CASE(ResultMapError, RvalueOnError) {
  auto r2 = Res(Error<String>("err")).MapError([](String&& e) {
    return int(e.Size());
  });
  CHECK_EQ(r2.GetError(), 3);
}

TEST_CASE(ResultMapError, RvalueOnOk) {
  auto r2 = Res(Ok<int>(5)).MapError([](String&&) { return 0; });
  CHECK_EQ(r2.GetValue(), 5);
}

TEST_CASE(ResultMapError, ConstRvalueOnError) {
  const Res r(Error<String>("err"));
  auto r2 = axio::Move(r).MapError([](const String&&) { return -1; });
  CHECK_EQ(r2.GetError(), -1);
}

TEST_CASE(ResultMapError, ConstRvalueOnOk) {
  const Res r(Ok<int>(3));
  auto r2 = axio::Move(r).MapError([](const String&&) { return -1; });
  CHECK_EQ(r2.GetValue(), 3);
}

TEST_CASE(ResultEquality, TwoOkEqual) {
  CHECK_EQ(Res(Ok(1)), Res(Ok(1)));
}

TEST_CASE(ResultEquality, TwoOkNotEqual) {
  CHECK_NE(Res(Ok(1)), Res(Ok(2)));
}

TEST_CASE(ResultEquality, TwoErrorEqual) {
  CHECK_EQ(Res(Error<String>("e")), Res(Error<String>("e")));
}

TEST_CASE(ResultEquality, TwoErrorNotEqual) {
  CHECK_NE(Res(Error<String>("a")), Res(Error<String>("b")));
}

TEST_CASE(ResultEquality, OkVsError) {
  CHECK_NE(Res(Ok<int>(1)), Res(Error<String>("e")));
}

TEST_CASE(ResultEquality, ResultVsOkWrapper) {
  Res r(Ok(5));
  CHECK_EQ(r, Ok(5));
  CHECK_NE(r, Ok(6));
  CHECK_EQ(Ok(5), r);
  CHECK_NE(Ok(6), r);
}

TEST_CASE(ResultEquality, ResultVsErrorWrapper) {
  Res r(Error<String>("e"));
  CHECK_EQ(r, Error<String>("e"));
  CHECK_NE(r, Error<String>("x"));
  CHECK_EQ(Error<String>("e"), r);
}

TEST_CASE(ResultEquality, OkResultVsErrorWrapper) {
  Res r(Ok<int>(1));
  CHECK_FALSE(r == Error<String>("e"));
  CHECK_FALSE(Error<String>("e") == r);
}

TEST_CASE(ResultEquality, ErrorResultVsOkWrapper) {
  Res r(Error<String>("e"));
  CHECK_FALSE(r == Ok<int>(0));
  CHECK_FALSE(Ok<int>(0) == r);
}

TEST_CASE(ResultLifetime, DestroyOkCallsDtor) {
  int dcount = 0;
  {
    Result<NTD, NTD> r(Ok(NTD(&dcount, 1)));
    int after_construct = dcount;
    (void)after_construct;
  }
  CHECK_GT(dcount, 0);
}

TEST_CASE(ResultLifetime, DestroyErrorCallsDtor) {
  int dcount = 0;
  {
    Result<NTD, NTD> r(Error(NTD(&dcount, 2)));
    int after_construct = dcount;
    (void)after_construct;
  }
  CHECK_GT(dcount, 0);
}

TEST_CASE(ResultLifetime, AssignCrossCallsDestructorOnOldValue) {
  int dcount = 0;
  Result<NTD, NTD> r(Ok(NTD(&dcount, 1)));
  dcount = 0;
  r = Error<NTD>(NTD(&dcount, 99));
  CHECK_GT(dcount, 0);
  CHECK_FALSE(r.HasValue());
  CHECK_EQ(r.GetError().v, 99);
}

TEST_CASE(ResultLifetime, TrackerBalanced) {
  Tracker::Reset();
  {
    ResT r1(Ok(Tracker(1)));
    ResT r2(r1);
    ResT r3(axio::Move(r2));
  }
  int total_created = Tracker::ctor + Tracker::copy_ctor + Tracker::move_ctor;
  CHECK_EQ(total_created, Tracker::dtor);
}

TEST_CASE(ResultMonostate, OkMonostate) {
  Result<std::monostate, int> r(Ok(std::monostate{}));
  CHECK_TRUE(r.HasValue());
}

TEST_CASE(ResultMonostate, ErrorMonostate) {
  Result<std::monostate, int> r(Error(42));
  CHECK_FALSE(r.HasValue());
  CHECK_EQ(r.GetError(), 42);
}

TEST_CASE(ResultStrongSafety, CopyAssignOkToErrorNothrowE) {
  Result<String, String> ok(Ok<String>("val"));
  Result<String, String> err(Error<String>("err"));
  err = ok;
  CHECK_TRUE(err.HasValue());
  CHECK_EQ(err.GetValue(), "val");
}

TEST_CASE(ResultStrongSafety, CopyAssignErrorToOkNothrowT) {
  Result<String, String> ok(Ok<String>("val"));
  Result<String, String> err(Error<String>("err"));
  ok = err;
  CHECK_FALSE(ok.HasValue());
  CHECK_EQ(ok.GetError(), "err");
}

TEST_CASE(ResultComplex, VectorOk) {
  Result<axio::Vector<int>, String> r(Ok(axio::Vector<int>{1, 2, 3}));
  CHECK_EQ(r.GetValue().Size(), 3u);
  CHECK_EQ(r.GetValue()[1], 2);
}

TEST_CASE(ResultComplex, StringError) {
  Result<int, String> r(Error<String>("fail"));
  CHECK_EQ(r.GetError(), "fail");
}

TEST_CASE(ResultComplex, MapToVector) {
  Res r(Ok<int>(3));
  auto r2 = r.Map([](int& v) { return axio::Vector<int>(v, v); });
  CHECK_EQ(r2.GetValue().Size(), 3u);
  CHECK_EQ(r2.GetValue()[0], 3);
}

TEST_CASE(OkErrorStruct, OkInPlace) {
  Ok<String> ok(std::in_place, 3, 'x');
  CHECK_EQ(ok.value, "xxx");
}

TEST_CASE(OkErrorStruct, ErrorInPlace) {
  Error<String> e(std::in_place, 2, 'y');
  CHECK_EQ(e.value, "yy");
}

TEST_CASE(OkErrorStruct, OkForwardingCtor) {
  Ok<String> ok(String("abc"));
  CHECK_EQ(ok.value, "abc");
}

TEST_CASE(ResultHasValue, Consistency) {
  Res ok(Ok<int>(1));
  Res err(Error<String>("e"));
  CHECK_EQ(ok.HasValue(), static_cast<bool>(ok));
  CHECK_EQ(err.HasValue(), static_cast<bool>(err));
}

TEST_CASE(ResultChaining, ThenOrElseMap) {
  auto result =
      Res(Ok<int>(2))
          .Map([](int v) { return v * 3; })
          .Then([](int v) -> Res {
            if (v > 5) {
              return Error<String>("too big");
            }
            return Ok<int>(v);
          })
          .OrElse([](String&& e) -> Res { return Ok<int>(int(e.Size())); });
  CHECK_TRUE(result.HasValue());
  CHECK_EQ(result.GetValue(), 7);
}

TEST_CASE(ResultTrivial, TrivialTypesWork) {
  Result<int, double> r(Ok<int>(7));
  Result<int, double> r2 = r;
  CHECK_EQ(r2.GetValue(), 7);
  Result<int, double> r3(Error<double>(3.14));
  Result<int, double> r4 = r3;
  CHECK_NEAR(r4.GetError(), 3.14, 0.00001);
}

TEST_CASE(ResultSameTypes, OkAndErrorSameType) {
  Result<int, int> ok(Ok<int>(1));
  Result<int, int> err(Error<int>(2));
  CHECK_TRUE(ok.HasValue());
  CHECK_EQ(ok.GetValue(), 1);
  CHECK_FALSE(err.HasValue());
  CHECK_EQ(err.GetError(), 2);
  ok = err;
  CHECK_FALSE(ok.HasValue());
  CHECK_EQ(ok.GetError(), 2);
}

TEST_CASE(ResultHetero, OkIntVsOkLong) {
  Result<int, int> r(Ok<int>(5));
  Ok<long> ok_l(5L);
  CHECK_TRUE(r == ok_l);
}