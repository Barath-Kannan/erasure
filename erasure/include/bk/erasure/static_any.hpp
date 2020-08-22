// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/policies/binding_policy.hpp"
#include "bk/erasure/policies/copy_policy.hpp"
#include "bk/erasure/policies/destructor_policy.hpp"
#include "bk/erasure/policies/enums.hpp"
#include "bk/erasure/policies/move_policy.hpp"
#include "bk/erasure/policies/policies_descriptor.hpp"
#include "bk/erasure/policies/type_checking_policy.hpp"
#include "bk/erasure/policies/type_id_policy.hpp"

#include <boost/mp11/function.hpp>
#include <boost/type_traits/remove_cv_ref.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>

namespace bk::erasure {

struct non_relocatable_any_policy
{
    using binding_policy =
        policies::binding_policy<
            policies::nullability::nullable,
            policies::rebindability::rebindable>;

    using destructor_policy =
        policies::non_trivial_destructor_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::inplace>;

    using type_id_policy =
        policies::non_trivial_type_id_policy<
            policies::function_pointer_lookup::inplace>;

    using type_checking_policy = policies::assert_type_checking_policy;
};

struct unique_any_policy
{
    using binding_policy =
        policies::binding_policy<
            policies::nullability::nullable,
            policies::rebindability::rebindable>;

    using move_policy =
        policies::non_trivial_move_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::pointer_switch>;

    using destructor_policy =
        policies::non_trivial_destructor_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::pointer_switch>;

    using type_id_policy =
        policies::non_trivial_type_id_policy<
            policies::function_pointer_lookup::inplace>;

    using type_checking_policy = policies::assert_type_checking_policy;
};

struct default_any_policy
    : unique_any_policy
{
    using copy_policy =
        policies::non_trivial_copy_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::pointer_switch>;
};

struct pointer_table_any_policy
{
    using binding_policy =
        policies::binding_policy<
            policies::nullability::nullable,
            policies::rebindability::rebindable>;

    using move_policy =
        policies::non_trivial_move_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::pointer_table>;

    using copy_policy =
        policies::non_trivial_copy_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::pointer_table>;

    using destructor_policy =
        policies::non_trivial_destructor_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::pointer_table>;

    using type_id_policy =
        policies::non_trivial_type_id_policy<
            policies::function_pointer_lookup::pointer_table>;

    using type_checking_policy = policies::assert_type_checking_policy;
};

struct trivial_any_policy
{
    using binding_policy =
        policies::binding_policy<
            policies::nullability::nullable,
            policies::rebindability::rebindable>;

    using destructor_policy = policies::trivial_destructor_policy;
    using copy_policy = policies::trivial_copy_policy;
    using move_policy = policies::trivial_move_policy;
    using type_id_policy = policies::trivial_type_id_policy;
    using type_checking_policy = policies::assert_type_checking_policy;
};

template <std::size_t, typename Policy = default_any_policy>
struct static_any;

template <std::size_t N>
using non_relocatable_any = static_any<N, non_relocatable_any_policy>;

template <std::size_t N>
using pointer_table_any = static_any<N, pointer_table_any_policy>;

template <std::size_t N>
using unique_any = static_any<N, unique_any_policy>;

template <std::size_t N>
using trivial_any = static_any<N, trivial_any_policy>;

template <std::size_t Size, typename Policy>
struct static_any
    : policies::any_policy_descriptor<static_any<Size, Policy>>::policy_holder
{
private:
    using policy_descriptor = policies::any_policy_descriptor<static_any<Size, Policy>>;
    using policy_holder = typename policy_descriptor::policy_holder;
    using self = typename policy_descriptor::self;
    using type_id_policy = typename policy_descriptor::type_id_policy;
    using type_checking_policy = typename policy_descriptor::type_checking_policy;

    static constexpr bool is_nullable = policy_descriptor::is_nullable;
    static constexpr bool is_rebindable = policy_descriptor::is_rebindable;
    static constexpr bool is_no_except_destroyable = policy_descriptor::is_no_except_destroyable;
    static constexpr bool is_no_exceptcopyable = policy_descriptor::is_no_exceptcopyable;
    static constexpr bool is_no_except_moveable = policy_descriptor::is_no_except_moveable;
    static constexpr bool is_no_except_type_checkable = policy_descriptor::is_no_except_type_checkable;

public:
    static constexpr auto max_size = Size;

    static_any() noexcept = default;

    template <
        typename V,
        typename Decayed = boost::remove_cv_ref_t<V>,
        typename = std::enable_if_t<!std::is_same_v<Decayed, static_any>>>
    explicit static_any(V&& v) noexcept :
        policy_holder{ boost::mp11::mp_identity<Decayed>{} }
    {
        check_constraints<Decayed>();
        new (this->data()) Decayed{ std::forward<V>(v) };
    }

    template <typename V, typename... Args>
    explicit static_any(std::in_place_type_t<V>, Args&&... args) :
        policy_holder{ boost::mp11::mp_identity<V>{} }
    {
        check_constraints<V>();
        new (this->data()) V{ std::forward<Args>(args)... };
    }

    static_any(const static_any&) noexcept(is_no_exceptcopyable) = default;
    static_any& operator=(const static_any&) noexcept(is_no_exceptcopyable) = default;

    static_any(static_any&&) noexcept(is_no_except_moveable) = default;
    static_any& operator=(static_any&&) noexcept(is_no_except_moveable) = default;

    template <
        typename V,
        typename Decayed = boost::remove_cv_ref_t<V>,
        typename = std::enable_if_t<!std::is_same_v<Decayed, static_any>>>
    static_any& operator=(V&& v) noexcept(is_no_except_destroyable)
    {
        static_assert(is_rebindable);
        emplace<Decayed>(std::forward<V>(v));
        return *this;
    }

    ~static_any() noexcept(is_no_except_destroyable) = default;

    template <
        typename V,
        typename... Args>
    V& emplace(Args&&... args) noexcept(is_no_except_destroyable)
    {
        static_assert(is_rebindable);
        check_constraints<V>();
        policy_holder::destroy();
        policy_holder::zero();
        auto p = new (this->data()) V{ std::forward<Args>(args)... };
        policy_holder::template bind<V>();
        return *p;
    }

    void reset() noexcept(is_no_except_destroyable)
    {
        static_assert(is_nullable);
        policy_holder::destroy();
        policy_holder::zero();
    }

    bool is_set() const noexcept
    {
        if constexpr (!is_nullable)
            return true;
        return type_id_policy::get_type_info();
    }

    constexpr const std::type_info& type() const noexcept(is_no_except_type_checkable)
    {
        const auto* type_info = type_id_policy::get_type_info();
        type_checking_policy::check(type_info);
        return *type_info;
    }

    template <typename V>
    V& get() noexcept(is_no_except_type_checkable)
    {
        type_checking_policy::check(type(), typeid(V));
        return reinterpret_cast<V&>(*this->data());
    }

    template <typename V>
    const V& get() const noexcept(is_no_except_type_checkable)
    {
        type_checking_policy::check(type(), typeid(V));
        return reinterpret_cast<const V&>(*this->data());
    }

    explicit operator bool() const noexcept
    {
        return is_set();
    }

private:
    template <typename V>
    static void check_constraints() noexcept
    {
        static_assert(sizeof(V) <= Size);
        static_assert(!boost::mp11::mp_similar<V, self>::value);
        policy_holder::template check_constraints<V>();
    }
};

template <typename V, std::size_t S, typename Policy>
V& get(static_any<S, Policy>& any) noexcept(noexcept(any.template get<V>()))
{
    return any.template get<V>();
}

template <typename V, std::size_t S, typename Policy>
const V& get(const static_any<S, Policy>& any) noexcept(noexcept(any.template get<V>()))
{
    return any.template get<V>();
}

} // namespace bk::erasure
