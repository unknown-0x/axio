#include <simpletest/simpletest.hpp>

#include <axio/utility/defer.hpp>
#include <vector>

TEST_CASE(Defer, RunWhenExitScope) {
  int count = 0;
  {
    AXIO_DEFER([&]() { count += 1; });
    CHECK_EQ(count, 0);
  }
  CHECK_EQ(count, 1);
}

TEST_CASE(Defer, ExecutesInReverseOrder) {
  std::vector<int> v;
  {
    AXIO_DEFER([&]() { v.push_back(1); });
    AXIO_DEFER([&]() { v.push_back(2); });
    AXIO_DEFER([&]() { v.push_back(3); });
  }

  CHECK_EQ(v.size(), 3);
  CHECK_EQ(v[0], 3);
  CHECK_EQ(v[1], 2);
  CHECK_EQ(v[2], 1);
}