#ifndef AXIO_CONTAINER_COMPRESSED_TUPLE_HPP_
#define AXIO_CONTAINER_COMPRESSED_TUPLE_HPP_

#include "../utility/forward.hpp"
#include "../utility/move.hpp"

#include "internal/tuple_fwd.hpp"

#include <utility>

#if defined(_MSC_VER)
#define AXIO_EMPTY_BASES __declspec(empty_bases)
#else
#define AXIO_EMPTY_BASES
#endif

namespace axio {
namespace compressed_tuple_details {
template <SizeT INDEX, typename T, Bool = V<ShouldUseEBO<T>>>
struct AXIO_EMPTY_BASES Element : T {
  constexpr Element() = default;
  template <typename U>
  explicit constexpr Element(U&& value) : T(Forward<U>(value)) {}

  constexpr const T& Get() const& { return *this; }
  constexpr T& Get() & { return *this; }
  constexpr const T&& Get() const&& { return Move(*this); }
  constexpr T&& Get() && { return Move(*this); }
};

template <SizeT INDEX, typename T>
struct Element<INDEX, T, false> {
  constexpr Element() = default;
  template <typename U>
  explicit constexpr Element(U&& v) : value(Forward<U>(v)) {}

  constexpr const T& Get() const& { return value; }
  constexpr T& Get() & { return value; }
  constexpr const T&& Get() const&& { return Move(*this).value; }
  constexpr T&& Get() && { return Move(*this).value; }

  T value;
};

template <typename IndexSequence, typename... Ts>
struct AXIO_EMPTY_BASES CompressedTupleImpl;

template <SizeT... Is, typename... Ts>
struct AXIO_EMPTY_BASES CompressedTupleImpl<std::index_sequence<Is...>, Ts...>
    : Element<Is, Ts>... {
  constexpr CompressedTupleImpl() = default;

  template <typename... Us>
  explicit constexpr CompressedTupleImpl(Us&&... args)
      : Element<Is, Ts>(Forward<Us>(args))... {}
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

template <bool, typename...>
struct PairwiseConstructible : FalseType {};

template <typename... Ts, typename... Us>
struct PairwiseConstructible<true, TypeList<Ts...>, TypeList<Us...>>
    : Conjunction<IsConstructible<Ts, Us&&>...> {};
}  // namespace compressed_tuple_details

template <typename... Ts>
class AXIO_EMPTY_BASES CompressedTuple
    : private compressed_tuple_details::
          CompressedTupleImpl<std::make_index_sequence<sizeof...(Ts)>, Ts...> {
  using Base = compressed_tuple_details::
      CompressedTupleImpl<std::make_index_sequence<sizeof...(Ts)>, Ts...>;

  template <SizeT I>
  using Element = compressed_tuple_details::
      Element<I, axio::T<compressed_tuple_details::TypeAt<I, Ts...>>>;

  template <typename... Us>
  using EnableTupleCtor = axio::EnableIf<
      compressed_tuple_details::PairwiseConstructible<sizeof...(Us) ==
                                                          sizeof...(Ts),
                                                      TypeList<Ts...>,
                                                      TypeList<Us...>>::value &&
          !(sizeof...(Us) == 1 &&
            IsSpecializationOf<
                RemoveCVRef<compressed_tuple_details::First<Us...>>,
                CompressedTuple>::value),
      bool>;

  template <typename... Us, SizeT... Is>
  constexpr CompressedTuple(
      const CompressedTuple<Us...>& other,
      std::index_sequence<
          Is...>) noexcept((IsNothrowConstructible<Ts, const Us&>::value &&
                            ...))
      : Base(other.template Get<Is>()...) {}

  template <typename... Us, SizeT... Is>
  constexpr CompressedTuple(
      CompressedTuple<Us...>&& other,
      std::index_sequence<
          Is...>) noexcept((IsNothrowConstructible<Ts,
                                                   decltype(Move(other)
                                                                .template Get<
                                                                    Is>())>::
                                value &&
                            ...))
      : Base(Move(other).template Get<Is>()...) {}

  template <typename... Us, SizeT... Is>
  constexpr void AssignFrom(
      const CompressedTuple<Us...>& other,
      std::index_sequence<
          Is...>) noexcept((IsNothrowAssignable<Ts&, const Us&>::value &&
                            ...)) {
    ((Get<Is>() = other.template Get<Is>()), ...);
  }

  template <typename... Us, SizeT... Is>
  constexpr void AssignFrom(
      CompressedTuple<Us...>&& other,
      std::index_sequence<
          Is...>) noexcept((IsNothrowAssignable<Ts&, Us&&>::value && ...)) {
    ((Get<Is>() = Move(other.template Get<Is>())), ...);
  }

 public:
  constexpr CompressedTuple() = default;

  template <typename... Us, axio::T<EnableTupleCtor<Us...>> = true>
  constexpr explicit CompressedTuple(Us&&... args) noexcept(
      (IsNothrowConstructible<Ts, Us&&>::value && ...))
      : Base(Forward<Us>(args)...) {}

  template <typename... Us,
            axio::T<EnableIf<sizeof...(Us) == sizeof...(Ts) &&
                                 (IsConstructible<Ts, const Us&>::value && ...),
                             bool>> = true>
  constexpr explicit CompressedTuple(
      const CompressedTuple<Us...>&
          other) noexcept((IsNothrowConstructible<Ts, const Us&>::value && ...))
      : CompressedTuple(other, std::make_index_sequence<sizeof...(Ts)>{}) {}

  template <typename... Us,
            axio::T<EnableIf<sizeof...(Us) == sizeof...(Ts) &&
                                 (IsConstructible<Ts, Us&&>::value && ...),
                             bool>> = true>
  constexpr explicit CompressedTuple(CompressedTuple<Us...>&& other) noexcept(
      (IsNothrowConstructible<Ts, Us&&>::value && ...))
      : CompressedTuple(axio::Move(other),
                        std::make_index_sequence<sizeof...(Ts)>{}) {}

  template <typename... Us,
            axio::T<EnableIf<sizeof...(Us) == sizeof...(Ts) &&
                                 (IsAssignable<Ts&, const Us&>::value && ...),
                             bool>> = true>
  constexpr CompressedTuple&
  operator=(const CompressedTuple<Us...>& other) noexcept(
      (IsNothrowAssignable<Ts&, const Us&>::value && ...)) {
    AssignFrom(other, std::make_index_sequence<sizeof...(Ts)>{});
    return *this;
  }

  template <typename... Us,
            axio::T<EnableIf<sizeof...(Us) == sizeof...(Ts) &&
                                 (IsAssignable<Ts&, Us&&>::value && ...),
                             bool>> = true>
  constexpr CompressedTuple& operator=(CompressedTuple<Us...>&& other) noexcept(
      (IsNothrowAssignable<Ts&, Us&&>::value && ...)) {
    AssignFrom(Move(other), std::make_index_sequence<sizeof...(Ts)>{});
    return *this;
  }

  template <SizeT I>
  constexpr decltype(auto) Get() const& {
    return static_cast<const Element<I>&>(*this).Get();
  }

  template <SizeT I>
  constexpr decltype(auto) Get() & {
    return static_cast<Element<I>&>(*this).Get();
  }

  template <SizeT I>
  constexpr decltype(auto) Get() const&& {
    return static_cast<const Element<I>&&>(*this).Get();
  }

  template <SizeT I>
  constexpr decltype(auto) Get() && {
    return static_cast<Element<I>&&>(*this).Get();
  }
};

template <SizeT I, typename... Ts>
constexpr decltype(auto) Get(CompressedTuple<Ts...>& tuple) noexcept(
    noexcept(tuple.template Get<I>())) {
  return tuple.template Get<I>();
}

template <SizeT I, typename... Ts>
constexpr decltype(auto) Get(const CompressedTuple<Ts...>& tuple) noexcept(
    noexcept(tuple.template Get<I>())) {
  return tuple.template Get<I>();
}

template <SizeT I, typename... Ts>
constexpr decltype(auto) Get(CompressedTuple<Ts...>&& tuple) noexcept(
    noexcept(Move(tuple).template Get<I>())) {
  return Move(tuple).template Get<I>();
}

template <SizeT I, typename... Ts>
constexpr decltype(auto) Get(const CompressedTuple<Ts...>&& tuple) noexcept(
    noexcept(Move(tuple).template Get<I>())) {
  return Move(tuple).template Get<I>();
}

namespace compressed_tuple_details {
template <typename Lhs, typename Rhs, SizeT... Is>
constexpr bool TupleEqualImpl(
    const Lhs& lhs,
    const Rhs& rhs,
    std::index_sequence<
        Is...>) noexcept((noexcept(Get<Is>(std::declval<Lhs>()) ==
                                   Get<Is>(std::declval<Rhs>())) &&
                          ...)) {
  return ((Get<Is>(lhs) == Get<Is>(rhs)) && ...);
}
}  // namespace compressed_tuple_details

template <typename... Ts,
          typename... Us,
          axio::T<EnableIf<sizeof...(Ts) == sizeof...(Us) &&
                               (IsEqualityComparable<Ts, Us>::value && ...),
                           bool>> = true>
constexpr bool operator==(
    const CompressedTuple<Ts...>& lhs,
    const CompressedTuple<Us...>&
        rhs) noexcept(noexcept(compressed_tuple_details::
                                   TupleEqualImpl(
                                       std::declval<
                                           const CompressedTuple<Ts...>&>(),
                                       std::declval<
                                           const CompressedTuple<Us...>&>(),
                                       std::make_index_sequence<
                                           sizeof...(Ts)>{}))) {
  return compressed_tuple_details::TupleEqualImpl(
      lhs, rhs, std::make_index_sequence<sizeof...(Ts)>{});
}

template <typename... Ts, typename... Us>
constexpr bool operator!=(const CompressedTuple<Ts...>& lhs,
                          const CompressedTuple<Us...>& rhs) {
  return !(lhs == rhs);
}

template <typename... Ts>
struct TupleSize<CompressedTuple<Ts...>>
    : IntegralConstant<SizeT, sizeof...(Ts)> {};

template <SizeT I, typename... Ts>
struct TupleElement<I, CompressedTuple<Ts...>> {
  using type = axio::T<compressed_tuple_details::TypeAt<I, Ts...>>;
};

template <typename T, typename U>
using CompressedPair = CompressedTuple<T, U>;
}  // namespace axio

#endif