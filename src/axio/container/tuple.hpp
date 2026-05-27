#ifndef AXIO_CONTAINER_TUPLE_HPP_
#define AXIO_CONTAINER_TUPLE_HPP_

#include "detail/tuple_fwd.hpp"

#include "../string/axio_repr.hpp"
#include "../utility/forward.hpp"
#include "../utility/move.hpp"

namespace axio {
namespace tuple_detail {
template <SizeT I, typename T, Bool = ShouldUseEBO_V<T>>
struct AXIO_EMPTY_BASES TupleValue : public T {
  using Type = T;
  static constexpr Bool kUseEBO = true;

  constexpr TupleValue() noexcept(IsNothrowDefaultConstructible_V<T>) : T() {}

  template <typename U,
            typename = EnableIf_T<!IsSame_V<TupleValue, Decay_T<U>>>>
  constexpr TupleValue(U&& arg) noexcept(IsNothrowConstructible_V<T, U&&>)
      : T(Forward<U>(arg)) {}
};

template <SizeT I, typename T>
struct TupleValue<I, T, false> {
  using Type = T;
  static constexpr Bool kUseEBO = false;

  constexpr TupleValue() noexcept(IsNothrowDefaultConstructible_V<T>)
      : value() {}

  template <typename U,
            typename = EnableIf_T<!IsSame_V<TupleValue, Decay_T<U>>>>
  constexpr TupleValue(U&& arg) noexcept(IsNothrowConstructible_V<T, U&&>)
      : value(Forward<U>(arg)) {}

  T value;
};

template <typename, typename...>
struct AXIO_EMPTY_BASES TupleImpl;

template <SizeT... Is, typename... Ts>
struct AXIO_EMPTY_BASES
    TupleImpl<std::index_sequence<Is...>, Ts...> : TupleValue<Is, Ts>... {
  constexpr TupleImpl() = default;

  constexpr explicit TupleImpl(const Ts&... args)
      : TupleValue<Is, Ts>(args)... {}

  template <typename... Us>
  constexpr explicit TupleImpl(Us&&... args)
      : TupleValue<Is, Ts>(Forward<Us>(args))... {}

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

  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsConstructible<Ts, Us&&>...>,
                       Bool> = true>
  constexpr TupleImpl(
      TupleImpl<std::index_sequence<Is...>, Us...>&&
          other) noexcept(Conjunction_V<IsNothrowConstructible<Ts, Us&&>...>)
      : TupleValue<Is, Ts>(
            GetValue(static_cast<TupleValue<Is, Us>&&>(other)))... {}

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
  template <SizeT I, typename U>
  constexpr U& GetValue(TupleValue<I, U>& element) {
    if constexpr (TupleValue<I, U>::kUseEBO) {
      return static_cast<U&>(element);
    } else {
      return element.value;
    }
  }

  template <SizeT I, typename U>
  constexpr const U& GetValue(const TupleValue<I, U>& element) {
    if constexpr (TupleValue<I, U>::kUseEBO) {
      return static_cast<const U&>(element);
    } else {
      return element.value;
    }
  }

  template <SizeT I, typename U>
  constexpr decltype(auto) GetValue(TupleValue<I, U>&& element) {
    if constexpr (TupleValue<I, U>::kUseEBO) {
      return static_cast<U&&>(element);
    } else {
      return static_cast<decltype(element.value)&&>(element.value);
    }
  }

  template <SizeT I, typename U>
  constexpr decltype(auto) GetValue(const TupleValue<I, U>&& element) {
    if constexpr (TupleValue<I, U>::kUseEBO) {
      return static_cast<const U&&>(element);
    } else {
      return static_cast<const decltype(element.value)&&>(element.value);
    }
  }
};

template <typename T, typename...>
struct FirstType {
  using type = T;
};

template <typename... Ts>
using First = typename FirstType<Ts...>::type;

template <SizeT I, typename... Ts>
struct TypeAt;

template <typename T, typename... Ts>
struct TypeAt<0, T, Ts...> {
  using type = T;
};

template <SizeT I, typename T, typename... Ts>
struct TypeAt<I, T, Ts...> : TypeAt<I - 1, Ts...> {};

template <typename>
struct Ignore : TrueType {};

template <typename T>
using UnwrapDecayT = typename UnwrapReferenceWrapper<Decay_T<T>>::type;
}  // namespace tuple_detail

template <typename... Ts>
struct AXIO_EMPTY_BASES Tuple
    : public tuple_detail::TupleImpl<std::make_index_sequence<sizeof...(Ts)>,
                                     Ts...> {
  template <SizeT I>
  using Element =
      tuple_detail::TupleValue<I,
                               typename tuple_detail::TypeAt<I, Ts...>::type>;

  using Base =
      tuple_detail::TupleImpl<std::make_index_sequence<sizeof...(Ts)>, Ts...>;

  template <typename Dummy = void,
            EnableIf_T<Conjunction_V<tuple_detail::Ignore<Dummy>,
                                     IsDefaultConstructible<Ts>...>,
                       Bool> = true>
  constexpr Tuple() noexcept(
      Conjunction_V<IsNothrowDefaultConstructible<Ts>...>)
      : Base() {}

  template <typename Dummy = void,
            EnableIf_T<Conjunction_V<tuple_detail::Ignore<Dummy>,
                                     IsCopyConstructible<Ts>...>,
                       Bool> = true>
  constexpr explicit Tuple(const Ts&... args) noexcept(
      Conjunction_V<IsNothrowCopyConstructible<Ts>...>)
      : Base(args...) {}

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

  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsConstructible<Ts, const Us&>...>,
                       Bool> = true>
  constexpr explicit Tuple(const Tuple<Us...>& other) noexcept(
      Conjunction_V<IsNothrowConstructible<Ts, const Us&>...>)
      : Base(static_cast<const typename Tuple<Us...>::Base&>(other)) {}

  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsConstructible<Ts, Us&&>...>,
                       Bool> = true>
  constexpr explicit Tuple(Tuple<Us...>&& other) noexcept(
      Conjunction_V<IsNothrowConstructible<Ts, Us&&>...>)
      : Base(static_cast<typename Tuple<Us...>::Base&&>(other)) {}

  template <typename... Us,
            EnableIf_T<sizeof...(Us) == sizeof...(Ts) &&
                           Conjunction_V<IsAssignable<Ts&, const Us&>...>,
                       Bool> = true>
  constexpr Tuple& operator=(const Tuple<Us...>& other) noexcept(
      Conjunction_V<IsNothrowAssignable<Ts&, const Us&>...>) {
    Base::operator=(static_cast<const typename Tuple<Us...>::Base&>(other));
    return *this;
  }

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

template <>
struct Tuple<> {};

template <typename... Ts>
struct TupleSize<Tuple<Ts...>> : IntegralConstant<SizeT, sizeof...(Ts)> {};

template <SizeT I, typename... Ts>
struct TupleElement<I, Tuple<Ts...>> {
  using type = typename tuple_detail::TypeAt<I, Ts...>::type;
};

template <SizeT I, typename... Ts>
constexpr decltype(auto) Get(Tuple<Ts...>& t) noexcept {
  using Element = typename Tuple<Ts...>::template Element<I>;
  if constexpr (Element::kUseEBO) {
    return static_cast<typename Element::Type&>(static_cast<Element&>(t));
  } else {
    return static_cast<typename Element::Type&>(static_cast<Element&>(t).value);
  }
}

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

template <typename... Ts>
constexpr Tuple<tuple_detail::UnwrapDecayT<Ts>...> MakeTuple(Ts&&... args) {
  return Tuple<tuple_detail::UnwrapDecayT<Ts>...>(Forward<Ts>(args)...);
}

template <typename... Ts>
constexpr Tuple<Ts&&...> ForwardAsTuple(Ts&&... args) {
  return Tuple<Ts&&...>(Forward<Ts>(args)...);
}

namespace tuple_detail {
template <typename T, typename TTuple, SizeT... I>
constexpr T MakeFromTupleImpl(TTuple&& t, std::index_sequence<I...>) {
  return T(Get<I>(Forward<TTuple>(t))...);
}

template <typename A, typename B>
struct Concat;

template <SizeT... A, SizeT... B>
struct Concat<std::index_sequence<A...>, std::index_sequence<B...>> {
  using type = std::index_sequence<A..., B...>;
};

template <SizeT I, typename Seq>
struct RepeatIndexHelper;

template <SizeT I, SizeT... Is>
struct RepeatIndexHelper<I, std::index_sequence<Is...>> {
  using type = std::index_sequence<(static_cast<void>(Is), I)...>;
};

template <SizeT I, SizeT N>
using RepeatIndex =
    typename RepeatIndexHelper<I, std::make_index_sequence<N>>::type;

template <typename Sizes, SizeT Index = 0>
struct BuildOuter;

template <SizeT Index>
struct BuildOuter<std::index_sequence<>, Index> {
  using type = std::index_sequence<>;
};

template <SizeT S, SizeT... REST, SizeT IDX>
struct BuildOuter<std::index_sequence<S, REST...>, IDX> {
  using type = typename Concat<
      RepeatIndex<IDX, S>,
      typename BuildOuter<std::index_sequence<REST...>, IDX + 1>::type>::type;
};

template <typename Sizes>
struct BuildInner;

template <>
struct BuildInner<std::index_sequence<>> {
  using type = std::index_sequence<>;
};

template <SizeT S, SizeT... REST>
struct BuildInner<std::index_sequence<S, REST...>> {
  using type = typename Concat<
      std::make_index_sequence<S>,
      typename BuildInner<std::index_sequence<REST...>>::type>::type;
};

template <typename Outer, typename Inner, typename TupleOfTuples>
struct TupleCatResult;

template <SizeT... Os, SizeT... Is, typename T>
struct TupleCatResult<std::index_sequence<Os...>,
                      std::index_sequence<Is...>,
                      T> {
  using type = axio::Tuple<
      typename TupleElement<Is, typename TupleElement<Os, T>::type>::type...>;
};

template <typename R, SizeT... Os, SizeT... Is, typename TupleOfTuples>
constexpr R TupleCatImpl(std::index_sequence<Os...>,
                         std::index_sequence<Is...>,
                         TupleOfTuples&& t) {
  return R(Get<Is>(Get<Os>(Forward<TupleOfTuples>(t)))...);
}
}  // namespace tuple_detail

template <typename T, typename TTuple>
constexpr T MakeFromTuple(TTuple&& t) {
  return tuple_detail::MakeFromTupleImpl<T>(
      Forward<TTuple>(t),
      std::make_index_sequence<TupleSize<RemoveReference_T<TTuple>>::value>{});
}

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

template <size_t I, typename... Ts>
constexpr decltype(auto) get(Tuple<Ts...>& t) noexcept {
  return Get<I>(t);
}

template <size_t I, typename... Ts>
constexpr decltype(auto) get(const Tuple<Ts...>& t) noexcept {
  return Get<I>(t);
}

template <size_t I, typename... Ts>
constexpr decltype(auto) get(Tuple<Ts...>&& t) noexcept {
  return Get<I>(Move(t));
}

template <size_t I, typename... Ts>
constexpr decltype(auto) get(const Tuple<Ts...>&& t) noexcept {
  return Get<I>(Move(t));
}

namespace tuple_detail {
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

template <typename... Ts, typename... Us>
constexpr Bool operator!=(const Tuple<Ts...>& lhs,
                          const Tuple<Us...>& rhs) noexcept(noexcept(lhs ==
                                                                     rhs)) {
  return !(lhs == rhs);
}

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

template <typename... Ts, typename... Us>
constexpr Bool operator<=(const Tuple<Ts...>& lhs,
                          const Tuple<Us...>& rhs) noexcept(noexcept(rhs <
                                                                     lhs)) {
  return !(rhs < lhs);
}

template <typename... Ts, typename... Us>
constexpr Bool operator>(const Tuple<Ts...>& lhs,
                         const Tuple<Us...>& rhs) noexcept(noexcept(rhs <
                                                                    lhs)) {
  return rhs < lhs;
}

template <typename... Ts, typename... Us>
constexpr Bool operator>=(const Tuple<Ts...>& lhs,
                          const Tuple<Us...>& rhs) noexcept(noexcept(lhs <
                                                                     rhs)) {
  return !(lhs < rhs);
}

template <typename Output>
void AxioRepr(Output& output, Tuple<>) {
  output.Append("()", 2);
}

template <typename Output, typename T>
void AxioRepr(Output& output, const Tuple<T>& tuple) {
  AppendToOutput(output, '(', Get<0>(tuple), ')');
}

namespace internal {
template <typename Output, typename... Ts, SizeT... Is>
void AppendSequence(Output& output,
                    const Tuple<Ts...>& tuple,
                    std::index_sequence<Is...>) {
  (AppendToOutput(output, Get<Is>(tuple), ", "), ...);
}
}  // namespace internal

template <typename Output, typename... Ts>
void AxioRepr(Output& output, const Tuple<Ts...>& tuple) {
  static constexpr auto kTupleSize = TupleSize<Tuple<Ts...>>::value;
  output.Append(1, '(');
  internal::AppendSequence(output, tuple,
                           std::make_index_sequence<kTupleSize - 1>{});
  AppendToOutput(output, Get<kTupleSize - 1>(tuple), ')');
}
}  // namespace axio

namespace std {
template <typename... Ts>
struct tuple_size<axio::Tuple<Ts...>>
    : std::integral_constant<size_t, sizeof...(Ts)> {};

template <size_t I, typename... Ts>
struct tuple_element<I, axio::Tuple<Ts...>> {
  using type = typename axio::tuple_detail::TypeAt<I, Ts...>::type;
};
}  // namespace std

#endif