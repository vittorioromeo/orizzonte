// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include "../../test_utils.hpp"
#include <chrono>
#include <orizzonte/utility/bool_latch.hpp>
#include <thread>

using namespace orizzonte::utility;

void t0()
{
    int i = 0;

    bool_latch l;
    std::thread t{[&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        i = 42;
        l.count_down();
    }};

    l.wait();

    EXPECT_EQ(i, 42);
    t.join();
}

void t1()
{
    int i = 0;

    std::thread t;
    {
        scoped_bool_latch l;
        t = std::thread{[&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            i = 42;
            l.count_down();
        }};
    }

    EXPECT_EQ(i, 42);
    t.join();
}

TEST_MAIN()
{
    t0();
    t1();
}
