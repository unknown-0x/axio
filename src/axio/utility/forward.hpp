#ifndef AXIO_UTILITY_FORWARD_HPP_
#define AXIO_UTILITY_FORWARD_HPP_

#include "../base/type_traits.hpp"

namespace axio {
template <typename T>
constexpr T&& Forward(axio::T<RemoveReference<T>>& value) noexcept {
  return static_cast<T&&>(value);
}

template <typename T>
constexpr T&& Forward(axio::T<RemoveReference<T>>&& value) noexcept {
  return static_cast<T&&>(value);
}
}  // namespace axio

#endif