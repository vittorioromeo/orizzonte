// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <type_traits>
#include <utility>

namespace orizzonte::meta
{
    /// @brief Empty class that "stores" the type `T`.
    template <typename T>
    struct type_wrapper
    {
        using type = T;
    };

    /// @copydoc type_wrapper
    template <typename T>
    using type = type_wrapper<T>;

    /// @brief `constexpr` variable template for `type_wrapper`.
    template <typename T>
    inline constexpr type_wrapper<T> t{};

    /// @brief Extracts the type "stored" into a `type_wrapper` instance.
    template <typename T>
    using unwrap = typename T::type;
}
