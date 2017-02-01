#pragma once

// TODO: to vrm_core?

#include "tuple.hpp"
#include <functional>
#include <type_traits>

namespace orizzonte::impl
{
    template <typename T>
    struct indirect
    {
        T _x;

        template <typename TFwd>
        constexpr indirect(TFwd&& x) noexcept(
            std::is_nothrow_constructible<T, TFwd&&>{})
            : _x(FWD(x))
        {
        }
    };

    template <typename T>
    class fwd_capture_wrapper : private indirect<std::tuple<T>>
    {
    private:
        using decay_element_type = std::decay_t<T>;
        using base_type = indirect<std::tuple<T>>;

        constexpr auto& as_tuple() noexcept
        {
            return static_cast<base_type&>(*this)._x;
        }

        constexpr const auto& as_tuple() const noexcept
        {
            return static_cast<const base_type&>(*this)._x;
        }

    public:
        template <typename TFwd>
        constexpr fwd_capture_wrapper(TFwd&& x) noexcept(
            std::is_nothrow_constructible<base_type, TFwd&&>{})
            : base_type(FWD(x))
        {
        }

        constexpr auto& get() & noexcept
        {
            return std::get<0>(as_tuple());
        }

        constexpr const auto& get() const & noexcept
        {
            return std::get<0>(as_tuple());
        }

        constexpr auto get() &&
            noexcept(std::is_nothrow_move_constructible<decay_element_type>{})
        {
            return std::move(std::get<0>(as_tuple()));
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

    template <typename TF, typename... TFwdCaptures>
    constexpr decltype(auto) apply_fwd_capture(TF&& f, TFwdCaptures&&... fcs)
    // TODO: noexcept
    {
        return orizzonte::impl::multi_apply(
            [&f](auto&&... xs) mutable -> decltype(
                auto) { return f(FWD(xs).get()...); },
            FWD(fcs)...);
    }
}

#define FWD_CAPTURE_PACK(...) \
    orizzonte::impl::fwd_capture_pack(FWD(__VA_ARGS__)...)

#define FWD_COPY_CAPTURE_PACK(...) \
    orizzonte::impl::fwd_copy_capture_pack(FWD(__VA_ARGS__)...)
