// TODO: fix copyrights
// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "./fwd.hpp"
#include <cstddef>
#include <tuple>

namespace orizzonte::utility
{
    // TODO: use `std::hardware_destructive_inference_size` instead of `64`
    inline constexpr std::size_t cache_line_size = 64;
}

#define ORIZZONTE_CACHE_ALIGNED alignas(::orizzonte::utility::cache_line_size)

namespace orizzonte::utility
{
    namespace detail
    {
        template <typename T>
        struct cache_aligned
        {
            ORIZZONTE_CACHE_ALIGNED T _x;

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

#define DEFINE_TYPE_GET(qualifier)                         \
    template <typename T>                                  \
    T qualifier get() qualifier                            \
    {                                                      \
        return static_cast<T qualifier>(                   \
            std::get<detail::cache_aligned<T>>(*this)._x); \
    }

        DEFINE_TYPE_GET(&)
        DEFINE_TYPE_GET(const&)
        DEFINE_TYPE_GET(&&)
#undef DEFINE_TYPE_GET

#define DEFINE_IDX_GET(qualifier)                               \
    template <std::size_t I>                                    \
    auto qualifier get() qualifier                              \
    {                                                           \
        using T = decltype(std::get<I>(*this)._x);              \
        return static_cast<T qualifier>(std::get<I>(*this)._x); \
    }

        DEFINE_IDX_GET(&)
        DEFINE_IDX_GET(const&)
        DEFINE_IDX_GET(&&)
#undef DEFINE_IDX_GET
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
