// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/bits/detectors.hpp"
#include "bk/erasure/bits/generate_pointer_table.hpp"
#include "bk/erasure/policies/enums.hpp"

#include <boost/mp11.hpp>

namespace bk::erasure::bits {

namespace detail {

template <typename Policy>
using requires_pointer_table = std::bool_constant<Policy::function_pointer_policy == policies::function_pointer_lookup::pointer_table>;

template <typename PolicySet>
constexpr bool policy_requires_pointer_table =
    boost::mp11::mp_any_of<
        std::tuple<
            typename PolicySet::destructor_policy,
            get_copy_policy<PolicySet>,
            get_move_policy<PolicySet>,
            get_call_policy<PolicySet>,
            get_type_id_policy<PolicySet>>,
        requires_pointer_table>::value;

struct empty_pointer_table_holder
{
    static constexpr bool has_pointer_table = false;

    template <typename... Args>
    constexpr empty_pointer_table_holder(Args&&...) noexcept {}

    constexpr empty_pointer_table_holder() noexcept = default;

    template <typename... Args>
    static constexpr void bind_pointer_table() noexcept {}

    static constexpr void zero_pointer_table() noexcept {}
};

template <typename Policy, typename Signature = void>
struct non_empty_pointer_table_holder
{
    static constexpr bool has_pointer_table = true;
    constexpr non_empty_pointer_table_holder() noexcept = default;

    template <typename HeldType>
    constexpr non_empty_pointer_table_holder(
        boost::mp11::mp_identity<HeldType>) :
        _pointer_table{ generate_out_of_line_pointer_table<Policy, HeldType, Signature>() }
    {}

    template <typename HeldType>
    void bind_pointer_table() noexcept
    {
        _pointer_table = generate_out_of_line_pointer_table<Policy, HeldType, Signature>();
    }

    void zero_pointer_table() noexcept
    {
        _pointer_table = generate_null_pointer_table<Policy, Signature>();
    }

    const pointer_table<Policy, Signature>& get_pointer_table() const noexcept { return *_pointer_table; }

private:
    const pointer_table<Policy, Signature>* _pointer_table{ generate_null_pointer_table<Policy, Signature>() };
};

} // namespace detail

template <typename Policy, typename Signature = void>
using pointer_table_holder =
    std::conditional_t<
        detail::policy_requires_pointer_table<Policy>,
        detail::non_empty_pointer_table_holder<Policy, Signature>,
        detail::empty_pointer_table_holder>;

} // namespace bk::erasure::bits
