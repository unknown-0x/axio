#ifndef AXIO_CONTAINER_DETAIL_ITERATOR_TRAITS_HPP_
#define AXIO_CONTAINER_DETAIL_ITERATOR_TRAITS_HPP_

#include <iterator>

#include "../../base/type_traits.hpp"

namespace axio {
template <typename It, typename = void>
struct IsInputIterator : FalseType {};

template <typename It>
struct IsInputIterator<
    It,
    Void_T<typename std::iterator_traits<It>::iterator_category>>
    : IsBaseOf<std::input_iterator_tag,
               typename std::iterator_traits<It>::iterator_category> {};

template <typename It, typename = void>
struct IsForwardIterator : FalseType {};

template <typename It>
struct IsForwardIterator<
    It,
    Void_T<typename std::iterator_traits<It>::iterator_category>>
    : IsBaseOf<std::forward_iterator_tag,
               typename std::iterator_traits<It>::iterator_category> {};

template <typename It>
inline constexpr auto IsInputIterator_V = IsInputIterator<It>::value;

template <typename It>
inline constexpr auto IsForwardIterator_V = IsForwardIterator<It>::value;
}  // namespace axio

#endif