/**
 * @file tuple.hpp
 * @brief Tuple<Ts...> — a fixed-size heterogeneous container, analogous to
 *        std::tuple, with empty-base optimization and structured-binding
 *        support.
 *
 * Two element-access spellings are provided side by side:
 *  - `axio::Get<I>(t)`   — the axio-style accessor, used throughout this
 *                          library and in generic axio code.
 *  - `axio::get<I>(t)`   — a lowercase alias that exists solely so
 *                          `Tuple` satisfies the structured-bindings
 *                          protocol (ADL-found `get`, plus the
 *                          `std::tuple_size`/`std::tuple_element`
 *                          specializations at the bottom of this file).
 *                          Prefer `Get` in your own code; `get` is for the
 *                          compiler.
 *
 */

#ifndef AXIO_CONTAINER_TUPLE_HPP_
#define AXIO_CONTAINER_TUPLE_HPP_

#include "detail/tuple_fwd.hpp"

#include "../string/axio_repr.hpp"
#include "../utility/forward.hpp"
#include "../utility/move.hpp"

namespace axio {
namespace tuple_detail {
/**
 * @brief Storage for a single tuple element at index I, using empty-base
 *        optimization (EBO) when T is empty and not final.
 *
 * Each `Tuple` element gets its own `TupleValue<I, T>` base subobject;
 * indexing by both `I` and `T` lets the same empty type appear at multiple
 * positions in a `Tuple` without becoming ambiguous as a base class.
 *
 * @tparam I  Index of this element within the owning Tuple.
 * @tparam T  The element's value type.
 */
template <SizeT I, typename T, Bool = ShouldUseEBO_V<T>>
struct AXIO_EMPTY_BASES TupleValue : public T {
  using Type = T;
  static constexpr Bool kUseEBO = true;

  /** @brief Default-constructs the element. */
  constexpr TupleValue() noexcept(IsNothrowDefaultConstructible_V<T>) : T() {}

  /** @brief Constructs the element by forwarding @p arg into T's constructor.
   */
  template <typename U,
            typename = EnableIf_T<!IsSame_V<TupleValue, Decay_T<U>>>>
  constexpr TupleValue(U&& arg) noexcept(IsNothrowConstructible_V<T, U&&>)
      : T(Forward<U>(arg)) {}
};

/**
 * @brief TupleValue specialization that stores T as a member when EBO is
 *        not applicable (T is non-empty, final, or otherwise unsuitable as
 *        a base class).
 *
 * @tparam I  Index of this element within the owning Tuple.
 * @tparam T  The element's value type.
 */
template <SizeT I, typename T>
struct TupleValue<I, T, false> {
  using Type = T;
  static constexpr Bool kUseEBO = false;

  /** @brief Default-constructs the element. */
  constexpr TupleValue() noexcept(IsNothrowDefaultConstructible_V<T>)
      : value() {}

  /** @brief Constructs the element by forwarding @p arg into T's constructor.
   */
  template <typename U,
            typename = EnableIf_T<!IsSame_V<TupleValue, Decay_T<U>>>>
  constexpr TupleValue(U&& arg) noexcept(IsNothrowConstructible_V<T, U&&>)
      : value(Forward<U>(arg)) {}

  T value;
};

template <typename, typename...>
struct AXIO_EMPTY_BASES TupleImpl;

/**
 * @brief Base implementation of Tuple: inherits from one TupleValue per
 *        element, giving each element its own indexed base subobject.
 *
 * @tparam Is  Index pack, one per element in Ts (always
 *             `std::index_sequence<0, 1, ..., sizeof...(Ts) - 1>`).
 * @tparam Ts  Element types.
 */
template <SizeT... Is, typename... Ts>
struct AXIO_EMPTY_BASES
    TupleImpl<std::index_sequence<Is...>, Ts...> : TupleValue<Is, Ts>... {
  constexpr TupleImpl() = default;

  /** @brief Constructs each element by copying the corresponding argument. */
  constexpr explicit TupleImpl(const Ts&... args)
      : TupleValue<Is, Ts>(args)... {}

  /**
   * @brief Constructs each element by forwarding the corresponding
   *        argument.
   * @tparam Us  Argument types, one per element.
   */
  template <typename... Us>
  constexpr explicit TupleImpl(Us&&... args)
      : TupleValue<Is, Ts>(Forward<Us>(args))... {}

  /**
   * @brief Converting copy constructor from a TupleImpl of compatible
   *        types.
   * @tparam Us  Source element types; each `Us` must be convertible to the
   *             corresponding `Ts`.
   * @param other  Source TupleImpl to copy from.
   */
  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsConstructible<Ts, const Us&>...>,
                       Bool> = true>
  constexpr TupleImpl(
      const TupleImpl<std::index_sequence<Is...>, Us...>&
          other) noexcept(Conjunction_V<IsNothrowConstructible<Ts,
                                                               const Us&>...>)
      : TupleValue<Is, Ts>(
            GetValue(static_cast<const TupleValue<Is, Us>&>(other)))... {}

  /**
   * @brief Converting move constructor from a TupleImpl of compatible
   *        types.
   * @tparam Us  Source element types; each `Us` must be convertible to the
   *             corresponding `Ts`.
   * @param other  Source TupleImpl to move from.
   */
  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsConstructible<Ts, Us&&>...>,
                       Bool> = true>
  constexpr TupleImpl(
      TupleImpl<std::index_sequence<Is...>, Us...>&&
          other) noexcept(Conjunction_V<IsNothrowConstructible<Ts, Us&&>...>)
      : TupleValue<Is, Ts>(
            GetValue(static_cast<TupleValue<Is, Us>&&>(other)))... {}

  /**
   * @brief Converting copy assignment from a TupleImpl of compatible types.
   * @tparam Us  Source element types; each `Us` must be assignable to the
   *             corresponding `Ts`.
   * @param other  Source TupleImpl to copy from.
   * @return       `*this`.
   */
  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsAssignable<Ts&, const Us&>...>,
                       Bool> = true>
  constexpr TupleImpl&
  operator=(const TupleImpl<std::index_sequence<Is...>, Us...>& other) noexcept(
      Conjunction_V<IsNothrowAssignable<Ts&, const Us&>...>) {
    ((GetValue(static_cast<TupleValue<Is, Ts>&>(*this)) =
          GetValue(static_cast<const TupleValue<Is, Us>&>(other))),
     ...);

    return *this;
  }

  /**
   * @brief Converting move assignment from a TupleImpl of compatible types.
   * @tparam Us  Source element types; each `Us` must be assignable to the
   *             corresponding `Ts`.
   * @param other  Source TupleImpl to move from.
   * @return       `*this`.
   */
  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsAssignable<Ts&, Us&&>...>,
                       Bool> = true>
  constexpr TupleImpl&
  operator=(TupleImpl<std::index_sequence<Is...>, Us...>&& other) noexcept(
      Conjunction_V<IsNothrowAssignable<Ts&, Us&&>...>) {
    ((GetValue(static_cast<TupleValue<Is, Ts>&>(*this)) =
          GetValue(static_cast<TupleValue<Is, Us>&&>(other))),
     ...);

    return *this;
  }

 private:
  /**
   * @brief Accesses the stored value of an lvalue TupleValue base,
   *        accounting for whether EBO is in use.
   * @tparam I  Element index.
   * @tparam U  Element type.
   */
  template <SizeT I, typename U>
  constexpr U& GetValue(TupleValue<I, U>& element) {
    if constexpr (TupleValue<I, U>::kUseEBO) {
      return static_cast<U&>(element);
    } else {
      return element.value;
    }
  }

  /** @brief Const overload of GetValue(). */
  template <SizeT I, typename U>
  constexpr const U& GetValue(const TupleValue<I, U>& element) {
    if constexpr (TupleValue<I, U>::kUseEBO) {
      return static_cast<const U&>(element);
    } else {
      return element.value;
    }
  }

  /** @brief Rvalue overload of GetValue(). */
  template <SizeT I, typename U>
  constexpr decltype(auto) GetValue(TupleValue<I, U>&& element) {
    if constexpr (TupleValue<I, U>::kUseEBO) {
      return static_cast<U&&>(element);
    } else {
      return static_cast<decltype(element.value)&&>(element.value);
    }
  }

  /** @brief Const rvalue overload of GetValue(). */
  template <SizeT I, typename U>
  constexpr decltype(auto) GetValue(const TupleValue<I, U>&& element) {
    if constexpr (TupleValue<I, U>::kUseEBO) {
      return static_cast<const U&&>(element);
    } else {
      return static_cast<const decltype(element.value)&&>(element.value);
    }
  }
};

/**
 * @brief Yields the first type in a parameter pack.
 * @tparam T  First type (the one selected).
 */
template <typename T, typename...>
struct FirstType {
  using type = T;
};

/** @brief Alias for `FirstType<Ts...>::type`. */
template <typename... Ts>
using First = typename FirstType<Ts...>::type;

/**
 * @brief Yields the I-th type in a parameter pack.
 * @tparam I   Zero-based index to look up.
 * @tparam Ts  Parameter pack to index into.
 */
template <SizeT I, typename... Ts>
struct TypeAt;

/** @brief Base case: index 0 selects the first type in the pack. */
template <typename T, typename... Ts>
struct TypeAt<0, T, Ts...> {
  using type = T;
};

/** @brief Recursive case: strips one type and decrements I. */
template <SizeT I, typename T, typename... Ts>
struct TypeAt<I, T, Ts...> : TypeAt<I - 1, Ts...> {};

/**
 * @brief No-op trait used to allow SFINAE conditions independent of a
 *        dummy template parameter.
 *
 * Lets a member-function template (e.g. Tuple's default constructor) carry
 * a `Dummy` parameter purely so its `EnableIf_T<...>` is re-evaluated per
 * instantiation rather than at class-template definition time.
 * @tparam T  Unused; any type.
 */
template <typename>
struct Ignore : TrueType {};

/**
 * @brief Decays T and unwraps `std::reference_wrapper`, mirroring the
 *        element type deduction rules of `std::make_tuple`.
 *
 * @tparam T  Argument type as deduced from a forwarding reference.
 */
template <typename T>
using UnwrapDecayT = typename UnwrapReferenceWrapper<Decay_T<T>>::type;
}  // namespace tuple_detail

/**
 * @brief A fixed-size heterogeneous container, analogous to std::tuple.
 * @tparam Ts Element types.
 *
 * @code
 *   axio::Tuple<int, double, const char*> t(1, 2.0, "three");
 *
 *   int x        = axio::Get<0>(t);
 *   auto [a, b, c] = t;
 * @endcode
 */
template <typename... Ts>
struct AXIO_EMPTY_BASES Tuple
    : public tuple_detail::TupleImpl<std::make_index_sequence<sizeof...(Ts)>,
                                     Ts...> {
  /** @brief TupleValue type for element I. */
  template <SizeT I>
  using Element =
      tuple_detail::TupleValue<I,
                               typename tuple_detail::TypeAt<I, Ts...>::type>;

  using Base =
      tuple_detail::TupleImpl<std::make_index_sequence<sizeof...(Ts)>, Ts...>;

  /**
   * @brief Default-constructs every element.
   *
   * Participates in overload resolution only if every `Ts` is default
   * constructible.
   */
  template <typename Dummy = void,
            EnableIf_T<Conjunction_V<tuple_detail::Ignore<Dummy>,
                                     IsDefaultConstructible<Ts>...>,
                       Bool> = true>
  constexpr Tuple() noexcept(
      Conjunction_V<IsNothrowDefaultConstructible<Ts>...>)
      : Base() {}

  /**
   * @brief Constructs each element by copying the corresponding argument.
   * @param args  One value per element, copied in.
   *
   * @code
   *   int x = 1;
   *   Tuple<int, int> t(x, 2);  // both elements copy-constructed
   * @endcode
   */
  template <typename Dummy = void,
            EnableIf_T<Conjunction_V<tuple_detail::Ignore<Dummy>,
                                     IsCopyConstructible<Ts>...>,
                       Bool> = true>
  constexpr explicit Tuple(const Ts&... args) noexcept(
      Conjunction_V<IsNothrowCopyConstructible<Ts>...>)
      : Base(args...) {}

  /**
   * @brief Constructs each element by forwarding the corresponding
   *        argument.
   * @tparam Us  Argument types, one per element; each must be constructible
   *             into the corresponding `Ts`.
   * @param args  One value per element, forwarded in.
   */
  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsConstructible<Ts, Us&&>...>,
                       Bool> = true>
  constexpr explicit Tuple(Us&&... args) noexcept(
      Conjunction_V<IsNothrowConstructible<Ts, Us&&>...>)
      : Base(Forward<Us>(args)...) {}

  constexpr Tuple(const Tuple&) = default;
  constexpr Tuple& operator=(const Tuple&) = default;
  constexpr Tuple(Tuple&&) noexcept(
      Conjunction_V<IsNothrowMoveConstructible<Ts>...>) = default;
  constexpr Tuple& operator=(Tuple&&) noexcept(
      Conjunction_V<IsNothrowMoveAssignable<Ts>...>) = default;

  /**
   * @brief Converting copy constructor from a Tuple of compatible types.
   * @tparam Us  Source element types; each `Us` must be convertible to the
   *             corresponding `Ts`.
   * @param other  Source Tuple to copy from.
   *
   * @code
   *   Tuple<int, double> a(1, 2.0);
   *   Tuple<long, double> b(a);  // int -> long widening
   * @endcode
   */
  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsConstructible<Ts, const Us&>...>,
                       Bool> = true>
  constexpr explicit Tuple(const Tuple<Us...>& other) noexcept(
      Conjunction_V<IsNothrowConstructible<Ts, const Us&>...>)
      : Base(static_cast<const typename Tuple<Us...>::Base&>(other)) {}

  /**
   * @brief Converting move constructor from a Tuple of compatible types.
   * @tparam Us  Source element types; each `Us` must be convertible to the
   *             corresponding `Ts`.
   * @param other  Source Tuple to move from.
   */
  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsConstructible<Ts, Us&&>...>,
                       Bool> = true>
  constexpr explicit Tuple(Tuple<Us...>&& other) noexcept(
      Conjunction_V<IsNothrowConstructible<Ts, Us&&>...>)
      : Base(static_cast<typename Tuple<Us...>::Base&&>(other)) {}

  /**
   * @brief Converting copy assignment from a Tuple of compatible types.
   * @tparam Us  Source element types; each `Us` must be assignable to the
   *             corresponding `Ts`.
   * @param other  Source Tuple to copy from.
   * @return       `*this`.
   */
  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsAssignable<Ts&, const Us&>...>,
                       Bool> = true>
  constexpr Tuple& operator=(const Tuple<Us...>& other) noexcept(
      Conjunction_V<IsNothrowAssignable<Ts&, const Us&>...>) {
    Base::operator=(static_cast<const typename Tuple<Us...>::Base&>(other));
    return *this;
  }

  /**
   * @brief Converting move assignment from a Tuple of compatible types.
   * @tparam Us  Source element types; each `Us` must be assignable to the
   *             corresponding `Ts`.
   * @param other  Source Tuple to move from.
   * @return       `*this`.
   */
  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsAssignable<Ts&, Us&&>...>,
                       Bool> = true>
  constexpr Tuple& operator=(Tuple<Us...>&& other) noexcept(
      Conjunction_V<IsNothrowAssignable<Ts&, Us&&>...>) {
    Base::operator=(static_cast<typename Tuple<Us...>::Base&&>(other));
    return *this;
  }
};

/** @brief Specialization for the empty tuple. */
template <>
struct Tuple<> {};

/** @brief Yields the number of elements in a Tuple. */
template <typename... Ts>
struct TupleSize<Tuple<Ts...>> : IntegralConstant<SizeT, sizeof...(Ts)> {};

/**
 * @brief Yields the type of the I-th element of a Tuple.
 * @tparam I   Zero-based element index.
 * @tparam Ts  Tuple element types.
 */
template <SizeT I, typename... Ts>
struct TupleElement<I, Tuple<Ts...>> {
  using type = typename tuple_detail::TypeAt<I, Ts...>::type;
};

/**
 * @brief Returns a mutable reference to the I-th element of @p t.
 *
 * The axio-style accessor — prefer this over the lowercase get(), which
 * exists only to satisfy structured bindings.
 *
 * @tparam I   Zero-based element index.
 * @tparam Ts  Tuple element types.
 * @param t    Tuple to access.
 */
template <SizeT I, typename... Ts>
constexpr decltype(auto) Get(Tuple<Ts...>& t) noexcept {
  using Element = typename Tuple<Ts...>::template Element<I>;
  if constexpr (Element::kUseEBO) {
    return static_cast<typename Element::Type&>(static_cast<Element&>(t));
  } else {
    return static_cast<typename Element::Type&>(static_cast<Element&>(t).value);
  }
}

/** @brief Returns a const reference to the I-th element of @p t. See Get(). */
template <SizeT I, typename... Ts>
constexpr decltype(auto) Get(const Tuple<Ts...>& t) noexcept {
  using Element = typename Tuple<Ts...>::template Element<I>;
  if constexpr (Element::kUseEBO) {
    return static_cast<const typename Element::Type&>(
        static_cast<const Element&>(t));
  } else {
    return static_cast<const typename Element::Type&>(
        static_cast<const Element&>(t).value);
  }
}

/** @brief Returns an rvalue reference to the I-th element of @p t. See Get().
 */
template <SizeT I, typename... Ts>
constexpr decltype(auto) Get(Tuple<Ts...>&& t) noexcept {
  using Element = typename Tuple<Ts...>::template Element<I>;
  if constexpr (Element::kUseEBO) {
    return static_cast<typename Element::Type&&>(static_cast<Element&&>(t));
  } else {
    return static_cast<typename Element::Type&&>(
        static_cast<Element&&>(t).value);
  }
}

/**
 * @brief Returns a const rvalue reference to the I-th element of @p t.
 *        See Get().
 */
template <SizeT I, typename... Ts>
constexpr decltype(auto) Get(const Tuple<Ts...>&& t) noexcept {
  using Element = typename Tuple<Ts...>::template Element<I>;
  if constexpr (Element::kUseEBO) {
    return static_cast<const typename Element::Type&&>(
        static_cast<const Element&&>(t));
  } else {
    return static_cast<const typename Element::Type&&>(
        static_cast<const Element&&>(t).value);
  }
}

/**
 * @brief Constructs a Tuple from @p args, decaying types and unwrapping
 *        `std::reference_wrapper`, mirroring `std::make_tuple`.
 *
 * @tparam Ts  Argument types as deduced from forwarding references.
 * @param args Values to store. `std::ref`/`std::cref` wrappers become
 *             references in the resulting Tuple.
 *
 * @code
 *   int x = 5;
 *   auto t = MakeTuple(1, "two", std::ref(x));
 * @endcode
 */
template <typename... Ts>
constexpr Tuple<tuple_detail::UnwrapDecayT<Ts>...> MakeTuple(Ts&&... args) {
  return Tuple<tuple_detail::UnwrapDecayT<Ts>...>(Forward<Ts>(args)...);
}

/**
 * @brief Constructs a Tuple of references that forward to @p args,
 *        mirroring `std::forward_as_tuple`.
 *
 * Unlike MakeTuple(), no decay happens — the resulting Tuple's element
 * types are exactly `Ts&&`. Primarily useful for passing arguments through
 * to another function without copying, e.g. inside TupleCat().
 *
 * @tparam Ts  Argument types as deduced from forwarding references.
 * @param args Values to reference.
 */
template <typename... Ts>
constexpr Tuple<Ts&&...> ForwardAsTuple(Ts&&... args) {
  return Tuple<Ts&&...>(Forward<Ts>(args)...);
}

namespace tuple_detail {
/**
 * @brief Constructs a T by unpacking elements of @p t as constructor
 *        arguments.
 * @tparam T      Type to construct.
 * @tparam TTuple Tuple-like type providing `Get<I>`.
 * @tparam I      Index pack covering every element of @p t.
 */
template <typename T, typename TTuple, SizeT... I>
constexpr T MakeFromTupleImpl(TTuple&& t, std::index_sequence<I...>) {
  return T(Get<I>(Forward<TTuple>(t))...);
}

/**
 * @brief Concatenates two `std::index_sequence` types.
 * @tparam A  First sequence.
 * @tparam B  Second sequence, appended after A.
 */
template <typename A, typename B>
struct Concat;

/** @brief Concatenates index packs A and B into a single sequence. */
template <SizeT... A, SizeT... B>
struct Concat<std::index_sequence<A...>, std::index_sequence<B...>> {
  using type = std::index_sequence<A..., B...>;
};

/**
 * @brief Builds an `index_sequence` of N copies of the value I.
 * @tparam I    Value to repeat.
 * @tparam Seq  Helper sequence whose length (N) determines the repeat count.
 */
template <SizeT I, typename Seq>
struct RepeatIndexHelper;

/** @brief Expands Seq's length into N copies of I. */
template <SizeT I, SizeT... Is>
struct RepeatIndexHelper<I, std::index_sequence<Is...>> {
  using type = std::index_sequence<(static_cast<void>(Is), I)...>;
};

/**
 * @brief Alias yielding an `index_sequence` of N copies of I.
 * @tparam I  Value to repeat.
 * @tparam N  Number of repetitions.
 */
template <SizeT I, SizeT N>
using RepeatIndex =
    typename RepeatIndexHelper<I, std::make_index_sequence<N>>::type;

/**
 * @brief For TupleCat: builds the "outer" index sequence mapping each
 *        flattened output position to its source tuple index.
 *
 * Given source tuple sizes `Sizes = {2, 1, 3}`, produces
 * `{0, 0, 1, 2, 2, 2}` — each source tuple's index repeated once per its
 * own element count.
 *
 * @tparam Sizes  Element counts of each source tuple, in order.
 * @tparam Index  Current source tuple index (used during recursion).
 */
template <typename Sizes, SizeT Index = 0>
struct BuildOuter;

/** @brief Base case: no more source tuples to process. */
template <SizeT Index>
struct BuildOuter<std::index_sequence<>, Index> {
  using type = std::index_sequence<>;
};

/** @brief Recursive case: repeats IDX, S times, then recurses on the rest. */
template <SizeT S, SizeT... REST, SizeT IDX>
struct BuildOuter<std::index_sequence<S, REST...>, IDX> {
  using type = typename Concat<
      RepeatIndex<IDX, S>,
      typename BuildOuter<std::index_sequence<REST...>, IDX + 1>::type>::type;
};

/**
 * @brief For TupleCat: builds the "inner" index sequence mapping each
 *        flattened output position to its index within its source tuple.
 *
 * Given source tuple sizes `Sizes = {2, 1, 3}`, produces
 * `{0, 1, 0, 0, 1, 2}` — each source tuple's own 0-based indices in turn.
 *
 * @tparam Sizes  Element counts of each source tuple, in order.
 */
template <typename Sizes>
struct BuildInner;

/** @brief Base case: no more source tuples to process. */
template <>
struct BuildInner<std::index_sequence<>> {
  using type = std::index_sequence<>;
};

/** @brief Recursive case: emits 0..S-1, then recurses on the rest. */
template <SizeT S, SizeT... REST>
struct BuildInner<std::index_sequence<S, REST...>> {
  using type = typename Concat<
      std::make_index_sequence<S>,
      typename BuildInner<std::index_sequence<REST...>>::type>::type;
};

/**
 * @brief Computes the resulting Tuple type for TupleCat given the
 *        outer/inner index sequences and the tuple-of-tuples being
 *        concatenated.
 *
 * @tparam Outer        Per-output-position source-tuple index (see
 *                       BuildOuter).
 * @tparam Inner         Per-output-position in-source-tuple index (see
 *                       BuildInner).
 * @tparam TupleOfTuples A Tuple whose elements are themselves Tuples.
 */
template <typename Outer, typename Inner, typename TupleOfTuples>
struct TupleCatResult;

/** @brief Builds the flattened element-type list via Outer/Inner indices. */
template <SizeT... Os, SizeT... Is, typename T>
struct TupleCatResult<std::index_sequence<Os...>,
                      std::index_sequence<Is...>,
                      T> {
  using type = axio::Tuple<
      typename TupleElement<Is, typename TupleElement<Os, T>::type>::type...>;
};

/**
 * @brief Builds the result of TupleCat by gathering elements from each
 *        source tuple according to the outer/inner index sequences.
 *
 * @tparam R             Resulting Tuple type (see TupleCatResult).
 * @tparam Os            Source-tuple index per output position.
 * @tparam Is            In-source-tuple index per output position.
 * @tparam TupleOfTuples  A Tuple whose elements are themselves Tuples.
 * @param t  Tuple-of-tuples to gather elements from.
 */
template <typename R, SizeT... Os, SizeT... Is, typename TupleOfTuples>
constexpr R TupleCatImpl(std::index_sequence<Os...>,
                         std::index_sequence<Is...>,
                         TupleOfTuples&& t) {
  return R(Get<Is>(Get<Os>(Forward<TupleOfTuples>(t)))...);
}
}  // namespace tuple_detail

/**
 * @brief Constructs a T by unpacking the elements of @p t as constructor
 *        arguments, mirroring `std::make_from_tuple`.
 *
 * @tparam T      Type to construct.
 * @tparam TTuple Tuple-like type providing `Get<I>` and `TupleSize`.
 * @param t  Source of constructor arguments.
 *
 * @code
 *   struct Point { Point(int x, int y); };
 *   Tuple<int, int> args(3, 4);
 *   Point p = MakeFromTuple<Point>(args);  // Point(3, 4)
 * @endcode
 */
template <typename T, typename TTuple>
constexpr T MakeFromTuple(TTuple&& t) {
  return tuple_detail::MakeFromTupleImpl<T>(
      Forward<TTuple>(t),
      std::make_index_sequence<TupleSize<RemoveReference_T<TTuple>>::value>{});
}

/**
 * @brief Concatenates any number of Tuples into a single flattened Tuple,
 *        mirroring `std::tuple_cat`.
 *
 * @tparam Tuples  Tuple types to concatenate, in order.
 * @param tuples   Tuples whose elements are flattened into the result.
 *
 * @code
 *   Tuple<int, double> a(1, 2.0);
 *   Tuple<char> b('x');
 *   auto c = TupleCat(a, b);  // Tuple<int, double, char>(1, 2.0, 'x')
 * @endcode
 */
template <
    typename... Tuples,
    typename Sizes = std::index_sequence<TupleSize<Decay_T<Tuples>>::value...>,
    typename Outer = typename tuple_detail::BuildOuter<Sizes>::type,
    typename Inner = typename tuple_detail::BuildInner<Sizes>::type,
    typename R = typename tuple_detail::
        TupleCatResult<Outer, Inner, Tuple<Decay_T<Tuples>...>>::type>
constexpr auto TupleCat(Tuples&&... tuples) -> R {
  return tuple_detail::TupleCatImpl<R>(
      Outer{}, Inner{}, ForwardAsTuple(Forward<Tuples>(tuples)...));
}

/**
 * @brief Lowercase, ADL-found element accessor required for structured
 *        bindings (`auto [a, b] = t;`).
 *
 * Functionally identical to Get(); use Get() in ordinary code and let the
 * compiler call get() implicitly for structured bindings.
 *
 * @tparam I   Zero-based element index.
 * @tparam Ts  Tuple element types.
 * @param t    Tuple to access.
 */
template <size_t I, typename... Ts>
constexpr decltype(auto) get(Tuple<Ts...>& t) noexcept {
  return Get<I>(t);
}

/** @brief Const overload of get(). See get(). */
template <size_t I, typename... Ts>
constexpr decltype(auto) get(const Tuple<Ts...>& t) noexcept {
  return Get<I>(t);
}

/** @brief Rvalue overload of get(). See get(). */
template <size_t I, typename... Ts>
constexpr decltype(auto) get(Tuple<Ts...>&& t) noexcept {
  return Get<I>(Move(t));
}

/** @brief Const rvalue overload of get(). See get(). */
template <size_t I, typename... Ts>
constexpr decltype(auto) get(const Tuple<Ts...>&& t) noexcept {
  return Get<I>(Move(t));
}

namespace tuple_detail {
/**
 * @brief Implements lexicographic equality comparison across tuple
 *        elements.
 * @tparam Lhs  Left-hand Tuple type.
 * @tparam Rhs  Right-hand Tuple type.
 * @tparam Is   Index pack covering every element.
 * @return      True if every corresponding element pair compares equal.
 */
template <typename Lhs, typename Rhs, SizeT... Is>
constexpr Bool TupleEqualImpl(
    const Lhs& lhs,
    const Rhs& rhs,
    std::index_sequence<
        Is...>) noexcept((noexcept(Get<Is>(std::declval<const Lhs&>()) ==
                                   Get<Is>(std::declval<const Rhs&>())) &&
                          ...)) {
  return ((Get<Is>(lhs) == Get<Is>(rhs)) && ...);
}

/**
 * @brief Implements lexicographic less-than comparison across tuple
 *        elements.
 *
 * Compares element 0 first; on a tie, falls through to element 1, and so
 * on — the same rule `std::tuple`'s `operator<` uses.
 *
 * @tparam Lhs  Left-hand Tuple type.
 * @tparam Rhs  Right-hand Tuple type.
 * @tparam Is   Index pack covering every element.
 * @return      True if @p lhs is lexicographically less than @p rhs.
 */
template <typename Lhs, typename Rhs, SizeT... Is>
constexpr Bool TupleLessThanImpl(
    const Lhs& lhs,
    const Rhs& rhs,
    std::index_sequence<
        Is...>) noexcept((noexcept(Get<Is>(std::declval<const Lhs&>()) <
                                   Get<Is>(std::declval<const Rhs&>())) &&
                          ...)) {
  Bool result = false;
  (void)((Get<Is>(lhs) < Get<Is>(rhs)   ? (result = true, true)
          : Get<Is>(rhs) < Get<Is>(lhs) ? (result = false, true)
                                        : false) ||
         ...);
  return result;
}
}  // namespace tuple_detail

/**
 * @brief Returns true if all corresponding elements of @p lhs and @p rhs
 *        are equal.
 * @tparam Ts  Left-hand element types.
 * @tparam Us  Right-hand element types; must be the same count as Ts.
 */
template <typename... Ts,
          typename... Us,
          EnableIf_T<sizeof...(Ts) == sizeof...(Us) &&
                         (IsEqualityComparable_V<Ts, Us> && ...),
                     Bool> = true>
constexpr Bool
operator==(const Tuple<Ts...>& lhs, const Tuple<Us...>& rhs) noexcept(noexcept(
    tuple_detail::TupleEqualImpl(lhs,
                                 rhs,
                                 std::make_index_sequence<sizeof...(Ts)>{}))) {
  return tuple_detail::TupleEqualImpl(
      lhs, rhs, std::make_index_sequence<sizeof...(Ts)>{});
}

/** @brief Returns true if @p lhs and @p rhs differ in at least one element. */
template <typename... Ts, typename... Us>
constexpr Bool operator!=(const Tuple<Ts...>& lhs,
                          const Tuple<Us...>& rhs) noexcept(noexcept(lhs ==
                                                                     rhs)) {
  return !(lhs == rhs);
}

/**
 * @brief Lexicographically compares @p lhs and @p rhs.
 *
 * @code
 *   Tuple<int, int> a(1, 9), b(1, 10);
 *   a < b;  // true — first elements tie, second element decides
 * @endcode
 */
template <typename... Ts,
          typename... Us,
          EnableIf_T<sizeof...(Ts) == sizeof...(Us) &&
                         (IsLessThanComparable_V<Ts, Us> && ...),
                     Bool> = true>
constexpr Bool
operator<(const Tuple<Ts...>& lhs, const Tuple<Us...>& rhs) noexcept(
    noexcept(tuple_detail::TupleLessThanImpl(
        lhs,
        rhs,
        std::make_index_sequence<sizeof...(Ts)>{}))) {
  return tuple_detail::TupleLessThanImpl(
      lhs, rhs, std::make_index_sequence<sizeof...(Ts)>{});
}

/** @brief Lexicographically compares @p lhs and @p rhs (`<=`). */
template <typename... Ts, typename... Us>
constexpr Bool operator<=(const Tuple<Ts...>& lhs,
                          const Tuple<Us...>& rhs) noexcept(noexcept(rhs <
                                                                     lhs)) {
  return !(rhs < lhs);
}

/** @brief Lexicographically compares @p lhs and @p rhs (`>`). */
template <typename... Ts, typename... Us>
constexpr Bool operator>(const Tuple<Ts...>& lhs,
                         const Tuple<Us...>& rhs) noexcept(noexcept(rhs <
                                                                    lhs)) {
  return rhs < lhs;
}

/** @brief Lexicographically compares @p lhs and @p rhs (`>=`). */
template <typename... Ts, typename... Us>
constexpr Bool operator>=(const Tuple<Ts...>& lhs,
                          const Tuple<Us...>& rhs) noexcept(noexcept(lhs <
                                                                     rhs)) {
  return !(lhs < rhs);
}

/** @brief Appends the textual representation of the empty tuple, `"()"`. */
template <typename Output>
void AxioRepr(Output& output, Tuple<>) {
  output.Append("()", 2);
}

/** @brief Appends the textual representation of a single-element tuple. */
template <typename Output, typename T>
void AxioRepr(Output& output, const Tuple<T>& tuple) {
  AppendToOutput(output, '(', Get<0>(tuple), ')');
}

namespace internal {
/**
 * @brief Appends every element of @p tuple followed by `", "`, used as a
 *        helper by the multi-element AxioRepr() overload below.
 * @tparam Is  Index pack covering every element except the last.
 */
template <typename Output, typename... Ts, SizeT... Is>
void AppendSequence(Output& output,
                    const Tuple<Ts...>& tuple,
                    std::index_sequence<Is...>) {
  (AppendToOutput(output, Get<Is>(tuple), ", "), ...);
}
}  // namespace internal

/**
 * @brief Appends the textual representation of a tuple with two or more
 *        elements.
 *
 * @code
 *   AxioRepr(out, Tuple<int, int, int>(1, 2, 3));  // appends "(1, 2, 3)"
 * @endcode
 */
template <typename Output, typename... Ts>
void AxioRepr(Output& output, const Tuple<Ts...>& tuple) {
  static constexpr auto kTupleSize = TupleSize<Tuple<Ts...>>::value;
  output.Append(1, '(');
  internal::AppendSequence(output, tuple,
                           std::make_index_sequence<kTupleSize - 1>{});
  AppendToOutput(output, Get<kTupleSize - 1>(tuple), ')');
}

namespace detail {
/**
 * @brief Invokes @p f with the elements of @p t unpacked as arguments.
 * @tparam F      Callable type.
 * @tparam Tuple  Tuple-like type providing `Get<I>`.
 * @tparam Is     Index pack covering every element of @p t.
 */
template <typename F, typename Tuple, SizeT... Is>
constexpr decltype(auto) ApplyImpl(F&& f,
                                   Tuple&& t,
                                   std::index_sequence<Is...>) {
  return axio::Forward<F>(f)(Get<Is>(axio::Forward<Tuple>(t))...);
}
}  // namespace detail

/**
 * @brief Invokes @p f with the elements of @p t as separate arguments,
 *        mirroring `std::apply`.
 *
 * @tparam F      Callable type.
 * @tparam Tuple  Tuple-like type providing `Get<I>` and `TupleSize`.
 * @param f  Callable to invoke.
 * @param t  Tuple supplying @p f's arguments.
 *
 * @code
 *   Tuple<int, int> args(2, 3);
 *   int sum = Apply([](int a, int b) { return a + b; }, args);  // 5
 * @endcode
 */
template <typename F, typename Tuple>
constexpr decltype(auto) Apply(F&& f, Tuple&& t) {
  using Indices =
      std::make_index_sequence<TupleSize<RemoveReference_T<Tuple>>::value>;
  return detail::ApplyImpl(axio::Forward<F>(f), axio::Forward<Tuple>(t),
                           Indices{});
}
}  // namespace axio

namespace std {
/** @brief Specializes `std::tuple_size` for structured bindings support. */
template <typename... Ts>
struct tuple_size<axio::Tuple<Ts...>>
    : std::integral_constant<size_t, sizeof...(Ts)> {};

/**
 * @brief Specializes `std::tuple_element` for structured bindings support.
 * @tparam I   Zero-based element index.
 * @tparam Ts  Tuple element types.
 */
template <size_t I, typename... Ts>
struct tuple_element<I, axio::Tuple<Ts...>> {
  using type = typename axio::tuple_detail::TypeAt<I, Ts...>::type;
};
}  // namespace std

#endif