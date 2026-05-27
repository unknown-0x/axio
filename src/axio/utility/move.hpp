#ifndef AXIO_UTILITY_MOVE_HPP_
#define AXIO_UTILITY_MOVE_HPP_

#include "../base/type_traits.hpp"

namespace axio {
template <typename T>
constexpr RemoveReference_T<T>&& Move(T&& value) noexcept {
  return static_cast<RemoveReference_T<T>&&>(value);
}
}  // namespace axio

#endif