#pragma once

// TODO: to vrm_core?

#include "tuple.hpp"
#include <functional>
#include <type_traits>
#include <vrm/core/type_traits/forward_like.hpp>

namespace orizzonte::impl
{
    template <typename T>
    class value_wrapper
    {
        static_assert(!std::is_reference<T>{});

    private:
        T _value;

    public:
        using type = T;

        template <typename TFwd>
        constexpr value_wrapper(TFwd&& x) noexcept(
            std::is_nothrow_constructible<T, TFwd&&>{})
            : _value(FWD(x))
        {
        }

        template <typename... TArgs,
            typename = std::enable_if_t<std::is_callable<T&(TArgs&&...)>{}>>
        constexpr decltype(auto) operator()(TArgs&&... args) noexcept(
            noexcept(_value(FWD(args)...)))
        {
            return _value(FWD(args)...);
        }

        template <typename... TArgs,
            typename =
                std::enable_if_t<std::is_callable<const T&(TArgs&&...)>{}>>
        constexpr decltype(auto) operator()(TArgs&&... args) const
            noexcept(noexcept(_value(FWD(args)...)))
        {
            return _value(FWD(args)...);
        }

        constexpr auto& get() & noexcept
        {
            return _value;
        }

        constexpr const auto& get() const & noexcept
        {
            return _value;
        }

        constexpr T get() && noexcept
        {
            return std::move(_value);
        }

        constexpr operator T&() & noexcept
        {
            return _value;
        }

        constexpr operator const T&() const noexcept
        {
            return _value;
        }

        constexpr operator T() && noexcept
        {
            return std::move(_value);
        }
    };

    template <typename T>
    using perfect_wrapper = std::conditional_t<             // .
        std::is_lvalue_reference<T>{},                      // .
        std::reference_wrapper<std::remove_reference_t<T>>, // .
        value_wrapper<T>                                    // .
        >;

    template <typename T>
    class fwd_capture_wrapper : public perfect_wrapper<T>
    {
    private:
        using base_type = perfect_wrapper<T>;

    public:
        template <typename TFwd>
        constexpr fwd_capture_wrapper(TFwd&& x) noexcept(
            std::is_nothrow_constructible<base_type, TFwd&&>{})
            : base_type(FWD(x))
        {
        }

        /// @brief Gets the wrapped perfectly-captured object by forwarding it
        /// into the return value.
        /// @details Returns `std::forward<T>(this->get())`. In practice:
        /// * If the wrapped object is a value, it gets moved out (instead of
        /// returning a reference to it like `get()` would).
        /// * If the wrapped object is a reference, a reference gets returned.
        /// This happens because `T` is a reference type.
        constexpr T fwd_get() noexcept(std::is_nothrow_constructible<T, T&&>{})
        {
            // Note: `FWD` is not applicable here.
            return std::forward<T>(this->get());
        }

        constexpr T fwd_get() const = delete;
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
                return static_cast<TF&&>(f)(xs.fwd_get()...);
            },
            FWD(fcs)...);
    }
}
