// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "../utility/fwd.hpp"
#include "./constant.hpp"
#include "./sequence.hpp"

namespace orizzonte::meta
{
    namespace detail
    {
        template <typename F, typename... Ts, auto... Is>
        void enumerate_args_impl(std::index_sequence<Is...>, F&& f, Ts&&... xs)
        {
            // `std::index_sequence` is used instead of `sequence_t` as
            // `sequence_for_v<>` is a `std::size_t` sequence.
            (f(c<Is>, FWD(xs)), ...);
        }
    }

    /// @brief Inkoves `f(i, x)` for every element in `xs...`, where `i` is a
    /// compile-time integer of the current "iteration" and `x` is the `i`-th
    /// element of `xs...`.
    template <typename F, typename... Ts>
    void enumerate_args(F&& f, Ts&&... xs)
    {
        detail::enumerate_args_impl(sequence_for_v<Ts...>, FWD(f), FWD(xs)...);
    }
}
