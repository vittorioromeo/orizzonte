#pragma once

#include "./fwd.hpp"
#include <tuple>
#include <type_traits>

namespace orizzonte::utility
{
    struct nothing
    {
    };

    inline constexpr nothing nothing_v{};

    template <typename T>
    using void_to_nothing_t = std::conditional_t<std::is_void_v<T>, nothing, T>;

    template <typename T>
    using is_nothing_t = std::is_same<std::decay_t<T>, nothing>;

    template <typename T>
    inline constexpr bool is_nothing_v = is_nothing_t<T>::value;

    // TODO: SFINAE-friendliness and `noexcept`-correctness
    template <typename F, typename... Ts>
    constexpr decltype(auto) returning_nothing_instead_of_void(
        F&& f, Ts&&... xs)
    {
        if constexpr(std::is_void_v<decltype(FWD(f)(FWD(xs)...))>)
        {
            FWD(f)(FWD(xs)...);
            return nothing{};
        }
        else
        {
            return FWD(f)(FWD(xs)...);
        }
    }

    // TODO: SFINAE-friendliness and `noexcept`-correctness
    template <typename F>
    constexpr decltype(auto) call_ignoring_nothing(F&& f)
    {
        return returning_nothing_instead_of_void(FWD(f));
    }

    // TODO: SFINAE-friendliness and `noexcept`-correctness
    // TODO: can this be done iteratively?
    template <typename F, typename T, typename... Ts>
    constexpr decltype(auto) call_ignoring_nothing(F&& f, T&& x, Ts&&... xs)
    {
        return call_ignoring_nothing(
            [&](auto&&... ys) -> decltype(auto) {
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
    template <typename F, typename T>
    constexpr decltype(auto) apply_ignoring_nothing(F&& f, T&& t)
    {
        return std::apply(
            [&](auto&&... xs) -> decltype(auto) {
                return call_ignoring_nothing(FWD(f), FWD(xs)...);
            },
            FWD(t));
    }

    template <typename F, typename... Ts>
    using result_of_ignoring_nothing_t = decltype(
        call_ignoring_nothing(std::declval<F>(), std::declval<Ts>()...));

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
