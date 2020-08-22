// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cassert>
#include <exception>

namespace bk::erasure::policies {

struct assert_callable_checking_policy
{
    template <typename CallablePtr>
    static void check(const CallablePtr& p) noexcept
    {
        assert(p != nullptr);
    }
};

struct throwing_callable_checking_policy
{
    class bad_static_function_call final
        : public std::exception
    {
        const char* what() const noexcept override
        {
            return "bad static_function_call";
        }
    };

    template <typename CallablePtr>
    static void check(const CallablePtr& p)
    {
        if (p == nullptr)
            throw bad_static_function_call();
    }
};

} // namespace bk::erasure::policies
