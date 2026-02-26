#ifndef AXIO_MEMORY_ALLOCATOR_HPP_
#define AXIO_MEMORY_ALLOCATOR_HPP_

#include "../base/macros.hpp"
#include "../base/types.hpp"

#include <new>

namespace axio {
template <typename T>
class Allocator {
 public:
  using value_type = T;
  using propagate_on_container_move_assignment = std::true_type;

  static constexpr SizeT kMaxSize = static_cast<SizeT>(-1) / sizeof(T);

  Allocator() noexcept = default;

  Allocator(const Allocator&) noexcept = default;
  Allocator(Allocator&&) noexcept = default;
  template <typename U>
  Allocator(const Allocator<U>&) noexcept {}

  Allocator& operator=(const Allocator&) noexcept = default;
  Allocator& operator=(Allocator&&) noexcept = default;

  AXIO_NODISCARD T* allocate(SizeT num) {
    if (num == 0) {
      return nullptr;
    }

    if (num > kMaxSize) {
      throw std::bad_array_new_length();
    }

    if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      return static_cast<T*>(
          ::operator new(num * sizeof(T), std::align_val_t{alignof(T)}));
    } else {
      return static_cast<T*>(::operator new(num * sizeof(T)));
    }
  }

  void deallocate(T* ptr, SizeT) {
    if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      ::operator delete(ptr, std::align_val_t{alignof(T)});
    } else {
      ::operator delete(ptr);
    }
  }
};
}  // namespace axio

#endif