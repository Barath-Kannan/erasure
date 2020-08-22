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

BOOST_AUTO_TEST_SUITE(static_any_basic_is_empty_tests)

BOOST_AUTO_TEST_CASE_TEMPLATE(basic_is_empty_test, policy, fixtures::test_any_nullable_policies)
{
    auto run_test = [](auto& a) {
        BOOST_TEST_REQUIRE(!a.is_set());
        BOOST_TEST_REQUIRE(!a);
        BOOST_CHECK_THROW(a.type(), policies::throwing_type_checking_policy::bad_static_any_get);
        BOOST_CHECK_THROW(a.template get<int>(), policies::throwing_type_checking_policy::bad_static_any_get);
    };

    if constexpr (policy::function_pointer_lookup == policies::function_pointer_lookup::not_present)
    {
        // triviality -> uninitialized memory for the pointers
        // so the pointers will be filled with junk. In order to
        // for the checks to work correctly, we must zero it
        static_any<1, policy> any{};
        run_test(any);
    }
    else
    {
        static_any<1, policy> any;
        run_test(any);
    }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace bk::erasure::test
