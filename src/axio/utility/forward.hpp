/**
 * @file forward.hpp
 * @brief Perfect-forwarding utility, equivalent to std::forward.
 */

#ifndef AXIO_UTILITY_FORWARD_HPP_
#define AXIO_UTILITY_FORWARD_HPP_

#include "../base/type_traits.hpp"

namespace axio {
/**
 * @brief Forwards an lvalue, preserving its original value category.
 *
 * Implements perfect forwarding. Depending on the template argument T,
 * this function preserves whether the argument is an lvalue or rvalue:
 * - If T is an lvalue reference, the value is forwarded as an lvalue.
 * - If T is an rvalue reference (or a plain type), the value is forwarded
 *   as an rvalue.
 *
 * @tparam T Deduced type parameter controlling value category.
 * @param value Value to forward.
 * @return Perfectly forwarded reference to @p value.
 *
 * @see std::forward
 */
template <typename T>
constexpr T&& Forward(RemoveReference_T<T>& value) noexcept {
  return static_cast<T&&>(value);
}

/**
 * @brief Forwards an rvalue, preserving its original value category.
 *
 * Overload for rvalue inputs to ensure correct forwarding behavior.
 *
 * @tparam T Deduced type parameter controlling value category.
 * @param value Rvalue to forward.
 * @return Perfectly forwarded reference to @p value.
 *
 * @see std::forward
 */
template <typename T>
constexpr T&& Forward(RemoveReference_T<T>&& value) noexcept {
  return static_cast<T&&>(value);
}
}  // namespace axio

#endif