// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include "../../test_utils.hpp"
#include <boost/variant.hpp>
#include <orizzonte/node.hpp>
#include <orizzonte/utility.hpp>
#include <thread>

struct S
{
    template <typename F>
    void operator()(F&& f)
    {
        std::thread{std::move(f)}.detach();
    }
};

using namespace orizzonte::node;
using orizzonte::utility::sync_execute;


void t0()
{
    auto graph = leaf{[] { return 42; }};
    sync_execute(S{}, graph, [](int r) { EXPECT_EQ(r, 42); });
}

void t1()
{
    auto graph = seq{
        leaf{[] { return 21; }},           //
        leaf{[](int x) { return x + 21; }} //
    };
    sync_execute(S{}, graph, [](int r) { EXPECT_EQ(r, 42); });
}

void t2()
{
    auto graph = all{leaf{[] { return 42; }}};
    sync_execute(S{}, graph, [](auto r) { EXPECT_EQ(get<0>(r), 42); });
}

void t3()
{
    auto graph = all{
        leaf{[] { return 0; }}, //
        leaf{[] { return 1; }}  //
    };
    sync_execute(S{}, graph, [](auto r) {
        EXPECT_EQ(get<0>(r), 0);
        EXPECT_EQ(get<1>(r), 1);
    });
}

void t4()
{
    auto graph = any{leaf{[] { return 42; }}};
    sync_execute(S{}, graph, [](auto r) { EXPECT_EQ(get<int>(r), 42); });
}

void t5()
{
    auto graph = any{
        leaf{[] { return 0; }}, //
        leaf{[] { return 1; }}  //
    };
    sync_execute(S{}, graph, [](auto r) {
        EXPECT(apply_visitor([](int y) { return y == 0 || y == 1; }, r));
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
        EXPECT(apply_visitor(
            [](int y) { return y == 0 || y == 1 || y == 2; }, get<0>(r)));
        EXPECT(apply_visitor(
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
        EXPECT(apply_visitor(
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
        EXPECT(apply_visitor(
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
            any{leaf{[] { return 0; }},                          //
                leaf{[] { return 1; }}},                         //
            leaf{[](orizzonte::variant<int, int>) { return 2; }} //
        },
        seq{
            any{leaf{[] { return 0; }},                          //
                leaf{[] { return 1; }}},                         //
            leaf{[](orizzonte::variant<int, int>) { return 2; }} //
        }};

    sync_execute(S{}, graph, [](auto r) {
        EXPECT(apply_visitor([](auto y) { return y == 2; }, r));
    });
}

TEST_MAIN()
{
    t0();
    t1();
    t2();
    t3();
    t4();
    t5();
    t6();
    t7();
    t8();
    t9();
}
