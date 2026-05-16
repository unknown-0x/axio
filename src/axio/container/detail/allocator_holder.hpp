#ifndef AXIO_CONTAINER_DETAIL_ALLOCATOR_HOLDER_HPP_
#define AXIO_CONTAINER_DETAIL_ALLOCATOR_HOLDER_HPP_

#include "../../base/type_traits.hpp"

namespace axio {
namespace detail {
template <typename A, Bool = ShouldUseEBO<A>::value>
struct AllocatorHolder : public A {
  AllocatorHolder() noexcept(noexcept(A())) = default;
  AllocatorHolder(const A& other) noexcept(noexcept(A(other))) : A(other) {}

  A& GetAlloc() noexcept { return *this; }
  const A& GetAlloc() const noexcept { return *this; }
};

template <typename A>
struct AllocatorHolder<A, false> {
  A alloc_;

  AllocatorHolder() noexcept(noexcept(A())) = default;
  AllocatorHolder(const A& other) noexcept(noexcept(A(other)))
      : alloc_(other) {}

  A& GetAlloc() noexcept { return alloc_; }
  const A& GetAlloc() const noexcept { return alloc_; }
};
}  // namespace detail
}  // namespace axio

#endif