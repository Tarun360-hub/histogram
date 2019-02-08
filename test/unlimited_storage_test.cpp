// Copyright 2015-2017 Hans Dembinski
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <boost/core/lightweight_test.hpp>
#include <boost/histogram/storage_adaptor.hpp>
#include <boost/histogram/unlimited_storage.hpp>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

namespace bh = boost::histogram;
using unlimited_storage_type = bh::unlimited_storage<>;
template <typename T>
using vector_storage = bh::storage_adaptor<std::vector<T>>;

template <typename T = std::uint8_t>
unlimited_storage_type prepare(std::size_t n, T x = T()) {
  std::unique_ptr<T[]> v(new T[n]);
  std::fill(v.get(), v.get() + n, static_cast<T>(0));
  v.get()[0] = x;
  return unlimited_storage_type(n, v.get());
}

template <class T>
auto max() {
  return std::numeric_limits<T>::max();
}

template <>
inline auto max<unlimited_storage_type::mp_int>() {
  return unlimited_storage_type::mp_int(1e300);
}

template <typename T>
void copy() {
  const auto b = prepare<T>(1);
  auto a(b);
  BOOST_TEST(a == b);
  ++a[0];
  BOOST_TEST(!(a == b));
  a = b;
  BOOST_TEST(a == b);
  ++a[0];
  BOOST_TEST(!(a == b));
  a = prepare<T>(2);
  BOOST_TEST(!(a == b));
  a = b;
  BOOST_TEST(a == b);
}

template <typename T>
void equal_1() {
  auto a = prepare(1);
  auto b = prepare(1, T(0));
  BOOST_TEST_EQ(a[0], 0.0);
  BOOST_TEST(a == b);
  ++b[0];
  BOOST_TEST(!(a == b));
}

template <typename T, typename U>
void equal_2() {
  auto a = prepare<T>(1);
  vector_storage<U> b;
  b.reset(1);
  BOOST_TEST(a == b);
  ++b[0];
  BOOST_TEST(!(a == b));
}

template <typename T>
void increase_and_grow() {
  auto tmax = std::numeric_limits<T>::max();
  auto s = prepare(2, tmax);
  auto n = s;
  auto n2 = s;

  ++n[0];

  auto x = prepare(2);
  ++x[0];
  n2[0] += x[0];

  double v = tmax;
  ++v;
  BOOST_TEST_EQ(n[0], v);
  BOOST_TEST_EQ(n2[0], v);
  BOOST_TEST_EQ(n[1], 0.0);
  BOOST_TEST_EQ(n2[1], 0.0);
}

template <typename T>
void convert_array_storage() {
  vector_storage<uint8_t> s;
  s.reset(1);
  ++s[0];
  BOOST_TEST_EQ(s[0], 1);

  // test "copy" ctor
  unlimited_storage_type b(s);
  BOOST_TEST_EQ(b[0], 1.0);
  BOOST_TEST(b == s);
  ++b[0];
  BOOST_TEST_NOT(b == s);

  // test assign and equal
  auto a = prepare<T>(1);
  a = s;
  BOOST_TEST_EQ(a[0], 1.0);
  BOOST_TEST(a == s);
  ++a[0];
  BOOST_TEST_NOT(a == s);

  // test radd
  auto c = prepare<T>(1);
  c[0] += s[0];
  BOOST_TEST_EQ(c[0], 1);
  BOOST_TEST(c == s);
  c[0] += s[0];
  BOOST_TEST_EQ(c[0], 2);
  BOOST_TEST_NOT(c == s);

  // test assign from float
  vector_storage<float> t;
  t.reset(1);
  t[0] = 1.5;
  auto d = prepare<T>(1);
  d = t;
  BOOST_TEST(d == t);
  BOOST_TEST(d[0] == 1.5);

  // test "copy" ctor from float
  unlimited_storage_type f(t);
  BOOST_TEST_EQ(f[0], 1.5);
  BOOST_TEST(f == t);

  // test radd from float
  auto g = prepare<T>(1);
  g[0] += t[0];
  BOOST_TEST_EQ(g[0], 1.5);
  BOOST_TEST(g == t);

  vector_storage<int8_t> u;
  u.reset(1);
  u[0] = -10;
  auto h = prepare<T>(1);
  BOOST_TEST_NOT(h == u);
  h = u;
  BOOST_TEST(h == u);
  BOOST_TEST_EQ(h[0], -10);
  h[0] -= u[0];
  BOOST_TEST_EQ(h[0], 0);
}

template <typename LHS, typename RHS>
void add() {
  auto a = prepare<LHS>(1);
  auto b = prepare<RHS>(1);
  b[0] += 2;
  a[0] += b[0];
  BOOST_TEST_EQ(a[0], 2);
  a[0] -= b[0];
  BOOST_TEST_EQ(a[0], 0);
  a[0] -= b[0];
  BOOST_TEST_EQ(a[0], -2);

  auto c = prepare<RHS>(1);
  c[0] = max<RHS>();
  auto d = prepare<LHS>(1);
  d[0] += c[0];
  BOOST_TEST_EQ(d[0], max<RHS>());
  auto e = prepare<LHS>(1);
  e[0] -= c[0];
  BOOST_TEST_EQ(e[0], unlimited_storage_type::negate(max<RHS>()));
}

template <typename LHS>
void add_all_rhs() {
  add<LHS, uint8_t>();
  add<LHS, uint16_t>();
  add<LHS, uint32_t>();
  add<LHS, uint64_t>();
  add<LHS, unlimited_storage_type::mp_int>();
  add<LHS, double>();
}

int main() {
  // low-level tools
  {
    uint8_t c = 0;
    BOOST_TEST_EQ(bh::detail::safe_increment(c), true);
    BOOST_TEST_EQ(c, 1);
    c = 255;
    BOOST_TEST_EQ(bh::detail::safe_increment(c), false);
    BOOST_TEST_EQ(c, 255);
    c = 0;
    BOOST_TEST_EQ(bh::detail::safe_radd(c, 255u), true);
    BOOST_TEST_EQ(c, 255);
    c = 0;
    BOOST_TEST_EQ(bh::detail::safe_radd(c, 255), true);
    BOOST_TEST_EQ(c, 255);
    c = 1;
    BOOST_TEST_EQ(bh::detail::safe_radd(c, 255), false);
    BOOST_TEST_EQ(c, 1);
    BOOST_TEST_EQ(bh::detail::safe_radd(c, 255u), false);
    BOOST_TEST_EQ(c, 1);
    c = 255;
    BOOST_TEST_EQ(bh::detail::safe_radd(c, 1u), false);
    BOOST_TEST_EQ(c, 255);
    BOOST_TEST_EQ(bh::detail::safe_radd(c, 1), false);
    BOOST_TEST_EQ(c, 255);
    BOOST_TEST_EQ(bh::detail::safe_radd(c, -255), true);
    BOOST_TEST_EQ(c, 0);
    BOOST_TEST_EQ(bh::detail::safe_radd(c, -1), false);
    BOOST_TEST_EQ(c, 0);

    uint32_t i = 0;
    BOOST_TEST_EQ(bh::detail::safe_radd(i, 2), true);
    BOOST_TEST_EQ(i, 2u);
    BOOST_TEST_EQ(bh::detail::safe_radd(i, -2), true);
    BOOST_TEST_EQ(i, 0u);
    BOOST_TEST_EQ(bh::detail::safe_radd(i, -2), false);
    BOOST_TEST_EQ(i, 0u);
  }

  // empty state
  {
    unlimited_storage_type a;
    BOOST_TEST_EQ(a.size(), 0);
  }

  // copy
  {
    copy<uint8_t>();
    copy<uint16_t>();
    copy<uint32_t>();
    copy<uint64_t>();
    copy<unlimited_storage_type::mp_int>();
    copy<double>();
  }

  // equal_operator
  {
    equal_1<uint8_t>();
    equal_1<uint16_t>();
    equal_1<uint32_t>();
    equal_1<uint64_t>();
    equal_1<unlimited_storage_type::mp_int>();
    equal_1<double>();

    equal_2<uint8_t, unsigned>();
    equal_2<uint16_t, unsigned>();
    equal_2<uint32_t, unsigned>();
    equal_2<uint64_t, unsigned>();
    equal_2<unlimited_storage_type::mp_int, unsigned>();
    equal_2<double, unsigned>();

    equal_2<unlimited_storage_type::mp_int, double>();

    auto a = prepare<double>(1);
    auto b = prepare<unlimited_storage_type::mp_int>(1);
    BOOST_TEST(a == b);
    ++a[0];
    BOOST_TEST_NOT(a == b);
  }

  // increase_and_grow
  {
    increase_and_grow<uint8_t>();
    increase_and_grow<uint16_t>();
    increase_and_grow<uint32_t>();
    increase_and_grow<uint64_t>();

    // only increase for mp_int
    auto a = prepare<unlimited_storage_type::mp_int>(2, 1);
    BOOST_TEST_EQ(a[0], 1);
    BOOST_TEST_EQ(a[1], 0);
    ++a[0];
    BOOST_TEST_EQ(a[0], 2);
    BOOST_TEST_EQ(a[1], 0);
  }

  // add
  {
    add_all_rhs<uint8_t>();
    add_all_rhs<uint16_t>();
    add_all_rhs<uint32_t>();
    add_all_rhs<uint64_t>();
    add_all_rhs<unlimited_storage_type::mp_int>();
    add_all_rhs<double>();
  }

  // add_and_grow
  {
    auto a = prepare(1);
    a[0] += a[0];
    BOOST_TEST_EQ(a[0], 0);
    ++a[0];
    double x = 1;
    auto b = prepare(1);
    ++b[0];
    BOOST_TEST_EQ(b[0], x);
    for (unsigned i = 0; i < 80; ++i) {
      x += x;
      a[0] += a[0];
      b[0] += b[0];
      BOOST_TEST_EQ(a[0], x);
      BOOST_TEST_EQ(b[0], x);
      auto c = prepare(1);
      c[0] += a[0];
      BOOST_TEST_EQ(c[0], x);
      c[0] += 0;
      BOOST_TEST_EQ(c[0], x);
      auto d = prepare(1);
      d[0] += x;
      BOOST_TEST_EQ(d[0], x);
    }
  }

  // multiply
  {
    auto a = prepare(2);
    ++a[0];
    a *= 3;
    BOOST_TEST_EQ(a[0], 3);
    BOOST_TEST_EQ(a[1], 0);
    a[1] += 2;
    a *= 3;
    BOOST_TEST_EQ(a[0], 9);
    BOOST_TEST_EQ(a[1], 6);
  }

  // convert_array_storage
  {
    convert_array_storage<uint8_t>();
    convert_array_storage<uint16_t>();
    convert_array_storage<uint32_t>();
    convert_array_storage<uint64_t>();
    convert_array_storage<unlimited_storage_type::mp_int>();
    convert_array_storage<double>();
  }

  // reference
  {
    auto a = prepare(1);
    auto b = prepare<uint32_t>(1);
    BOOST_TEST_EQ(a[0], b[0]);
    BOOST_TEST_GE(a[0], b[0]);
    BOOST_TEST_LE(a[0], b[0]);
    a[0] = 1;
    BOOST_TEST_NE(a[0], b[0]);
    BOOST_TEST_LT(b[0], a[0]);
    BOOST_TEST_GT(a[0], b[0]);
    BOOST_TEST_EQ(a[0], 1);
    BOOST_TEST_GE(a[0], 1);
    BOOST_TEST_LE(a[0], 1);
    ++b[0];
    BOOST_TEST_EQ(a[0], b[0]);
    b[0] += 2;
    a[0] = b[0];
    BOOST_TEST_EQ(a[0], 3);
    a[0] -= 10;
    BOOST_TEST_EQ(a[0], -7);
  }

  // iterators
  {
    auto a = prepare(2);
    for (auto&& x : a) BOOST_TEST_EQ(x, 0);

    std::vector<double> b(2, 1);
    std::copy(b.begin(), b.end(), a.begin());

    const auto aconst = a;
    BOOST_TEST(std::equal(aconst.begin(), aconst.end(), b.begin(), b.end()));

    unlimited_storage_type::iterator it1 = a.begin();
    *it1 = 3;
    unlimited_storage_type::const_iterator it2 = a.begin();
    BOOST_TEST_EQ(*it2, 3);
    unlimited_storage_type::const_iterator it3 = aconst.begin();
    BOOST_TEST_EQ(*it3, 1);
    // unlimited_storage_type::iterator it3 = aconst.begin();
  }

  return boost::report_errors();
}