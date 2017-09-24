// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include "../../test_utils.hpp"
#include <orizzonte/utility/nothing.hpp>

using namespace orizzonte::utility;

SA_SAME_TYPE(void_to_nothing_t<int>, int);
SA_SAME_TYPE(void_to_nothing_t<void>, nothing);
SA_SAME_TYPE(void_to_nothing_t<void*>, void*);

static_assert(is_nothing_v<nothing>);
static_assert(is_nothing_v<nothing&>);
static_assert(is_nothing_v<nothing&&>);
static_assert(is_nothing_v<const nothing>);
static_assert(is_nothing_v<const nothing&>);
static_assert(is_nothing_v<const nothing&&>);
static_assert(!is_nothing_v<nothing*>);
static_assert(!is_nothing_v<const nothing*>);
static_assert(!is_nothing_v<int>);
static_assert(!is_nothing_v<float>);

void t0()
{
    {
        auto l = [] {};
        SA_TYPE((l()), (void));
        SA_TYPE((returning_nothing_instead_of_void(l)), (nothing));
    }

    {
        auto l = [] { return 0; };
        SA_TYPE((l()), (int));
        SA_TYPE((returning_nothing_instead_of_void(l)), (int));
    }

    {
        int a;
        auto l = [&]() -> int& { return a; };
        SA_TYPE((l()), (int&));
        SA_TYPE((returning_nothing_instead_of_void(l)), (int&));
    }

    {
        int a;
        auto l = [&]() -> const int& { return a; };
        SA_TYPE((l()), (const int&));
        SA_TYPE((returning_nothing_instead_of_void(l)), (const int&));
    }

    {
        int a;
        auto l = [&](auto) -> int& { return a; };
        SA_TYPE((l(move_only{})), (int&));
        SA_TYPE((returning_nothing_instead_of_void(l, move_only{})), (int&));
    }
}

void t1()
{
    int a;
    nothing n;
    const nothing& cn = n;
    auto l = [&](int&) -> int& { return a; };

    SA_TYPE((l(a)), (int&));
    SA_TYPE((call_ignoring_nothing(l, a)), (int&));
    SA_TYPE((call_ignoring_nothing(l, nothing_v, a)), (int&));
    SA_TYPE((call_ignoring_nothing(l, a, nothing_v)), (int&));
    SA_TYPE((call_ignoring_nothing(l, nothing_v, a, nothing_v)), (int&));
    SA_TYPE((call_ignoring_nothing(l, n, a)), (int&));
    SA_TYPE((call_ignoring_nothing(l, a, n)), (int&));
    SA_TYPE((call_ignoring_nothing(l, n, a, n)), (int&));
    SA_TYPE((call_ignoring_nothing(l, cn, a)), (int&));
    SA_TYPE((call_ignoring_nothing(l, a, cn)), (int&));
    SA_TYPE((call_ignoring_nothing(l, cn, a, cn)), (int&));
    SA_TYPE((call_ignoring_nothing(l, std::move(n), a)), (int&));
    SA_TYPE((call_ignoring_nothing(l, a, std::move(n))), (int&));
    SA_TYPE((call_ignoring_nothing(l, std::move(n), a, std::move(n))), (int&));
}

void t2()
{
    int a;
    nothing n;
    const nothing& cn = n;
    auto l = [&](int&) -> int& { return a; };

    SA_TYPE((l(a)), (int&));
    SA_TYPE((apply_ignoring_nothing(l, std::forward_as_tuple(a))), (int&));
    SA_TYPE((apply_ignoring_nothing(l, std::forward_as_tuple(nothing_v, a))),
        (int&));
    SA_TYPE((apply_ignoring_nothing(l, std::forward_as_tuple(a, nothing_v))),
        (int&));
    SA_TYPE((apply_ignoring_nothing(
                l, std::forward_as_tuple(nothing_v, a, nothing_v))),
        (int&));
    SA_TYPE((apply_ignoring_nothing(l, std::forward_as_tuple(n, a))), (int&));
    SA_TYPE((apply_ignoring_nothing(l, std::forward_as_tuple(a, n))), (int&));
    SA_TYPE(
        (apply_ignoring_nothing(l, std::forward_as_tuple(n, a, n))), (int&));
    SA_TYPE((apply_ignoring_nothing(l, std::forward_as_tuple(cn, a))), (int&));
    SA_TYPE((apply_ignoring_nothing(l, std::forward_as_tuple(a, cn))), (int&));
    SA_TYPE(
        (apply_ignoring_nothing(l, std::forward_as_tuple(cn, a, cn))), (int&));
    SA_TYPE((apply_ignoring_nothing(l, std::forward_as_tuple(std::move(n), a))),
        (int&));
    SA_TYPE((apply_ignoring_nothing(l, std::forward_as_tuple(a, std::move(n)))),
        (int&));
    SA_TYPE((apply_ignoring_nothing(
                l, std::forward_as_tuple(std::move(n), a, std::move(n)))),
        (int&));
}

void t3()
{
    int a;
    nothing n;
    const nothing& cn = n;
    auto l = [&](int&) -> int& { return a; };
    auto wl = ignore_nothing(std::move(l));

    SA_TYPE((l(a)), (int&));
    SA_TYPE((wl(a)), (int&));
    SA_TYPE((wl(nothing_v, a)), (int&));
    SA_TYPE((wl(a, nothing_v)), (int&));
    SA_TYPE((wl(nothing_v, a, nothing_v)), (int&));
    SA_TYPE((wl(n, a)), (int&));
    SA_TYPE((wl(a, n)), (int&));
    SA_TYPE((wl(n, a, n)), (int&));
    SA_TYPE((wl(cn, a)), (int&));
    SA_TYPE((wl(a, cn)), (int&));
    SA_TYPE((wl(cn, a, cn)), (int&));
    SA_TYPE((wl(std::move(n), a)), (int&));
    SA_TYPE((wl(a, std::move(n))), (int&));
    SA_TYPE((wl(std::move(n), a, std::move(n))), (int&));
}

TEST_MAIN()
{
    t0();
    t1();
    t2();
    t3();
}
