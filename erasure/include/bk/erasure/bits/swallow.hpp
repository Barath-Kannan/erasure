// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace bk::erasure::bits {

struct swallow
{
    template <typename... Args>
    constexpr swallow(Args&&...)
    {}
};

} // namespace bk::erasure::bits
