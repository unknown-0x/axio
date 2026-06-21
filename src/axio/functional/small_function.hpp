/**
 * @file small_function.hpp
 * @brief A type-erased callable wrapper with small-buffer optimization.
 */

#ifndef AXIO_FUNCTIONAL_SMALL_FUNCTION_HPP_
#define AXIO_FUNCTIONAL_SMALL_FUNCTION_HPP_

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"

#include "../utility/forward.hpp"
#include "../utility/move.hpp"

#include <exception>
#include <functional>

namespace axio {
/**
 * @brief Exception thrown when an empty SmallFunction is invoked.
 *
 * @code
 *   SmallFunction<void()> f;
 *   try {
 *     f();
 *   } catch (const BadFunctionCall& e) {
 *     // e.what() == "BadFunctionCall"
 *   }
 * @endcode
 */
class BadFunctionCall : public std::exception {
  const char* message_;

 public:
  /** @brief Constructs the exception with a static message string. */
  explicit BadFunctionCall(const char* msg) : message_(msg) {}

  /** @brief Returns the message passed to the constructor. */
  const char* what() const noexcept override { return message_; }
};

template <typename, SizeT = 16, SizeT = alignof(std::max_align_t)>
class SmallFunction;

/**
 * @brief A type-erased callable wrapper with small-buffer optimization.
 *
 * Stores any callable matching `R(Args...)` inline when it fits within
 * `STORAGE_SIZE`/`STORAGE_ALIGN`; larger callables are heap-allocated
 * automatically. Supports copy, move, and member-function-pointer targets.
 * Closely mirrors `std::function`, but with configurable inline storage and
 * no allocation for callables that fit it.
 *
 * @tparam R             Return type.
 * @tparam Args          Argument types.
 * @tparam STORAGE_SIZE  Size in bytes of the inline storage buffer
 *                        (default 16).
 * @tparam STORAGE_ALIGN Alignment of the inline storage buffer
 *                        (default `alignof(std::max_align_t)`).
 *
 * ### Construction
 * @code
 *   SmallFunction<int(int)> f = [](int x) { return x * 2; };
 *   SmallFunction<int(int)> g = &SomeFreeFunction;
 *   SmallFunction<void(Foo&)> h = &Foo::Method;  // member function pointer
 * @endcode
 *
 * ### Querying
 * @code
 *   if (f) { f(21); }                 // operator bool
 *   if (f.Target<MyLambda>()) { ... } // typed target access
 *   f.TargetType();                   // typeid of the stored callable
 * @endcode
 *
 * ### Resetting
 * @code
 *   f = nullptr;  // or f.Reset();
 * @endcode
 */
template <typename R, typename... Args, SizeT STORAGE_SIZE, SizeT STORAGE_ALIGN>
class SmallFunction<R(Args...), STORAGE_SIZE, STORAGE_ALIGN> {
  /**
   * @brief Type-erasure dispatch table for the currently stored callable.
   *
   * One static `VTable` instance exists per concrete callable type (see
   * `LocalModel`/`HeapModel`); `vtable_` points to it for the lifetime of
   * the stored target, and is `nullptr` when the SmallFunction is empty.
   */
  struct VTable {
    R (*invoke)(const SmallFunction*, Args&&...);
    void (*destroy)(SmallFunction*);
    void (*copy)(SmallFunction*, const SmallFunction*);
    void (*move)(SmallFunction*, SmallFunction*);

    const std::type_info& (*target_type)() noexcept;
    void* (*target)(SmallFunction*) noexcept;
    const void* (*const_target)(const SmallFunction*) noexcept;
  };

  /**
   * @brief Returns a pointer to the inline-stored callable of type F.
   * @tparam F  Concrete callable type currently stored inline.
   */
  template <typename F>
  F* GetStack() {
    return std::launder(reinterpret_cast<F*>(storage_.stack));
  }

  /** @brief Const overload of GetStack(). */
  template <typename F>
  const F* GetStack() const {
    return std::launder(reinterpret_cast<const F*>(storage_.stack));
  }

  /**
   * @brief VTable operations for a callable F stored inline (small-buffer
   *        path).
   *
   * Selected by `SelectedModel` when F's size/alignment fit the inline
   * buffer; see `HeapModel` for the oversized fallback.
   *
   * @tparam F  Concrete callable type.
   */
  template <typename F>
  struct LocalModel {
    /** @brief Invokes the inline-stored F with @p args. */
    static R Invoke(const SmallFunction* self, Args&&... args) {
      if constexpr (IsMemberFunctionPointer_V<F>) {
        return std::invoke(
            *const_cast<SmallFunction*>(self)->template GetStack<F>(),
            axio::Forward<Args>(args)...);
      } else {
        return (*const_cast<SmallFunction*>(self)->template GetStack<F>())(
            axio::Forward<Args>(args)...);
      }
    }

    /** @brief Destroys the inline-stored F in place. */
    static void Destroy(SmallFunction* self) {
      self->template GetStack<F>()->~F();
    }

    /** @brief Copy-constructs F from @p src's inline storage into @p dst's. */
    static void Copy(SmallFunction* dst, const SmallFunction* src) {
      new (dst->template GetStack<F>()) F(*src->template GetStack<F>());
      dst->vtable_ = &kVTable;
    }

    /**
     * @brief Move-constructs F from @p src's inline storage into @p dst's,
     *        then destroys and empties @p src.
     */
    static void Move(SmallFunction* dst, SmallFunction* src) {
      new (dst->template GetStack<F>())
          F(axio::Move(*src->template GetStack<F>()));
      dst->vtable_ = &kVTable;

      src->template GetStack<F>()->~F();
      src->vtable_ = nullptr;
    }

    /** @brief Returns `typeid(F)`. */
    static const std::type_info& TargetType() noexcept { return typeid(F); }

    /** @brief Returns a pointer to the inline-stored F. */
    static void* Target(SmallFunction* self) noexcept {
      return self->template GetStack<F>();
    }

    /** @brief Const overload of Target(). */
    static const void* ConstTarget(const SmallFunction* self) noexcept {
      return self->template GetStack<F>();
    }

    /** @brief The shared VTable instance for this F, used inline. */
    static constexpr VTable kVTable{Invoke,     Destroy, Copy,       Move,
                                    TargetType, Target,  ConstTarget};
  };

  /**
   * @brief VTable operations for a callable F stored on the heap (oversized
   *        path).
   *
   * Selected by `SelectedModel` when F does not fit the inline buffer.
   * `storage_.heap` holds the `new`-allocated `F*`.
   *
   * @tparam F  Concrete callable type.
   */
  template <typename F>
  struct HeapModel {
    /** @brief Invokes the heap-stored F with @p args. */
    static R Invoke(const SmallFunction* self, Args&&... args) {
      if constexpr (IsMemberFunctionPointer_V<F>) {
        return std::invoke(
            *static_cast<F*>(const_cast<SmallFunction*>(self)->storage_.heap),
            axio::Forward<Args>(args)...);
      } else {
        return (
            *static_cast<F*>(const_cast<SmallFunction*>(self)->storage_.heap))(
            axio::Forward<Args>(args)...);
      }
    }

    /** @brief Deletes the heap-allocated F. */
    static void Destroy(SmallFunction* self) {
      delete static_cast<F*>(self->storage_.heap);
    }

    /** @brief Heap-allocates a copy of @p src's F and assigns it to @p dst. */
    static void Copy(SmallFunction* dst, const SmallFunction* src) {
      dst->storage_.heap = new F(*static_cast<const F*>(src->storage_.heap));
      dst->vtable_ = &kVTable;
    }

    /**
     * @brief Heap-allocates a move-constructed copy of @p src's F, assigns
     *        it to @p dst, then deletes @p src's original and empties it.
     */
    static void Move(SmallFunction* dst, SmallFunction* src) {
      dst->storage_.heap =
          new F(axio::Move(*static_cast<const F*>(src->storage_.heap)));
      dst->vtable_ = &kVTable;

      delete static_cast<F*>(src->storage_.heap);
      src->vtable_ = nullptr;
    }

    /** @brief Returns `typeid(F)`. */
    static const std::type_info& TargetType() noexcept { return typeid(F); }

    /** @brief Returns the heap pointer to F. */
    static void* Target(SmallFunction* self) noexcept {
      return self->storage_.heap;
    }

    /** @brief Const overload of Target(). */
    static const void* ConstTarget(const SmallFunction* self) noexcept {
      return self->storage_.heap;
    }

    /** @brief The shared VTable instance for this F, used on the heap. */
    static constexpr VTable kVTable{Invoke,     Destroy, Copy,       Move,
                                    TargetType, Target,  ConstTarget};
  };

 public:
  /** @brief The callable's return type, `R`. */
  using ReturnType = R;

  /** @brief Alignment of the inline storage buffer. */
  static constexpr auto kStorageAlign = STORAGE_ALIGN;
  /** @brief Size in bytes of the inline storage buffer. */
  static constexpr auto kStorageSize = STORAGE_SIZE;

  /**
   * @brief True if a type with the given size/alignment fits in inline
   *        storage (and its alignment evenly divides `kStorageAlign`).
   * @tparam SIZE   Candidate type's `sizeof`.
   * @tparam ALIGN  Candidate type's `alignof`.
   */
  template <SizeT SIZE, SizeT ALIGN>
  static constexpr auto kFitsStorage =
      (SIZE <= kStorageSize) && (ALIGN <= kStorageAlign) &&
      (kStorageAlign % ALIGN == 0);

  /**
   * @brief Selects `LocalModel<F>` or `HeapModel<F>` depending on whether F
   *        fits inline storage.
   * @tparam F  Concrete callable type.
   */
  template <typename F>
  using SelectedModel = Conditional_T<kFitsStorage<sizeof(F), alignof(F)>,
                                      LocalModel<F>,
                                      HeapModel<F>>;

  /** @brief Constructs an empty SmallFunction. */
  SmallFunction() noexcept : vtable_{nullptr} {}

  /** @brief Constructs an empty SmallFunction. */
  SmallFunction(NullPtrT) noexcept : vtable_{nullptr} {}

  /**
   * @brief Constructs a SmallFunction wrapping @p callable, choosing inline
   *        or heap storage based on its size/alignment.
   *
   * @tparam Callable  Any type invocable as `R(Args...)`, other than
   *                    SmallFunction itself.
   * @param callable   The callable to store; forwarded into storage.
   *
   * @code
   *   SmallFunction<int(int)> f = [](int x) { return x + 1; };
   * @endcode
   */
  template <typename Callable,
            typename DecayedCallable = Decay_T<Callable>,
            typename = EnableIf_T<!IsSame_V<DecayedCallable, SmallFunction> &&
                                  IsInvocableR_V<R, DecayedCallable, Args...>>>
  SmallFunction(Callable&& callable)
      : vtable_{&SelectedModel<DecayedCallable>::kVTable} {
    if constexpr (kFitsStorage<sizeof(DecayedCallable),
                               alignof(DecayedCallable)>) {
      new (GetStack<DecayedCallable>())
          DecayedCallable(axio::Forward<Callable>(callable));
    } else {
      storage_.heap = new DecayedCallable(axio::Forward<Callable>(callable));
    }
  }

  /** @brief Copy-constructs from @p other, deep-copying its target, if any. */
  SmallFunction(const SmallFunction& other) {
    if (other.vtable_) {
      other.vtable_->copy(this, &other);
    }
  }

  /** @brief Move-constructs from @p other, leaving it empty. */
  SmallFunction(SmallFunction&& other) {
    if (other.vtable_) {
      other.vtable_->move(this, &other);
    }
  }

  /** @brief Resets this SmallFunction to the empty state. */
  SmallFunction& operator=(NullPtrT) {
    Reset();
    return *this;
  }

  /**
   * @brief Copy-assigns from @p other, deep-copying its target, if any.
   *
   * Destroys any previously held target before copying.
   */
  SmallFunction& operator=(const SmallFunction& other) {
    if (this == &other) {
      return *this;
    }
    if (vtable_) {
      vtable_->destroy(this);
    }
    if (other.vtable_) {
      other.vtable_->copy(this, &other);
    }

    return *this;
  }

  /**
   * @brief Move-assigns from @p other, leaving it empty.
   *
   * Destroys any previously held target before moving.
   */
  SmallFunction& operator=(SmallFunction&& other) {
    if (this == &other) {
      return *this;
    }
    if (vtable_) {
      vtable_->destroy(this);
    }
    if (other.vtable_) {
      other.vtable_->move(this, &other);
    }

    return *this;
  }

  /**
   * @brief Assigns a new callable, destroying and replacing any previous
   *        target.
   *
   * @tparam Callable  Any type invocable as `R(Args...)`, other than
   *                    SmallFunction itself.
   * @param callable   The callable to store; forwarded into storage.
   *
   * @code
   *   SmallFunction<int(int)> f;
   *   f = [](int x) { return x * x; };
   * @endcode
   */
  template <typename Callable,
            typename DecayedCallable = Decay_T<Callable>,
            typename = EnableIf_T<!IsSame_V<DecayedCallable, SmallFunction> &&
                                  IsInvocableR_V<R, DecayedCallable, Args...>>>
  SmallFunction& operator=(Callable&& callable) {
    if (vtable_) {
      vtable_->destroy(this);
    }

    static constexpr Bool kUseLocal =
        kFitsStorage<sizeof(DecayedCallable), alignof(DecayedCallable)>;

    vtable_ = &SelectedModel<DecayedCallable>::kVTable;
    if constexpr (kUseLocal) {
      new (GetStack<DecayedCallable>())
          DecayedCallable(axio::Forward<Callable>(callable));
    } else {
      storage_.heap = new DecayedCallable(axio::Forward<Callable>(callable));
    }

    return *this;
  }

  /** @brief Destroys the stored target, if any. */
  ~SmallFunction() { Reset(); }

  /**
   * @brief Destroys the stored target, if any, and resets to the empty state.
   */
  void Reset() {
    if (vtable_) {
      vtable_->destroy(this);
      vtable_ = nullptr;
    }
  }

  /** @brief Returns true if this SmallFunction holds a target. */
  constexpr explicit operator bool() const noexcept {
    return vtable_ != nullptr;
  }

  /**
   * @brief Invokes the stored target with @p args.
   * @param args  Arguments forwarded to the stored callable.
   * @return      The callable's result.
   * @throws BadFunctionCall  If this SmallFunction is empty.
   */
  constexpr R operator()(Args... args) const {
    if (!vtable_) {
      throw axio::BadFunctionCall("BadFunctionCall");
    }
    return vtable_->invoke(this, axio::Forward<Args>(args)...);
  }

  /** @brief Returns the typeid of the stored target, or `typeid(void)` if
   * empty. */
  const std::type_info& TargetType() const noexcept {
    return vtable_ ? vtable_->target_type() : typeid(void);
  }

  /**
   * @brief Returns a pointer to the stored target if it is of type T, else
   *        `nullptr`.
   * @tparam T  Expected concrete callable type (decayed for comparison).
   *
   * @code
   *   if (auto* p = f.Target<MyFunctor>()) { p->DoExtra(); }
   * @endcode
   */
  template <typename T>
  T* Target() noexcept {
    using DecayedT = typename Decay<T>::type;
    if (vtable_ && vtable_->target_type() == typeid(DecayedT)) {
      return static_cast<T*>(vtable_->target(this));
    }
    return nullptr;
  }

  /** @brief Const overload of Target(). */
  template <typename T>
  const T* Target() const noexcept {
    using DecayedT = typename Decay<T>::type;
    if (vtable_ && vtable_->target_type() == typeid(DecayedT)) {
      return static_cast<const T*>(vtable_->const_target(this));
    }
    return nullptr;
  }

 private:
  /**
   * @brief Storage for the callable: an inline buffer for small callables,
   *        or a pointer to a heap-allocated callable for larger ones.
   */
  union Storage {
    alignas(kStorageAlign) unsigned char stack[kStorageSize];
    void* heap;
  };

  Storage storage_;
  const VTable* vtable_;
};

/** @brief Returns true if @p f is empty. */
template <typename R, typename... Args, SizeT STORAGE_SIZE, SizeT STORAGE_ALIGN>
AXIO_INLINE Bool
operator==(const SmallFunction<R(Args...), STORAGE_SIZE, STORAGE_ALIGN>& f,
           NullPtrT) noexcept {
  return !bool(f);
}

/** @brief Returns true if @p f holds a target. */
template <typename R, typename... Args, SizeT STORAGE_SIZE, SizeT STORAGE_ALIGN>
AXIO_INLINE Bool
operator!=(const SmallFunction<R(Args...), STORAGE_SIZE, STORAGE_ALIGN>& f,
           NullPtrT) noexcept {
  return bool(f);
}

/** @brief Returns true if @p f is empty. */
template <typename R, typename... Args, SizeT STORAGE_SIZE, SizeT STORAGE_ALIGN>
AXIO_INLINE Bool operator==(
    NullPtrT,
    const SmallFunction<R(Args...), STORAGE_SIZE, STORAGE_ALIGN>& f) noexcept {
  return !bool(f);
}

/** @brief Returns true if @p f holds a target. */
template <typename R, typename... Args, SizeT STORAGE_SIZE, SizeT STORAGE_ALIGN>
AXIO_INLINE Bool operator!=(
    NullPtrT,
    const SmallFunction<R(Args...), STORAGE_SIZE, STORAGE_ALIGN>& f) noexcept {
  return bool(f);
}
}  // namespace axio

#endif