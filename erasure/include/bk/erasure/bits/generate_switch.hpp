// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/bits/detectors.hpp"
#include "bk/erasure/bits/signature.hpp"
#include "bk/erasure/policies/enums.hpp"

#include <cstdlib>
#include <type_traits>

namespace bk::erasure::bits {

namespace detail::switcher {

template <typename Policy, typename HeldType, typename Signature = void>
static void* switch_on_operation(policies::operation op) noexcept
{
    using function_pointer_lookup = policies::function_pointer_lookup;
    using operation = policies::operation;

    switch (op)
    {
    case operation::call:
    {
        if constexpr (!std::is_same_v<Signature, void>)
        {
            if constexpr (has_call<Policy>)
            {
                if constexpr (function_pointer_lookup::pointer_switch == Policy::call_policy::function_pointer_policy)
                {
                    return reinterpret_cast<void*>(generate_call<HeldType, Signature, Policy::call_policy::exception_policy>());
                }
            }
        }
    }
    break;
    case operation::destroy:
    {
        if constexpr (function_pointer_lookup::pointer_switch == Policy::destructor_policy::function_pointer_policy)
        {
            return reinterpret_cast<void*>(generate_destroy<HeldType, Policy::destructor_policy::exception_policy>());
        }
    }
    break;
    case operation::move:
    {
        if constexpr (has_move<Policy>)
        {
            if constexpr (function_pointer_lookup::pointer_switch == Policy::move_policy::function_pointer_policy)
            {
                return reinterpret_cast<void*>(generate_move<HeldType, Policy::move_policy::exception_policy>());
            }
        }
    }
    break;
    case operation::copy:
    {
        if constexpr (has_copy<Policy>)
        {
            if constexpr (function_pointer_lookup::pointer_switch == Policy::copy_policy::function_pointer_policy)
            {
                return reinterpret_cast<void*>(generate_copy<HeldType, Policy::copy_policy::exception_policy>());
            }
        }
    }
    break;
    case operation::type_id:
    {
        if constexpr (has_typeid<Policy>)
        {
            if constexpr (function_pointer_lookup::pointer_switch == Policy::type_id_policy::function_pointer_policy)
            {
                return const_cast<void*>(reinterpret_cast<const void*>(generate_typeid<HeldType>()));
            }
        }
    }
    break;
    }

    std::abort();
};

} // namespace detail::switcher

using switch_signature = void* (*)(policies::operation) noexcept;

template <typename Policy, typename HeldType, typename Signature = void>
constexpr switch_signature generate_switch() noexcept
{
    return [](policies::operation op) noexcept->void*
    {
        return detail::switcher::switch_on_operation<Policy, HeldType, Signature>(op);
    };
}

constexpr switch_signature generate_null_switch() noexcept
{
    return [](policies::operation) noexcept->void*
    {
        return nullptr;
    };
}

} // namespace bk::erasure::bits
