#include <simpletest/simpletest.hpp>

#include <axio/base/type_traits.hpp>
#include <axio/memory/allocator.hpp>

#include <memory>
#include <string>
#include <vector>

TEST_CASE(Allocator, StdAllocatorTraitsCompatibility) {
  IGNORE_RESULT();

  using Traits = std::allocator_traits<axio::Allocator<int>>;

  static_assert(
      axio::V<axio::IsSame<Traits::allocator_type, axio::Allocator<int>>>, "");

  static_assert(axio::V<axio::IsSame<Traits::value_type, int>>, "");
  static_assert(axio::V<axio::IsSame<Traits::pointer, int*>>, "");
  static_assert(axio::V<axio::IsSame<Traits::const_pointer, const int*>>, "");
  static_assert(axio::V<axio::IsSame<Traits::void_pointer, void*>>, "");
  static_assert(axio::V<axio::IsSame<Traits::const_void_pointer, const void*>>,
                "");
  static_assert(axio::V<axio::IsSame<Traits::difference_type, std::ptrdiff_t>>,
                "");
  static_assert(axio::V<axio::IsSame<Traits::size_type, std::size_t>>, "");
}

TEST_CASE(Allocator, BasicTest) {
  using Alloc = axio::Allocator<int>;
  using AllocTraits = std::allocator_traits<Alloc>;

  Alloc allocator;
  int* p = AllocTraits::allocate(allocator, 4);
  CHECK_NE(p, nullptr);
  AllocTraits::deallocate(allocator, p, 4);
}

TEST_CASE(Allocator, WithStdVector) {
  std::vector<int, axio::Allocator<int>> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  CHECK_EQ(v.size(), 3);
  CHECK_EQ(v[0], 1);
  CHECK_EQ(v[1], 2);
  CHECK_EQ(v[2], 3);
}

TEST_CASE(Allocator, TestOverflow) {
  using Alloc = axio::Allocator<int>;
  using AllocTraits = std::allocator_traits<Alloc>;

  bool threw = false;
  Alloc alloc;
  try {
    const auto n = std::numeric_limits<axio::SizeT>::max();
    auto* p = alloc.allocate(n);
    AllocTraits::deallocate(alloc, p, n);
  } catch (const std::bad_array_new_length&) {
    threw = true;
  }

  CHECK_TRUE(threw);
}

struct alignas(64) OverAligned {
  float x[16];
};

TEST_CASE(Allocator, WithOverAligned) {
  using Alloc = axio::Allocator<OverAligned>;

  Alloc alloc;
  OverAligned* p = alloc.allocate(2);

  CHECK_EQ(reinterpret_cast<std::uintptr_t>(p) % 64, 0);

  alloc.deallocate(p, 2);
}

TEST_CASE(Allocator, ConstructAndDestroy) {
  using Alloc = axio::Allocator<std::string>;
  using AllocTraits = std::allocator_traits<Alloc>;

  const auto n = 100;

  Alloc alloc;
  auto* p = AllocTraits::allocate(alloc, n);

  for (auto i = 0; i < n; ++i) {
    AllocTraits::construct(alloc, p + i, "abcdef");
    CHECK_EQ(p[i], "abcdef");
  }

  for (auto i = 0; i < n; ++i) {
    AllocTraits::destroy(alloc, p + i);
  }

  AllocTraits::deallocate(alloc, p, n);
}