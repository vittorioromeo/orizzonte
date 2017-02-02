#pragma once

// TODO: to vrm_core?

#include "tuple.hpp"
#include <functional>
#include <type_traits>
#include <vrm/core/type_traits/forward_like.hpp>

namespace orizzonte::impl
{
    template <typename T>
    class fwd_capture_wrapper
    {
    private:
        using decay_element_type = std::decay_t<T>;
        using tuple_type = std::tuple<T>;
        tuple_type _t;

    public:
        template <typename TFwd>
        constexpr fwd_capture_wrapper(TFwd&& x) noexcept(
            std::is_nothrow_constructible<tuple_type, TFwd&&>{})
            : _t(FWD(x))
        {
        }

        constexpr auto& get() & noexcept
        {
            return std::get<0>(_t);
        }

        constexpr const auto& get() const & noexcept
        {
            return std::get<0>(_t);
        }

        constexpr auto get() &&
            noexcept(std::is_nothrow_move_constructible<decay_element_type>{})
        {
            return std::move(std::get<0>(_t));
        }

        constexpr T forward_get()
        {
            return std::forward<T>(std::get<0>(_t));
        }
    };

    template <typename T>
    constexpr auto fwd_capture(T&& x) noexcept(
        noexcept(fwd_capture_wrapper<T>(FWD(x))))
    {
        return fwd_capture_wrapper<T>(FWD(x));
    }

    template <typename T>
    constexpr auto fwd_copy_capture(T&& x) noexcept(
        noexcept(fwd_capture_wrapper<T>(x)))
    {
        return fwd_capture_wrapper<T>(x);
    }
}

#define FWD_CAPTURE(...) ::orizzonte::impl::fwd_capture(FWD(__VA_ARGS__))

#define FWD_COPY_CAPTURE(...) \
    ::orizzonte::impl::fwd_copy_capture(FWD(__VA_ARGS__))

namespace orizzonte::impl
{
    template <typename... Ts>
    constexpr auto fwd_capture_pack(Ts&&... xs) noexcept(
        noexcept(std::make_tuple(FWD_CAPTURE(xs)...)))
    {
        return std::make_tuple(FWD_CAPTURE(xs)...);
    }

    template <typename... Ts>
    constexpr auto fwd_copy_capture_pack(Ts&&... xs) noexcept(
        noexcept(std::make_tuple(FWD_COPY_CAPTURE(xs)...)))
    {
        return std::make_tuple(FWD_COPY_CAPTURE(xs)...);
    }
}

#define FWD_CAPTURE_PACK(...) \
    orizzonte::impl::fwd_capture_pack(FWD(__VA_ARGS__)...)

#define FWD_COPY_CAPTURE_PACK(...) \
    orizzonte::impl::fwd_copy_capture_pack(FWD(__VA_ARGS__)...)

namespace orizzonte
{
    template <typename TF, typename... TFwdCaptures>
    constexpr decltype(auto) apply_fwd_capture(TF&& f, TFwdCaptures&&... fcs)
    // TODO: noexcept
    {
        return orizzonte::impl::multi_apply(
            [&f](auto&&... xs) -> decltype(auto) {
                return vrm::core::forward_like<TF>(f)(FWD(xs).forward_get()...);
            },
            FWD(fcs)...);
    }
}
