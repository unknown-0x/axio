#ifndef AXIO_UTILITY_MOVE_HPP_
#define AXIO_UTILITY_MOVE_HPP_

#include "../base/type_traits.hpp"

namespace axio {
template <typename T>
constexpr axio::T<RemoveReference<T>>&& Move(T&& value) noexcept {
  return static_cast<axio::T<RemoveReference<T>>&&>(value);
}
}  // namespace axio

#endif