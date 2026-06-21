/**
 * @file allocator.hpp
 * @brief Stateless allocator backed by global operator new/delete.
 */

#ifndef AXIO_MEMORY_ALLOCATOR_HPP_
#define AXIO_MEMORY_ALLOCATOR_HPP_

#include "../base/macros.hpp"
#include "../base/types.hpp"

#include <new>

namespace axio {

/**
 * @brief Simple stateless allocator using global operator new/delete.
 *
 * @tparam T Type of allocated elements.
 */
template <typename T>
class Allocator {
 public:
  using value_type = T;

  /// @brief STL propagation trait: allocator can be moved safely.
  using propagate_on_container_move_assignment = std::true_type;

  /**
   * @brief Maximum number of elements that can be allocated in one request.
   */
  static constexpr SizeT kMaxSize = static_cast<SizeT>(-1) / sizeof(T);

  /// @brief Constructs a stateless allocator.
  Allocator() noexcept = default;

  /// @brief Copy-constructs an allocator (no-op, stateless).
  Allocator(const Allocator&) noexcept = default;
  /// @brief Move-constructs an allocator (no-op, stateless).
  Allocator(Allocator&&) noexcept = default;
  /**
   * @brief Converts from an allocator of a different element type.
   * @tparam U Element type of the source allocator.
   */
  template <typename U>
  Allocator(const Allocator<U>&) noexcept {}

  /// @brief Copy-assigns an allocator (no-op, stateless).
  Allocator& operator=(const Allocator&) noexcept = default;
  /// @brief Move-assigns an allocator (no-op, stateless).
  Allocator& operator=(Allocator&&) noexcept = default;

  /**
   * @brief Allocates raw uninitialized storage for N objects of type T.
   *
   * Allocation is performed using global ::operator new.
   * If T requires over-aligned storage, aligned allocation is used.
   *
   * @param num Number of elements to allocate.
   * @return Pointer to raw memory suitable for constructing T objects, or
   * `nullptr` if @p num is 0.
   *
   * @throws std::bad_array_new_length if @p num exceeds kMaxSize (overflow
   * protection).
   *
   * @warning Returned memory is uninitialized.
   */
  AXIO_NODISCARD T* allocate(SizeT num) {
    if (AXIO_LIKELY(num == 0)) {
      return nullptr;
    }

    if (AXIO_LIKELY(num > kMaxSize)) {
      throw std::bad_array_new_length();
    }

    if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      return static_cast<T*>(
          ::operator new(num * sizeof(T), std::align_val_t{alignof(T)}));
    } else {
      return static_cast<T*>(::operator new(num * sizeof(T)));
    }
  }

  /**
   * @brief Deallocates previously allocated memory.
   *
   * Must match a prior call to allocate(). Alignment is automatically
   * handled based on `alignof(T)`.
   *
   * @param ptr Pointer returned by allocate(). May be `nullptr`.
   */
  void deallocate(T* ptr, SizeT) {
    if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      ::operator delete(ptr, std::align_val_t{alignof(T)});
    } else {
      ::operator delete(ptr);
    }
  }
};

/**
 * @brief Compares two allocators for equality.
 *
 * Allocators of all types are always equal, since this allocator is
 * stateless and all instances compare equal.
 *
 * @tparam T Element type of the left-hand allocator.
 * @tparam U Element type of the right-hand allocator.
 * @return Always `true`.
 */
template <typename T, typename U>
inline Bool operator==(const Allocator<T>&, const Allocator<U>&) noexcept {
  return true;
}

/**
 * @brief Compares two allocators for inequality.
 *
 * @tparam T Element type of the left-hand allocator.
 * @tparam U Element type of the right-hand allocator.
 * @return Always `false`.
 */
template <typename T, typename U>
inline Bool operator!=(const Allocator<T>&, const Allocator<U>&) noexcept {
  return false;
}
}  // namespace axio

#endif