// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/bits/signature.hpp"
#include "bk/erasure/bits/swallow.hpp"
#include "bk/erasure/bits/traits.hpp"
#include "bk/erasure/policies/enums.hpp"

#include <boost/mp11/utility.hpp>

#include <cassert>
#include <cstring>

namespace bk::erasure::policies {

namespace detail::move {

template <
    typename Accessor,
    triviality = triviality::trivial,
    nullability = nullability::non_nullable,
    rebindability = rebindability::non_rebindable,
    exception_specification = exception_specification::no_except,
    function_pointer_lookup = function_pointer_lookup::not_present>
struct type_erased_move_policy
{
    constexpr type_erased_move_policy() noexcept = default;
    constexpr type_erased_move_policy(bits::swallow) noexcept {}
    ~type_erased_move_policy() noexcept = default;

    static constexpr void move(const std::byte*) noexcept {}

    template <typename V>
    static constexpr void bind_move() noexcept {}

    template <typename V>
    static constexpr void check_constraints() noexcept
    {
        static_assert(std::is_trivially_move_constructible_v<V>);
    }
};

static_assert(std::is_trivial_v<type_erased_move_policy<void>>);

template <exception_specification E>
struct move_holder
{
    move_holder() = default;

    template <typename V>
    move_holder(boost::mp11::mp_identity<V>) noexcept :
        _move{ bits::generate_move<V, E>() }
    {}

    template <typename V>
    void bind_move()
    {
        _move = bits::generate_move<V, E>();
    }

    bits::move_signature<E> get_move() const
    {
        return _move;
    }

private:
    bits::move_signature<E> _move{ nullptr };
};

struct empty_move_holder
{
    constexpr empty_move_holder(bits::swallow) noexcept {}
    constexpr empty_move_holder() noexcept = default;

    constexpr void zeromove() noexcept {}
};

template <typename Accessor, nullability N, rebindability R, exception_specification E, function_pointer_lookup F>
struct type_erased_move_policy<Accessor, triviality::non_trivial, N, R, E, F>
    : private std::conditional_t<F == function_pointer_lookup::inplace, move_holder<E>, empty_move_holder>
{
    using base = std::conditional_t<F == function_pointer_lookup::inplace, move_holder<E>, empty_move_holder>;
    using base::base;

    bits::move_signature<E> get_move() const
    {
        if constexpr (F == function_pointer_lookup::inplace)
        {
            return base::get_move();
        }
        else if constexpr (F == function_pointer_lookup::pointer_switch)
        {
            return reinterpret_cast<bits::move_signature<E>>(
                static_cast<const Accessor&>(*this).get_switch_function()(operation::move));
        }
        else if constexpr (F == function_pointer_lookup::pointer_table)
        {
            return static_cast<const Accessor&>(*this).get_pointer_table().template get<operation::move>();
        }
        else
        {
            static_assert(bits::always_false_v<Accessor>);
        }
    }

    template <typename V>
    void bind_move() noexcept
    {
        if constexpr (F == function_pointer_lookup::inplace)
        {
            base::template bind_move<V>();
        }
    }

    void move(std::byte* other) noexcept(E == exception_specification::no_except)
    {
        auto move = get_move();
        auto data = static_cast<Accessor*>(this)->data();
        if constexpr (N == nullability::non_nullable)
        {
            assert(move);
            move(other, data);
        }
        else
        {
            if (move)
                move(other, data);
        }
    }

    template <typename V>
    static constexpr void check_constraints() noexcept
    {}
};

} // namespace detail::move

struct trivial_move_policy
    : policy_description<exception_specification::no_except, function_pointer_lookup::not_present>
{
    template <typename BindingPolicy, typename Accessor>
    using policy = detail::move::type_erased_move_policy<Accessor, triviality::trivial>;
};

template <exception_specification E, function_pointer_lookup F>
struct non_trivial_move_policy
    : policy_description<E, F>
{
    template <typename BindingPolicy, typename Accessor>
    using policy =
        detail::move::type_erased_move_policy<
            Accessor,
            triviality::non_trivial,
            BindingPolicy::nullability,
            BindingPolicy::rebindability,
            E,
            F>;
};

} // namespace bk::erasure::policies
