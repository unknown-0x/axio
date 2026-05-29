#ifndef AXIO_UTILITY_RESULT_HPP_
#define AXIO_UTILITY_RESULT_HPP_

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"

#include "forward.hpp"
#include "move.hpp"

namespace axio {
template <typename>
struct Ok;

template <typename>
struct Error;

namespace result_detail {
struct OkTag {
  explicit constexpr OkTag() noexcept = default;
};

struct ErrorTag {
  explicit constexpr ErrorTag() noexcept = default;
};

struct UninitTag {
  explicit constexpr UninitTag() noexcept = default;
};

inline constexpr OkTag kOkTag{};
inline constexpr ErrorTag kErrorTag{};
inline constexpr UninitTag kUninitTag{};

template <typename T, typename E>
struct Traits {
  static constexpr bool kTrivialDtor =
      IsTriviallyDestructible_V<T> && IsTriviallyDestructible_V<E>;

  static constexpr bool kTrivialCopyCtor =
      IsTriviallyCopyConstructible_V<T> && IsTriviallyCopyConstructible_V<E>;

  static constexpr bool kTrivialMoveCtor =
      IsTriviallyMoveConstructible_V<T> && IsTriviallyMoveConstructible_V<E>;

  static constexpr bool kTrivialCopyAssign = IsTriviallyCopyAssignable_V<T> &&
                                             IsTriviallyCopyAssignable_V<E> &&
                                             kTrivialDtor;

  static constexpr bool kTrivialMoveAssign = IsTriviallyMoveAssignable_V<T> &&
                                             IsTriviallyMoveAssignable_V<E> &&
                                             kTrivialDtor;

  static constexpr bool kNothrowCopyCtor =
      IsNothrowCopyConstructible_V<T> && IsNothrowCopyConstructible_V<E>;

  static constexpr bool kNothrowMoveCtor =
      IsNothrowMoveConstructible_V<T> && IsNothrowMoveConstructible_V<E>;

  static constexpr bool kNothrowCopyAssign = kNothrowCopyCtor &&
                                             IsNothrowCopyAssignable_V<T> &&
                                             IsNothrowCopyAssignable_V<E>;

  static constexpr bool kNothrowMoveAssign = kNothrowMoveCtor &&
                                             IsNothrowMoveAssignable_V<T> &&
                                             IsNothrowMoveAssignable_V<E>;
};

template <typename T, typename E, bool /*trivially destructible*/ = true>
struct StorageBase {
  union {
    T value;
    E error;
  };
  bool has;

  template <typename... Args>
  constexpr StorageBase(OkTag, Args&&... a) noexcept(
      IsNothrowConstructible_V<T, Args...>)
      : value(axio::Forward<Args>(a)...), has(true) {}

  template <typename... Args>
  constexpr StorageBase(ErrorTag, Args&&... a) noexcept(
      IsNothrowConstructible_V<E, Args...>)
      : error(axio::Forward<Args>(a)...), has(false) {}

  constexpr StorageBase(UninitTag) noexcept : has(false) {}

  StorageBase() = delete;
  ~StorageBase() = default;

  constexpr void DestroyValue() noexcept {}
  constexpr void DestroyError() noexcept {}
};

template <typename T, typename E>
struct StorageBase<T, E, false> {
  union {
    T value;
    E error;
  };
  bool has;

  template <typename... Args>
  constexpr StorageBase(OkTag, Args&&... a) noexcept(
      IsNothrowConstructible_V<T, Args...>)
      : value(axio::Forward<Args>(a)...), has(true) {}

  template <typename... Args>
  constexpr StorageBase(ErrorTag, Args&&... a) noexcept(
      IsNothrowConstructible_V<E, Args...>)
      : error(axio::Forward<Args>(a)...), has(false) {}

  StorageBase(UninitTag) noexcept : has(false) {}

  StorageBase() = delete;

  constexpr void DestroyValue() noexcept { value.~T(); }
  constexpr void DestroyError() noexcept { error.~E(); }

  ~StorageBase() noexcept {
    if (has) {
      value.~T();
    } else {
      error.~E();
    }
  }
};

template <typename T, typename E, bool TRIVIAL = Traits<T, E>::kTrivialCopyCtor>
struct CopyBase : StorageBase<T, E, Traits<T, E>::kTrivialDtor> {
  using SB = StorageBase<T, E, Traits<T, E>::kTrivialDtor>;
  using SB::SB;

  CopyBase(const CopyBase& cb) noexcept(Traits<T, E>::kNothrowCopyCtor)
      : SB(kUninitTag) {
    this->has = cb.has;
    if (cb.has) {
      ::new (static_cast<void*>(&this->value)) T(cb.value);
    } else {
      ::new (static_cast<void*>(&this->error)) E(cb.error);
    }
  }

  CopyBase(CopyBase&&) = default;
  CopyBase& operator=(const CopyBase&) = default;
  CopyBase& operator=(CopyBase&&) = default;
};

template <typename T, typename E>
struct CopyBase<T, E, true> : StorageBase<T, E, Traits<T, E>::kTrivialDtor> {
  using SB = StorageBase<T, E, Traits<T, E>::kTrivialDtor>;
  using SB::SB;
};

template <typename T, typename E, bool TRIVIAL = Traits<T, E>::kTrivialMoveCtor>
struct MoveBase : CopyBase<T, E> {
  using CB = CopyBase<T, E>;
  using CB::CB;

  MoveBase(MoveBase&& mb) noexcept(Traits<T, E>::kNothrowMoveCtor)
      : CB(kUninitTag) {
    this->has = mb.has;
    if (mb.has) {
      ::new (static_cast<void*>(&this->value)) T(axio::Move(mb.value));
    } else {
      ::new (static_cast<void*>(&this->error)) E(axio::Move(mb.error));
    }
  }

  MoveBase(const MoveBase&) = default;
  MoveBase& operator=(const MoveBase&) = default;
  MoveBase& operator=(MoveBase&&) = default;
};

template <typename T, typename E>
struct MoveBase<T, E, true> : CopyBase<T, E> {
  using CB = CopyBase<T, E>;
  using CB::CB;
};

template <typename T, typename E>
struct OpsBase : MoveBase<T, E> {
  using MB = MoveBase<T, E>;
  using MB::MB;

  constexpr void CopyAssignFrom(const OpsBase& o) noexcept(
      Traits<T, E>::kNothrowCopyAssign) {
    if (this->has == o.has) {
      if (this->has) {
        this->value = o.value;
      } else {
        this->error = o.error;
      }
    } else if (this->has) {
      if constexpr (IsNothrowCopyConstructible_V<E>) {
        this->DestroyValue();
        ::new (static_cast<void*>(&this->error)) E(o.error);
        this->has = false;
      } else {
        E error_temp(o.error);
        this->DestroyValue();
        ::new (static_cast<void*>(&this->error)) E(axio::Move(error_temp));
        this->has = false;
      }
    } else {
      if constexpr (IsNothrowCopyConstructible_V<T>) {
        this->DestroyError();
        ::new (static_cast<void*>(&this->value)) T(o.value);
        this->has = true;
      } else {
        T temp(o.value);
        this->DestroyError();
        ::new (static_cast<void*>(&this->value)) T(axio::Move(temp));
        this->has = true;
      }
    }
  }

  constexpr void MoveAssignFrom(OpsBase&& o) noexcept(
      Traits<T, E>::kNothrowMoveAssign) {
    if (this->has == o.has) {
      if (this->has) {
        this->value = axio::Move(o.value);
      } else {
        this->error = axio::Move(o.error);
      }
    } else if (this->has) {
      this->DestroyValue();
      ::new (static_cast<void*>(&this->error)) E(axio::Move(o.error));
      this->has = false;
    } else {
      this->DestroyError();
      ::new (static_cast<void*>(&this->value)) T(axio::Move(o.value));
      this->has = true;
    }
  }
};

template <typename T,
          typename E,
          bool TRIVIAL = Traits<T, E>::kTrivialCopyAssign>
struct CopyAssignBase : OpsBase<T, E> {
  using OB = OpsBase<T, E>;
  using OB::OB;

  constexpr CopyAssignBase& operator=(const CopyAssignBase& o) noexcept(
      Traits<T, E>::kNothrowCopyAssign) {
    if (this != &o) {
      this->CopyAssignFrom(o);
    }
    return *this;
  }
  CopyAssignBase(const CopyAssignBase&) = default;
  CopyAssignBase(CopyAssignBase&&) = default;
  CopyAssignBase& operator=(CopyAssignBase&&) = default;
};

template <typename T, typename E>
struct CopyAssignBase<T, E, true> : OpsBase<T, E> {
  using OB = OpsBase<T, E>;
  using OB::OB;
};

template <typename T,
          typename E,
          bool TRIVIAL = Traits<T, E>::kTrivialMoveAssign>
struct MoveAssignBase : CopyAssignBase<T, E> {
  using CA = CopyAssignBase<T, E>;
  using CA::CA;

  constexpr MoveAssignBase& operator=(MoveAssignBase&& o) noexcept(
      Traits<T, E>::kNothrowMoveAssign) {
    if (this != &o) {
      this->MoveAssignFrom(axio::Move(o));
    }
    return *this;
  }
  MoveAssignBase(const MoveAssignBase&) = default;
  MoveAssignBase(MoveAssignBase&&) = default;
  MoveAssignBase& operator=(const MoveAssignBase&) = default;
};

template <typename T, typename E>
struct MoveAssignBase<T, E, true> : CopyAssignBase<T, E> {
  using CA = CopyAssignBase<T, E>;
  using CA::CA;
};

template <typename T, typename E>
using ResultBase = MoveAssignBase<T, E>;
}  // namespace result_detail

template <typename T>
struct Ok {
  T value;

  template <typename U,
            typename = EnableIf_T<IsConstructible_V<T, U&&> &&
                                  !IsSame_V<Decay_T<U>, std::in_place_t>>>
  constexpr explicit Ok(U&& v) noexcept(IsNothrowConstructible_V<T, U&&>)
      : value(axio::Forward<U>(v)) {}

  template <typename... Args,
            typename = EnableIf_T<IsConstructible_V<T, Args...>>>
  constexpr explicit Ok(std::in_place_t, Args&&... a) noexcept(
      IsNothrowConstructible_V<T, Args...>)
      : value(axio::Forward<Args>(a)...) {}
};

template <typename E>
struct Error {
  E value;

  template <typename U,
            typename = EnableIf_T<IsConstructible_V<E, U&&> &&
                                  !IsSame_V<Decay_T<U>, std::in_place_t>>>
  constexpr explicit Error(U&& v) noexcept(IsNothrowConstructible_V<E, U&&>)
      : value(axio::Forward<U>(v)) {}

  template <typename... Args,
            typename = EnableIf_T<IsConstructible_V<E, Args...>>>
  constexpr explicit Error(std::in_place_t, Args&&... a) noexcept(
      IsNothrowConstructible_V<E, Args...>)
      : value(axio::Forward<Args>(a)...) {}
};

template <typename T>
Ok(T&&) -> Ok<Decay_T<T>>;

template <typename E>
Error(E&&) -> Error<Decay_T<E>>;

template <typename T, typename E>
class AXIO_NODISCARD Result : private result_detail::ResultBase<T, E> {
  static_assert(!IsReference_V<T>, "T must not be a reference");
  static_assert(!IsReference_V<E>, "E must not be a reference");
  static_assert(!IsVoid_V<T>, "Use Result<std::monostate, E>");
  static_assert(!IsVoid_V<E>, "E must not be void");
  static_assert(!IsSame_V<Decay_T<T>, std::in_place_t>,
                "T may not be in_place_t");

  using Base = result_detail::ResultBase<T, E>;

 public:
  using ValueType = T;
  using ErrorType = E;

  template <typename U, typename = EnableIf_T<IsConstructible_V<T, U&&>>>
  constexpr Result(Ok<U>&& ok) noexcept(IsNothrowConstructible_V<T, U&&>)
      : Base(result_detail::kOkTag, axio::Move(ok.value)) {}

  template <typename U, typename = EnableIf_T<IsConstructible_V<T, const U&>>>
  constexpr Result(const Ok<U>& ok) noexcept(
      IsNothrowConstructible_V<T, const U&>)
      : Base(result_detail::kOkTag, ok.value) {}

  template <typename U, typename = EnableIf_T<IsConstructible_V<E, U&&>>>
  constexpr Result(Error<U>&& err) noexcept(IsNothrowConstructible_V<E, U&&>)
      : Base(result_detail::kErrorTag, axio::Move(err.value)) {}

  template <typename U, typename = EnableIf_T<IsConstructible_V<E, const U&>>>
  constexpr Result(const Error<U>& err) noexcept(
      IsNothrowConstructible_V<E, const U&>)
      : Base(result_detail::kErrorTag, err.value) {}

  template <typename... Args,
            typename = EnableIf_T<IsConstructible_V<T, Args...>>>
  constexpr explicit Result(std::in_place_t, Args&&... args) noexcept(
      IsNothrowConstructible_V<T, Args...>)
      : Base(result_detail::kOkTag, axio::Forward<Args>(args)...) {}

  template <typename U,
            typename... Args,
            typename = EnableIf_T<
                IsConstructible_V<T, std::initializer_list<U>&, Args...>>>
  constexpr explicit Result(
      std::in_place_t,
      std::initializer_list<U> il,
      Args&&... args) noexcept(IsNothrowConstructible_V<T,
                                                        std::initializer_list<
                                                            U>&,
                                                        Args...>)
      : Base(result_detail::kOkTag, il, axio::Forward<Args>(args)...) {}

  constexpr Result(const Result&) = default;
  constexpr Result(Result&&) = default;
  constexpr Result& operator=(const Result&) = default;
  constexpr Result& operator=(Result&&) = default;
  ~Result() = default;

  template <typename U,
            typename = EnableIf_T<IsConstructible_V<T, U&&> &&
                                  IsAssignable_V<T&, U&&>>>
  constexpr Result& operator=(Ok<U>&& ok) noexcept(
      IsNothrowConstructible_V<T, U&&> && IsNothrowAssignable_V<T&, U&&>) {
    if (this->has) {
      this->value = axio::Move(ok.value);
    } else {
      this->DestroyError();
      ::new (static_cast<void*>(&this->value)) T(axio::Move(ok.value));
      this->has = true;
    }
    return *this;
  }

  template <typename U,
            typename = EnableIf_T<IsConstructible_V<E, U&&> &&
                                  IsAssignable_V<E&, U&&>>>
  constexpr Result& operator=(Error<U>&& err) noexcept(
      IsNothrowConstructible_V<E, U&&> && IsNothrowAssignable_V<E&, U&&>) {
    if (!this->has) {
      this->error = axio::Move(err.value);
    } else {
      this->DestroyValue();
      ::new (static_cast<void*>(&this->error)) E(axio::Move(err.value));
      this->has = false;
    }
    return *this;
  }

  AXIO_INLINE constexpr bool HasValue() const noexcept { return this->has; }
  AXIO_INLINE constexpr explicit operator bool() const noexcept {
    return this->has;
  }

  AXIO_INLINE constexpr T& GetValue() & noexcept { return this->value; }
  AXIO_INLINE constexpr const T& GetValue() const& noexcept {
    return this->value;
  }
  AXIO_INLINE constexpr T&& GetValue() && noexcept {
    return axio::Move(this->value);
  }
  AXIO_INLINE constexpr const T&& GetValue() const&& noexcept {
    return axio::Move(this->value);
  }

  AXIO_INLINE constexpr E& GetError() & noexcept { return this->error; }
  AXIO_INLINE constexpr const E& GetError() const& noexcept {
    return this->error;
  }
  AXIO_INLINE constexpr E&& GetError() && noexcept {
    return axio::Move(this->error);
  }
  AXIO_INLINE constexpr const E&& GetError() const&& noexcept {
    return axio::Move(this->error);
  }

  AXIO_INLINE constexpr T& operator*() & noexcept { return this->value; }
  AXIO_INLINE constexpr const T& operator*() const& noexcept {
    return this->value;
  }
  AXIO_INLINE constexpr T&& operator*() && noexcept {
    return axio::Move(this->value);
  }
  AXIO_INLINE constexpr const T&& operator*() const&& noexcept {
    return axio::Move(this->value);
  }
  AXIO_INLINE constexpr T* operator->() noexcept { return &this->value; }
  AXIO_INLINE constexpr const T* operator->() const noexcept {
    return &this->value;
  }

  template <typename U>
  AXIO_NODISCARD constexpr T ValueOr(U&& fb) const& noexcept(
      IsNothrowCopyConstructible_V<T> && IsNothrowConstructible_V<T, U&&>) {
    return this->has ? this->value : static_cast<T>(axio::Forward<U>(fb));
  }

  template <typename U>
  AXIO_NODISCARD constexpr T ValueOr(U&& fb) && noexcept(
      IsNothrowMoveConstructible_V<T> && IsNothrowConstructible_V<T, U&&>) {
    return this->has ? axio::Move(this->value)
                     : static_cast<T>(axio::Forward<U>(fb));
  }

  template <typename U>
  AXIO_NODISCARD constexpr E ErrorOr(U&& fb) const& noexcept(
      IsNothrowCopyConstructible_V<E> && IsNothrowConstructible_V<E, U&&>) {
    return this->has ? static_cast<E>(axio::Forward<U>(fb)) : this->error;
  }

  template <typename U>
  AXIO_NODISCARD constexpr E ErrorOr(U&& fb) && noexcept(
      IsNothrowMoveConstructible_V<E> && IsNothrowConstructible_V<E, U&&>) {
    return this->has ? static_cast<E>(axio::Forward<U>(fb))
                     : axio::Move(this->error);
  }

 private:
  template <typename F, typename... Args>
  using EnableIfInvocable_T = EnableIf_T<IsInvocable_V<F, Args...>, int>;

 public:
  template <typename F, EnableIfInvocable_T<F, T&> = 0>
  constexpr auto Then(F&& f) & -> InvokeResult_T<F, T&> {
    using R = InvokeResult_T<F, T&>;
    return this->has ? axio::Forward<F>(f)(this->value)
                     : R(Error<E>(this->error));
  }

  template <typename F, EnableIfInvocable_T<F, const T&> = 0>
  constexpr auto Then(F&& f) const& -> InvokeResult_T<F, const T&> {
    using R = InvokeResult_T<F, const T&>;
    return this->has ? axio::Forward<F>(f)(this->value)
                     : R(Error<E>(this->error));
  }
  template <typename F, EnableIfInvocable_T<F, T&&> = 0>
  constexpr auto Then(F&& f) && -> InvokeResult_T<F, T&&> {
    using R = InvokeResult_T<F, T&&>;
    return this->has ? axio::Forward<F>(f)(axio::Move(this->value))
                     : R(Error<E>(axio::Move(this->error)));
  }
  template <typename F, EnableIfInvocable_T<F, const T&&> = 0>
  constexpr auto Then(F&& f) const&& -> InvokeResult_T<F, const T&&> {
    using R = InvokeResult_T<F, const T&&>;
    return this->has ? axio::Forward<F>(f)(axio::Move(this->value))
                     : R(Error<E>(axio::Move(this->error)));
  }

  template <typename F, EnableIfInvocable_T<F, E&> = 0>
  constexpr auto OrElse(F&& f) & -> InvokeResult_T<F, E&> {
    using R = InvokeResult_T<F, E&>;
    return this->has ? R(Ok<T>(this->value)) : axio::Forward<F>(f)(this->error);
  }
  template <typename F, EnableIfInvocable_T<F, const E&> = 0>
  constexpr auto OrElse(F&& f) const& -> InvokeResult_T<F, const E&> {
    using R = InvokeResult_T<F, const E&>;
    return this->has ? R(Ok<T>(this->value)) : axio::Forward<F>(f)(this->error);
  }
  template <typename F, EnableIfInvocable_T<F, E&&> = 0>
  constexpr auto OrElse(F&& f) && -> InvokeResult_T<F, E&&> {
    using R = InvokeResult_T<F, E&&>;
    return this->has ? R(Ok<T>(axio::Move(this->value)))
                     : axio::Forward<F>(f)(axio::Move(this->error));
  }
  template <typename F, EnableIfInvocable_T<F, const E&&> = 0>
  constexpr auto OrElse(F&& f) const&& -> InvokeResult_T<F, const E&&> {
    using R = InvokeResult_T<F, const E&&>;
    return this->has ? R(Ok<T>(axio::Move(this->value)))
                     : axio::Forward<F>(f)(axio::Move(this->error));
  }

  template <typename F, EnableIfInvocable_T<F, T&> = 0>
  constexpr auto Map(F&& f) & -> Result<InvokeResult_T<F, T&>, E> {
    using U = InvokeResult_T<F, T&>;
    return this->has ? Result<U, E>(Ok<U>(axio::Forward<F>(f)(this->value)))
                     : Result<U, E>(Error<E>(this->error));
  }
  template <typename F, EnableIfInvocable_T<F, const T&> = 0>
  constexpr auto Map(F&& f) const& -> Result<InvokeResult_T<F, const T&>, E> {
    using U = InvokeResult_T<F, const T&>;
    return this->has ? Result<U, E>(Ok<U>(axio::Forward<F>(f)(this->value)))
                     : Result<U, E>(Error<E>(this->error));
  }
  template <typename F, EnableIfInvocable_T<F, T&&> = 0>
  constexpr auto Map(F&& f) && -> Result<InvokeResult_T<F, T&&>, E> {
    using U = InvokeResult_T<F, T&&>;
    return this->has ? Result<U, E>(
                           Ok<U>(axio::Forward<F>(f)(axio::Move(this->value))))
                     : Result<U, E>(Error<E>(axio::Move(this->error)));
  }
  template <typename F, EnableIfInvocable_T<F, const T&&> = 0>
  constexpr auto Map(F&& f) const&& -> Result<InvokeResult_T<F, const T&&>, E> {
    using U = InvokeResult_T<F, const T&&>;
    return this->has ? Result<U, E>(
                           Ok<U>(axio::Forward<F>(f)(axio::Move(this->value))))
                     : Result<U, E>(Error<E>(axio::Move(this->error)));
  }

  template <typename F, EnableIfInvocable_T<F, E&> = 0>
  constexpr auto MapError(F&& f) & -> Result<T, InvokeResult_T<F, E&>> {
    using G = InvokeResult_T<F, E&>;
    return this->has ? Result<T, G>(Ok<T>(this->value))
                     : Result<T, G>(Error<G>(axio::Forward<F>(f)(this->error)));
  }
  template <typename F, EnableIfInvocable_T<F, const E&> = 0>
  constexpr auto MapError(
      F&& f) const& -> Result<T, InvokeResult_T<F, const E&>> {
    using G = InvokeResult_T<F, const E&>;
    return this->has ? Result<T, G>(Ok<T>(this->value))
                     : Result<T, G>(Error<G>(axio::Forward<F>(f)(this->error)));
  }
  template <typename F, EnableIfInvocable_T<F, E&&> = 0>
  constexpr auto MapError(F&& f) && -> Result<T, InvokeResult_T<F, E&&>> {
    using G = InvokeResult_T<F, E&&>;
    return this->has ? Result<T, G>(Ok<T>(axio::Move(this->value)))
                     : Result<T, G>(Error<G>(
                           axio::Forward<F>(f)(axio::Move(this->error))));
  }
  template <typename F, EnableIfInvocable_T<F, const E&&> = 0>
  constexpr auto MapError(
      F&& f) const&& -> Result<T, InvokeResult_T<F, const E&&>> {
    using G = InvokeResult_T<F, const E&&>;
    return this->has ? Result<T, G>(Ok<T>(axio::Move(this->value)))
                     : Result<T, G>(Error<G>(
                           axio::Forward<F>(f)(axio::Move(this->error))));
  }

  template <typename T2, typename E2>
  friend constexpr bool
  operator==(const Result& l, const Result<T2, E2>& r) noexcept(
      noexcept(l.GetValue() == r.GetValue()) &&
      noexcept(l.GetError() == r.GetError())) {
    if (l.HasValue() != r.HasValue()) {
      return false;
    }
    return l.HasValue() ? bool(l.GetValue() == r.GetValue())
                        : bool(l.GetError() == r.GetError());
  }

  template <typename T2, typename E2>
  friend constexpr bool operator!=(
      const Result& l,
      const Result<T2, E2>& r) noexcept(noexcept(l == r)) {
    return !(l == r);
  }

  template <typename U>
  friend constexpr bool operator==(const Result& l,
                                   const Ok<U>& r) noexcept(noexcept(l.value ==
                                                                     r.value)) {
    return l.has && bool(l.value == r.value);
  }

  template <typename U>
  friend constexpr bool operator!=(const Result& l,
                                   const Ok<U>& r) noexcept(noexcept(l.value ==
                                                                     r.value)) {
    return !(l == r);
  }

  template <typename U>
  friend constexpr bool operator==(const Ok<U>& l,
                                   const Result& r) noexcept(noexcept(r == l)) {
    return r == l;
  }

  template <typename U>
  friend constexpr bool operator!=(const Ok<U>& l,
                                   const Result& r) noexcept(noexcept(r == l)) {
    return !(r == l);
  }

  template <typename U>
  friend constexpr bool operator==(const Result& l, const Error<U>& r) noexcept(
      noexcept(l.error == r.value)) {
    return !l.has && bool(l.error == r.value);
  }

  template <typename U>
  friend constexpr bool operator!=(const Result& l, const Error<U>& r) noexcept(
      noexcept(l.error == r.value)) {
    return !(l == r);
  }

  template <typename U>
  friend constexpr bool operator==(const Error<U>& l,
                                   const Result& r) noexcept(noexcept(r == l)) {
    return r == l;
  }

  template <typename U>
  friend constexpr bool operator!=(const Error<U>& l,
                                   const Result& r) noexcept(noexcept(r == l)) {
    return !(r == l);
  }
};
}  // namespace axio

#endif