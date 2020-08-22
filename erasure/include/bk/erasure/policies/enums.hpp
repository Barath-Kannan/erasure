// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace bk::erasure::policies {

enum class triviality
{
    trivial,
    non_trivial
};

enum class nullability
{
    nullable,
    non_nullable
};

enum class rebindability
{
    rebindable,
    non_rebindable
};

enum class exception_specification
{
    no_except,
    throwing
};

enum class function_pointer_lookup
{
    inplace,
    pointer_table,
    pointer_switch,
    not_present
};

enum class operation
{
    call,
    destroy,
    move,
    copy,
    type_id
};

template <exception_specification E, function_pointer_lookup F>
struct policy_description
{
    static constexpr auto function_pointer_policy = F;
    static constexpr auto exception_policy = E;
};

} // namespace bk::erasure::policies
