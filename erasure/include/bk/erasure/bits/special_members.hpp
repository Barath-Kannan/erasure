// Copyright (c) 2020 Barath Kannan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file license.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

namespace bk::erasure::bits {

template <typename PolicyHolder, bool IsCopyable, bool TriviallyCopyable, bool NoExceptCopyable>
struct copy_behaviour;

template <typename PolicyHolder>
struct copy_behaviour<PolicyHolder, false, false, true>
{
    constexpr copy_behaviour() noexcept = default;
    copy_behaviour(const copy_behaviour&) noexcept = delete;
    copy_behaviour& operator=(const copy_behaviour&) noexcept = delete;
    copy_behaviour(copy_behaviour&&) noexcept = default;
    copy_behaviour& operator=(copy_behaviour&&) noexcept = default;
};

template <typename PolicyHolder>
struct copy_behaviour<PolicyHolder, true, true, true>
{
    constexpr copy_behaviour() noexcept = default;
    copy_behaviour(const copy_behaviour&) noexcept = default;
    copy_behaviour& operator=(const copy_behaviour&) noexcept = default;
    copy_behaviour(copy_behaviour&&) noexcept = default;
    copy_behaviour& operator=(copy_behaviour&&) noexcept = default;
};

static_assert(std::is_trivial_v<copy_behaviour<void, true, true, true>>);

template <typename PolicyHolder, bool NoExcept>
struct copy_behaviour<PolicyHolder, true, false, NoExcept>
{
    copy_behaviour() noexcept = default;
    copy_behaviour(copy_behaviour&&) noexcept = default;
    copy_behaviour& operator=(copy_behaviour&&) noexcept = default;

    copy_behaviour(const copy_behaviour& other) noexcept(NoExcept)
    {
        auto& self = static_cast<PolicyHolder&>(*this);
        const auto src = static_cast<const PolicyHolder&>(other).data();
        self.copy(src);
    }

    copy_behaviour& operator=(const copy_behaviour& other) noexcept(NoExcept)
    {
        auto& self = static_cast<PolicyHolder&>(*this);
        const auto src = static_cast<const PolicyHolder&>(other).data();
        self.copy(src);
        return *this;
    }
};

template <typename PolicyHolder, bool IsMoveable, bool TriviallyMoveable, bool NoExceptCopyable>
struct move_behaviour;

template <typename PolicyHolder>
struct move_behaviour<PolicyHolder, false, false, true>
{
    constexpr move_behaviour() noexcept = default;
    move_behaviour(move_behaviour&&) noexcept = delete;
    move_behaviour& operator=(move_behaviour&&) noexcept = delete;
    move_behaviour(const move_behaviour&) noexcept = default;
    move_behaviour& operator=(const move_behaviour&) noexcept = default;
};

template <typename PolicyHolder>
struct move_behaviour<PolicyHolder, true, true, true>
{
    constexpr move_behaviour() noexcept = default;
    move_behaviour(move_behaviour&&) noexcept = default;
    move_behaviour& operator=(move_behaviour&&) noexcept = default;
    move_behaviour(const move_behaviour&) noexcept = default;
    move_behaviour& operator=(const move_behaviour&) noexcept = default;
};

static_assert(std::is_trivial_v<move_behaviour<void, true, true, true>>);

template <typename PolicyHolder, bool NoExcept>
struct move_behaviour<PolicyHolder, true, false, NoExcept>
{
    move_behaviour() noexcept = default;
    move_behaviour(const move_behaviour&) noexcept = default;
    move_behaviour& operator=(const move_behaviour&) noexcept = default;

    move_behaviour(move_behaviour&& other) noexcept(NoExcept)
    {
        auto& self = static_cast<PolicyHolder&>(*this);
        auto& moved_from = static_cast<PolicyHolder&>(other);
        auto src = moved_from.data();
        self.move(src);
        moved_from.destroy();
        moved_from.zero();
    }

    move_behaviour& operator=(move_behaviour&& other) noexcept(NoExcept)
    {
        auto& self = static_cast<PolicyHolder&>(*this);
        auto& moved_from = static_cast<PolicyHolder&>(other);
        auto src = moved_from.data();
        self.move(src);
        moved_from.destroy();
        moved_from.zero();
        return *this;
    }
};

template <typename PolicyHolder, bool TriviallyDestroyable, bool NoExceptCopyable>
struct pre_destroy_behaviour;

template <typename PolicyHolder>
struct pre_destroy_behaviour<PolicyHolder, true, true>
{
    constexpr pre_destroy_behaviour() noexcept = default;
    constexpr pre_destroy_behaviour(const pre_destroy_behaviour&) noexcept = default;
    constexpr pre_destroy_behaviour(pre_destroy_behaviour&&) noexcept = default;
    constexpr pre_destroy_behaviour& operator=(const pre_destroy_behaviour&) noexcept = default;
    constexpr pre_destroy_behaviour& operator=(pre_destroy_behaviour&&) noexcept = default;
    ~pre_destroy_behaviour() noexcept = default;
};

static_assert(std::is_trivial_v<pre_destroy_behaviour<void, true, true>>);

template <typename PolicyHolder, bool NoExcept>
struct pre_destroy_behaviour<PolicyHolder, false, NoExcept>
{
    constexpr pre_destroy_behaviour() noexcept = default;
    constexpr pre_destroy_behaviour(const pre_destroy_behaviour&) noexcept = default;
    constexpr pre_destroy_behaviour(pre_destroy_behaviour&&) noexcept = default;

    constexpr pre_destroy_behaviour& operator=(const pre_destroy_behaviour&) noexcept 
    {
        static_cast<PolicyHolder&>(*this).destroy();
        return *this;
    }

    constexpr pre_destroy_behaviour& operator=(pre_destroy_behaviour&&) noexcept 
    {
        static_cast<PolicyHolder&>(*this).destroy();
        return *this;
    }
};


template <typename PolicyHolder, bool TriviallyDestroyable, bool NoExceptCopyable>
struct destroy_behaviour;

template <typename PolicyHolder>
struct destroy_behaviour<PolicyHolder, true, true>
{
    constexpr destroy_behaviour() noexcept = default;
    constexpr destroy_behaviour(const destroy_behaviour&) noexcept = default;
    constexpr destroy_behaviour(destroy_behaviour&&) noexcept = default;
    constexpr destroy_behaviour& operator=(const destroy_behaviour&) noexcept = default;
    constexpr destroy_behaviour& operator=(destroy_behaviour&&) noexcept = default;
    ~destroy_behaviour() noexcept = default;
};

static_assert(std::is_trivial_v<destroy_behaviour<void, true, true>>);

template <typename PolicyHolder, bool NoExcept>
struct destroy_behaviour<PolicyHolder, false, NoExcept>
{
    constexpr destroy_behaviour() noexcept = default;
    constexpr destroy_behaviour(const destroy_behaviour&) noexcept = default;
    constexpr destroy_behaviour(destroy_behaviour&&) noexcept = default;
    constexpr destroy_behaviour& operator=(const destroy_behaviour&) noexcept = default;
    constexpr destroy_behaviour& operator=(destroy_behaviour&&) noexcept = default;

    ~destroy_behaviour() noexcept(NoExcept)
    {
        static_cast<PolicyHolder&>(*this).destroy();
    }
};

} // namespace bk::erasure::bits
