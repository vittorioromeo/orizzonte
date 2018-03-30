#include "../include/orizzonte.hpp"
#include <atomic>
#include <boost/variant.hpp>
#include <chrono>
#include <experimental/type_traits>
#include <iostream>
#include <thread>
#include <tuple>
#include <utility>

namespace ou = orizzonte::utility;

// TODO: define executor interface/concept
struct S
{
    template <typename F>
    void operator()(F&& f)
    {
        std::thread{std::move(f)}.detach();
    }
};

template <typename Scheduler, typename Computation,
    typename Input = ou::nothing>
void execute_and_block(
    Scheduler& s, Computation&& c, Input&& input = ou::nothing{})
{
    orizzonte::utility::scoped_bool_latch l{false};
    FWD(c).execute(s, FWD(input), [&l](auto&&...) { l.count_down(); });
}

using namespace orizzonte::node;

void t0()
{
    auto scheduler = S{};

    auto l0 = [] {};
    auto l1 = [] {};
    auto l2 = [] {};

    auto s0 = seq{leaf{in<void>, std::move(l0)}, leaf{in<void>, std::move(l1)}};

    auto s1 = seq{std::move(s0), leaf{in<void>, std::move(l2)}};
    execute_and_block(scheduler, s1);
}

void t1()
{
    auto scheduler = S{};

    auto l0 = [] {};
    auto l1 = [] { return 42; };
    auto l2 = [](int x) { (void)x;assert(x == 42); };

    auto s0 = seq{leaf{in<void>, std::move(l0)}, leaf{in<void>, std::move(l1)}};

    auto s1 = seq{std::move(s0), leaf{in<int>, std::move(l2)}};
    execute_and_block(scheduler, s1);
}

#define ENSURE(...)       \
    if(!(__VA_ARGS__))    \
    {                     \
        std::terminate(); \
    }



void t2()
{
    auto scheduler = S{};

    std::atomic<int> ctr = 0;

    auto l0 = [&] {
        ++ctr;
        return 0;
    };
    auto l1 = [&] {
        ++ctr;
        return 1;
    };
    auto l2 = [&] {
        ++ctr;
        return 2;
    };

    auto total = all{//
        leaf{in<void>, std::move(l0)}, leaf{in<void>, std::move(l1)},
        leaf{in<void>, std::move(l2)}};

    execute_and_block(scheduler, total);

    ENSURE(ctr == 3);
}

void t6()
{
std::cout << "t6 start\n";

    auto scheduler = S{};

    orizzonte::utility::int_latch l{2};

    //auto l0 = [&] {
    //    l.count_down();
    //    return 0;
    //};
    //auto l1 = [&] {
    //    l.count_down();
    //    return 1;
    //};
    auto l2 = [&] {
        l.count_down();
        return 2;
    };
    {
    auto total = any{                  //
   //     leaf{in<void>, std::move(l0)}, //
   //     leaf{in<void>, std::move(l1)}, //
        leaf{in<void>, std::move(l2)}};


    // constexpr auto count = total.visit([](auto&&){
    // constexpr auto count = total.count(...);
    //
    // total.execute(scheduler, ou::nothing_v,
    //     [&](variant<int>) { l.count_down(); },
    //     [&]{ /* finalizer */ });

    total.execute(scheduler, ou::nothing_v,
        [&](variant<int>) { l.count_down(); });

    l.wait();
    }
    std::cout << "waited\n\n";


}

void t3()
{
    orizzonte::utility::int_latch l{4};

    auto scheduler = S{};

    std::atomic<int> ctr = 0;

    auto l0 = [] { return 0; };
    auto l1 = [] { return 1; };
    auto l2 = [] { return 2; };

    auto cp0 = [&](int x) { ctr += x; l.count_down(); };
    auto cp1 = [&](int x) { ctr += x; l.count_down(); };
    auto cp2 = [&](int x) { ctr += x; l.count_down(); };

    auto lf0 = leaf{in<void>, std::move(l0)};
    auto lf1 = leaf{in<void>, std::move(l1)};
    auto lf2 = leaf{in<void>, std::move(l2)};

    auto total = any{//
        seq{std::move(lf0), leaf{in<int>, std::move(cp0)}},
        seq{std::move(lf1), leaf{in<int>, std::move(cp1)}},
        seq{std::move(lf2), leaf{in<int>, std::move(cp2)}}};

    total.execute(scheduler, ou::nothing_v, [&](auto&&...) { l.count_down(); });

    l.wait();
    ENSURE(ctr == 3);
}

void t4()
{
    auto scheduler = S{};

    std::atomic<int> ctr = 0;

    auto l0 = [] { return 0; };
    auto l1 = [] { return 1; };
    auto l2 = [] { return 2; };

    auto cp0 = [&](int x) { ctr += x; };
    auto cp1 = [&](int x) { ctr += x; };
    auto cp2 = [&](int x) { ctr += x; };

    auto lf0 = leaf{in<void>, std::move(l0)};
    auto lf1 = leaf{in<void>, std::move(l1)};
    auto lf2 = leaf{in<void>, std::move(l2)};

    auto top0 = any{//
        seq{std::move(lf0), leaf{in<int>, std::move(cp0)}},
        seq{std::move(lf1), leaf{in<int>, std::move(cp1)}},
        seq{std::move(lf2), leaf{in<int>, std::move(cp2)}}};

    auto total = seq{std::move(top0),
        leaf{in<boost::variant<int, int, int>>, [](const auto&) {}}};

    execute_and_block(scheduler, total);

    // std::cout <<"ctr: " << ctr << '\n';
    // ENSURE(ctr == 3);
}

void t5()
{
    auto scheduler = S{};

    std::atomic<int> ctr = 0;

    auto top0 = any{//
        leaf{in<int>,
            [&](int x) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                ctr += x;
            }},
        leaf{in<int>, [&](int x) { ctr += x; }}};

    auto total = seq{leaf{in<void>, [] { return 0; }}, std::move(top0)};

    execute_and_block(scheduler, total);

    // ENSURE(ctr == 2);
}

void t9()
{
    using namespace std;
    auto scheduler = S{};

    // clang-format off
    auto l0 = []()      { cout <<               "l0\n"; return 0; };
    auto l1 = [](int x) { cout << "(" << x << ") l1\n"; return 1; };

    auto l2 = []()      { cout <<               "l2\n"; return 2; };
    auto l3 = [](int x) { cout << "(" << x << ") l3\n"; return 3; };

    auto l4 = []()      { cout <<               "l4\n"; return 4; };
    auto l5 = [](int x) { cout << "(" << x << ") l5\n"; return 5; };

    auto l6 = []()      { cout <<               "l6\n"; return 6; };
    auto l7 = [](int x) { cout << "(" << x << ") l7\n"; return 7; };

    auto l8 = []()      { cout <<               "l8\n"; return 8; };
    auto l9 = [](int x) { cout << "(" << x << ") l9\n"; return 9; };
    // clang-format on

    using namespace orizzonte::utility;

    auto s0 = seq{leaf{in<nothing>, move(l0)}, leaf{in<int>, move(l1)}};
    auto s1 = seq{leaf{in<nothing>, move(l2)}, leaf{in<int>, move(l3)}};
    auto s2 = seq{leaf{in<nothing>, move(l4)}, leaf{in<int>, move(l5)}};

    auto s3 = seq{leaf{in<nothing>, move(l6)}, leaf{in<int>, move(l7)}};
    auto s4 = seq{leaf{in<nothing>, move(l8)}, leaf{in<int>, move(l9)}};

    auto a0 = any{move(s0), move(s1), move(s2)};
    auto a1 = all{move(s3), move(s4)};

    auto top0 = seq{move(a0), leaf{in<variant<int, int, int>>,
                                  [](const auto&) { return -1; }}};

    auto top1 = seq{move(a1), leaf{in<tuple<int, int>>, [](const auto&) {
                                       // cout << std::get<0>(t) << ", " <<
                                       // std::get<1>(t) << '\n';
                                       return -1;
                                   }}};

    auto total = all{std::move(top0), std::move(top1)};
    execute_and_block(scheduler, total);
    // this_thread::sleep_for(chrono::milliseconds(1));
}

template <typename F>
void do_test(const char* name, F&& f)
{
    std::cout << name << '\n';
    for(volatile int i = 0; i < 50000; ++i) f();
    std::cout << name << " end\n";
}

int main()
{
#define DO_T(n) do_test("t" #n, [] { t##n(); })

    // DO_T(0);
    // DO_T(1);
    // DO_T(2);
    // DO_T(3);
    DO_T(6);
    // DO_T(4);
    // DO_T(5);
    // DO_T(9);
}
