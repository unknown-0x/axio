#include <simpletest/simpletest.hpp>

#include <axio/utility/move.hpp>

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

bool IsRvalue(Tracker&&) {
  return true;
}
bool IsRvalue(const Tracker&) {
  return false;
}

TEST_CASE(Move, ReturnsRvalueReferenceFromLvalue) {
  Tracker t;
  CHECK_TRUE(IsRvalue(axio::Move(t)));
}

TEST_CASE(Move, ReturnsRvalueReferenceFromRvalue) {
  CHECK_TRUE(IsRvalue(axio::Move(Tracker{})));
}

TEST_CASE(Move, MoveConstructorTriggeredFromLvalue) {
  Tracker::Reset();

  Tracker t;
  Tracker moved = axio::Move(t);

  CHECK_EQ(Tracker::copy_count, 0);
  CHECK_EQ(Tracker::move_count, 1);
  (void)moved;
}

TEST_CASE(Move, MoveConstructorTriggeredFromTemporary) {
  Tracker::Reset();

  Tracker moved = axio::Move(Tracker{});

  CHECK_EQ(Tracker::copy_count, 0);
  CHECK_EQ(Tracker::move_count, 1);
  (void)moved;
}

TEST_CASE(Move, DoesNotCopyWhenMoving) {
  Tracker::Reset();

  Tracker t;
  auto&& ref = axio::Move(t);
  (void)ref;

  CHECK_EQ(Tracker::copy_count, 0);
  CHECK_EQ(Tracker::move_count, 0);
}

TEST_CASE(Move, ConstObjectStillUsesMoveCastButCallsCopyCtor) {
  Tracker::Reset();

  const Tracker t{};
  Tracker moved = axio::Move(t);

  CHECK_EQ(Tracker::copy_count, 1);
  CHECK_EQ(Tracker::move_count, 0);
  (void)moved;
}

TEST_CASE(Move, MoveIntoFunctionParameter) {
  Tracker::Reset();

  auto sink = [](Tracker value) { (void)value; };

  Tracker t;
  sink(axio::Move(t));

  CHECK_EQ(Tracker::copy_count, 0);
  CHECK_EQ(Tracker::move_count, 1);
}