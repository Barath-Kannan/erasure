// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cassert>
#include <exception>

namespace bk::erasure::policies {

struct assert_type_checking_policy
{
    static void check(const std::type_info& stored, const std::type_info& target) noexcept
    {
        assert(stored == target);
    }

    static void check(const std::type_info* stored) noexcept
    {
        assert(stored);
    }
};

struct throwing_type_checking_policy
{
    class bad_static_any_get final
        : public std::exception
    {
        const char* what() const noexcept override
        {
            return "bad static_any::get";
        }
    };

    static void check(const std::type_info& stored, const std::type_info& target)
    {
        if (stored != target)
            throw bad_static_any_get{};
    }

    static void check(const std::type_info* stored)
    {
        if (!stored)
            throw bad_static_any_get{};
    }
};

} // namespace bk::erasure::policies
