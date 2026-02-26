#include <simpletest/simpletest.hpp>

#include <axio/utility/forward.hpp>

namespace {
struct Tracker {
  static int copy_count;
  static int move_count;

  Tracker() = default;
  Tracker(const Tracker&) { ++copy_count; }
  Tracker(Tracker&&) noexcept { ++move_count; }

  static void Reset() {
    copy_count = 0;
    move_count = 0;
  }
};

int Tracker::copy_count = 0;
int Tracker::move_count = 0;
}  // namespace

bool IsLvalue(const Tracker&) {
  return true;
}

bool IsLvalue(Tracker&&) {
  return false;
}

template <typename T>
bool ForwardCategory(T&& v) {
  return IsLvalue(axio::Forward<T>(v));
}

TEST_CASE(Forward, LvaluePreserved) {
  Tracker t;
  CHECK_TRUE(ForwardCategory(t));
}

TEST_CASE(Forward, RvaluePreserved) {
  CHECK_FALSE(ForwardCategory(Tracker{}));
}

TEST_CASE(Forward, ConstLvaluePreserved) {
  const Tracker t{};
  CHECK_TRUE(ForwardCategory(t));
}

TEST_CASE(Forward, MoveConstructorTriggered) {
  Tracker::Reset();

  Tracker t;
  Tracker moved = axio::Forward<Tracker>(t);

  CHECK_EQ(Tracker::copy_count, 0);
  CHECK_EQ(Tracker::move_count, 1);
  (void)moved;
}

TEST_CASE(Forward, CopyWhenForwardedAsLvalueReference) {
  Tracker::Reset();

  Tracker t;
  Tracker result = axio::Forward<Tracker&>(t);

  CHECK_EQ(Tracker::copy_count, 1);
  CHECK_EQ(Tracker::move_count, 0);
  (void)result;
}

TEST_CASE(Forward, MoveWhenForwardingRvalue) {
  Tracker::Reset();

  Tracker result = axio::Forward<Tracker>(Tracker{});

  CHECK_EQ(Tracker::copy_count, 0);
  CHECK_EQ(Tracker::move_count, 1);
  (void)result;
}

TEST_CASE(Forward, PerfectForwardingWrapperMove) {
  Tracker::Reset();

  auto wrapper = [](auto&& arg) -> Tracker {
    return Tracker(axio::Forward<decltype(arg)>(arg));
  };

  Tracker t;
  Tracker result = wrapper(axio::Forward<Tracker>(t));

  CHECK_EQ(Tracker::copy_count, 0);
  CHECK_EQ(Tracker::move_count, 1);
  (void)result;
}

TEST_CASE(Forward, PerfectForwardingWrapperCopy) {
  Tracker::Reset();

  auto wrapper = [](auto&& arg) -> Tracker {
    return Tracker(axio::Forward<decltype(arg)>(arg));
  };

  Tracker t;
  Tracker result = wrapper(t);

  CHECK_EQ(Tracker::copy_count, 1);
  CHECK_EQ(Tracker::move_count, 0);
  (void)result;
}