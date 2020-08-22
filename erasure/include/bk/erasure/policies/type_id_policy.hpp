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

#include <typeinfo>

namespace bk::erasure::policies {

namespace detail::type_id {

struct trivial_typeid_holder
{
    trivial_typeid_holder() noexcept = default;

    template <typename V>
    trivial_typeid_holder(boost::mp11::mp_identity<V>) noexcept :
        _type_info{ &typeid(V) }
    {}

    const std::type_info* _type_info;
};

static_assert(std::is_trivial_v<trivial_typeid_holder>);

struct base_typeid_holder
{
    base_typeid_holder() noexcept = default;

    template <typename V>
    base_typeid_holder(boost::mp11::mp_identity<V>) noexcept :
        _type_info{ &typeid(V) }
    {}

    const std::type_info* _type_info{ nullptr };
};

template <triviality T>
struct typeid_holder
    : std::conditional_t<T == triviality::trivial, trivial_typeid_holder, base_typeid_holder>
{
    using base = std::conditional_t<T == triviality::trivial, trivial_typeid_holder, base_typeid_holder>;
    using base::base;

    template <typename V>
    void bind_type_id() noexcept
    {
        base::_type_info = &typeid(V);
    }

    const std::type_info* get_type_info() const noexcept
    {
        return base::_type_info;
    }

    void zero_type_id() noexcept
    {
        base::_type_info = nullptr;
    }
};

static_assert(std::is_trivial_v<typeid_holder<triviality::trivial>>);

struct empty_typeid_holder
{
    constexpr empty_typeid_holder(bits::swallow) noexcept {}
    constexpr empty_typeid_holder() = default;

    constexpr void zero_type_id() noexcept {}
};

template <typename Accessor, triviality T, nullability N, rebindability R, function_pointer_lookup F>
struct type_erased_type_id_policy
    : private std::conditional_t<F == function_pointer_lookup::inplace, typeid_holder<T>, empty_typeid_holder>
{
    using base = std::conditional_t<F == function_pointer_lookup::inplace, typeid_holder<T>, empty_typeid_holder>;
    using base::base;

    void zero_type_id() noexcept
    {
        static_assert(N == nullability::nullable);
        base::zero_type_id();
    }

    template <typename V>
    void bind_type_id() noexcept
    {
        if constexpr (F == function_pointer_lookup::inplace)
        {
            static_assert(R == rebindability::rebindable);
            base::template bind_type_id<V>();
        }
    }

    const std::type_info* get_type_info() const noexcept
    {
        if constexpr (F == function_pointer_lookup::inplace)
        {
            return base::get_type_info();
        }
        else if constexpr (F == function_pointer_lookup::pointer_switch)
        {
            return reinterpret_cast<bits::typeid_signature>(
                static_cast<const Accessor&>(*this).get_switch_function()(operation::type_id));
        }
        else if constexpr (F == function_pointer_lookup::pointer_table)
        {
            return static_cast<const Accessor&>(*this).get_pointer_table().template get<operation::type_id>();
        }
        else
        {
            static_assert(bits::always_false_v<Accessor>);
        }
    }
};

} // namespace detail::type_id

struct trivial_type_id_policy
    : policy_description<exception_specification::no_except, function_pointer_lookup::inplace>
{
    template <typename binding_policy, typename Accessor>
    using policy = detail::type_id ::type_erased_type_id_policy<
        Accessor,
        triviality::trivial,
        binding_policy::nullability,
        binding_policy::rebindability,
        function_pointer_lookup::inplace>;
};

template <function_pointer_lookup F>
struct non_trivial_type_id_policy
    : policy_description<exception_specification::no_except, F>
{
    template <typename binding_policy, typename Accessor>
    using policy = detail::type_id ::type_erased_type_id_policy<
        Accessor,
        triviality::non_trivial,
        binding_policy::nullability,
        binding_policy::rebindability,
        F>;
};

} // namespace bk::erasure::policies
