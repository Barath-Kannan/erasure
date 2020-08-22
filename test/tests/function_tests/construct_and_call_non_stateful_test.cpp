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

BOOST_AUTO_TEST_SUITE(static_function_construct_and_call_non_stateful_tests)

BOOST_AUTO_TEST_CASE_TEMPLATE(construct_and_call_non_stateful_test, policy, fixtures::test_function_policies)
{
    using function = static_function<int(int), 0, policy>;
    function f{ [](int x) { return x + 1; } };
    BOOST_TEST_REQUIRE(f);
    BOOST_TEST(f(1) == 2);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace bk::erasure::test
