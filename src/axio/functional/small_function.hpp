#ifndef AXIO_FUNCTIONAL_SMALL_FUNCTION_HPP_
#define AXIO_FUNCTIONAL_SMALL_FUNCTION_HPP_

#include "../base/macros.hpp"
#include "../base/type_traits.hpp"

#include "../utility/forward.hpp"
#include "../utility/move.hpp"

#include <exception>
#include <functional>

namespace axio {
class BadFunctionCall : public std::exception {
  const char* message_;

 public:
  explicit BadFunctionCall(const char* msg) : message_(msg) {}

  const char* what() const noexcept override { return message_; }
};

template <typename, SizeT = 16, SizeT = alignof(std::max_align_t)>
class SmallFunction;

template <typename R, typename... Args, SizeT STORAGE_SIZE, SizeT STORAGE_ALIGN>
class SmallFunction<R(Args...), STORAGE_SIZE, STORAGE_ALIGN> {
  struct VTable {
    R (*invoke)(const SmallFunction*, Args&&...);
    void (*destroy)(SmallFunction*);
    void (*copy)(SmallFunction*, const SmallFunction*);
    void (*move)(SmallFunction*, SmallFunction*);

    const std::type_info& (*target_type)() noexcept;
    void* (*target)(SmallFunction*) noexcept;
    const void* (*const_target)(const SmallFunction*) noexcept;
  };

  template <typename F>
  F* GetStack() {
    return std::launder(reinterpret_cast<F*>(storage_.stack));
  }

  template <typename F>
  const F* GetStack() const {
    return std::launder(reinterpret_cast<const F*>(storage_.stack));
  }

  template <typename F>
  struct LocalModel {
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

    static void Destroy(SmallFunction* self) {
      self->template GetStack<F>()->~F();
    }

    static void Copy(SmallFunction* dst, const SmallFunction* src) {
      new (dst->template GetStack<F>()) F(*src->template GetStack<F>());
      dst->vtable_ = &kVTable;
    }

    static void Move(SmallFunction* dst, SmallFunction* src) {
      new (dst->template GetStack<F>())
          F(axio::Move(*src->template GetStack<F>()));
      dst->vtable_ = &kVTable;

      src->template GetStack<F>()->~F();
      src->vtable_ = nullptr;
    }

    static const std::type_info& TargetType() noexcept { return typeid(F); }

    static void* Target(SmallFunction* self) noexcept {
      return self->template GetStack<F>();
    }

    static const void* ConstTarget(const SmallFunction* self) noexcept {
      return self->template GetStack<F>();
    }

    static constexpr VTable kVTable{Invoke,     Destroy, Copy,       Move,
                                    TargetType, Target,  ConstTarget};
  };

  template <typename F>
  struct HeapModel {
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

    static void Destroy(SmallFunction* self) {
      delete static_cast<F*>(self->storage_.heap);
    }

    static void Copy(SmallFunction* dst, const SmallFunction* src) {
      dst->storage_.heap = new F(*static_cast<const F*>(src->storage_.heap));
      dst->vtable_ = &kVTable;
    }

    static void Move(SmallFunction* dst, SmallFunction* src) {
      dst->storage_.heap =
          new F(axio::Move(*static_cast<const F*>(src->storage_.heap)));
      dst->vtable_ = &kVTable;

      delete static_cast<F*>(src->storage_.heap);
      src->vtable_ = nullptr;
    }

    static const std::type_info& TargetType() noexcept { return typeid(F); }

    static void* Target(SmallFunction* self) noexcept {
      return self->storage_.heap;
    }

    static const void* ConstTarget(const SmallFunction* self) noexcept {
      return self->storage_.heap;
    }

    static constexpr VTable kVTable{Invoke,     Destroy, Copy,       Move,
                                    TargetType, Target,  ConstTarget};
  };

 public:
  using ReturnType = R;

  static constexpr auto kStorageAlign = STORAGE_ALIGN;
  static constexpr auto kStorageSize = STORAGE_SIZE;

  template <SizeT SIZE, SizeT ALIGN>
  static constexpr auto kFitsStorage =
      (SIZE <= kStorageSize) && (ALIGN <= kStorageAlign) &&
      (kStorageAlign % ALIGN == 0);

  template <typename F>
  using SelectedModel = Conditional_T<kFitsStorage<sizeof(F), alignof(F)>,
                                      LocalModel<F>,
                                      HeapModel<F>>;

  SmallFunction() noexcept : vtable_{nullptr} {}

  SmallFunction(NullPtrT) noexcept : vtable_{nullptr} {}

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

  SmallFunction(const SmallFunction& other) {
    if (other.vtable_) {
      other.vtable_->copy(this, &other);
    }
  }

  SmallFunction(SmallFunction&& other) {
    if (other.vtable_) {
      other.vtable_->move(this, &other);
    }
  }

  SmallFunction& operator=(NullPtrT) {
    Reset();
    return *this;
  }

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

  ~SmallFunction() { Reset(); }

  void Reset() {
    if (vtable_) {
      vtable_->destroy(this);
      vtable_ = nullptr;
    }
  }

  constexpr explicit operator bool() const noexcept {
    return vtable_ != nullptr;
  }

  constexpr R operator()(Args... args) const {
    if (!vtable_) {
      throw axio::BadFunctionCall("BadFunctionCall");
    }
    return vtable_->invoke(this, axio::Forward<Args>(args)...);
  }

  const std::type_info& TargetType() const noexcept {
    return vtable_ ? vtable_->target_type() : typeid(void);
  }

  template <typename T>
  T* Target() noexcept {
    using DecayedT = typename Decay<T>::type;
    if (vtable_ && vtable_->target_type() == typeid(DecayedT)) {
      return static_cast<T*>(vtable_->target(this));
    }
    return nullptr;
  }

  template <typename T>
  const T* Target() const noexcept {
    using DecayedT = typename Decay<T>::type;
    if (vtable_ && vtable_->target_type() == typeid(DecayedT)) {
      return static_cast<const T*>(vtable_->const_target(this));
    }
    return nullptr;
  }

 private:
  union Storage {
    alignas(kStorageAlign) unsigned char stack[kStorageSize];
    void* heap;
  };

  Storage storage_;
  const VTable* vtable_;
};

template <typename R, typename... Args, SizeT STORAGE_SIZE, SizeT STORAGE_ALIGN>
AXIO_INLINE Bool
operator==(const SmallFunction<R(Args...), STORAGE_SIZE, STORAGE_ALIGN>& f,
           NullPtrT) noexcept {
  return !bool(f);
}

template <typename R, typename... Args, SizeT STORAGE_SIZE, SizeT STORAGE_ALIGN>
AXIO_INLINE Bool
operator!=(const SmallFunction<R(Args...), STORAGE_SIZE, STORAGE_ALIGN>& f,
           NullPtrT) noexcept {
  return bool(f);
}

template <typename R, typename... Args, SizeT STORAGE_SIZE, SizeT STORAGE_ALIGN>
AXIO_INLINE Bool operator==(
    NullPtrT,
    const SmallFunction<R(Args...), STORAGE_SIZE, STORAGE_ALIGN>& f) noexcept {
  return !bool(f);
}

template <typename R, typename... Args, SizeT STORAGE_SIZE, SizeT STORAGE_ALIGN>
AXIO_INLINE Bool operator!=(
    NullPtrT,
    const SmallFunction<R(Args...), STORAGE_SIZE, STORAGE_ALIGN>& f) noexcept {
  return bool(f);
}
}  // namespace axio

#endif