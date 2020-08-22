// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/policies/enums.hpp"

namespace bk::erasure::policies {

template <nullability N, rebindability R>
struct binding_policy
{
    static constexpr auto nullability = N;
    static constexpr auto rebindability = R;
};

} // namespace bk::erasure::policies
