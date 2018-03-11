// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

namespace orizzonte::utility
{
    /// @brief Function object that can be invoked with arbitrary arguments and
    /// does nothing.
    struct noop
    {
        template <typename... Ts>
        constexpr void operator()(Ts&&...) const noexcept
        {
        }
    };

    inline constexpr noop noop_v{};
}
