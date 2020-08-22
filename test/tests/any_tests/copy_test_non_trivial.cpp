// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "bk/erasure/fixtures/erasure_test_policies.hpp"

#include "bk/erasure/static_any.hpp"

#include <boost/mp11.hpp>
#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>

namespace bk::erasure::test {

BOOST_AUTO_TEST_SUITE(static_any_copy_non_trivial_tests)

BOOST_AUTO_TEST_CASE_TEMPLATE(copy_test_non_trivial, policy, fixtures::test_any_non_trivial_nullable_rebindable_policies)
{
    using any = static_any<sizeof(std::string), policy>;

    any a;
    a = any{ std::string{ "test" } };
    BOOST_TEST_REQUIRE(a);
    BOOST_TEST_REQUIRE(get<std::string>(a) == "test");
    any b{ a };
    BOOST_TEST_REQUIRE(b);
    BOOST_TEST_REQUIRE(get<std::string>(b) == "test");
    get<std::string>(b) = "blah";
    a = b;
    BOOST_TEST_REQUIRE(get<std::string>(a) == "blah");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace bk::erasure::test
