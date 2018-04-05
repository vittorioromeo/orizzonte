// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "../meta/type_wrapper.hpp"
#include "../utility/nothing.hpp"
#include <boost/callable_traits.hpp>
#include <experimental/type_traits>
#include <utility>

namespace orizzonte::node::detail
{
    template <typename T>
    struct in_t
    {
    };

    template <bool B, typename Scheduler, typename F>
    void schedule_if(Scheduler& scheduler, F&& f)
    {
        if constexpr(B)
        {
            FWD(f)();
        }
        else
        {
            // The computation has to be moved here as it will die at
            // the end of the `enumerate_args` lambda scope.
            scheduler([g = std::move(f)]() mutable { g(); });
        }
    }

    template <typename... Xs, typename Index, typename Scheduler, typename F>
    void schedule_if_last(Index, Scheduler& scheduler, F&& f)
    {
        constexpr bool is_last = Index{} == sizeof...(Xs) - 1;
        schedule_if<is_last>(scheduler, FWD(f));
    }

    template <typename Tuple>
    struct first_arg_impl;

    template <typename C>
    struct first_arg_impl<std::tuple<C>> : meta::type<utility::nothing>
    {
    };

    template <typename C, typename T>
    struct first_arg_impl<std::tuple<C, T>> : meta::type<T>
    {
    };

    template <typename F>
    using first_arg_t =
        typename first_arg_impl<boost::callable_traits::args_t<F>>::type;

    namespace detail
    {
        template <typename T>
        using is_executable_impl = decltype(std::declval<T&>().execute(
            std::declval<int&>(), std::declval<int&&>(), std::declval<int&&>(),
            std::declval<int&&>()));
    }

    template <typename T>
    using is_executable =
        std::experimental::is_detected<detail::is_executable_impl, T>;
}

namespace orizzonte::node
{
    template <typename T>
    inline constexpr detail::in_t<utility::void_to_nothing_t<T>> in{};
}
