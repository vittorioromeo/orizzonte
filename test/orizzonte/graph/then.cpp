// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include "../../test_utils.hpp"
#include <boost/variant.hpp>
#include <orizzonte/node.hpp>
#include <orizzonte/utility.hpp>
#include <string>
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
using orizzonte::get;
using orizzonte::utility::sync_execute;

void t0()
{
    auto graph = leaf{[] { return 21; }} //
                     .then(leaf{[](int x) { return x + 21; }});

    sync_execute(S{}, graph, [](int r) { EXPECT_EQ(r, 42); });
}

void t1()
{
    using namespace std::literals;

    auto graph = leaf{[] { return "a"s; }}                              //
                     .then(leaf{[](std::string x) { return x + "b"; }}) //
                     .then(leaf{[](std::string x) { return x + "c"; }});

    sync_execute(S{}, graph, [](std::string r) { EXPECT_EQ(r, "abc"); });
}

void t2()
{
    auto graph = leaf{[] { return 21; }} //
                     .then([](int x) { return x + 21; });

    sync_execute(S{}, graph, [](int r) { EXPECT_EQ(r, 42); });
}

void t3()
{
    using namespace std::literals;

    auto graph = leaf{[] { return "a"s; }}                        //
                     .then([](std::string x) { return x + "b"; }) //
                     .then([](std::string x) { return x + "c"; });

    sync_execute(S{}, graph, [](std::string r) { EXPECT_EQ(r, "abc"); });
}

void t4()
{
    std::atomic<int> i = 0;
    auto graph = leaf{[&i] { i = 42; }}       //
                     .then([&i] { i += 10; }) //
                     .then([&i] { i += 10; });

    sync_execute(S{}, graph, [&i](auto&&...) { EXPECT_TRUE(i == 62); });
}

void t5()
{
    std::atomic<int> i = 0;
    auto graph =
        leaf{[&i] { i = 42; }} //
            .then(all{leaf{[&i] { i += 10; }}, leaf{[&i] { i += 10; }}});

    sync_execute(S{}, graph, [&i](auto&&...) { EXPECT_TRUE(i == 62); });
}

TEST_MAIN()
{
    t0();
    t1();
    t2();
    t3();
    t4();
    t5();
}
