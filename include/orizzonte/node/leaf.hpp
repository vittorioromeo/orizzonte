// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "../utility/noop.hpp"
#include "../utility/nothing.hpp"
#include "./helper.hpp"
#include <boost/callable_traits.hpp>
#include <type_traits>

namespace orizzonte::node
{
    template <typename In, typename F>
    struct leaf : F
    {
    public:
        using in_type = In;
        using out_type = utility::result_of_ignoring_nothing_t<F&, in_type>;

        constexpr leaf(F&& f) : F{std::move(f)}
        {
        }

        constexpr leaf(detail::in_t<In>, F&& f) : F{std::move(f)}
        {
        }

        template <typename Scheduler, typename Input, typename Then,
            typename Cleanup>
        void execute(Scheduler&, Input&& input, Then&& then = utility::noop_v,
            Cleanup&& cleanup = utility::noop_v) &
        {
            (void)cleanup;

            // A `leaf` doesn't schedule a computation on a separate thread
            // by default. The parent of the `leaf` takes care of this if
            // desired.
            FWD(then)(utility::call_ignoring_nothing(*this, FWD(input)));
            // FWD(cleanup)();
        }

        static constexpr std::size_t count() noexcept
        {
            return 0;
        }
    };

    template <typename R, typename Arg>
    leaf(R (*)(Arg))->leaf<Arg, R (*)(Arg)>;

    template <typename F, typename S = decltype(&std::decay_t<F>::operator())>
    leaf(F &&)->leaf<detail::second_arg_t<S>, std::decay_t<F>>;
}
