// Copyright 2015-2017 Hans Dembinski
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef _BOOST_HISTOGRAM_HISTOGRAM_FWD_HPP_
#define _BOOST_HISTOGRAM_HISTOGRAM_FWD_HPP_

#include <boost/histogram/detail/meta.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/vector.hpp>
#include <initializer_list>
#include <set>
#include <type_traits>

namespace boost {
namespace histogram {

using Static = std::integral_constant<int, 0>;
using Dynamic = std::integral_constant<int, 1>;

template <template <class> class Allocator = std::allocator>
class adaptive_storage;

template <class Variant, class Axes, class Storage = adaptive_storage<>>
class histogram;

struct weight {
  weight(double w) : value(w) {}
  double value;
};

struct count {
  count(unsigned n) : value(n) {}
  unsigned value;
};

// for static and dynamic histogram
template <int N, typename... Rest>
inline auto keep(mpl::int_<N>, Rest...)
    -> detail::unique_sorted<mpl::vector<mpl::int_<N>, Rest...>> {
  return {};
}

// for dynamic histogram only
namespace detail {
using keep_dynamic = std::set<unsigned>;
inline void insert(keep_dynamic &) {} // end recursion
template <typename... Rest>
inline void insert(keep_dynamic &s, unsigned i, Rest... rest) {
  s.insert(i);
  insert(s, rest...);
}
} // namespace detail

template <typename Iterator, typename = detail::is_iterator<Iterator>>
inline detail::keep_dynamic keep(Iterator begin, Iterator end) {
  return {begin, end};
}

template <typename... Rest>
inline detail::keep_dynamic keep(unsigned i, Rest... rest) {
  detail::keep_dynamic s;
  detail::insert(s, i, rest...);
  return s;
}

} // namespace histogram
} // namespace boost

#endif
