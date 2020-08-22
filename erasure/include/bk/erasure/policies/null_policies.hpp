// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/bits/signature.hpp"
#include "bk/erasure/bits/swallow.hpp"
#include "bk/erasure/policies/enums.hpp"

namespace bk::erasure::policies {

struct non_callable_policy
    : policy_description<exception_specification::no_except, function_pointer_lookup::not_present>
{
    template <typename binding_policy, typename Accessor, typename Signature>
    struct policy
    {
        constexpr policy() noexcept = default;
        constexpr policy(bits::swallow) noexcept {}
        ~policy() noexcept = default;

        static constexpr bits::call_signature<Signature, exception_specification::no_except>
        get_call() noexcept { return nullptr; }
        static constexpr void bind_call() noexcept {}
    };
};

struct non_copyable_policy
    : policy_description<exception_specification::no_except, function_pointer_lookup::not_present>
{
    template <typename binding_policy, typename Accessor>
    struct policy
    {
        constexpr policy() noexcept = default;
        constexpr policy(bits::swallow) noexcept {}
        ~policy() noexcept = default;

        template <typename... Args>
        static constexpr void copy(Args&&...) noexcept {}
        static constexpr void bind_copy() noexcept {}
        template <typename V>
        static constexpr void check_constraints() noexcept {}
    };
};

struct non_moveable_policy
    : policy_description<exception_specification::no_except, function_pointer_lookup::not_present>
{
    template <typename binding_policy, typename Accessor>
    struct policy
    {
        constexpr policy() noexcept = default;
        constexpr policy(bits::swallow) noexcept {}
        ~policy() noexcept = default;

        template <typename Args>
        static constexpr void move(Args&&...) noexcept {}
        static constexpr void bind_move() noexcept {}
        template <typename V>
        static constexpr void check_constraints() noexcept {}
    };
};

struct no_typeid_policy
    : policy_description<exception_specification::no_except, function_pointer_lookup::not_present>
{
    template <typename binding_policy, typename Accessor>
    struct policy
    {
        constexpr policy() noexcept = default;
        constexpr policy(bits::swallow) noexcept {}
        ~policy() noexcept = default;

        template <typename V>
        static constexpr void bind_type_id() noexcept {}

        static constexpr const void* get_type_info() noexcept { return nullptr; }
    };
};

} // namespace bk::erasure::policies
