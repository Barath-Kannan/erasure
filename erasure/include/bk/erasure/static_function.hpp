// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/policies/binding_policy.hpp"
#include "bk/erasure/policies/call_policy.hpp"
#include "bk/erasure/policies/callable_checking_policy.hpp"
#include "bk/erasure/policies/copy_policy.hpp"
#include "bk/erasure/policies/destructor_policy.hpp"
#include "bk/erasure/policies/enums.hpp"
#include "bk/erasure/policies/move_policy.hpp"
#include "bk/erasure/policies/policies_descriptor.hpp"

#include <boost/mp11/function.hpp>
#include <boost/type_traits/remove_cv_ref.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>

namespace bk::erasure {

struct non_relocatable_function_policy
{
    using binding_policy =
        policies::binding_policy<
            policies::nullability::nullable,
            policies::rebindability::rebindable>;

    using destructor_policy =
        policies::non_trivial_destructor_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::inplace>;

    using call_policy =
        policies::non_trivial_call_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::inplace>;

    using callable_checking_policy = policies::assert_callable_checking_policy;
};

struct unique_function_policy
{
    using binding_policy =
        policies::binding_policy<
            policies::nullability::nullable,
            policies::rebindability::rebindable>;

    using destructor_policy =
        policies::non_trivial_destructor_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::pointer_switch>;

    using move_policy =
        policies::non_trivial_move_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::pointer_switch>;

    using call_policy =
        policies::non_trivial_call_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::inplace>;

    using callable_checking_policy = policies::assert_callable_checking_policy;
};

struct default_function_policy
    : unique_function_policy
{
    using copy_policy =
        policies::non_trivial_copy_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::pointer_switch>;
};

struct pointer_table_function_policy
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

    using call_policy =
        policies::non_trivial_call_policy<
            policies::exception_specification::no_except,
            policies::function_pointer_lookup::pointer_table>;

    using callable_checking_policy = policies::assert_callable_checking_policy;
};

struct trivial_non_relocatable_function_policy
{
    using binding_policy =
        policies::binding_policy<
            policies::nullability::nullable,
            policies::rebindability::rebindable>;

    using destructor_policy = policies::trivial_destructor_policy;
    using call_policy = policies::trivial_call_policy<policies::exception_specification::no_except>;
    using callable_checking_policy = policies::assert_callable_checking_policy;
};

struct trivial_function_policy
    : trivial_non_relocatable_function_policy
{
    using copy_policy = policies::trivial_copy_policy;
    using move_policy = policies::trivial_move_policy;
};

template <typename R, std::size_t, typename policy = default_function_policy>
struct static_function;

template <typename R, std::size_t N>
using non_relocatable_function = static_function<R, N, non_relocatable_function_policy>;

template <typename R, std::size_t N>
using pointer_table_function = static_function<R, N, pointer_table_function_policy>;

template <typename R, std::size_t N>
using unique_function = static_function<R, N, unique_function_policy>;

template <typename R, std::size_t N>
using trivial_non_relocatable_function = static_function<R, N, trivial_non_relocatable_function_policy>;

template <typename R, std::size_t N>
using trivial_function = static_function<R, N, trivial_function_policy>;

template <typename R>
using stateless_function = trivial_function<R, 0>;

template <typename R, typename... Args, std::size_t Size, typename Policy>
struct static_function<R(Args...), Size, Policy>
    : policies::function_policy_descriptor<static_function<R(Args...), Size, Policy>>::policy_holder
{
private:
    using policy_descriptor = policies::function_policy_descriptor<static_function<R(Args...), Size, Policy>>;
    using policy_holder = typename policy_descriptor::policy_holder;
    using self = typename policy_descriptor::self;
    using call_policy = typename policy_descriptor::call_policy;
    using callable_checking_policy = typename policy_descriptor::callable_checking_policy;

    static constexpr bool is_nullable = policy_descriptor::is_nullable;
    static constexpr bool is_rebindable = policy_descriptor::is_rebindable;
    static constexpr bool is_no_except_destroyable = policy_descriptor::is_no_except_destroyable;
    static constexpr bool is_no_exceptcopyable = policy_descriptor::is_no_exceptcopyable;
    static constexpr bool is_no_except_moveable = policy_descriptor::is_no_except_moveable;
    static constexpr bool is_no_except_callable = policy_descriptor::is_no_except_callable;
    static constexpr bool is_no_except_callable_checkable = policy_descriptor::is_no_except_callable_checkable;

public:
    static constexpr auto max_size = Size;

    static_function() noexcept = default;

    template <
        typename F,
        typename Decayed = boost::remove_cv_ref_t<F>,
        typename = std::enable_if_t<!std::is_same_v<Decayed, static_function>>>
    static_function(F&& f) noexcept :
        policy_holder{ boost::mp11::mp_identity<Decayed>{} }
    {
        check_constraints<Decayed>();
        new (this->data()) Decayed{ std::forward<F>(f) };
    }

    template <typename F, typename... CArgs>
    explicit static_function(std::in_place_type_t<F>, CArgs&&... args) :
        policy_holder{ boost::mp11::mp_identity<F>{} }
    {
        check_constraints<F>();
        new (this->data()) F{ std::forward<CArgs>(args)... };
    }

    static_function(const static_function&) noexcept(is_no_exceptcopyable) = default;
    static_function& operator=(const static_function&) noexcept(is_no_exceptcopyable) = default;

    static_function(static_function&&) noexcept(is_no_except_moveable) = default;
    static_function& operator=(static_function&&) noexcept(is_no_except_moveable) = default;

    template <
        typename F,
        typename Decayed = boost::remove_cv_ref_t<F>,
        typename = std::enable_if_t<!std::is_same_v<Decayed, static_function>>,
        typename = std::enable_if_t<!std::is_same_v<Decayed, decltype(nullptr)>>>
    static_function& operator=(F&& f) noexcept(is_no_except_destroyable)
    {
        static_assert(is_rebindable);
        check_constraints<Decayed>();
        policy_holder::destroy();
        policy_holder::zero();
        new (this->data()) Decayed{ std::forward<F>(f) };
        policy_holder::template bind<Decayed>();
        return *this;
    }

    template <
        typename F,
        typename = std::enable_if_t<std::is_same_v<F, decltype(nullptr)>>>
    static_function& operator=(F&&) noexcept(is_no_except_destroyable)
    {
        static_assert(is_nullable);
        policy_holder::destroy();
        policy_holder::zero();
        return *this;
    }

    R operator()(Args... args) noexcept((is_no_except_callable && is_no_except_callable_checkable))
    {
        auto callable = call_policy::get_call();
        callable_checking_policy::check(callable);
        return std::invoke(callable, this->data(), std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(call_policy::get_call());
    }

    bool operator==(decltype(nullptr)) const noexcept
    {
        return !static_cast<bool>(*this);
    }

private:
    template <typename F>
    static constexpr void check_constraints() noexcept
    {
        static_assert(sizeof(F) <= Size || (Size == 0 && std::is_empty_v<F>));
        static_assert(std::is_invocable_r_v<R, F, Args...>);
        static_assert(!boost::mp11::mp_similar<F, self>::value);
        policy_holder::template check_constraints<F>();
    }
};

} // namespace bk::erasure
