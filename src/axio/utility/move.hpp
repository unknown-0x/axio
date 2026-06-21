/**
 * @file move.hpp
 * @brief Unconditional rvalue-cast utility, equivalent to std::move.
 */

#ifndef AXIO_UTILITY_MOVE_HPP_
#define AXIO_UTILITY_MOVE_HPP_

#include "../base/type_traits.hpp"

namespace axio {
/**
 * @brief Converts an object into an rvalue reference.
 *
 * @tparam T Deduced type of the object.
 * @param value Object to be cast to an rvalue reference.
 * @return Rvalue reference to @p value.
 *
 * @see std::move
 */
template <typename T>
constexpr RemoveReference_T<T>&& Move(T&& value) noexcept {
  return static_cast<RemoveReference_T<T>&&>(value);
}
}  // namespace axio

#endif