// Copyright 2015-2016 Hans Dembinski
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/core/lightweight_test.hpp>
#include <boost/variant.hpp>
#include <boost/histogram/axis.hpp>
#include <boost/histogram/axis_ostream_operators.hpp>
#include <boost/math/constants/constants.hpp>
#include <limits>

#define BOOST_TEST_NOT(expr) BOOST_TEST(!(expr))

int main() {
    using namespace boost::histogram;
    using axis_t = typename boost::make_variant_over<default_axes>::type;

    // bad_ctors
    {
        BOOST_TEST_THROWS(regular_axis<>(0, 0, 1), std::logic_error);
        BOOST_TEST_THROWS(regular_axis<>(1, 1, -1), std::logic_error);
        BOOST_TEST_THROWS(circular_axis<>(0), std::logic_error);
        BOOST_TEST_THROWS(variable_axis<>({}), std::logic_error);
        BOOST_TEST_THROWS(variable_axis<>({1.0}), std::logic_error);
        BOOST_TEST_THROWS(integer_axis(1, -1), std::logic_error);
        BOOST_TEST_THROWS(category_axis({}), std::logic_error);
    }

    // regular_axis_operators
    {
        regular_axis<> a{4, -2, 2};
        BOOST_TEST_EQ(a[-1], -std::numeric_limits<double>::infinity());
        BOOST_TEST_EQ(a[a.bins() + 1], std::numeric_limits<double>::infinity());
        regular_axis<> b;
        BOOST_TEST_NOT(a == b);
        b = a;
        BOOST_TEST_EQ(a, b);
        b = b;
        BOOST_TEST_EQ(a, b);
        BOOST_TEST_EQ(a.index(-10.), -1);
        BOOST_TEST_EQ(a.index(-2.1), -1);
        BOOST_TEST_EQ(a.index(-2.0), 0);
        BOOST_TEST_EQ(a.index(-1.1), 0);
        BOOST_TEST_EQ(a.index(0.0), 2);
        BOOST_TEST_EQ(a.index(0.9), 2);
        BOOST_TEST_EQ(a.index(1.0), 3);
        BOOST_TEST_EQ(a.index(10.), 4);
        BOOST_TEST_EQ(a.index(std::numeric_limits<double>::infinity()), 4);
        BOOST_TEST_EQ(a.index(-std::numeric_limits<double>::infinity()), -1);
        BOOST_TEST_EQ(a.index(std::numeric_limits<double>::quiet_NaN()), -1);
    }

    // circular_axis_operators
    {
        circular_axis<> a{4};
        BOOST_TEST_EQ(a[-1], a[a.bins() - 1] - a.perimeter());
        circular_axis<> b;
        BOOST_TEST_NOT(a == b);
        b = a;
        BOOST_TEST_EQ(a, b);
        b = b;
        BOOST_TEST_EQ(a, b);
        BOOST_TEST_EQ(a.index(-1.0 * a.perimeter()), 0);
        BOOST_TEST_EQ(a.index(0.0), 0);
        BOOST_TEST_EQ(a.index(0.25 * a.perimeter()), 1);
        BOOST_TEST_EQ(a.index(0.5 * a.perimeter()), 2);
        BOOST_TEST_EQ(a.index(0.75 * a.perimeter()), 3);
        BOOST_TEST_EQ(a.index(1.00 * a.perimeter()), 0);
        BOOST_TEST_EQ(a.index(std::numeric_limits<double>::infinity()), 0);
        BOOST_TEST_EQ(a.index(-std::numeric_limits<double>::infinity()), 0);
        BOOST_TEST_EQ(a.index(std::numeric_limits<double>::quiet_NaN()), 0);
    }

    // variable_axis_operators
    {
        variable_axis<> a{-1, 0, 1};
        BOOST_TEST_EQ(a[-1], -std::numeric_limits<double>::infinity());
        BOOST_TEST_EQ(a[a.bins() + 1], std::numeric_limits<double>::infinity());
        variable_axis<> b;
        BOOST_TEST_NOT(a == b);
        b = a;
        BOOST_TEST_EQ(a, b);
        b = b;
        BOOST_TEST_EQ(a, b);
        variable_axis<> c{-2, 0, 2};
        BOOST_TEST_NOT(a == c);
        BOOST_TEST_EQ(a.index(-10.), -1);
        BOOST_TEST_EQ(a.index(-1.), 0);
        BOOST_TEST_EQ(a.index(0.), 1);
        BOOST_TEST_EQ(a.index(1.), 2);
        BOOST_TEST_EQ(a.index(10.), 2);
        BOOST_TEST_EQ(a.index(std::numeric_limits<double>::infinity()), 2);
        BOOST_TEST_EQ(a.index(-std::numeric_limits<double>::infinity()), -1);
        BOOST_TEST_EQ(a.index(std::numeric_limits<double>::quiet_NaN()), 2);
    }

    // integer_axis_operators
    {
        integer_axis a{-1, 1};
        integer_axis b;
        BOOST_TEST_NOT(a == b);
        b = a;
        BOOST_TEST_EQ(a, b);
        b = b;
        BOOST_TEST_EQ(a, b);
        BOOST_TEST_EQ(a.index(-10), -1);
        BOOST_TEST_EQ(a.index(-2), -1);
        BOOST_TEST_EQ(a.index(-1), 0);
        BOOST_TEST_EQ(a.index(0), 1);
        BOOST_TEST_EQ(a.index(1), 2);
        BOOST_TEST_EQ(a.index(2), 3);
        BOOST_TEST_EQ(a.index(10), 3);
    }

    // category_axis_operators
    {
        category_axis a{{"A", "B", "C"}};
        category_axis b;
        BOOST_TEST_NOT(a == b);
        b = a;
        BOOST_TEST_EQ(a, b);
        b = b;
        BOOST_TEST_EQ(a, b);
        category_axis c = std::move(b);
        BOOST_TEST(c == a);
        BOOST_TEST_NOT(b == a);
        BOOST_TEST_EQ(a.index(0), 0);
        BOOST_TEST_EQ(a.index(1), 1);
        BOOST_TEST_EQ(a.index(2), 2);

        BOOST_TEST_THROWS(a.index(-1), std::out_of_range);
        BOOST_TEST_THROWS(a.index(3), std::out_of_range);
    }

    // axis_t_copyable
    {
        axis_t a(regular_axis<>(2, -1, 1));
        axis_t b(a);
        BOOST_TEST(a == b);
        axis_t c;
        BOOST_TEST_NOT(a == c);
        c = a;
        BOOST_TEST(a == c);
    }

    // axis_t_movable
    {
        axis_t a(regular_axis<>(2, -1, 1));
        axis_t r(a);
        axis_t b(std::move(a));
        BOOST_TEST(b == r);
        axis_t c;
        BOOST_TEST_NOT(a == c);
        c = std::move(b);
        BOOST_TEST(c == r);
    }

    // axis_t_streamable
    {
        std::vector<axis_t> axes;
        axes.push_back(regular_axis<>{2, -1, 1, "regular", false});
        axes.push_back(circular_axis<>{4, 0.1, 1.0, "polar"});
        axes.push_back(variable_axis<>{{-1, 0, 1}, "variable", false});
        axes.push_back(category_axis{"A", "B", "C"});
        axes.push_back(integer_axis{-1, 1, "integer", false});
        std::ostringstream os;
        for(const auto& a : axes)
            os << a;
        BOOST_TEST(!os.str().empty());
    }

    // axis_t_equal_comparable
    {
        std::vector<axis_t> axes;
        axes.push_back(regular_axis<>{2, -1, 1});
        axes.push_back(circular_axis<>{4});
        axes.push_back(variable_axis<>{-1, 0, 1});
        axes.push_back(category_axis{"A", "B", "C"});
        axes.push_back(integer_axis{-1, 1});
        for (const auto& a : axes) {
            BOOST_TEST(!(a == axis_t()));
            BOOST_TEST_EQ(a, a);
        }
        BOOST_TEST_NOT(axes == std::vector<axis_t>());
        BOOST_TEST(axes == std::vector<axis_t>(axes));
    }

    return boost::report_errors();
}
