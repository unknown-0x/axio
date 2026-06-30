#ifndef AXIO_FUNCTIONAL_OVERLOAD_HPP_
#define AXIO_FUNCTIONAL_OVERLOAD_HPP_

#include "../utility/move.hpp"

namespace axio {
/**
 * @brief Combines multiple callable objects into a single overloaded function
 * object.
 *
 * Similar to `absl::Overload`
 *
 * `axio::Overload` inherits from each callable and exposes all of their
 * `operator()` overloads through a single object. This is particularly useful
 * for constructing visitors for `std::variant`, or for grouping multiple
 * lambda expressions into a single callable.
 *
 * Example:
 *
 * ```cpp
 * auto overloaded = axio::Overload{
 *     [](int x) { return x + 1; },
 *     [](int a, int b) { return a * b; },
 *     []() { return "none"; }
 * };
 *
 * overloaded(2);      // 3
 * overloaded(3, 7);   // 21
 * overloaded();       // "none"
 * ```
 *
 * @tparam T Types of the callable objects.
 */
template <typename... T>
struct Overload final : T... {
  /// Imports every inherited call operator into the overload set.
  using T::operator()...;

  /**
   * @brief Constructs an overloaded function object from multiple callables.
   *
   * Each callable is move-constructed into its corresponding base class.
   *
   * @param ts Callable objects to combine.
   */
  constexpr explicit Overload(T... ts) : T(axio::Move(ts))... {}
};

/**
 * @brief Deduction guide for `axio::Overload`.
 *
 * Allows constructing an `Overload` without explicitly specifying template
 * arguments.
 *
 * Example:
 *
 * ```cpp
 * auto f = axio::Overload{
 *     [](int) {},
 *     [](double) {}
 * };
 * ```
 */
template <typename... T>
Overload(T...) -> Overload<T...>;
}  // namespace axio

#endif