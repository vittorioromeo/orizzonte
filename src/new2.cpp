#include "../include/orizzonte.hpp"
#include <atomic>
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

template <typename In, typename F>
struct leaf : F
{
    using in_type = In;
    using out_type = decltype(std::declval<F&>()(std::declval<in_type>()));

    leaf(in_t<In>, F&& f) : F{std::move(f)}
    {
    }

    template <typename Scheduler, typename Input, typename Then>
    void execute(Scheduler& scheduler, Input&& input, Then&& then)
    {
        (void)scheduler;
        FWD(then)((*this)(FWD(input)));
    }

    template <typename Scheduler, typename Input>
    void execute(Scheduler& scheduler, Input&& input)
    {
        (void)scheduler;
        (*this)(FWD(input));
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
    void execute(Scheduler& scheduler, Input&& input, Then&& then)
    {
        static_cast<A&>(*this).execute(
            scheduler, FWD(input), [this, &scheduler, then](auto&& out) {
                static_cast<B&>(*this).execute(scheduler, out, then);
            });
    }

    template <typename Scheduler, typename Input>
    void execute(Scheduler& scheduler, Input&& input)
    {
        static_cast<A&>(*this).execute(
            scheduler, FWD(input), [this, &scheduler](auto&& out) {
                static_cast<B&>(*this).execute(scheduler, out);
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

    orizzonte::utility::movable_atomic<int> _left{sizeof...(Fs)};
    orizzonte::utility::aligned_storage_for<in_type> _input_buf;
    out_type _values;

    node_and(Fs&&... fs) : Fs{std::move(fs)}...
    {
    }

    template <typename Scheduler, typename Input, typename Then>
    void execute(Scheduler& scheduler, Input&& input, Then&& then)
    {
        // don't construct/destroy if lvalue?
        _input_buf.construct(FWD(input));

        auto doit = [&, then](auto i, auto& f) {
            scheduler([&, then] {
                f.execute(scheduler, _input_buf.access(), [&](auto&& out) {
                    std::get<i>(_values) = out;

                    if(_left.fetch_sub(1) == 1)
                    {
                        _input_buf.destroy();
                        then(_values);
                    }
                });
            });
        };

        orizzonte::meta::enumerate_args(
            [&](auto i, auto t) {
                doit(i,
                    static_cast<orizzonte::meta::unwrap<decltype(t)>&>(*this));
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

    auto a0 = node_and{move(s0), move(s1), move(s2)};
    auto a1 = node_and{move(s3), move(s4)};

    auto top0 = node_seq{move(a0), leaf{in<std::tuple<int, int, int>>,
                                       [](const std::tuple<int, int, int>& t) {
                                           cout << std::get<0>(t) << ", "
                                                << std::get<1>(t) << ", "
                                                << std::get<2>(t) << '\n';
                                           return -1;
                                       }}};

    auto top1 = node_seq{move(a1),
        leaf{in<std::tuple<int, int>>, [](const std::tuple<int, int>& t) {
                 cout << std::get<0>(t) << ", " << std::get<1>(t) << '\n';
                 return -1;
             }}};

    auto total = node_and{move(top0), move(top1)};
    total.execute(scheduler, 0, [](auto&&...) {});

    std::this_thread::sleep_for(std::chrono::seconds(2));
}
