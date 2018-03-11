// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include "../../test_utils.hpp"
#include <orizzonte/utility/cache_aligned_tuple.hpp>

using namespace orizzonte::utility;

static_assert( //
    sizeof(std::tuple<int>) < sizeof(cache_aligned_tuple<int>));

static_assert( //
    sizeof(cache_aligned_tuple<int>) >= cache_line_size);

static_assert( //
    sizeof(cache_aligned_tuple<int, int>) >= cache_line_size * 2);

static_assert( //
    sizeof(cache_aligned_tuple<int, int, int>) >= cache_line_size * 3);

struct big_type
{
    char k[cache_line_size + 1];
};

static_assert( //
    sizeof(cache_aligned_tuple<big_type>) >= cache_line_size * 2);

void get_op()
{
    cache_aligned_tuple<int, float> t0;
    const cache_aligned_tuple<int, float> t1;

    SA_TYPE((orizzonte::utility::get<int>(t0)), (int&));
    SA_TYPE((orizzonte::utility::get<int>(t1)), (const int&));
    SA_TYPE((orizzonte::utility::get<int>(std::move(t0))), (int&&));

    SA_TYPE((orizzonte::utility::get<0>(t0)), (int&));
    SA_TYPE((orizzonte::utility::get<0>(t1)), (const int&));
    SA_TYPE((orizzonte::utility::get<0>(std::move(t0))), (int&&));

    SA_TYPE((orizzonte::utility::get<float>(t0)), (float&));
    SA_TYPE((orizzonte::utility::get<float>(t1)), (const float&));
    SA_TYPE((orizzonte::utility::get<float>(std::move(t0))), (float&&));

    SA_TYPE((orizzonte::utility::get<1>(t0)), (float&));
    SA_TYPE((orizzonte::utility::get<1>(t1)), (const float&));
    SA_TYPE((orizzonte::utility::get<1>(std::move(t0))), (float&&));
}

void copy_ctor()
{
    cache_aligned_tuple<int> t0{42};
    cache_aligned_tuple<int> t1{t0};

    EXPECT_EQ(orizzonte::utility::get<int>(t1), 42);
}

void assignment_op()
{
    cache_aligned_tuple<int> t0{42};
    cache_aligned_tuple<int> t1;

    t1 = t0;

    EXPECT_EQ(orizzonte::utility::get<int>(t1), 42);
}

void swap_op()
{
    cache_aligned_tuple<int> t0{42};
    cache_aligned_tuple<int> t1;

    using std::swap;
    swap(t0, t1);

    EXPECT_EQ(orizzonte::utility::get<int>(t1), 42);
}

TEST_MAIN()
{
    get_op();
    copy_ctor();
    assignment_op();
    swap_op();
}
