#pragma once

#include "fwd_capture.hpp"
#include "tuple.hpp"
#include <type_traits>
#include <utility>
#include <vrm/core/type_traits/forward_like.hpp>

namespace orizzonte::impl
{
    using vrm::core::forward_like;

    struct nothing_t
    {
    };

    constexpr nothing_t nothing{};

    template <typename T>
    constexpr auto is_nothing(T&&) noexcept
    {
        return std::is_same<std::decay_t<T>, nothing_t>{};
    }

    template <typename T>
    using void_to_nothing_t =
        std::conditional_t<std::is_same_v<T, void>, nothing_t, T>;

    template <typename TF>
    decltype(auto) call_ignoring_nothing(TF&& f);
    //    noexcept(std::is_nothrow_callable<TF()>{});

    template <typename TF, typename... Ts>
    decltype(auto) call_ignoring_nothing(TF&& f, nothing_t, Ts&&... xs);

    template <typename TF, typename T, typename... Ts>
    decltype(auto) call_ignoring_nothing(TF&& f, T&& x, Ts&&... xs)
    {
        // Bind `x` to the function call and recurse.
        return call_ignoring_nothing([ f = FWD_CAPTURE(f), x = FWD_CAPTURE(x) ](
                                         auto&&... ys) mutable->decltype(auto) {
            return forward_like<TF>(f.get())(
                forward_like<T>(x.get()), FWD(ys)...);
        },
            FWD(xs)...);
    }

    template <typename TF, typename... Ts>
    decltype(auto) call_ignoring_nothing(TF&& f, nothing_t, Ts&&... xs)
    {
        // Skip the `nothing_t` argument and recurse.
        return call_ignoring_nothing(FWD(f), FWD(xs)...);
    }

    template <typename TF>
    decltype(auto) call_ignoring_nothing(TF&& f)
    //    noexcept(std::is_nothrow_callable<TF()>{})
    {
        // Base case.
        return FWD(f)();
    }

    template <typename TF, typename... Ts>
    decltype(auto) with_void_to_nothing(TF&& f, Ts&&... xs)
    {
#define BOUND_F() call_ignoring_nothing(f, FWD(xs)...)

        if
            constexpr(std::is_same<decltype(BOUND_F()), void>{})
            {
                BOUND_F();
                return nothing;
            }
        else
        {
            return BOUND_F();
        }

#undef BOUND_F
    }

    template <typename TF>
    auto bind_return_void_to_nothing(TF&& f)
    {
        return [f = FWD_CAPTURE(f)](auto&&... xs) mutable->decltype(auto)
        {
            return with_void_to_nothing(forward_like<TF>(f.get()), FWD(xs)...);
        };
    }

    template <typename TF, typename TTuple>
    decltype(auto) apply_ignore_nothing(TF&& f, TTuple&& t)
    {
        return orizzonte::impl::apply(
            [f = FWD_CAPTURE(f)](auto&&... xs) mutable->decltype(auto) {
                return with_void_to_nothing(
                    forward_like<TF>(f.get()), FWD(xs)...);
            },
            FWD(t));
    }

    template <typename TF, typename TTuple>
    decltype(auto) for_tuple_ignore_nothing(TF&& f, TTuple&& t)
    {
        return for_tuple(
            [&f](auto&&... xs) { return with_void_to_nothing(f, FWD(xs)...); },
            FWD(t));
    }
}
