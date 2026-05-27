#ifndef AXIO_UTILITY_DEFER_HPP_
#define AXIO_UTILITY_DEFER_HPP_

#include "../base/macros.hpp"
#include "forward.hpp"
#include "move.hpp"

namespace axio {
template <typename F>
struct AXIO_NODISCARD Defer {
  template <typename T>
  explicit Defer(T&& f) : func_{axio::Forward<T>(f)}, active_{true} {}

  Defer(const Defer&) = delete;
  Defer& operator=(Defer&&) = delete;
  Defer& operator=(const Defer&) = delete;

  Defer(Defer&& other) noexcept
      : func_{axio::Move(other.func_)}, active_{true} {
    other.active_ = false;
  }

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

#define AXIO_DEFER(...) \
  [[maybe_unused]]      \
  auto AXIO_CONCAT(__defer__, __LINE__) = axio::Defer(__VA_ARGS__)
}  // namespace axio

#endif