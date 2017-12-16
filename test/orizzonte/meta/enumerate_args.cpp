// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include "../../test_utils.hpp"
#include <orizzonte/meta/enumerate_args.hpp>

using namespace orizzonte::meta;

void t0()
{
    enumerate_args([](auto, auto) { FAIL(); });
}

void t1()
{
    int acc = 0;
    enumerate_args([&acc](auto idx, auto) { acc += idx; }, 1, 1, 1, 1);
    EXPECT(acc == 0 + 1 + 2 + 3);
}

void t2()
{
    int acc = 0;
    enumerate_args([&acc](auto, auto v) { acc += v; }, 1, 1, 1, 1);
    EXPECT(acc == 1 + 1 + 1 + 1);
}

TEST_MAIN()
{
    t0();
    t1();
    t2();
}
