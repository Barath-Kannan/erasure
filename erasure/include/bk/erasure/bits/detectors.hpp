// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/policies/null_policies.hpp"

#include <boost/type_traits/detected_or.hpp>
#include <boost/type_traits/is_detected.hpp>

namespace bk::erasure::bits {

template <typename P>
using call_detector = typename P::call_policy;

template <typename P>
using typeid_detector = typename P::type_id_policy;

template <typename P>
using copy_detector = typename P::copy_policy;

template <typename P>
using move_detector = typename P::move_policy;

template <typename P>
constexpr bool has_call = boost::is_detected_v<call_detector, P>;

template <typename P>
constexpr bool has_typeid = boost::is_detected_v<typeid_detector, P>;

template <typename P>
constexpr bool has_copy = boost::is_detected_v<copy_detector, P>;

template <typename P>
constexpr bool has_move = boost::is_detected_v<move_detector, P>;

template <typename P>
using get_copy_policy = boost::detected_or_t<policies::non_copyable_policy, copy_detector, P>;

template <typename P>
using get_move_policy = boost::detected_or_t<policies::non_moveable_policy, move_detector, P>;

template <typename P>
using get_call_policy = boost::detected_or_t<policies::non_callable_policy, call_detector, P>;

template <typename P>
using get_type_id_policy = boost::detected_or_t<policies::no_typeid_policy, typeid_detector, P>;

} // namespace bk::erasure::bits
