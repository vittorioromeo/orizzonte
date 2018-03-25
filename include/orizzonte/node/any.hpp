// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "../utility/aligned_storage.hpp"
#include "./helper.hpp"
#include <atomic>
#include <boost/variant.hpp>
#include <iostream>
#include <type_traits>

namespace orizzonte::node
{
    template <typename... Fs>
    class any : Fs...
    {
    public:
        using in_type = std::common_type_t<typename Fs::in_type...>;

        // TODO: if they're all the same type, no need for variant
        using out_type = boost::variant<typename Fs::out_type...>;

    private:
        struct shared_state
        {
            ORIZZONTE_CACHE_ALIGNED in_type _input;
            ORIZZONTE_CACHE_ALIGNED std::atomic<int> _left;

            template <typename Input>
            shared_state(Input&& input) : _input{FWD(input)}
            {
                _left.store(sizeof...(Fs));
            }
        };

        using shared_state_storage = utility::aligned_storage_for<shared_state>;

        ORIZZONTE_CACHE_ALIGNED shared_state_storage _state;
        ORIZZONTE_CACHE_ALIGNED out_type _values;

    public:
        constexpr any(Fs&&... fs) : Fs{std::move(fs)}...
        {
        }

        template <typename Scheduler, typename Input, typename Then,
            typename Cleanup>
        void execute(Scheduler& scheduler, Input&& input, Then&& then,
            Cleanup&& cleanup) &
        {
            // TODO: don't construct/destroy if lvalue?
            _state.construct(FWD(input));

            meta::enumerate_args(
                [&](auto i, auto t) {
                    auto& f = static_cast<meta::unwrap<decltype(t)>&>(*this);
                    auto computation = [this, &scheduler, &f, then,
                                           cleanup /* TODO: fwd capture */] {
                        f.execute(scheduler, _state->_input,
                            [this, then, cleanup](auto&& out) {
                                const auto r = _state->_left.fetch_sub(1);
                                if(r == sizeof...(Fs))
                                {
                                    _values = out; // TODO: fwd?
                                    then(_values); // TODO: move?
                                }

                                if(r == 1)
                                {
                                    _state.destroy();
                                    cleanup();
                                }
                            },
                            cleanup);
                    };

                    detail::schedule_if_last<Fs...>(
                        i, scheduler, std::move(computation));
                },
                meta::t<Fs>...);
        }

        static constexpr std::size_t cleanup_count() noexcept
        {
            return (Fs::cleanup_count() + ...) + 1;
        }
    };
}

// TODO:
template <int N, typename... Ts>
using NthTypeOf = typename std::tuple_element<N, std::tuple<Ts...>>::type;

template <int N, typename... Ts>
auto& get(boost::variant<Ts...>& v)
{
    using target = NthTypeOf<N, Ts...>;
    return boost::get<target>(v);
}

template <int N, typename... Ts>
auto& get(const boost::variant<Ts...>& v)
{
    using target = NthTypeOf<N, Ts...>;
    return boost::get<target>(v);
}
