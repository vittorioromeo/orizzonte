// TODO: fix copyrights
// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <cstddef>
#include <tuple>
#include "./fwd.hpp"

namespace orizzonte::utility
{
    // TODO: use `std::hardware_destructive_inference_size` instead of `64`
    inline constexpr std::size_t cache_line_size = 64;

    namespace detail
    {
        template <typename T>
        struct cache_aligned
        {
            alignas(cache_line_size) T _x;

            template <typename... Args>
            constexpr cache_aligned(Args&&... args) : _x{FWD(args)...}
            {
            }
        };
    } // namespace detail

    /// @brief TODO
    template <typename... Ts>
    class cache_aligned_tuple : private std::tuple<detail::cache_aligned<Ts>...>
    {
    private:
        using base_type = std::tuple<detail::cache_aligned<Ts>...>;

    public:
        template <typename... TFwds,
            typename = std::enable_if_t<(
                !std::is_same_v<cache_aligned_tuple, std::decay_t<TFwds>> &&
                ...)>>
        constexpr cache_aligned_tuple(TFwds&&... xs)
            : base_type{detail::cache_aligned<TFwds>{FWD(xs)}...}
        {
        }

        cache_aligned_tuple(const cache_aligned_tuple&) = default;
        cache_aligned_tuple(cache_aligned_tuple&&) = default;

        cache_aligned_tuple& operator=(const cache_aligned_tuple&) = default;
        cache_aligned_tuple& operator=(cache_aligned_tuple&&) = default;

        using base_type::swap;

        template <typename T>
        T& get() &
        {
            return std::get<detail::cache_aligned<T>>(*this)._x;
        }

        template <typename T>
        const T& get() const&
        {
            return std::get<detail::cache_aligned<T>>(*this)._x;
        }

        template <typename T>
        T&& get() &&
        {
            return std::move(std::get<detail::cache_aligned<T>>(*this)._x);
        }

        template <std::size_t I>
        auto& get() &
        {
            return std::get<I>(*this)._x;
        }

        template <std::size_t I>
        const auto& get() const&
        {
            return std::get<I>(*this)._x;
        }

        template <std::size_t I>
        auto&& get() &&
        {
            return std::move(std::get<I>(*this)._x);
        }
    };

    template <typename T, typename Tuple>
    auto get(Tuple&& tuple) -> decltype(FWD(tuple).template get<T>())
    {
        return FWD(tuple).template get<T>();
    }

    template <std::size_t I, typename Tuple>
    auto get(Tuple&& tuple) -> decltype(FWD(tuple).template get<I>())
    {
        return FWD(tuple).template get<I>();
    }
} // namespace orizzonte::utility
