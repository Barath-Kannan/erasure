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

BOOST_AUTO_TEST_SUITE(static_function_basic_is_empty_tests)

BOOST_AUTO_TEST_CASE_TEMPLATE(basic_is_empty_test, policy, fixtures::test_function_nullable_policies)
{
    auto run_test = [](auto& f) {
        BOOST_TEST_REQUIRE(!f);
        BOOST_TEST_REQUIRE(f == nullptr);
        BOOST_CHECK_THROW(f(), policies::throwing_callable_checking_policy::bad_static_function_call);
    };

    if constexpr (policy::function_pointer_lookup == policies::function_pointer_lookup::not_present)
    {
        // trivial_ity -> uninitialized memory for the pointers
        // so the pointers will be filled with junk. In order to
        // for the checks to work correctly, we must zero it
        static_function<void(), 1, policy> any{};
        run_test(any);
    }
    else
    {
        [[maybe_unused]] static_function<void(), 1, policy> any;
        run_test(any);
    }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace bk::erasure::test
