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

BOOST_AUTO_TEST_SUITE(static_function_destructor_tests)

BOOST_AUTO_TEST_CASE_TEMPLATE(destructor_test, policy, fixtures::test_function_non_trivial_nullable_rebindable_policies)
{
    struct callable_destructor
    {
        bool& _flag;
        ~callable_destructor() noexcept { _flag = true; }
        void operator()() noexcept {}
    };

    using function = static_function<void(), sizeof(callable_destructor), policy>;

    // test destructor
    {
        bool destructor_called = false;
        {
            function f{ std::in_place_type<callable_destructor>, destructor_called };
            BOOST_TEST_REQUIRE(!destructor_called);
        }
        BOOST_TEST_REQUIRE(destructor_called);
    }

    // test rebind
    {
        bool destructor_called = false;
        function f{ std::in_place_type<callable_destructor>, destructor_called };
        BOOST_TEST_REQUIRE(!destructor_called);
        f = []() {};
        BOOST_TEST_REQUIRE(destructor_called);
    }

    // test reset
    {
        bool destructor_called = false;
        function f{ std::in_place_type<callable_destructor>, destructor_called };
        BOOST_TEST_REQUIRE(!destructor_called);
        f = nullptr;
        BOOST_TEST_REQUIRE(destructor_called);
    }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace bk::erasure::test
