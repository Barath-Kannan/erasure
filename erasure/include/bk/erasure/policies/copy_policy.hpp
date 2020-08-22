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
#include <type_traits>

namespace bk::erasure::policies {

namespace detail::copy {

template <
    typename Accessor,
    triviality = triviality::trivial,
    nullability = nullability::non_nullable,
    rebindability = rebindability::non_rebindable,
    exception_specification = exception_specification::no_except,
    function_pointer_lookup = function_pointer_lookup::not_present>
struct type_erased_copy_policy
{
    constexpr type_erased_copy_policy() noexcept = default;
    constexpr type_erased_copy_policy(bits::swallow) noexcept {}
    ~type_erased_copy_policy() noexcept = default;

    static constexpr void copy(const std::byte*) noexcept {}

    template <typename V>
    static constexpr void bind_copy() noexcept {}

    template <typename V>
    static constexpr void check_constraints() noexcept
    {
        static_assert(std::is_trivially_copyable_v<V>);
    }
};

static_assert(std::is_trivial_v<type_erased_copy_policy<void>>);

template <exception_specification E>
struct copy_holder
{
    copy_holder() = default;

    template <typename V>
    copy_holder(boost::mp11::mp_identity<V>) noexcept :
        _copy{ bits::generate_copy<V, E>() }
    {}

    template <typename V>
    void bind_copy()
    {
        _copy = bits::generate_copy<V, E>();
    }

    bits::copy_signature<E> get_copy() const
    {
        return _copy;
    }

private:
    bits::copy_signature<E> _copy{ nullptr };
};

struct empty_copy_holder
{
    constexpr empty_copy_holder(bits::swallow) noexcept {}
    constexpr empty_copy_holder() noexcept = default;
};

template <typename Accessor, nullability N, rebindability R, exception_specification E, function_pointer_lookup F>
struct type_erased_copy_policy<Accessor, triviality::non_trivial, N, R, E, F>
    : private std::conditional_t<F == function_pointer_lookup::inplace, copy_holder<E>, empty_copy_holder>
{
    using base = std::conditional_t<F == function_pointer_lookup::inplace, copy_holder<E>, empty_copy_holder>;
    using base::base;

    bits::copy_signature<E> get_copy() const
    {
        if constexpr (F == function_pointer_lookup::inplace)
        {
            return base::get_copy();
        }
        else if constexpr (F == function_pointer_lookup::pointer_switch)
        {
            return reinterpret_cast<bits::copy_signature<E>>(
                static_cast<const Accessor&>(*this).get_switch_function()(operation::copy));
        }
        else if constexpr (F == function_pointer_lookup::pointer_table)
        {
            return static_cast<const Accessor&>(*this).get_pointer_table().template get<operation::copy>();
        }
        else
        {
            static_assert(bits::always_false_v<Accessor>);
        }
    }

    template <typename V>
    void bind_copy() noexcept
    {
        if constexpr (F == function_pointer_lookup::inplace)
        {
            base::template bind_copy<V>();
        }
    }

    void copy(const std::byte* other) noexcept(E == exception_specification::no_except)
    {
        auto copy = get_copy();
        auto data = static_cast<Accessor*>(this)->data();
        if constexpr (N == nullability::non_nullable)
        {
            assert(copy);
            copy(other, data);
        }
        else
        {
            if (copy)
                copy(other, data);
        }
    }

    template <typename V>
    static constexpr void check_constraints() noexcept
    {}
};

} // namespace detail::copy

struct trivial_copy_policy
    : policy_description<exception_specification::no_except, function_pointer_lookup::not_present>
{
    template <typename BindingPolicy, typename Accessor>
    using policy = detail::copy::type_erased_copy_policy<Accessor, triviality::trivial>;
};

template <exception_specification E, function_pointer_lookup F>
struct non_trivial_copy_policy
    : policy_description<E, F>
{
    template <typename BindingPolicy, typename Accessor>
    using policy =
        detail::copy::type_erased_copy_policy<
            Accessor,
            triviality::non_trivial,
            BindingPolicy::nullability,
            BindingPolicy::rebindability,
            E,
            F>;
};

} // namespace bk::erasure::policies
