// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "./bool_latch.hpp"
#include "./fwd.hpp"
#include "./nothing.hpp"
#include <type_traits>

namespace orizzonte::utility
{
    /// @brief TODO
    template <typename Scheduler, typename Graph, typename Then>
    void sync_execute(Scheduler&& scheduler, Graph&& graph, Then&& then)
    {
        constexpr int count = std::decay_t<Graph>::cleanup_count() + 1;
        utility::scoped_int_latch l{count};

        graph.execute(scheduler, nothing_v,
            [&](auto&&... res) {
                then(FWD(res)...);
                // call_ignoring_nothing(then, FWD(res)...);
                l.count_down();
            },
            [&] { l.count_down(); });
    }
}
