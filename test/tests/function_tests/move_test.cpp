// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "bk/erasure/fixtures/erasure_test_policies.hpp"
#include "bk/erasure/static_function.hpp"

#include <boost/mp11.hpp>
#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>

namespace bk::erasure::test {

BOOST_AUTO_TEST_SUITE(static_function_move_tests)

BOOST_AUTO_TEST_CASE_TEMPLATE(move_test, policy, fixtures::test_function_nullable_rebindable_policies)
{
    using function = static_function<int(int), 8, policy>;

    function a{};
    BOOST_TEST_REQUIRE(!a);

    function b = a;
    BOOST_TEST_REQUIRE(!b);

    a = [x = 5](int y) { return x + y; };

    BOOST_TEST_REQUIRE(a);
    BOOST_TEST_REQUIRE(a(1) == 6);
    BOOST_TEST_REQUIRE(!b);
    b = a;
    BOOST_TEST_REQUIRE(b);
    BOOST_TEST_REQUIRE(b(1) == 6);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace bk::erasure::test
