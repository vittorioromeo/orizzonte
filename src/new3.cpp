#include "../include/orizzonte.hpp"
#include <atomic>
#include <boost/thread/thread_pool.hpp>
#include <boost/variant.hpp>
#include <chrono>
#include <experimental/type_traits>
#include <iostream>
#include <thread>
#include <tuple>
#include <utility>

namespace ou = orizzonte::utility;

boost::executors::basic_thread_pool pool;

// TODO: define executor interface/concept
struct S
{
    template <typename F>
    void operator()(F&& f)
    {
        pool.submit(std::move(f));
        // std::thread{std::move(f)}.detach();
    }
};

#define ENSURE(...)       \
    if(!(__VA_ARGS__))    \
    {                     \
        std::terminate(); \
    }

using namespace orizzonte::node;
using orizzonte::utility::sync_execute;

using orizzonte::tuple;
using orizzonte::variant;
using orizzonte::get;

void t0()
{
    auto graph = leaf{[] { return 42; }};
    sync_execute(S{}, graph, [](int r) { ENSURE(r == 42); });
}

void t1()
{
    auto graph = seq{
        leaf{[] { return 21; }},           //
        leaf{[](int x) { return x + 21; }} //
    };
    sync_execute(S{}, graph, [](int r) { ENSURE(r == 42); });
}

void t2()
{
    auto graph = all{leaf{[] { return 42; }}};
    sync_execute(S{}, graph, [](auto r) { ENSURE(get<0>(r) == 42); });
}

void t3()
{
    auto graph = all{
        leaf{[] { return 0; }}, //
        leaf{[] { return 1; }}  //
    };
    sync_execute(S{}, graph, [](auto r) {
        ENSURE(get<0>(r) == 0);
        ENSURE(get<1>(r) == 1);
    });
}

void t4()
{
    auto graph = any{leaf{[] { return 42; }}};
    sync_execute(S{}, graph, [](auto r) { ENSURE(get<int>(r) == 42); });
}

void t5()
{
    auto graph = any{
        leaf{[] { return 0; }}, //
        leaf{[] { return 1; }}  //
    };
    sync_execute(S{}, graph, [](auto r) {
        ENSURE(apply_visitor([](int y) { return y == 0 || y == 1; }, r));
    });
}

void t6()
{
    auto graph = all{//
        any{
            leaf{[] { return 0; }}, //
            leaf{[] { return 1; }}, //
            leaf{[] { return 2; }}  //
        },
        any{
            leaf{[] { return 3; }}, //
            leaf{[] { return 4; }}, //
            leaf{[] { return 5; }}  //
        }};

    sync_execute(S{}, graph, [](auto r) {
        ENSURE(apply_visitor(
            [](int y) { return y == 0 || y == 1 || y == 2; }, get<0>(r)));
        ENSURE(apply_visitor(
            [](int y) { return y == 3 || y == 4 || y == 5; }, get<1>(r)));
    });
}

void t7()
{
    auto graph = any{//
        all{
            leaf{[] { return 0; }}, //
            leaf{[] { return 1; }}, //
            leaf{[] { return 2; }}  //
        },
        all{
            leaf{[] { return 3; }}, //
            leaf{[] { return 4; }}, //
            leaf{[] { return 5; }}  //
        }};

    sync_execute(S{}, graph, [](auto r) {
        ENSURE(apply_visitor(
            [](auto y) {
                const auto a = get<0>(y);
                const auto b = get<1>(y);
                const auto c = get<2>(y);
                return (a == 0 && b == 1 && c == 2) ||
                       (a == 3 && b == 4 && c == 5);
            },
            r));
    });
}

void t8()
{
    auto graph = any{//
        all{
            any{leaf{[] { return 0; }},  //
                leaf{[] { return 1; }}}, //
            leaf{[] { return 2; }}       //
        },
        all{
            any{leaf{[] { return 0; }},  //
                leaf{[] { return 1; }}}, //
            leaf{[] { return 2; }}       //
        }};

    sync_execute(S{}, graph, [](auto r) {
        ENSURE(apply_visitor(
            [](auto y) {
                const auto v = get<0>(y);
                const auto x = get<1>(y);

                return apply_visitor(
                           [](auto z) { return z == 0 || z == 1; }, v) &&
                       x == 2;
            },
            r));
    });
}

void t9()
{
    auto graph = any{//
        seq{
            any{leaf{[] { return 0; }},                      //
                leaf{[] { return 1; }}},                     //
            leaf{[](variant<int, int>) { return 2; }} //
        },
        seq{
            any{leaf{[] { return 0; }},                      //
                leaf{[] { return 1; }}},                     //
            leaf{[](variant<int, int>) { return 2; }} //
        }};

    sync_execute(S{}, graph, [](auto r) {
        ENSURE(apply_visitor([](auto y) { return y == 2; }, r));
    });
}



void t61()
{
    // std::cout << "t6 start\n";

    auto scheduler = S{};

    auto l0 = [&] { return 0; };
    auto l1 = [&] { return 1; };
    auto l2 = [&] { return 2; };

    {
        auto total = any{        //
            leaf{std::move(l0)}, //
            leaf{std::move(l1)}, //
            leaf{std::move(l2)}};

        constexpr auto count = total.cleanup_count();
        assert(count == 1);
        // std::cout << count << '\n';
        orizzonte::utility::int_latch l{count + 1};

        total.execute(scheduler, ou::nothing_v,           //
            [&](variant<int>) { l.count_down(); }, //
            [&] { /*std::cout << "cd\n";*/
                l.count_down();
            });

        l.wait();
    }

    // std::cout << "waited\n\n";
}



template <typename F>
void do_test(const char* name, F&& f)
{
    std::cout << name << '\n';
    for(volatile int i = 0; i < 5000; ++i) f();
    std::cout << name << " end\n";
}

#include <iostream>
#include <string>

struct MO
{
    MO() = default;

    MO(const MO&) = delete;
    MO& operator=(const MO&) = delete;

    MO(MO&&) = default;
    MO& operator=(MO&&) = default;
};


int main()
{
    {
        auto graph = leaf{[] { return MO{}; }};
        sync_execute(S{}, graph, [](MO) {});
    }

    {
        auto graph = seq{leaf{[] { return MO{}; }}, leaf{[](MO) {}}};
        sync_execute(S{}, graph, []() {});
    }

    {
        auto graph = all{leaf{[] { return MO{}; }}, leaf{[] { return MO{}; }}};
        sync_execute(S{}, graph, [](tuple<MO, MO>) {});
    }

    {
        auto graph = any{leaf{[] { return MO{}; }}, leaf{[] { return MO{}; }}};
        sync_execute(S{}, graph, [](variant<MO, MO>) {});
    }


    /*
    auto graph = seq{
        any{leaf{[] { return "hello"; }}, leaf{[] { return "goodbye"; }}},
        leaf{[](variant<const char*, const char*> x) {
            return apply_visitor([](std::string y) { return y + " world"; }, x);
        }}};

    sync_execute(S{}, graph, [](auto r) { std::cout << r << '\n'; });
    */
    /*
        auto graph = any{//
            seq{
                any{leaf{[] { return download("data.com/0"); }}, //
                    leaf{[] { return download("data.com/0"); }}}, //
                leaf{[](variant<int, int>) { return 2; }} //
            },
            seq{
                any{leaf{[] { return 0; }},                      //
                    leaf{[] { return 1; }}},                     //
                leaf{[](variant<int, int>) { return 2; }} //
            }};

        sync_execute(S{}, graph, [](auto r) {
            ENSURE(apply_visitor([](auto y) { return y == 2; }, r));
        });*/


#define DO_T(n) do_test("t" #n, [] { t##n(); })

    DO_T(0);
    DO_T(1);
    DO_T(2);
    DO_T(3);
    DO_T(4);
    DO_T(5);
    DO_T(6);
    DO_T(7);
    DO_T(8);
    DO_T(9);

    DO_T(61);
    // DO_T(4);
    // DO_T(5);
    // DO_T(9);
}
