#include <math/math.hpp>
#include <simpletest/simpletest.hpp>

TEST_CASE(Math, Add) {
  CHECK_EQ(math::Add(1, 2), 3);
  CHECK_EQ(math::Add(4, 1), 5);
}

TEST_CASE(Math, Subtract) {
  CHECK_EQ(math::Subtract(1, 2), -1);
  CHECK_EQ(math::Subtract(4, 1), 3);
}

TEST_CASE(Math, Max) {
  CHECK_EQ(math::Max(1, 2), 2);
  CHECK_EQ(math::Max(4, 1), 4);
}