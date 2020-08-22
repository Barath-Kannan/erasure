// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/bits/detectors.hpp"
#include "bk/erasure/bits/signature.hpp"
#include "bk/erasure/policies/enums.hpp"

#include <boost/mp11.hpp>
#include <boost/type_traits/is_detected.hpp>

#include <cassert>

namespace bk::erasure::bits {

template <typename Generator>
struct pointer_holder
{
    template <typename T>
    constexpr pointer_holder(boost::mp11::mp_identity<T>) noexcept :
        _t{ Generator::template generate<T>() }
    {}

    constexpr pointer_holder() noexcept = default;

    typename Generator::type _t{ nullptr };
};

struct empty_pointer_holder
{};

template <typename Policy, typename Generator>
using pointer_holder_type =
    std::conditional_t<
        Policy::function_pointer_policy == policies::function_pointer_lookup::pointer_table,
        pointer_holder<Generator>,
        empty_pointer_holder>;

template <typename Policy, typename Signature = void>
struct pointer_table
{
    using copy_policy = get_copy_policy<Policy>;
    using copy_generator = detail::copy_signature<copy_policy::exception_policy == policies::exception_specification::no_except>;
    using copy_holder_type = pointer_holder_type<copy_policy, copy_generator>;

    using move_policy = get_move_policy<Policy>;
    using move_generator = detail::move_signature<move_policy::exception_policy == policies::exception_specification::no_except>;
    using move_holder_type = pointer_holder_type<move_policy, move_generator>;

    using destructor_policy = typename Policy::destructor_policy;
    using destroy_generator = detail::destroy_signature<destructor_policy::exception_policy == policies::exception_specification::no_except>;
    using destroy_holder_type = pointer_holder_type<destructor_policy, destroy_generator>;

    using type_id_policy = get_type_id_policy<Policy>;
    using typeid_generator = detail::typeid_signature;
    using typeid_holder_type = pointer_holder_type<type_id_policy, typeid_generator>;

    using call_policy = get_call_policy<Policy>;
    using call_generator = detail::call_signature<Signature, call_policy::exception_policy == policies::exception_specification::no_except>;
    using call_holder_type = pointer_holder_type<call_policy, call_generator>;

    using holders =
        boost::mp11::mp_remove<
            std::tuple<
                copy_holder_type,
                move_holder_type,
                destroy_holder_type,
                typeid_holder_type,
                call_holder_type>,
            empty_pointer_holder>;

    template <typename V>
    constexpr pointer_table(boost::mp11::mp_identity<V>) noexcept :
        _holders{
            std::apply(
                [](auto... identities) {
                    [[maybe_unused]] auto f = [](auto i) { return typename decltype(i)::type{ boost::mp11::mp_identity<V>{} }; };
                    return holders{ f(identities)... };
                },
                boost::mp11::mp_transform<boost::mp11::mp_identity, holders>{})
        }
    {}

    constexpr pointer_table() noexcept
    {
        //need to override the noexcept specification of tuples so we can't default this
    }

    template <policies::operation O>
    auto get() const noexcept
    {
        using mapped_type =
            boost::mp11::mp_cond<
                boost::mp11::mp_bool<O == policies::operation::copy>,
                copy_holder_type,
                boost::mp11::mp_bool<O == policies::operation::move>,
                move_holder_type,
                boost::mp11::mp_bool<O == policies::operation::destroy>,
                destroy_holder_type,
                boost::mp11::mp_bool<O == policies::operation::type_id>,
                typeid_holder_type,
                boost::mp11::mp_bool<O == policies::operation::call>,
                call_holder_type>;

        return std::get<mapped_type>(_holders)._t;
    }

    holders _holders{};
};

template <typename Policy, typename HeldType, typename Signature = void>
struct pointer_table_impl
{
    static constexpr pointer_table<Policy, Signature> _s_table{ boost::mp11::mp_identity<HeldType>{} };
};

template <typename Policy, typename Signature = void>
struct null_pointer_table_impl
{
    static constexpr pointer_table<Policy, Signature> _s_table{};
};

template <typename Policy, typename HeldType, typename Signature = void>
static const pointer_table<Policy, Signature>* generate_out_of_line_pointer_table() noexcept
{
    static pointer_table_impl<Policy, HeldType, Signature> impl{};
    return &impl._s_table;
}

// FIXME: Use this for inline function pointer lookup
template <typename Policy, typename HeldType, typename Signature = void>
static const pointer_table<Policy, Signature> generate_pointer_table() noexcept
{
    return pointer_table_impl<Policy, HeldType, Signature>{}._s_table;
}

template <typename Policy, typename Signature = void>
static const pointer_table<Policy, Signature>* generate_null_pointer_table() noexcept
{
    static null_pointer_table_impl<Policy, Signature> impl{};
    return &impl._s_table;
}

} // namespace bk::erasure::bits
