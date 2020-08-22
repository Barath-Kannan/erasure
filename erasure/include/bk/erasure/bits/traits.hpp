// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

namespace bk::erasure::bits {

template <typename...>
struct always_false : std::false_type
{};

template <typename... Args>
using always_false_t = typename always_false<Args...>::type;

template <typename... Args>
constexpr bool always_false_v = always_false<Args...>::value;

} // namespace bk::erasure::bits
