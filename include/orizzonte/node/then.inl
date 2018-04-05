// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "./helper.hpp"
#include "./leaf.hpp"
#include "./seq.hpp"

// TODO:

namespace orizzonte::node
{
    template <typename In, typename F>
    template <typename X>
    auto leaf<In, F>::then(X&& x)
    {
        if constexpr(detail::is_executable<X>{})
        {
            return orizzonte::node::seq{std::move(*this), FWD(x)};
        }
        else
        {
            return orizzonte::node::seq{
                std::move(*this), orizzonte::node::leaf{FWD(x)}};
        }
    }

    template <typename A, typename B>
    template <typename X>
    auto seq<A, B>::then(X&& x)
    {
        if constexpr(detail::is_executable<X>{})
        {
            return orizzonte::node::seq{std::move(*this), FWD(x)};
        }
        else
        {
            return orizzonte::node::seq{
                std::move(*this), orizzonte::node::leaf{FWD(x)}};
        }
    }
}
