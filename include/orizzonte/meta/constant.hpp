// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <utility>

namespace orizzonte::meta
{
    /// @brief Alias for `std::integral_constant` that deduces the constant type.
    template <auto X>
    using constant_t = std::integral_constant<decltype(X), X>;

    /// @brief `constexpr` variable template for `constant_t`.
    template <auto X>
    inline constexpr constant_t<X> constant_v{};

    /// @copydoc constant_v
    template <auto X>
    inline constexpr constant_t<X> c{};
}
