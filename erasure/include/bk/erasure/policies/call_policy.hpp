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

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace bk::erasure::policies {

namespace detail::call {

template <typename Signature, exception_specification E>
struct trivial_call_holder
{
    trivial_call_holder() noexcept = default;

    template <typename V>
    trivial_call_holder(boost::mp11::mp_identity<V>) noexcept :
        _call{ bits::generate_call<V, Signature, E>() }
    {}

    bits::call_signature<Signature, E> _call;
};

static_assert(std::is_trivial_v<trivial_call_holder<void(), exception_specification::no_except>>);

template <typename Signature, exception_specification E>
struct base_call_holder
{
    constexpr base_call_holder() noexcept = default;

    template <typename V>
    base_call_holder(boost::mp11::mp_identity<V>) noexcept :
        _call{ bits::generate_call<V, Signature, E>() }
    {}

    bits::call_signature<Signature, E> _call{ nullptr };
};

template <triviality T, nullability N, rebindability R, exception_specification E, typename Signature>
struct call_holder
    : std::conditional_t<T == triviality::trivial, trivial_call_holder<Signature, E>, base_call_holder<Signature, E>>
{
    using base = std::conditional_t<T == triviality::trivial, trivial_call_holder<Signature, E>, base_call_holder<Signature, E>>;
    using base::base;

    template <typename V>
    void bind_call() noexcept
    {
        static_assert(R == rebindability::rebindable);
        base::_call = bits::generate_call<V, Signature, E>();
    }

    bits::call_signature<Signature, E> get_call() const noexcept
    {
        return base::_call;
    }

    void zero_call() noexcept
    {
        static_assert(N == nullability::nullable);
        base::_call = nullptr;
    }
};

struct empty_call_holder
{
    constexpr empty_call_holder(bits::swallow) noexcept {}
    constexpr empty_call_holder() = default;

    constexpr void zero_call() noexcept {}
};

template <
    typename Accessor,
    typename Signature,
    triviality T,
    nullability N,
    rebindability R,
    exception_specification E,
    function_pointer_lookup F = function_pointer_lookup::inplace>
struct type_erased_call_policy
    : std::conditional_t<F == function_pointer_lookup::inplace, call_holder<T, N, R, E, Signature>, empty_call_holder>
{
    using base = std::conditional_t<F == function_pointer_lookup::inplace, call_holder<T, N, R, E, Signature>, empty_call_holder>;
    using base::base;

    void zero_call() noexcept
    {
        static_assert(N == nullability::nullable);
        base::zero_call();
    }

    bits::call_signature<Signature, E> get_call() const noexcept
    {
        if constexpr (F == function_pointer_lookup::inplace)
        {
            return base::get_call();
        }
        else if constexpr (F == function_pointer_lookup::pointer_switch)
        {
            return reinterpret_cast<bits::call_signature<Signature, E>>(
                static_cast<const Accessor&>(*this).get_switch_function()(operation::call));
        }
        else if constexpr (F == function_pointer_lookup::pointer_table)
        {
            return static_cast<const Accessor&>(*this).get_pointer_table().template get<operation::call>();
        }
        else
        {
            static_assert(bits::always_false_v<Accessor>);
        }
    }

    template <typename V>
    void bind_call() noexcept
    {
        if constexpr (F == function_pointer_lookup::inplace)
        {
            base::template bind_call<V>();
        }
    }
};

} // namespace detail::call

template <exception_specification E>
struct trivial_call_policy
    : policy_description<E, function_pointer_lookup::inplace>
{
    template <typename BindingPolicy, typename Accessor, typename Signature>
    using policy = detail::call::type_erased_call_policy<
        Accessor,
        Signature,
        triviality::trivial,
        BindingPolicy::nullability,
        BindingPolicy::rebindability,
        E>;
};

template <exception_specification E, function_pointer_lookup F>
struct non_trivial_call_policy
    : policy_description<E, F>
{
    template <typename BindingPolicy, typename Accessor, typename Signature>
    using policy =
        detail::call::type_erased_call_policy<
            Accessor,
            Signature,
            triviality::non_trivial,
            BindingPolicy::nullability,
            BindingPolicy::rebindability,
            E,
            F>;
};

} // namespace bk::erasure::policies
