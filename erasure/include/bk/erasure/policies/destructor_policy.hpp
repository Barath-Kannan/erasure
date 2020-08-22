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

namespace bk::erasure::policies {

namespace detail::destructor {

template <
    typename Accessor = void,
    triviality = triviality::trivial,
    nullability = nullability::non_nullable,
    rebindability = rebindability::non_rebindable,
    exception_specification = exception_specification::no_except,
    function_pointer_lookup = function_pointer_lookup::not_present>
struct type_erased_destructor_policy
{
    type_erased_destructor_policy() = default;
    type_erased_destructor_policy(bits::swallow) noexcept {}
    ~type_erased_destructor_policy() = default;

    template <typename V>
    static constexpr void bind_destructor() noexcept {}

    static constexpr void destroy() noexcept {}

    template <typename V>
    static constexpr void check_constraints() noexcept
    {
        static_assert(std::is_trivially_destructible_v<V>);
    }
};

template <exception_specification E>
struct destructor_holder
{
    destructor_holder() = default;

    template <typename V>
    destructor_holder(boost::mp11::mp_identity<V>) noexcept :
        _destructor{ bits::generate_destroy<V, E>() }
    {}

    template <typename V>
    void bind_destructor()
    {
        _destructor = bits::generate_destroy<V, E>();
    }

    bits::destroy_signature<E> get_destructor() const
    {
        return _destructor;
    }

private:
    bits::destroy_signature<E> _destructor{ nullptr };
};

struct empty_destructor_holder
{
    constexpr empty_destructor_holder(bits::swallow) noexcept {}
    constexpr empty_destructor_holder() noexcept = default;
};

template <typename Accessor, nullability N, rebindability R, exception_specification E, function_pointer_lookup F>
struct type_erased_destructor_policy<Accessor, triviality::non_trivial, N, R, E, F>
    : private std::conditional_t<F == function_pointer_lookup::inplace, destructor_holder<E>, empty_destructor_holder>
{
    using base = std::conditional_t<F == function_pointer_lookup::inplace, destructor_holder<E>, empty_destructor_holder>;
    using base::base;

    bits::destroy_signature<E> get_destructor() const
    {
        if constexpr (F == function_pointer_lookup::inplace)
        {
            return base::get_destructor();
        }
        else if constexpr (F == function_pointer_lookup::pointer_switch)
        {
            return reinterpret_cast<bits::destroy_signature<E>>(
                static_cast<const Accessor&>(*this).get_switch_function()(operation::destroy));
        }
        else if constexpr (F == function_pointer_lookup::pointer_table)
        {
            return static_cast<const Accessor&>(*this).get_pointer_table().template get<operation::destroy>();
        }
        else
        {
            static_assert(bits::always_false_v<Accessor>);
        }
    }

    template <typename V>
    void bind_destructor() noexcept
    {
        if constexpr (F == function_pointer_lookup::inplace)
        {
            base::template bind_destructor<V>();
        }
    }

    void destroy() noexcept(E == exception_specification::no_except)
    {
        auto destructor = get_destructor();
        auto data = static_cast<Accessor*>(this)->data();
        if constexpr (N == nullability::non_nullable)
        {
            assert(destructor);
            destructor(data);
        }
        else
        {
            if (destructor)
                destructor(data);
        }
    }

    template <typename V>
    static constexpr void check_constraints() noexcept
    {}
};

static_assert(std::is_trivial_v<type_erased_destructor_policy<void, triviality::trivial>>);

} // namespace detail::destructor

struct trivial_destructor_policy
    : policy_description<exception_specification::no_except, function_pointer_lookup::not_present>
{
    template <typename BindingPolicy, typename Accessor>
    using policy = detail::destructor::type_erased_destructor_policy<Accessor, triviality::trivial>;
};

template <exception_specification E, function_pointer_lookup F>
struct non_trivial_destructor_policy
    : policy_description<E, F>
{
    template <typename BindingPolicy, typename Accessor>
    using policy =
        detail::destructor::type_erased_destructor_policy<
            Accessor,
            triviality::non_trivial,
            BindingPolicy::nullability,
            BindingPolicy::rebindability,
            E,
            F>;
};

} // namespace bk::erasure::policies
