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

BOOST_AUTO_TEST_SUITE(static_any_construct_and_get_tests)

BOOST_AUTO_TEST_CASE_TEMPLATE(construct_and_get_test, policy, fixtures::test_any_policies)
{
    using any = erasure::static_any<sizeof(int), policy>;

    any a{ 10 };
    BOOST_TEST_REQUIRE(a.is_set());
    BOOST_TEST_REQUIRE(a);
    BOOST_CHECK_THROW(get<float>(a), policies::throwing_type_checking_policy::bad_static_any_get);
    BOOST_TEST(get<int>(a) == 10);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace bk::erasure::test
