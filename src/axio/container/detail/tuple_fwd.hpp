#ifndef AXIO_CONTAINER_DETAIL_TUPLE_FWD_HPP_
#define AXIO_CONTAINER_DETAIL_TUPLE_FWD_HPP_

#include "../../base/type_traits.hpp"

#if defined(_MSC_VER)
#define AXIO_EMPTY_BASES __declspec(empty_bases)
#else
#define AXIO_EMPTY_BASES
#endif

namespace axio {
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
  using type = AddConst_T<typename TupleElement<I, T>::type>;
};
template <SizeT I, typename T>
struct TupleElement<I, volatile T> {
  using type = AddVolatile_T<typename TupleElement<I, T>::type>;
};
template <SizeT I, typename T>
struct TupleElement<I, const volatile T> {
  using type = AddCV_T<typename TupleElement<I, T>::type>;
};
}  // namespace axio

#endif