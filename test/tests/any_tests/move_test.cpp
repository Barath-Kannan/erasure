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

BOOST_AUTO_TEST_SUITE(static_any_move_tests)

BOOST_AUTO_TEST_CASE_TEMPLATE(move_test, policy, fixtures::test_any_nullable_policies)
{
    using any = static_any<sizeof(int), policy>;

    any a{};
    BOOST_TEST_REQUIRE(!a);

    any b = std::move(a);
    BOOST_TEST_REQUIRE(!b);

    a = any{ 5 };
    BOOST_TEST_REQUIRE(a);
    BOOST_TEST_REQUIRE((a.type() == typeid(int)));
    BOOST_TEST_REQUIRE(get<int>(a) == 5);
    BOOST_TEST_REQUIRE(!b);
    b = std::move(a);
    BOOST_TEST_REQUIRE(b);
    if constexpr (!std::is_same_v<typename policy::move_policy, policies::trivial_move_policy>)
    {
        // trivial moves will not zero the source memory as that is not a trivial_ operation
        // We will only have this guarantee that the moved-from any will be zero'd when dealing
        // with using a non-trivial_ move policy.
        BOOST_TEST_REQUIRE(!a);
    }
    BOOST_TEST_REQUIRE((b.type() == typeid(int)));
    BOOST_TEST_REQUIRE(get<int>(b) == 5);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace bk::erasure::test
