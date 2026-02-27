#ifndef AXIO_CONTAINER_INTERNAL_TUPLE_FWD_HPP_
#define AXIO_CONTAINER_INTERNAL_TUPLE_FWD_HPP_

#include "../../base/type_traits.hpp"

namespace axio {
template <typename... Ts>
class CompressedTuple;

template <typename T>
struct TupleSize;
template <typename T>
struct TupleSize<const T> : IntegralConstant<SizeT, TupleSize<T>::value> {};
template <typename T>
struct TupleSize<volatile T> : IntegralConstant<SizeT, TupleSize<T>::value> {};
template <typename T>
struct TupleSize<const volatile T>
    : IntegralConstant<SizeT, TupleSize<T>::value> {};

template <SizeT I, typename T>
struct TupleElement;
template <SizeT I, typename T>
struct TupleElement<I, const T> {
  using type = axio::T<AddConst<axio::T<TupleElement<I, T>>>>;
};
template <SizeT I, typename T>
struct TupleElement<I, volatile T> {
  using type = axio::T<AddVolatile<axio::T<TupleElement<I, T>>>>;
};
template <SizeT I, typename T>
struct TupleElement<I, const volatile T> {
  using type = axio::T<AddCV<axio::T<TupleElement<I, T>>>>;
};
}

#endif