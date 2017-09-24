// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "./fwd.hpp"
#include <tuple>
#include <type_traits>

namespace orizzonte::utility
{
    /// @brief Empty `struct` used to detect and represent a missing/placeholder
    /// value/type.
    struct nothing
    {
    };

    /// @brief Instance of `nothing`.
    inline constexpr nothing nothing_v{};

    /// @brief If `T` is `void`, evaluates to `nothing`. Otherwise evaluates to
    /// `T`.
    /// @details This evaluates to `false` if `T` is not exactly `void` (e.g.
    /// `void*` or `void&`).
    template <typename T>
    using void_to_nothing_t = std::conditional_t<std::is_void_v<T>, nothing, T>;

    /// @brief Evaluates to `true` if `std::decay_t<T>` is `nothing`.
    /// @details This covers `nothing&` and `const nothing&`, but doesn't cover
    /// `nothing*`.
    template <typename T>
    using is_nothing_t = std::is_same<std::decay_t<T>, nothing>;

    /// @brief Variable template for `is_nothing_t`.
    template <typename T>
    inline constexpr bool is_nothing_v = is_nothing_t<T>::value;

    // TODO: SFINAE-friendliness and `noexcept`-correctness
    /// @brief Invokes `f(xs...)`. If the invocation would return `void`,
    /// `nothing` is returned instead. Otherwise, the return value is
    /// untouched.
    template <typename F, typename... Ts>
    constexpr decltype(auto) returning_nothing_instead_of_void(
        F&& f, Ts&&... xs)
    {
        if constexpr(std::is_void_v<decltype(FWD(f)(FWD(xs)...))>)
        {
            FWD(f)(FWD(xs)...);

            // Not using `nothing_v` here as a reference would be returned
            // instead.
            return nothing{};
        }
        else
        {
            return FWD(f)(FWD(xs)...);
        }
    }

    // TODO: SFINAE-friendliness and `noexcept`-correctness
    /// @copydoc call_ignoring_nothing
    template <typename F>
    constexpr decltype(auto) call_ignoring_nothing(F&& f)
    {
        return returning_nothing_instead_of_void(FWD(f));
    }

    // TODO: SFINAE-friendliness and `noexcept`-correctness
    // TODO: can this be done iteratively?
    /// @brief Invokes `f(x, xs...)`, skipping every argument that is `nothing`.
    template <typename F, typename T, typename... Ts>
    constexpr decltype(auto) call_ignoring_nothing(F&& f, T&& x, Ts&&... xs)
    {
        return call_ignoring_nothing(
            [&f, &x](auto&&... ys) -> decltype(auto) {
                if constexpr(is_nothing_v<T>)
                {
                    return FWD(f)(FWD(ys)...);
                }
                else
                {
                    return FWD(f)(FWD(x), FWD(ys)...);
                }
            },
            FWD(xs)...);
    }

    // TODO: SFINAE-friendliness and `noexcept`-correctness
    /// @brief Applies `t` to `f`, using `call_ignoring_nothing` during the
    /// expansion.
    template <typename F, typename T>
    constexpr decltype(auto) apply_ignoring_nothing(F&& f, T&& t)
    {
        return std::apply(
            [&f](auto&&... xs) -> decltype(auto) {
                return call_ignoring_nothing(FWD(f), FWD(xs)...);
            },
            FWD(t));
    }

    /// @brief Evaluates to the result of `call_ignoring_nothing(F{}(Ts{}...))`.
    template <typename F, typename... Ts>
    using result_of_ignoring_nothing_t = decltype(
        call_ignoring_nothing(std::declval<F>(), std::declval<Ts>()...));

    /// @brief Wraps `F` in a new object that uses `call_ignoring_nothing` upon
    /// `operator()` invocation.
    template <typename F>
    struct ignore_nothing : F
    {
        template <typename FFwd>
        constexpr ignore_nothing(FFwd&& f) noexcept(noexcept(F{FWD(f)}))
            : F{FWD(f)}
        {
        }

        // clang-format off
        #define CALL_OPERATOR(ref_qual)                                                       \
        template <typename... Ts>                                                             \
        constexpr auto operator()(Ts&&... xs) ref_qual                                        \
        noexcept(noexcept(call_ignoring_nothing(std::declval<F ref_qual>(),     FWD(xs)...))) \
              -> decltype(call_ignoring_nothing(std::declval<F ref_qual>(),     FWD(xs)...))  \
        {          return call_ignoring_nothing(static_cast<F ref_qual>(*this), FWD(xs)...);  \
        }

        CALL_OPERATOR(&)
        CALL_OPERATOR(&&)
        CALL_OPERATOR(const&)
        #undef CALL_OPERATOR
        // clang-format on
    };

    template <typename FFwd>
    ignore_nothing(FFwd &&)->ignore_nothing<std::decay_t<FFwd>>;
}
