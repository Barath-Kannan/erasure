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

BOOST_AUTO_TEST_SUITE(static_function_copy_non_trivial_tests)

BOOST_AUTO_TEST_CASE_TEMPLATE(copy_test_non_trivial, policy, fixtures::test_function_non_trivial_nullable_rebindable_policies)
{
    using function = static_function<std::string(const std::string&), sizeof(std::string), policy>;

    function a{};
    BOOST_TEST_REQUIRE(!a);

    function b = a;
    BOOST_TEST_REQUIRE(!b);

    a = [x = std::string{ "abcdefg" }](const std::string& y) { return x + y; };

    BOOST_TEST_REQUIRE(a);
    BOOST_TEST_REQUIRE(a("hijk") == "abcdefghijk");
    BOOST_TEST_REQUIRE(!b);
    b = a;
    BOOST_TEST_REQUIRE(b);
    BOOST_TEST_REQUIRE(b("hi") == "abcdefghi");

    b = [x = std::string{ "zyxwvut" }](const std::string& y) { return y + x; };
    a = b;
    BOOST_TEST_REQUIRE(a("srqponm") == "srqponmzyxwvut");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace bk::erasure::test
