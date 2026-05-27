#ifndef AXIO_UTILITY_STRONG_TYPE_HPP_
#define AXIO_UTILITY_STRONG_TYPE_HPP_

#include "../base/macros.hpp"
#include "move.hpp"

namespace axio {
template <typename T, typename Tag, template <typename> class... Skills>
struct StrongType : public Skills<StrongType<T, Tag, Skills...>>... {
  static_assert(!IsReference<T>::value, "Reference types are not supported");

  using ValueType = T;

  constexpr StrongType() : value_{} {}

  constexpr StrongType(const StrongType&) = default;
  constexpr StrongType(StrongType&&) = default;
  constexpr StrongType& operator=(const StrongType&) = default;
  constexpr StrongType& operator=(StrongType&&) = default;

  explicit constexpr StrongType(const T& value) : value_(value) {}
  explicit constexpr StrongType(T&& value) : value_(axio::Move(value)) {}

  AXIO_NODISCARD constexpr T& Get() noexcept { return value_; }
  AXIO_NODISCARD constexpr const T& Get() const noexcept { return value_; }

  AXIO_NODISCARD explicit constexpr operator T() const noexcept {
    return value_;
  }

 private:
  ValueType value_;
};
}  // namespace axio

#endif