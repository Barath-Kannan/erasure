// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/static_any.hpp"
#include "bk/erasure/static_function.hpp"

#include <ostream>

namespace bk::erasure {

// workaround for boost test to not complain
template <typename Signature, std::size_t S, typename Policy>
std::ostream& operator<<(std::ostream& os, const static_function<Signature, S, Policy>&)
{
    return os;
}

// workaround for boost test to not complain
template <std::size_t S, typename Policy>
std::ostream& operator<<(std::ostream& os, const static_any<S, Policy>&)
{
    return os;
}

} // namespace bk::erasure

namespace bk::erasure::fixtures {

static_assert(std::is_trivial_v<trivial_any<8>>);
static_assert(sizeof(trivial_any<8>) == 16);
static_assert(sizeof(static_any<8>) == 24);
static_assert(sizeof(unique_any<8>) == 24);
static_assert(sizeof(non_relocatable_any<8>) == 24);
static_assert(sizeof(pointer_table_any<8>) == 16);

static_assert(std::is_trivial_v<trivial_function<void(), 0>>);
static_assert(std::is_trivial_v<trivial_function<void(), 8>>);
static_assert(sizeof(trivial_function<void(), 8>) == 16);
static_assert(sizeof(static_function<void(), 8>) == 24);
static_assert(sizeof(unique_function<void(), 8>) == 24);
static_assert(sizeof(non_relocatable_function<void(), 8>) == 24);
static_assert(sizeof(pointer_table_function<void(), 8>) == 16);
static_assert(sizeof(stateless_function<void()>) == 8);

// if we're using 0 size, we definitely want triviality but I'll leave these here just as proof that it "works"
static_assert(sizeof(static_function<void(), 0>) == 16);
static_assert(sizeof(unique_function<void(), 0>) == 16);

template <typename T, T... Args>
using seq = boost::mp11::mp_from_sequence<std::integer_sequence<T, Args...>>;

template <policies::nullability... Args>
using N = seq<policies::nullability, Args...>;

template <policies::rebindability... Args>
using R = seq<policies::rebindability, Args...>;

template <policies::function_pointer_lookup... Args>
using F = seq<policies::function_pointer_lookup, Args...>;

using nullability_types =
    N<
        policies::nullability::nullable,
        policies::nullability::non_nullable>;

using rebindability_types =
    R<
        policies::rebindability::rebindable,
        policies::rebindability::non_rebindable>;

using function_pointer_lookup_types =
    F<
        policies::function_pointer_lookup::inplace,
        policies::function_pointer_lookup::pointer_switch,
        policies::function_pointer_lookup::pointer_table,
        policies::function_pointer_lookup::not_present>;

using non_trivial_function_pointer_lookup_types =
    boost::mp11::mp_remove<
        function_pointer_lookup_types,
        std::integral_constant<policies::function_pointer_lookup, policies::function_pointer_lookup::not_present>>;

template <
    policies::nullability N,
    policies::rebindability R,
    policies::function_pointer_lookup DestructorFP,
    policies::function_pointer_lookup CallFP,
    policies::function_pointer_lookup CopyFP,
    policies::function_pointer_lookup MoveFP>
struct test_function_policy
{
    template <policies::function_pointer_lookup FP>
    static constexpr bool is_trivial_ = FP == policies::function_pointer_lookup::not_present;
    static constexpr auto function_pointer_lookup = CallFP;

    using binding_policy = policies::binding_policy<N, R>;

    using destructor_policy = std::conditional_t<
        is_trivial_<DestructorFP>,
        policies::trivial_destructor_policy,
        policies::non_trivial_destructor_policy<
            policies::exception_specification::no_except,
            DestructorFP>>;

    using call_policy = std::conditional_t<
        is_trivial_<CallFP>,
        policies::trivial_call_policy<policies::exception_specification::no_except>,
        policies::non_trivial_call_policy<
            policies::exception_specification::no_except,
            CallFP>>;

    using copy_policy = std::conditional_t<
        is_trivial_<CopyFP>,
        policies::trivial_copy_policy,
        policies::non_trivial_copy_policy<
            policies::exception_specification::no_except,
            CopyFP>>;

    using move_policy = std::conditional_t<
        is_trivial_<MoveFP>,
        policies::trivial_move_policy,
        policies::non_trivial_move_policy<
            policies::exception_specification::no_except,
            MoveFP>>;

    using callable_checking_policy = policies::throwing_callable_checking_policy;
};

template <typename N, typename R, typename DestructorFP, typename CallFP, typename CopyFP, typename MoveFP>
using test_function_policy_from_constants = test_function_policy<N::value, R::value, DestructorFP::value, CallFP::value, CopyFP::value, MoveFP::value>;

using test_function_policies = boost::mp11::mp_product<
    test_function_policy_from_constants,
    nullability_types,
    rebindability_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types>;

using test_function_nullable_policies = boost::mp11::mp_product<
    test_function_policy_from_constants,
    N<policies::nullability::nullable>,
    rebindability_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types>;

using test_function_nullable_rebindable_policies = boost::mp11::mp_product<
    test_function_policy_from_constants,
    N<policies::nullability::nullable>,
    R<policies::rebindability::rebindable>,
    function_pointer_lookup_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types>;

using test_function_non_trivial_nullable_rebindable_policies = boost::mp11::mp_product<
    test_function_policy_from_constants,
    N<policies::nullability::nullable>,
    R<policies::rebindability::rebindable>,
    non_trivial_function_pointer_lookup_types,
    non_trivial_function_pointer_lookup_types,
    non_trivial_function_pointer_lookup_types,
    non_trivial_function_pointer_lookup_types>;

template <
    policies::nullability N,
    policies::rebindability R,
    policies::function_pointer_lookup DestructorFP,
    policies::function_pointer_lookup TypeIdFP,
    policies::function_pointer_lookup CopyFP,
    policies::function_pointer_lookup MoveFP>
struct test_any_policy
{
    template <policies::function_pointer_lookup FPLtype>
    static constexpr bool is_trivial_ = FPLtype == policies::function_pointer_lookup::not_present;

    using binding_policy = policies::binding_policy<N, R>;
    static constexpr auto function_pointer_lookup = TypeIdFP;

    using destructor_policy = std::conditional_t<
        is_trivial_<DestructorFP>,
        policies::trivial_destructor_policy,
        policies::non_trivial_destructor_policy<
            policies::exception_specification::no_except,
            DestructorFP>>;

    using type_id_policy = std::conditional_t<
        is_trivial_<TypeIdFP>,
        policies::trivial_type_id_policy,
        policies::non_trivial_type_id_policy<
            TypeIdFP>>;

    using copy_policy = std::conditional_t<
        is_trivial_<CopyFP>,
        policies::trivial_copy_policy,
        policies::non_trivial_copy_policy<
            policies::exception_specification::no_except,
            CopyFP>>;

    using move_policy = std::conditional_t<
        is_trivial_<MoveFP>,
        policies::trivial_move_policy,
        policies::non_trivial_move_policy<
            policies::exception_specification::no_except,
            MoveFP>>;

    using type_checking_policy = policies::throwing_type_checking_policy;
};

template <typename N, typename R, typename DestructorFP, typename TypeIdFP, typename CopyFP, typename MoveFP>
using test_any_policy_from_constants = test_any_policy<N::value, R::value, DestructorFP::value, TypeIdFP::value, CopyFP::value, MoveFP::value>;

using test_any_policies = boost::mp11::mp_product<
    test_any_policy_from_constants,
    nullability_types,
    rebindability_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types>;

using test_any_nullable_policies = boost::mp11::mp_product<
    test_any_policy_from_constants,
    N<policies::nullability::nullable>,
    rebindability_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types,
    function_pointer_lookup_types>;

using test_any_non_trivial_nullable_policies = boost::mp11::mp_product<
    test_any_policy_from_constants,
    N<policies::nullability::nullable>,
    rebindability_types,
    non_trivial_function_pointer_lookup_types,
    non_trivial_function_pointer_lookup_types,
    non_trivial_function_pointer_lookup_types,
    non_trivial_function_pointer_lookup_types>;

using test_any_non_trivial_nullable_rebindable_policies = boost::mp11::mp_product<
    test_any_policy_from_constants,
    N<policies::nullability::nullable>,
    R<policies::rebindability::rebindable>,
    non_trivial_function_pointer_lookup_types,
    non_trivial_function_pointer_lookup_types,
    non_trivial_function_pointer_lookup_types,
    non_trivial_function_pointer_lookup_types>;

} // namespace bk::erasure::fixtures
