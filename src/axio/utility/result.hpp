/**
 * @file result.hpp
 * @brief A Result<T, E> type representing either a success value or an error.
 *
 * Usage:
 * @code
 *   Result<int, std::string> r = Ok(42);
 *   if (r) { DoSomething(r.GetValue()); }
 *
 *   Result<int, std::string> e = Error(std::string("oops"));
 *   auto msg = e.ErrorOr("default");
 * @endcode
 */

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

/// @cond INTERNAL

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

/**
 * @brief Compile-time trait aggregator for Result storage/copy/move/assign
 *        noexcept and triviality properties.
 */
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

/**
 * @brief Raw union storage for T or E.
 *
 * The trivially-destructible specialization (primary) omits the destructor
 * so the compiler can keep the type trivial.  The non-trivial specialization
 * runs the active member's destructor on destruction.
 *
 * @tparam T  Value type.
 * @tparam E  Error type.
 */
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

  /** @brief No-op: T is trivially destructible in this specialization. */
  constexpr void DestroyValue() noexcept {}
  /** @brief No-op: E is trivially destructible in this specialization. */
  constexpr void DestroyError() noexcept {}
};

/** @brief Non-trivially-destructible storage specialization. */
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

  /**
   * @brief Explicitly destroys the active `value` member.
   *
   * Caller must ensure `has == true`; used by code paths (e.g. assignment)
   * that need to tear down the value before constructing a new active
   * member in its place.
   */
  constexpr void DestroyValue() noexcept { value.~T(); }

  /**
   * @brief Explicitly destroys the active `error` member.
   *
   * Caller must ensure `has == false`; used by code paths (e.g. assignment)
   * that need to tear down the error before constructing a new active
   * member in its place.
   */
  constexpr void DestroyError() noexcept { error.~E(); }

  ~StorageBase() noexcept {
    if (has) {
      value.~T();
    } else {
      error.~E();
    }
  }
};

/**
 * @brief Adds a non-trivial copy constructor when T or E is not trivially
 *        copy-constructible.  The trivial specialization inherits the
 *        defaulted copy from StorageBase.
 */
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

/** @brief Trivially copy-constructible specialization. */
template <typename T, typename E>
struct CopyBase<T, E, true> : StorageBase<T, E, Traits<T, E>::kTrivialDtor> {
  using SB = StorageBase<T, E, Traits<T, E>::kTrivialDtor>;
  using SB::SB;
};

/**
 * @brief Adds a non-trivial move constructor when T or E is not trivially
 *        move-constructible.
 */
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

/** @brief Trivially move-constructible specialization. */
template <typename T, typename E>
struct MoveBase<T, E, true> : CopyBase<T, E> {
  using CB = CopyBase<T, E>;
  using CB::CB;
};

/**
 * @brief Provides CopyAssignFrom / MoveAssignFrom helpers used by the
 *        assignment operator bases.
 *
 * Handles all four cases: same-state copy/move and cross-state transitions,
 * including the exception-safe intermediary-copy path for copy-assign.
 */
template <typename T, typename E>
struct OpsBase : MoveBase<T, E> {
  using MB = MoveBase<T, E>;
  using MB::MB;

  /**
   * @brief Copy-assigns from @p o, handling same-state and cross-state
   *        transitions.
   *
   * If both objects hold the same alternative (value/value or error/error),
   * delegates to that member's `operator=`. Otherwise destroys the
   * currently-active member and copy-constructs the other alternative in
   * its place. When the target type's copy constructor can throw, an
   * intermediary copy is built first so a thrown exception leaves `*this`
   * untouched (strong exception guarantee).
   *
   * @param o  Source to copy from. Must not be `*this`.
   */
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

  /**
   * @brief Move-assigns from @p o, handling same-state and cross-state
   *        transitions.
   *
   * If both objects hold the same alternative, delegates to that member's
   * move `operator=`. Otherwise destroys the currently-active member and
   * move-constructs the other alternative from @p o in its place. Unlike
   * CopyAssignFrom(), no intermediary is needed since move construction is
   * generally noexcept for well-behaved types.
   *
   * @param o  Source to move from. Must not be `*this`.
   */
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

/**
 * @brief Adds operator= for copy-assignment when not trivially
 *        copy-assignable.
 */
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

/** @brief Trivially copy-assignable specialization. */
template <typename T, typename E>
struct CopyAssignBase<T, E, true> : OpsBase<T, E> {
  using OB = OpsBase<T, E>;
  using OB::OB;
};

/**
 * @brief Adds operator= for move-assignment when not trivially
 *        move-assignable.
 */
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

/** @brief Trivially move-assignable specialization. */
template <typename T, typename E>
struct MoveAssignBase<T, E, true> : CopyAssignBase<T, E> {
  using CA = CopyAssignBase<T, E>;
  using CA::CA;
};

/** @brief The fully-assembled storage/copy/move/assign base for Result<T, E>.
 */
template <typename T, typename E>
using ResultBase = MoveAssignBase<T, E>;

/// @endcond
}  // namespace result_detail

/**
 * @brief Wraps a success value for construction or assignment into Result.
 *
 * @tparam T The value type.
 *
 * @code
 *   Result<int, Err> r = Ok(42);
 *   Result<Foo, Err> r = Ok(std::in_place, arg1, arg2, ...);
 * @endcode
 */
template <typename T>
struct Ok {
  T value;

  /** @brief Construct from a single forwarded value. */
  template <typename U,
            typename = EnableIf_T<IsConstructible_V<T, U&&> &&
                                  !IsSame_V<Decay_T<U>, std::in_place_t>>>
  constexpr explicit Ok(U&& v) noexcept(IsNothrowConstructible_V<T, U&&>)
      : value(axio::Forward<U>(v)) {}

  /** @brief In-place construct the value from @p args. */
  template <typename... Args,
            typename = EnableIf_T<IsConstructible_V<T, Args...>>>
  constexpr explicit Ok(std::in_place_t, Args&&... a) noexcept(
      IsNothrowConstructible_V<T, Args...>)
      : value(axio::Forward<Args>(a)...) {}
};

/**
 * @brief Wraps an error value for construction or assignment into Result.
 *
 * @tparam E  The error type.
 *
 * @code
 *   Result<int, std::string> r = Error(std::string("bad"));
 * @endcode
 */
template <typename E>
struct Error {
  E value;

  /** @brief Construct from a single forwarded value. */
  template <typename U,
            typename = EnableIf_T<IsConstructible_V<E, U&&> &&
                                  !IsSame_V<Decay_T<U>, std::in_place_t>>>
  constexpr explicit Error(U&& v) noexcept(IsNothrowConstructible_V<E, U&&>)
      : value(axio::Forward<U>(v)) {}

  /** @brief In-place construct the error from @p args. */
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

/**
 * @brief Holds either a success value of type T or an error value of type E.
 *
 * Inspired by Rust's `Result` and `std::expected` (C++23).  All
 * special-member functions are conditionally trivial — the type is trivially
 * copyable/movable whenever T and E both are.
 *
 * @tparam T  Value type.  Must not be a reference or void.
 * @tparam E  Error type.  Must not be a reference or void.
 *
 * ### Construction
 * @code
 *   Result<int, Err> ok  = Ok(42);
 *   Result<int, Err> err = Error(Err{...});
 *   Result<Foo, Err> inplace(std::in_place, ctor_args...);
 * @endcode
 *
 * ### Querying
 * @code
 *   if (r.HasValue()) { ... r.GetValue() ... }
 *   if (r)            { ... *r ... }          // same as HasValue()
 *   int v = r.ValueOr(0);
 * @endcode
 *
 * ### Chaining
 * @code
 *   auto r2 = r.Then([](int v) -> Result<Str, Err> { ... });
 *   auto r3 = r.Map([](int v) { return v * 2; });      // Result<int, Err>
 *   auto r4 = r.MapError([](Err e) { return str(e); }); // Result<int, Str>
 *   auto r5 = r.OrElse([](Err e) -> Result<int, Err> { ... });
 * @endcode
 */
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

  /** @brief Construct from a moved Ok wrapper. */
  template <typename U, typename = EnableIf_T<IsConstructible_V<T, U&&>>>
  constexpr Result(Ok<U>&& ok) noexcept(IsNothrowConstructible_V<T, U&&>)
      : Base(result_detail::kOkTag, axio::Move(ok.value)) {}

  /** @brief Construct from a const Ok wrapper. */
  template <typename U, typename = EnableIf_T<IsConstructible_V<T, const U&>>>
  constexpr Result(const Ok<U>& ok) noexcept(
      IsNothrowConstructible_V<T, const U&>)
      : Base(result_detail::kOkTag, ok.value) {}

  /** @brief Construct from a moved Error wrapper. */
  template <typename U, typename = EnableIf_T<IsConstructible_V<E, U&&>>>
  constexpr Result(Error<U>&& err) noexcept(IsNothrowConstructible_V<E, U&&>)
      : Base(result_detail::kErrorTag, axio::Move(err.value)) {}

  /** @brief Construct from a const Error wrapper. */
  template <typename U, typename = EnableIf_T<IsConstructible_V<E, const U&>>>
  constexpr Result(const Error<U>& err) noexcept(
      IsNothrowConstructible_V<E, const U&>)
      : Base(result_detail::kErrorTag, err.value) {}

  /** @brief In-place construct the success value from @p args. */
  template <typename... Args,
            typename = EnableIf_T<IsConstructible_V<T, Args...>>>
  constexpr explicit Result(std::in_place_t, Args&&... args) noexcept(
      IsNothrowConstructible_V<T, Args...>)
      : Base(result_detail::kOkTag, axio::Forward<Args>(args)...) {}

  /** @brief In-place construct with an initializer_list and extra args. */
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

  /** @brief Assign a new success value, replacing any existing state. */
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

  /** @brief Assign a new error value, replacing any existing state. */
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

  /** @brief Returns true if the result holds a value. */
  AXIO_INLINE constexpr bool HasValue() const noexcept { return this->has; }
  /** @brief Explicit bool conversion; true when HasValue(). */
  AXIO_INLINE constexpr explicit operator bool() const noexcept {
    return this->has;
  }

  /**
   * @name Value accessors
   * Undefined behaviour if HasValue() is false.
   * @{
   */
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
  /** @} */

  /**
   * @name Error accessors
   * Undefined behaviour if HasValue() is true.
   * @{
   */
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
  /** @} */

  /**
   * @name Dereference operators
   * Undefined behaviour if HasValue() is false.
   * @{
   */
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
  /** @} */

  /**
   * @brief Returns the success value, or @p fb converted to T if this is an
   * error.
   * @param fb  Fallback value.
   */
  template <typename U>
  AXIO_NODISCARD constexpr T ValueOr(U&& fb) const& noexcept(
      IsNothrowCopyConstructible_V<T> && IsNothrowConstructible_V<T, U&&>) {
    return this->has ? this->value : static_cast<T>(axio::Forward<U>(fb));
  }

  /** @overload (rvalue overload — moves the stored value). */
  template <typename U>
  AXIO_NODISCARD constexpr T ValueOr(U&& fb) && noexcept(
      IsNothrowMoveConstructible_V<T> && IsNothrowConstructible_V<T, U&&>) {
    return this->has ? axio::Move(this->value)
                     : static_cast<T>(axio::Forward<U>(fb));
  }

  /**
   * @brief Returns the error value, or @p fb converted to E if this is a
   * success.
   * @param fb  Fallback error.
   */
  template <typename U>
  AXIO_NODISCARD constexpr E ErrorOr(U&& fb) const& noexcept(
      IsNothrowCopyConstructible_V<E> && IsNothrowConstructible_V<E, U&&>) {
    return this->has ? static_cast<E>(axio::Forward<U>(fb)) : this->error;
  }

  /** @overload (rvalue overload — moves the stored error). */
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
  /**
   * @brief Flat-maps the success value.
   *
   * If this holds a value, calls @p f with it and returns the result.
   * If this holds an error, propagates the error into the return type.
   *
   * @param f  Callable: `(T) -> Result<U, E>`.
   * @return   `InvokeResult_T<F, T>` (must itself be a Result with error E).
   */
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

  /**
   * @brief Flat-maps the error value.
   *
   * If this holds an error, calls @p f with it and returns the result.
   * If this holds a value, propagates the value into the return type.
   *
   * @param f  Callable: `(E) -> Result<T, F>`.
   * @return   `InvokeResult_T<F, E>` (must itself be a Result with value T).
   */
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

  /**
   * @brief Transforms the success value, preserving the error type.
   *
   * @param f  Callable: `(T) -> U`.
   * @return   `Result<U, E>`.
   */
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

  /**
   * @brief Transforms the error value, preserving the success type.
   *
   * @param f  Callable: `(E) -> G`.
   * @return   `Result<T, G>`.
   */
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

  /**
   * @brief Two Results are equal when both hold values and those values compare
   *        equal, or both hold errors and those errors compare equal.
   */
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

  /** @brief Inverse of operator==. */
  template <typename T2, typename E2>
  friend constexpr bool operator!=(
      const Result& l,
      const Result<T2, E2>& r) noexcept(noexcept(l == r)) {
    return !(l == r);
  }

  /** @brief Equal to Ok<U> when this holds a value equal to @p r.value. */
  template <typename U>
  friend constexpr bool operator==(const Result& l,
                                   const Ok<U>& r) noexcept(noexcept(l.value ==
                                                                     r.value)) {
    return l.has && bool(l.value == r.value);
  }

  /** @brief Inverse of operator==(const Result&, const Ok<U>&). */
  template <typename U>
  friend constexpr bool operator!=(const Result& l,
                                   const Ok<U>& r) noexcept(noexcept(l.value ==
                                                                     r.value)) {
    return !(l == r);
  }

  /** @brief Symmetric overload: `Ok<U> == Result`. */
  template <typename U>
  friend constexpr bool operator==(const Ok<U>& l,
                                   const Result& r) noexcept(noexcept(r == l)) {
    return r == l;
  }

  /** @brief Symmetric overload: `Ok<U> != Result`. */
  template <typename U>
  friend constexpr bool operator!=(const Ok<U>& l,
                                   const Result& r) noexcept(noexcept(r == l)) {
    return !(r == l);
  }

  /** @brief Equal to Error<U> when this holds an error equal to @p r.value. */
  template <typename U>
  friend constexpr bool operator==(const Result& l, const Error<U>& r) noexcept(
      noexcept(l.error == r.value)) {
    return !l.has && bool(l.error == r.value);
  }

  /** @brief Inverse of operator==(const Result&, const Error<U>&). */
  template <typename U>
  friend constexpr bool operator!=(const Result& l, const Error<U>& r) noexcept(
      noexcept(l.error == r.value)) {
    return !(l == r);
  }

  /** @brief Symmetric overload: `Error<U> == Result`. */
  template <typename U>
  friend constexpr bool operator==(const Error<U>& l,
                                   const Result& r) noexcept(noexcept(r == l)) {
    return r == l;
  }

  /** @brief Symmetric overload: `Error<U> != Result`. */
  template <typename U>
  friend constexpr bool operator!=(const Error<U>& l,
                                   const Result& r) noexcept(noexcept(r == l)) {
    return !(r == l);
  }
};
}  // namespace axio

#endif