// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/bits/detectors.hpp"
#include "bk/erasure/bits/generate_switch.hpp"
#include "bk/erasure/policies/enums.hpp"

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/utility.hpp>

#include <type_traits>

namespace bk::erasure::bits {

namespace detail {

template <typename Policy>
using requires_switch_pointer = std::bool_constant<Policy::function_pointer_policy == policies::function_pointer_lookup::pointer_switch>;

template <typename PolicySet>
constexpr bool policy_requires_switch_pointer =
    boost::mp11::mp_any_of<
        std::tuple<
            typename PolicySet::destructor_policy,
            get_copy_policy<PolicySet>,
            get_move_policy<PolicySet>,
            get_call_policy<PolicySet>,
            get_type_id_policy<PolicySet>>,
        requires_switch_pointer>::value;

struct empty_switch_holder
{
    static constexpr bool has_switch_function = false;

    template <typename... Args>
    constexpr empty_switch_holder(Args&&...) noexcept {}

    constexpr empty_switch_holder() noexcept = default;

    template <typename... Args>
    static constexpr void bind_switch_function() noexcept {}

    static constexpr void zero_switch_function() noexcept {}
};

static_assert(std::is_trivial_v<empty_switch_holder>);

struct non_empty_switch_holder
{
    static constexpr bool has_switch_function = true;
    constexpr non_empty_switch_holder() noexcept = default;

    template <typename Policy, typename HeldType, typename Signature = void>
    constexpr non_empty_switch_holder(
        boost::mp11::mp_identity<Policy>,
        boost::mp11::mp_identity<HeldType>,
        boost::mp11::mp_identity<Signature> = {}) noexcept :
        _switch_function{ generate_switch<Policy, HeldType, Signature>() }
    {}

    template <typename Policy, typename HeldType, typename Signature = void>
    void bind_switch_function() noexcept
    {
        _switch_function = generate_switch<Policy, HeldType, Signature>();
    }

    void zero_switch_function() noexcept
    {
        _switch_function = generate_null_switch();
    }

    switch_signature get_switch_function() const noexcept { return _switch_function; }

private:
    switch_signature _switch_function{ generate_null_switch() };
};

} // namespace detail

template <typename Policy>
using switch_function_pointer_holder =
    std::conditional_t<
        detail::policy_requires_switch_pointer<Policy>,
        detail::non_empty_switch_holder,
        detail::empty_switch_holder>;

} // namespace bk::erasure::bits
