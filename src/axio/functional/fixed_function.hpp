#ifndef AXIO_FUNCTIONAL_FIXED_FUNCTION_HPP_
#define AXIO_FUNCTIONAL_FIXED_FUNCTION_HPP_

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"

#include "../utility/forward.hpp"
#include "../utility/move.hpp"

#include <exception>
#include <functional>

// Reference:
// https://github.com/bitwizeshift/Delegate/blob/master/include/delegate.hpp

#if AXIO_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4646)
#pragma warning(disable : 4324)
#endif

namespace axio {
class BadFunctionCall : public std::runtime_error {
 public:
  BadFunctionCall() : runtime_error("BadFunctionCall") {}
};

namespace detail {
static constexpr auto kMinimumFunctionStorageSize =
    AXIO_MAX(sizeof(void (*)()), sizeof(void*));

template <typename T>
struct EffectiveSignatureImpl;

//----------------------------------------------------------------------------

template <typename R, typename... Args>
struct EffectiveSignatureImpl<R (*)(Args...)> {
  using type = R(Args...);
};

template <typename R, typename... Args>
struct EffectiveSignatureImpl<R (*)(Args..., ...)> {
  using type = R(Args..., ...);
};

template <typename R, typename... Args>
struct EffectiveSignatureImpl<R (*)(Args...) noexcept> {
  using type = R(Args...);
};

template <typename R, typename... Args>
struct EffectiveSignatureImpl<R (*)(Args..., ...) noexcept> {
  using type = R(Args..., ...);
};

//----------------------------------------------------------------------------

template <typename R, typename C, typename... Args>
struct EffectiveSignatureImpl<R (C::*)(Args...)> {
  using type = R(Args...);
};

template <typename R, typename C, typename... Args>
struct EffectiveSignatureImpl<R (C::*)(Args..., ...)> {
  using type = R(Args..., ...);
};

template <typename R, typename C, typename... Args>
struct EffectiveSignatureImpl<R (C::*)(Args...) noexcept> {
  using type = R(Args...);
};

template <typename R, typename C, typename... Args>
struct EffectiveSignatureImpl<R (C::*)(Args..., ...) noexcept> {
  using type = R(Args..., ...);
};

//----------------------------------------------------------------------------

template <typename R, typename C, typename... Args>
struct EffectiveSignatureImpl<R (C::*)(Args...) const> {
  using type = R(Args...);
};

template <typename R, typename C, typename... Args>
struct EffectiveSignatureImpl<R (C::*)(Args..., ...) const> {
  using type = R(Args..., ...);
};

template <typename R, typename C, typename... Args>
struct EffectiveSignatureImpl<R (C::*)(Args...) const noexcept> {
  using type = R(Args...);
};

template <typename R, typename C, typename... Args>
struct EffectiveSignatureImpl<R (C::*)(Args..., ...) const noexcept> {
  using type = R(Args..., ...);
};

//----------------------------------------------------------------------------

template <typename T>
using EffectiveSignature = typename EffectiveSignatureImpl<T>::type;
}  // namespace detail

inline namespace targets {
template <auto Function>
struct FunctionBindTarget {};

template <auto MemberFunction, typename T>
struct MemberBindTarget {
  T* object;
};

template <typename Signature>
struct OpaqueFunctionBindTarget;

template <typename R, typename... Args>
struct OpaqueFunctionBindTarget<R(Args...)> {
  R (*target)(Args...);
};

template <typename Callable>
struct CallableRefBindTarget {
  Callable* target;
};

template <typename Callable>
struct EmptyCallableBindTarget {};

template <typename Callable>
struct CallableBindTarget {
  Callable target;
};
}  // namespace targets

template <typename, SizeT = detail::kMinimumFunctionStorageSize>
class FixedFunction;

template <typename R, typename... Args, SizeT STORAGE_SIZE>
class FixedFunction<R(Args...), STORAGE_SIZE> {
 public:
  static constexpr auto kStorageAlign = (alignof(void*));
  static constexpr auto kStorageSize =
      AXIO_MAX(STORAGE_SIZE, detail::kMinimumFunctionStorageSize);

  template <typename U>
  static constexpr auto kFitsStorage =
      (sizeof(U) <= kStorageSize) && (alignof(U) <= kStorageAlign);

  using ReturnType = R;

  FixedFunction() noexcept : empty_{}, stub_{&NullStub} {}

  template <auto F,
            typename = typename EnableIf<
                IsInvocableR<R, decltype(F), Args...>::value>::type>
  FixedFunction(FunctionBindTarget<F>) noexcept
      : empty_{}, stub_{&FunctionStub<F>} {}

  template <
      auto MemberFunction,
      typename T,
      typename = typename EnableIf<
          IsInvocableR<R, decltype(MemberFunction), T&, Args...>::value>::type>
  FixedFunction(MemberBindTarget<MemberFunction, T> target) noexcept
      : object_{target.object}, stub_{&MemberFunctionStub<MemberFunction, T>} {}

  template <auto MemberFunction,
            typename T,
            typename = typename EnableIf<
                IsInvocableR<R, decltype(MemberFunction), const T&, Args...>::
                    value>::type>
  FixedFunction(MemberBindTarget<MemberFunction, const T> target) noexcept
      : const_object_{target.object},
        stub_{&MemberFunctionStub<MemberFunction, const T>} {}

  template <typename Fn,
            typename = typename EnableIf<IsInvocableR<R, Fn, Args...>::value &&
                                         !IsFunction<Fn>::value>::type>
  FixedFunction(CallableRefBindTarget<Fn> target) noexcept
      : object_{target.target}, stub_{&CallableViewStub<Fn>} {}

  template <
      typename Fn,
      typename = typename EnableIf<IsInvocableR<R, const Fn, Args...>::value &&
                                   !IsFunction<Fn>::value>::type>
  FixedFunction(CallableRefBindTarget<const Fn> target) noexcept
      : const_object_{target.target}, stub_{&CallableViewStub<const Fn>} {}

  template <typename Fn,
            typename = typename EnableIf<
                IsEmpty<Fn>::value && IsDefaultConstructible<Fn>::value &&
                IsInvocableR<R, Fn, Args...>::value>::type>
  FixedFunction(EmptyCallableBindTarget<Fn> target) noexcept
      : empty_{}, stub_{&EmptyCallableStub<Fn>} {}

  template <
      typename Fn,
      typename DecayedFn = typename Decay<Fn>::type,
      typename = typename EnableIf<
          !IsFunction<typename RemovePointer<Fn>::type>::value &&
          kFitsStorage<DecayedFn> && IsConstructible<DecayedFn, Fn>::value &&
          IsTriviallyDestructible<DecayedFn>::value &&
          IsTriviallyCopyable<DecayedFn>::value &&
          IsInvocableR<R, const Fn&, Args...>::value>::type>
  FixedFunction(CallableBindTarget<Fn> target) noexcept
      : empty_{}, stub_{&SmallCallableStub<Fn>} {
    new (static_cast<void*>(storage_)) Fn(axio::Move(target.target));
  }

  template <typename UR,
            typename... UArgs,
            typename = typename EnableIf<
                IsInvocableR<R, UR (*)(UArgs...), Args...>::value>::type>
  FixedFunction(OpaqueFunctionBindTarget<UR(UArgs...)> target) noexcept
      : function_{reinterpret_cast<AnyFunction>(target.target)},
        stub_{&FunctionPtrStub<UR, UArgs...>} {}

  FixedFunction(FixedFunction&&) noexcept = default;
  FixedFunction(const FixedFunction&) = default;

  FixedFunction& operator=(FixedFunction&&) noexcept = default;
  FixedFunction& operator=(const FixedFunction&) = default;

  constexpr explicit operator bool() const noexcept {
    return stub_ != &NullStub;
  }

  template <typename... UArgs,
            typename =
                std::enable_if_t<std::is_invocable_v<R (*)(Args...), UArgs...>>>
  constexpr R operator()(UArgs&&... args) const {
    return stub_(this, axio::Forward<UArgs>(args)...);
  }

 private:
  [[noreturn]] static R NullStub(const FixedFunction*, Args...) {
    throw BadFunctionCall{};
  }

  template <auto Function>
  static R FunctionStub(const FixedFunction*, Args... args) {
    if constexpr (IsVoid<R>::value) {
      std::invoke(Function, axio::Forward<Args>(args)...);
    } else {
      return std::invoke(Function, axio::Forward<Args>(args)...);
    }
  }

  template <auto MemberFunction, typename T>
  static R MemberFunctionStub(const FixedFunction* self, Args... args) {
    auto* const c = [&self] {
      if constexpr (IsConst<T>::value) {
        return static_cast<T*>(self->const_object_);
      } else {
        return static_cast<T*>(self->object_);
      }
    }();

    if constexpr (IsVoid<R>::value) {
      std::invoke(MemberFunction, *c, axio::Forward<Args>(args)...);
    } else {
      return std::invoke(MemberFunction, *c, axio::Forward<Args>(args)...);
    }
  }

  template <typename Fn>
  static R CallableViewStub(const FixedFunction* self, Args... args) {
    auto* const f = [&self] {
      if constexpr (IsConst<Fn>::value) {
        return static_cast<Fn*>(self->const_object_);
      } else {
        return static_cast<Fn*>(self->object_);
      }
    }();

    if constexpr (IsVoid<R>::value) {
      // std::invoke(*f, axio::Forward<Args>(args)...);
      (*f)(axio::Forward<Args>(args)...);
    } else {
      return (*f)(axio::Forward<Args>(args)...);

      // return std::invoke(*f, axio::Forward<Args>(args)...);
    }
  }

  template <typename Fn>
  static R EmptyCallableStub(const FixedFunction*, Args... args) {
    if constexpr (IsVoid<R>::value) {
      Fn{}(axio::Forward<Args>(args)...);
      // std::invoke(Fn{}, axio::Forward<Args>(args)...);
    } else {
      return Fn{}(axio::Forward<Args>(args)...);
      // return std::invoke(Fn{}, axio::Forward<Args>(args)...);
    }
  }

  template <typename Fn>
  static R SmallCallableStub(const FixedFunction* self, Args... args) {
    const auto& f = *std::launder(reinterpret_cast<const Fn*>(self->storage_));

    if constexpr (IsVoid<R>::value) {
      // std::invoke(f, axio::Forward<Args>(args)...);
      f(axio::Forward<Args>(args)...);
    } else {
      // return std::invoke(f, axio::Forward<Args>(args)...);
      return f(axio::Forward<Args>(args)...);
    }
  }

  template <typename R2, typename... Args2>
  static R FunctionPtrStub(const FixedFunction* self, Args... args) {
    const auto f = reinterpret_cast<R2 (*)(Args...)>(self->function_);

    if constexpr (IsVoid<R>::value) {
      // std::invoke(f, axio::Forward<Args>(args)...);
      f(axio::Forward<Args>(args)...);
    } else {
      // return std::invoke(f, axio::Forward<Args>(args)...);
      return f(axio::Forward<Args>(args)...);
    }
  }

  using AnyFunction = void (*)();
  using StubFunction = R (*)(const FixedFunction*, Args...);

  struct EmptyType {};

  union {
    EmptyType empty_;
    void* object_{};
    const void* const_object_;
    AnyFunction function_;
    alignas(kStorageAlign) unsigned char storage_[kStorageSize];
  };
  StubFunction stub_;
};

template <auto F>
AXIO_INLINE FunctionBindTarget<F> Bind() noexcept {
  return {};
}

template <auto MemberFunction, typename T>
AXIO_INLINE MemberBindTarget<MemberFunction, T> Bind(T* p) noexcept {
  AXIO_ASSERT(p);
  return {p};
}

template <typename Callable>
AXIO_INLINE CallableRefBindTarget<Callable> Bind(Callable* fn) noexcept {
  AXIO_ASSERT(fn);
  return {fn};
}

template <typename R, typename... Args>
AXIO_INLINE OpaqueFunctionBindTarget<R(Args...)> Bind(
    R (*fn)(Args...)) noexcept {
  AXIO_ASSERT(fn);
  return {fn};
}

template <typename Callable>
AXIO_INLINE EmptyCallableBindTarget<Callable> Bind() noexcept {
  return {};
}

template <
    typename Callable,
    typename = typename EnableIf<IsEmpty<Callable>::value &&
                                 IsDefaultConstructible<Callable>::value>::type>
AXIO_INLINE EmptyCallableBindTarget<Callable> Bind(Callable callable) noexcept {
  return {};
}

template <typename Callable,
          typename DecayedCallable = typename Decay<Callable>::type,
          typename = typename EnableIf<
              IsTriviallyCopyable<DecayedCallable>::value &&
              IsTriviallyDestructible<DecayedCallable>::value>::type>
AXIO_INLINE CallableBindTarget<DecayedCallable> Bind(
    Callable&& callable) noexcept {
  return {axio::Forward<Callable>(callable)};
}
}  // namespace axio

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif