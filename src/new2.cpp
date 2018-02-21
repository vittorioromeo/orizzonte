#include "../include/orizzonte.hpp"
#include <atomic>
#include <boost/variant.hpp>
#include <chrono>
#include <experimental/type_traits>
#include <iostream>
#include <thread>
#include <tuple>
#include <utility>

struct S
{
    template <typename F>
    void operator()(F&& f)
    {
        std::thread{std::move(f)}.detach();
    }
};

template <typename T>
struct in_t
{
};

template <typename T>
inline constexpr in_t<T> in{};

struct do_nothing final
{
    template <typename... Ts>
    constexpr void operator()(Ts&&...) const noexcept
    {
    }
};

inline constexpr do_nothing do_nothing_v{};

template <typename In, typename F>
struct leaf : F
{
    using in_type = In;
    using out_type = decltype(std::declval<F&>()(std::declval<in_type>()));

    leaf(in_t<In>, F&& f) : F{std::move(f)}
    {
    }

    template <typename Scheduler, typename Input, typename Then>
    void execute(
        Scheduler& scheduler, Input&& input, Then&& then = do_nothing_v)
    {
        (void)scheduler;
        FWD(then)((*this)(FWD(input)));
    }
};

template <typename A, typename B>
struct node_seq : A, B
{
    using in_type = typename A::in_type;
    using out_type = typename B::out_type;

    node_seq(A&& a, B&& b) : A{std::move(a)}, B{std::move(b)}
    {
    }

    template <typename Scheduler, typename Input, typename Then>
    void execute(
        Scheduler& scheduler, Input&& input, Then&& then = do_nothing_v)
    {
        static_cast<A&>(*this).execute(
            scheduler, FWD(input), [this, &scheduler, then](auto&& out) {
                static_cast<B&>(*this).execute(scheduler, out, then);
            });
    }
};

template <typename...>
using int_t = int;

template <typename... Fs>
struct node_and : Fs...
{
    using in_type = std::common_type_t<typename Fs::in_type...>;
    using out_type = std::tuple<typename Fs::out_type...>;

    struct shared_state
    {
        in_type _input;
        // TODO: padding
        std::atomic<int> _left{sizeof...(Fs)};

        template <typename Input>
        shared_state(Input&& input) : _input{FWD(input)}
        {
        }
    };

    orizzonte::utility::aligned_storage_for<shared_state> _state;
    // TODO: padding
    out_type _values;

    node_and(Fs&&... fs) : Fs{std::move(fs)}...
    {
    }

    template <bool B, typename Scheduler, typename F>
    void schedule_if(Scheduler& s, F&& f)
    {
        if constexpr(B)
        {
            FWD(f)();
        }
        else
        {
            // The computation has to be moved here as it will die at
            // the end of the `enumerate_args` lambda scope.
            s([g = std::move(f)]() mutable { g(); });
        }
    }

    template <typename Scheduler, typename Input, typename Then>
    void execute(Scheduler& scheduler, Input&& input, Then&& then)
    {
        // TODO: don't construct/destroy if lvalue?
        _state.construct(FWD(input));

        orizzonte::meta::enumerate_args(
            [&](auto i, auto t) {
                auto& f =
                    static_cast<orizzonte::meta::unwrap<decltype(t)>&>(*this);
                auto computation = [&, then /* TODO: fwd capture */] {
                    f.execute(scheduler, _state->_input, [&](auto&& out) {
                        // TODO: padded tuple
                        std::get<i>(_values) = out;

                        if(_state->_left.fetch_sub(1) == 1)
                        {
                            _state.destroy();
                            then(_values); // TODO: move?
                        }
                    });
                };

                constexpr bool is_last = decltype(i){} == sizeof...(Fs) - 1;
                schedule_if<is_last>(scheduler, std::move(computation));
            },
            orizzonte::meta::t<Fs>...);
    }

    /*template <typename Scheduler, typename Input>
    void execute(Scheduler& scheduler, Input&& input)
    {
        auto doit = [&](auto& f)
        {
            scheduler([&]
            {
                f.execute(scheduler, input, [&](auto&& out)
                {
                    // std::get<0>(_values) = out;
                });
            });
        };

        (doit(static_cast<Fs&>(*this)), ...);
        _left = 0;
    }*/
};

template <typename... Fs>
struct node_any : Fs...
{
    using in_type = std::common_type_t<typename Fs::in_type...>;
    using out_type = boost::variant<typename Fs::out_type...>;

    struct shared_state
    {
        in_type _input;
        // TODO: padding
        std::atomic<int> _left{sizeof...(Fs)};

        template <typename Input>
        shared_state(Input&& input) : _input{FWD(input)}
        {
        }
    };

    orizzonte::utility::aligned_storage_for<shared_state> _state;
    // TODO: padding
    out_type _values;

    node_any(Fs&&... fs) : Fs{std::move(fs)}...
    {
    }

    template <bool B, typename Scheduler, typename F>
    void schedule_if(Scheduler& s, F&& f)
    {
        if constexpr(B)
        {
            FWD(f)();
        }
        else
        {
            // The computation has to be moved here as it will die at
            // the end of the `enumerate_args` lambda scope.
            s([g = std::move(f)]() mutable { g(); });
        }
    }

    template <typename Scheduler, typename Input, typename Then>
    void execute(Scheduler& scheduler, Input&& input, Then&& then)
    {
        // TODO: don't construct/destroy if lvalue?
        _state.construct(FWD(input));

        orizzonte::meta::enumerate_args(
            [&](auto i, auto t) {
                auto& f =
                    static_cast<orizzonte::meta::unwrap<decltype(t)>&>(*this);
                auto computation = [&, then /* TODO: fwd capture */] {
                    f.execute(scheduler, _state->_input, [&](auto&& out) {
                        const auto old_left = _state->_left.fetch_sub(1);
                        if(old_left == sizeof...(Fs))
                        {
                            _values = out;
                            then(_values); // TODO: move?
                        }
                        else if(old_left == 1)
                        {
                            _state.destroy();
                        }
                    });
                };

                constexpr bool is_last = decltype(i){} == sizeof...(Fs) - 1;
                schedule_if<is_last>(scheduler, std::move(computation));
            },
            orizzonte::meta::t<Fs>...);
    }

    /*template <typename Scheduler, typename Input>
    void execute(Scheduler& scheduler, Input&& input)
    {
        auto doit = [&](auto& f)
        {
            scheduler([&]
            {
                f.execute(scheduler, input, [&](auto&& out)
                {
                    // std::get<l0>(_values) = out;
                });
            });
        };

        (doit(static_cast<Fs&>(*this)), ...);
        _left = 0;
    }*/
};

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

int main()
{

    using namespace std;
    auto scheduler = S{};

    // clang-format off
    auto l0 = [](int)   { cout <<               "l0\n"; return 0; };
    auto l1 = [](int x) { cout << "(" << x << ") l1\n"; return 1; };

    auto l2 = [](int)   { cout <<               "l2\n"; return 2; };
    auto l3 = [](int x) { cout << "(" << x << ") l3\n"; return 3; };

    auto l4 = [](int)   { cout <<               "l4\n"; return 4; };
    auto l5 = [](int x) { cout << "(" << x << ") l5\n"; return 5; };

    auto l6 = [](int)   { cout <<               "l6\n"; return 6; };
    auto l7 = [](int x) { cout << "(" << x << ") l7\n"; return 7; };

    auto l8 = [](int)   { cout <<               "l8\n"; return 8; };
    auto l9 = [](int x) { cout << "(" << x << ") l9\n"; return 9; };
    // clang-format on


    auto s0 = node_seq{leaf{in<int>, move(l0)}, leaf{in<int>, move(l1)}};
    auto s1 = node_seq{leaf{in<int>, move(l2)}, leaf{in<int>, move(l3)}};
    auto s2 = node_seq{leaf{in<int>, move(l4)}, leaf{in<int>, move(l5)}};

    auto s3 = node_seq{leaf{in<int>, move(l6)}, leaf{in<int>, move(l7)}};
    auto s4 = node_seq{leaf{in<int>, move(l8)}, leaf{in<int>, move(l9)}};

    auto a0 = node_any{move(s0), move(s1), move(s2)};
    auto a1 = node_and{move(s3), move(s4)};

    auto top0 = node_seq{
        move(a0), leaf{in<boost::variant<int, int, int>>, [](const auto& t) {
                           cout << get<0>(t) << ", " << get<1>(t) << ", "
                                << get<2>(t) << '\n';
                           return -1;
                       }}};

    auto top1 = node_seq{move(a1),
        leaf{in<std::tuple<int, int>>, [](const auto& t) {
                 cout << std::get<0>(t) << ", " << std::get<1>(t) << '\n';
                 return -1;
             }}};

    auto total = node_and{std::move(top0), std::move(top1)};
    total.execute(scheduler, 0, [](auto&&...) {});

    std::this_thread::sleep_for(std::chrono::seconds(2));
}
