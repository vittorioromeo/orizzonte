// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <type_traits>
#include <utility>

namespace orizzonte::meta
{
    namespace detail
    {
        template <auto... Is>
        struct seq_type : std::common_type<decltype(Is)...>
        {
        };

        template <>
        struct seq_type<>
        {
            using type = int;
        };
    }

    /// @brief Alias for `std::integer_sequence` that deduces the constant type.
    /// If the sequence is empty, `int` is deduced.
    template <auto... Is>
    using sequence_t =
        std::integer_sequence<typename detail::seq_type<Is...>::type, Is...>;

    /// @brief `constexpr` variable template for `sequence_t`.
    template <auto... Is>
    inline constexpr sequence_t<Is...> sequence_v{};

    /// @brief `constexpr` variable template for `std::index_sequence_for`.
    template <typename... Ts>
    inline constexpr std::index_sequence_for<Ts...> sequence_for_v{};
}
