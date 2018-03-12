// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <utility>
#include "../utility/nothing.hpp"

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
}

namespace orizzonte::node
{
    template <typename T>
    inline constexpr detail::in_t<utility::void_to_nothing_t<T>> in{};
}
