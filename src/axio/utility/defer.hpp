/**
 * @file defer.hpp
 * @brief Scope-based deferred execution utility (RAII cleanup helper).
 */

#ifndef AXIO_UTILITY_DEFER_HPP_
#define AXIO_UTILITY_DEFER_HPP_

#include "../base/macros.hpp"
#include "forward.hpp"
#include "move.hpp"

namespace axio {
/**
 * @brief Executes a callable automatically when leaving scope.
 *
 * @tparam F Type of callable object to execute on destruction.
 */
template <typename F>
struct AXIO_NODISCARD Defer {
  /**
   * @brief Constructs a deferred action from a callable.
   *
   * The callable is stored internally and will be invoked when the object
   * goes out of scope unless it has been moved.
   *
   * @tparam T Callable type (deduced).
   * @param f Callable object to be executed on destruction.
   */
  template <typename T>
  explicit Defer(T&& f) : func_{axio::Forward<T>(f)}, active_{true} {}

  /// @brief Deleted: deferred actions are not copyable.
  Defer(const Defer&) = delete;
  /// @brief Deleted: deferred actions cannot be move-assigned.
  Defer& operator=(Defer&&) = delete;
  /// @brief Deleted: deferred actions cannot be copy-assigned.
  Defer& operator=(const Defer&) = delete;

  /**
   * @brief Move constructor transferring ownership of the deferred action.
   *
   * The moved-from object is disabled and will not execute the callable.
   *
   * @param other Defer instance to move from.
   */
  Defer(Defer&& other) noexcept
      : func_{axio::Move(other.func_)}, active_{true} {
    other.active_ = false;
  }

  /**
   * @brief Invokes the stored callable, unless this instance was moved from.
   */
  ~Defer() noexcept {
    if (active_) {
      func_();
    }
  }

 private:
  Decay_T<F> func_;
  bool active_;
};

template <typename F>
Defer(F&&) -> Defer<F>;

/**
 * @brief Creates a named deferred execution object in current scope.
 *
 * @param ... Callable invoked when the enclosing scope is exited.
 *
 * Example:
 * ```cpp
 * AXIO_DEFER([] {
 *   CleanUp();
 * });
 * ```
 * @note The object is marked `[[maybe_unused]]` to avoid compiler warnings.
 */
#define AXIO_DEFER(...) \
  [[maybe_unused]]      \
  auto AXIO_CONCAT(__defer__, __LINE__) = axio::Defer(__VA_ARGS__)
}  // namespace axio

#endif