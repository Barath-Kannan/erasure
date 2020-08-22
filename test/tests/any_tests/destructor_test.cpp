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

BOOST_AUTO_TEST_SUITE(static_any_destructor_tests)

BOOST_AUTO_TEST_CASE_TEMPLATE(destructor_test, policy, fixtures::test_any_non_trivial_nullable_rebindable_policies)
{
    struct destructor_type
    {
        bool& _flag;
        ~destructor_type() noexcept { _flag = true; }
    };

    using any = static_any<sizeof(destructor_type), policy>;

    // test destructor
    {
        bool destructor_called = false;
        {
            any a{ std::in_place_type<destructor_type>, destructor_called };
            BOOST_TEST_REQUIRE(!destructor_called);
        }
        BOOST_TEST_REQUIRE(destructor_called);
    }

    // test rebind
    {
        bool destructor_called = false;
        any a{ std::in_place_type<destructor_type>, destructor_called };
        BOOST_TEST_REQUIRE(!destructor_called);
        a = 1;
        BOOST_TEST_REQUIRE(destructor_called);
    }

    // test reset
    {
        bool destructor_called = false;
        any a;
        a.template emplace<destructor_type>(destructor_called);
        BOOST_TEST_REQUIRE(!destructor_called);
        a.reset();
        BOOST_TEST_REQUIRE(destructor_called);
    }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace bk::erasure::test
