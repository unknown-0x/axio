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
    Void<typename std::iterator_traits<It>::iterator_category>>
    : IsBaseOf<std::input_iterator_tag,
               typename std::iterator_traits<It>::iterator_category> {};

template <typename It, typename = void>
struct IsForwardIterator : FalseType {};

template <typename It>
struct IsForwardIterator<
    It,
    Void<typename std::iterator_traits<It>::iterator_category>>
    : IsBaseOf<std::forward_iterator_tag,
               typename std::iterator_traits<It>::iterator_category> {};
}  // namespace axio

#endif