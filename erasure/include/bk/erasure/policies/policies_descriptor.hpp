// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/bits/pointer_table_holder.hpp"
#include "bk/erasure/bits/special_members.hpp"
#include "bk/erasure/bits/switch_function_pointer_holder.hpp"
#include "bk/erasure/bits/traits.hpp"
#include "bk/erasure/policies/enums.hpp"
#include "bk/erasure/policies/null_policies.hpp"
#include "bk/erasure/policies/type_id_policy.hpp"

#include <boost/callable_traits/is_noexcept.hpp>
#include <boost/mp11.hpp>
#include <boost/type_traits/detected_or.hpp>

#include <array>
#include <cstddef>
#include <type_traits>
#include <typeinfo>

namespace bk::erasure::policies {

struct trivial_copy_policy;
struct trivial_move_policy;
struct trivial_destructor_policy;

template <typename any>
struct any_policy_descriptor
{
private:
    template <typename X>
    struct trait_helper;

    template <template <std::size_t, typename> typename AnyType, std::size_t N, typename PolicyType>
    struct trait_helper<AnyType<N, PolicyType>>
    {
        static constexpr std::size_t max_size = N;
        using policy = PolicyType;
    };

    template <typename P>
    using get_copy_policy_wrapper = typename P::copy_policy;

    template <typename P>
    using get_move_policy_wrapper = typename P::move_policy;

    using policy = typename trait_helper<any>::policy;

    using copy_policy_wrapper = boost::detected_or_t<non_copyable_policy, get_copy_policy_wrapper, policy>;
    using move_policy_wrapper = boost::detected_or_t<non_moveable_policy, get_move_policy_wrapper, policy>;
    using destructor_policy_wrapper = typename policy::destructor_policy;

    using binding_policy = typename policy::binding_policy;
    using destructor_policy = typename destructor_policy_wrapper::template policy<binding_policy, any>;
    using copy_policy = typename copy_policy_wrapper::template policy<binding_policy, any>;
    using move_policy = typename move_policy_wrapper::template policy<binding_policy, any>;
    using switch_holder = bits::switch_function_pointer_holder<policy>;
    using pointer_table_holder = bits::pointer_table_holder<policy>;

public:
    using self = any;
    static constexpr std::size_t max_size = trait_helper<any>::max_size;

    using type_id_policy = typename policy::type_id_policy::template policy<binding_policy, any>;
    using type_checking_policy = typename policy::type_checking_policy;

    static constexpr bool is_nullable = binding_policy::nullability == policies::nullability::nullable;
    static constexpr bool is_rebindable = binding_policy::rebindability == policies::rebindability::rebindable;

    static constexpr bool is_copyable = !std::is_same_v<copy_policy_wrapper, non_copyable_policy>;
    static constexpr bool is_moveable = !std::is_same_v<move_policy_wrapper, non_moveable_policy>;

    static constexpr bool is_no_except_type_checkable = noexcept(type_checking_policy::check(std::declval<std::type_info>(), std::declval<std::type_info>()));
    static constexpr bool is_no_exceptcopyable = noexcept(std::declval<copy_policy>().copy(std::declval<const std::byte*>()));
    static constexpr bool is_no_except_moveable = noexcept(std::declval<move_policy>().move(std::declval<std::byte*>()));
    static constexpr bool is_no_except_destroyable = noexcept(std::declval<destructor_policy>().destroy());

    static constexpr bool is_trivially_copyable = std::is_same_v<copy_policy_wrapper, policies::trivial_copy_policy>;
    static constexpr bool is_trivially_moveable = std::is_same_v<move_policy_wrapper, policies::trivial_move_policy>;
    static constexpr bool is_trivially_destroyable = std::is_same_v<destructor_policy_wrapper, policies::trivial_destructor_policy>;

    struct data_holder
    {
        friend self;

        std::byte* data() noexcept { return mdata.data(); }
        const std::byte* data() const noexcept { return mdata.data(); }

        std::array<std::byte, max_size> mdata;
    };

    struct policy_holder
        : bits::pre_destroy_behaviour<policy_holder, is_trivially_destroyable, is_no_except_destroyable>
        , private destructor_policy
        , private copy_policy
        , private move_policy
        , private type_id_policy
        , private switch_holder
        , private pointer_table_holder
        , private data_holder
        , bits::copy_behaviour<policy_holder, is_copyable, is_trivially_copyable, is_no_exceptcopyable>
        , bits::move_behaviour<policy_holder, is_moveable, is_trivially_moveable, is_no_except_moveable>
        , bits::destroy_behaviour<policy_holder, is_trivially_destroyable, is_no_except_destroyable>
    {
        friend any;
        friend destructor_policy;
        friend copy_policy;
        friend move_policy;
        friend type_id_policy;

        using pre_destroy_behaviour = bits::pre_destroy_behaviour<policy_holder, is_trivially_destroyable, is_no_except_destroyable>;
        using copy_behaviour = bits::copy_behaviour<policy_holder, is_copyable, is_trivially_copyable, is_no_exceptcopyable>;
        using move_behaviour = bits::move_behaviour<policy_holder, is_moveable, is_trivially_moveable, is_no_except_moveable>;
        using destroy_behaviour = bits::destroy_behaviour<policy_holder, is_trivially_destroyable, is_no_except_destroyable>;

        friend pre_destroy_behaviour;
        friend copy_behaviour;
        friend move_behaviour;
        friend destroy_behaviour;

        policy_holder() noexcept = default;

        template <typename V>
        policy_holder(boost::mp11::mp_identity<V> v) noexcept :
            destructor_policy{ v },
            copy_policy{ v },
            move_policy{ v },
            type_id_policy{ v },
            switch_holder{ boost::mp11::mp_identity<policy>{}, v },
            pointer_table_holder{ v }
        {}

    private:
        void destroy() noexcept(is_no_except_destroyable)
        {
            destructor_policy::destroy();
        }

        void zero() noexcept
        {
            // this is the only thing that is actually used for checking
            // (as its compulsory to have a type id in an any)
            type_id_policy::zero_type_id();
            switch_holder::zero_switch_function();
            pointer_table_holder::zero_pointer_table();
        }

        template <typename V>
        void bind() noexcept
        {
            destructor_policy::template bind_destructor<V>();
            copy_policy::template bind_copy<V>();
            move_policy::template bind_move<V>();
            type_id_policy::template bind_type_id<V>();
            switch_holder::template bind_switch_function<policy, V>();
            pointer_table_holder::template bind_pointer_table<V>();
        }

        template <typename V>
        static constexpr void check_constraints() noexcept
        {
            destructor_policy::template check_constraints<V>();
            move_policy::template check_constraints<V>();
            copy_policy::template check_constraints<V>();
        }
    };
};

template <typename Function>
struct function_policy_descriptor
{
private:
    template <typename X>
    struct trait_helper;

    template <template <typename, std::size_t, typename> typename FunctionType, typename CallSignature, std::size_t N, typename PolicyType>
    struct trait_helper<FunctionType<CallSignature, N, PolicyType>>
    {
        static constexpr std::size_t max_size = N;
        using policy = PolicyType;
        using signature = CallSignature;
    };

    using signature = typename trait_helper<Function>::signature;

    template <typename P>
    using get_copy_policy_wrapper = typename P::copy_policy;

    template <typename P>
    using get_move_policy_wrapper = typename P::move_policy;

    using policy = typename trait_helper<Function>::policy;

    using copy_policy_wrapper = boost::detected_or_t<non_copyable_policy, get_copy_policy_wrapper, policy>;
    using move_policy_wrapper = boost::detected_or_t<non_moveable_policy, get_move_policy_wrapper, policy>;
    using destructor_policy_wrapper = typename policy::destructor_policy;

    using binding_policy = typename policy::binding_policy;
    using destructor_policy = typename destructor_policy_wrapper::template policy<binding_policy, Function>;
    using copy_policy = typename copy_policy_wrapper::template policy<binding_policy, Function>;
    using move_policy = typename move_policy_wrapper::template policy<binding_policy, Function>;
    using switch_holder = bits::switch_function_pointer_holder<policy>;
    using pointer_table_holder = bits::pointer_table_holder<policy, signature>;

public:
    using self = Function;
    static constexpr std::size_t max_size = trait_helper<Function>::max_size;

    using callable_checking_policy = typename policy::callable_checking_policy;
    using call_policy = typename policy::call_policy::template policy<binding_policy, Function, signature>;

    static constexpr bool is_nullable = binding_policy::nullability == policies::nullability::nullable;
    static constexpr bool is_rebindable = binding_policy::rebindability == policies::rebindability::rebindable;

    using call_signature_type = bits::call_signature<signature, policy::call_policy::exception_policy>;

    static constexpr bool is_no_except_callable = 
        policy::call_policy::exception_policy == policies::exception_specification::no_except ||
        boost::callable_traits::is_noexcept_v<signature>;

    static constexpr bool is_copyable = !std::is_same_v<copy_policy_wrapper, non_copyable_policy>;
    static constexpr bool is_moveable = !std::is_same_v<move_policy_wrapper, non_moveable_policy>;

    static constexpr bool is_no_except_callable_checkable = noexcept(callable_checking_policy::check(nullptr));
    static constexpr bool is_no_exceptcopyable = noexcept(std::declval<copy_policy>().copy(std::declval<const std::byte*>()));
    static constexpr bool is_no_except_moveable = noexcept(std::declval<move_policy>().move(std::declval<std::byte*>()));
    static constexpr bool is_no_except_destroyable = noexcept(std::declval<destructor_policy>().destroy());

    static constexpr bool is_trivially_copyable = std::is_same_v<copy_policy_wrapper, policies::trivial_copy_policy>;
    static constexpr bool is_trivially_moveable = std::is_same_v<move_policy_wrapper, policies::trivial_move_policy>;
    static constexpr bool is_trivially_destroyable = std::is_same_v<destructor_policy_wrapper, policies::trivial_destructor_policy>;

    struct sized_data_holder
    {
        friend self;

        std::byte* data() noexcept { return _data.data(); }
        const std::byte* data() const noexcept { return _data.data(); }

        std::array<std::byte, max_size> _data;
    };

    struct empty_data_holder
    {
        friend self;

        std::byte* data() noexcept { return nullptr; }
        const std::byte* data() const noexcept { return nullptr; }
    };

    using data_holder = std::conditional_t<max_size != 0, sized_data_holder, empty_data_holder>;

    struct policy_holder
        : bits::pre_destroy_behaviour<policy_holder, is_trivially_destroyable, is_no_except_destroyable>
        , private destructor_policy
        , private copy_policy
        , private move_policy
        , private call_policy
        , private switch_holder
        , private pointer_table_holder
        , private data_holder
        , bits::copy_behaviour<policy_holder, is_copyable, is_trivially_copyable, is_no_exceptcopyable>
        , bits::move_behaviour<policy_holder, is_moveable, is_trivially_moveable, is_no_except_moveable>
        , bits::destroy_behaviour<policy_holder, is_trivially_destroyable, is_no_except_destroyable>
    {
        friend Function;
        friend destructor_policy;
        friend copy_policy;
        friend move_policy;
        friend call_policy;

        using pre_destroy_behaviour = bits::pre_destroy_behaviour<policy_holder, is_trivially_destroyable, is_no_except_destroyable>;
        using copy_behaviour = bits::copy_behaviour<policy_holder, is_copyable, is_trivially_copyable, is_no_exceptcopyable>;
        using move_behaviour = bits::move_behaviour<policy_holder, is_moveable, is_trivially_moveable, is_no_except_moveable>;
        using destroy_behaviour = bits::destroy_behaviour<policy_holder, is_trivially_destroyable, is_no_except_destroyable>;

        friend pre_destroy_behaviour;
        friend copy_behaviour;
        friend move_behaviour;
        friend destroy_behaviour;

        policy_holder() noexcept = default;

        template <typename V>
        policy_holder(boost::mp11::mp_identity<V> v) noexcept :
            destructor_policy{ v },
            copy_policy{ v },
            move_policy{ v },
            call_policy{ v },
            switch_holder{ boost::mp11::mp_identity<policy>{}, v, boost::mp11::mp_identity<signature>{} },
            pointer_table_holder{ v }
        {}

    private:
        void destroy() noexcept(is_no_except_destroyable)
        {
            destructor_policy::destroy();
        }

        void zero() noexcept
        {
            // this is the only thing that is actually used for checking
            // (as its compulsory to have a type id in an any)
            call_policy::zero_call();
            if constexpr (is_nullable)
            {
                destructor_policy::zero_destructor();
            }
            switch_holder::zero_switch_function();
            pointer_table_holder::zero_pointer_table();
        }

        template <typename V>
        void bind() noexcept
        {
            destructor_policy::template bind_destructor<V>();
            copy_policy::template bind_copy<V>();
            move_policy::template bind_move<V>();
            call_policy::template bind_call<V>();
            switch_holder::template bind_switch_function<policy, V, signature>();
            pointer_table_holder::template bind_pointer_table<V>();
        }

        template <typename V>
        static constexpr void check_constraints() noexcept
        {
            destructor_policy::template check_constraints<V>();
            move_policy::template check_constraints<V>();
            copy_policy::template check_constraints<V>();
        }
    };
};

} // namespace bk::erasure::policies
