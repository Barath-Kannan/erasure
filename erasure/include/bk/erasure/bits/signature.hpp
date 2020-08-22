// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bk/erasure/policies/enums.hpp"

#include <cstddef>
#include <new>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace bk::erasure::bits {

namespace detail {

template <typename Signature, bool IsNoExcept>
struct call_signature;

template <typename R, typename... Args, bool IsNoExcept>
struct call_signature<R(Args...), IsNoExcept>
{
    using type = R (*)(std::byte*, Args...) noexcept(IsNoExcept);

    template <typename F>
    static constexpr type generate() noexcept
    {
        return [](std::byte * f, Args... args) noexcept(IsNoExcept)->R
        {
            return reinterpret_cast<F&>(*f)(std::forward<Args>(args)...);
        };
    }
};

template <bool IsNoExcept>
struct destroy_signature
{
    using type = void (*)(std::byte* data) noexcept(IsNoExcept);

    template <typename HeldType>
    static constexpr type generate() noexcept
    {
        return [](std::byte * d) noexcept(IsNoExcept)
        {
            static_assert(!std::is_reference_v<HeldType>);
            HeldType& current = reinterpret_cast<HeldType&>(*d);
            current.~HeldType();
        };
    }
};

template <bool IsNoExcept>
struct move_signature
{
    using type = void (*)(std::byte* src, std::byte* dest) noexcept(IsNoExcept);

    template <typename HeldType>
    static constexpr type generate() noexcept
    {
        return [](std::byte * src, std::byte * dest) noexcept(IsNoExcept)
        {
            static_assert(!std::is_reference_v<HeldType>);
            HeldType& current = reinterpret_cast<HeldType&>(*src);
            ::new (dest) HeldType{ std::move(current) };
        };
    }
};

template <bool IsNoExcept>
struct copy_signature
{
    using type = void (*)(const std::byte* src, std::byte* dest) noexcept(IsNoExcept);

    template <typename HeldType>
    static constexpr type generate() noexcept
    {
        return [](const std::byte* src, std::byte* dest) noexcept(IsNoExcept)
        {
            static_assert(!std::is_reference_v<HeldType>);
            const HeldType& current = reinterpret_cast<const HeldType&>(*src);
            ::new (dest) HeldType{ current };
        };
    }
};

struct typeid_signature
{
    using type = const std::type_info*;

    template <typename HeldType>
    static constexpr type generate() noexcept
    {
        return &typeid(HeldType);
    }
};

} // namespace detail

template <policies::exception_specification E>
using destroy_signature = typename detail::destroy_signature<E == policies::exception_specification::no_except>::type;

template <policies::exception_specification E>
using copy_signature = typename detail::copy_signature<E == policies::exception_specification::no_except>::type;

template <policies::exception_specification E>
using move_signature = typename detail::move_signature<E == policies::exception_specification::no_except>::type;

template <typename Signature, policies::exception_specification E>
using call_signature = typename detail::call_signature<Signature, E == policies::exception_specification::no_except>::type;

using typeid_signature = typename detail::typeid_signature::type;

template <typename HeldType, policies::exception_specification E>
constexpr destroy_signature<E> generate_destroy() noexcept
{
    return detail::destroy_signature<E == policies::exception_specification::no_except>::template generate<HeldType>();
}

template <typename HeldType, policies::exception_specification E>
constexpr copy_signature<E> generate_copy() noexcept
{
    return detail::copy_signature<E == policies::exception_specification::no_except>::template generate<HeldType>();
}

template <typename HeldType, policies::exception_specification E>
constexpr move_signature<E> generate_move() noexcept
{
    return detail::move_signature<E == policies::exception_specification::no_except>::template generate<HeldType>();
}

template <typename F, typename Signature, policies::exception_specification E>
constexpr call_signature<Signature, E> generate_call() noexcept
{
    return detail::call_signature<Signature, E == policies::exception_specification::no_except>::template generate<F>();
}

template <typename HeldType>
constexpr typeid_signature generate_typeid() noexcept
{
    return detail::typeid_signature::template generate<HeldType>();
}

} // namespace bk::erasure::bits
