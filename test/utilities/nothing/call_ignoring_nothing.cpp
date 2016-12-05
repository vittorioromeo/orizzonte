#include "../../utils.hpp"
#include <orizzonte/utilities/nothing.hpp>

TEST_MAIN()
{
    using namespace orizzonte::impl;

    {
        bool state{false};
        auto a0 = [&state] { state = true; };

        call_ignoring_nothing(a0);
        EXPECT_TRUE(state);
        state = false;

        call_ignoring_nothing(a0, nothing);
        EXPECT_TRUE(state);
        state = false;

        call_ignoring_nothing(a0, nothing, nothing);
        EXPECT_TRUE(state);
        state = false;
    }

    {
        int state{0};
        auto a1 = [&state](int x) { state = x; };

        call_ignoring_nothing(a1, 1);
        EXPECT_EQ(state, 1);

        call_ignoring_nothing(a1, 2);
        EXPECT_EQ(state, 2);

        call_ignoring_nothing(a1, 3, nothing);
        EXPECT_EQ(state, 3);

        call_ignoring_nothing(a1, 4, nothing, nothing);
        EXPECT_EQ(state, 4);

        call_ignoring_nothing(a1, nothing, 5);
        EXPECT_EQ(state, 5);

        call_ignoring_nothing(a1, nothing, nothing, 6);
        EXPECT_EQ(state, 6);

        call_ignoring_nothing(a1, nothing, 7, nothing);
        EXPECT_EQ(state, 7);

        call_ignoring_nothing(a1, nothing, nothing, 8, nothing, nothing);
        EXPECT_EQ(state, 8);
    }

    {
        auto expects_int = [](int x) { return x; };
        EXPECT_EQ(call_ignoring_nothing(expects_int, 0), 0);
        EXPECT_EQ(call_ignoring_nothing(expects_int, nothing, 1), 1);
        EXPECT_EQ(call_ignoring_nothing(expects_int, 2, nothing), 2);
        EXPECT_EQ(call_ignoring_nothing(expects_int, nothing, 3, nothing), 3);
        EXPECT_EQ(
            call_ignoring_nothing(expects_int, nothing, nothing, 4, nothing),
            4);
    }

    {
        using namespace test;

        expect_ops([] {
            auto l = [](anything&&) {};
            call_ignoring_nothing(l, anything{});
        })
            .copies(0)
            .moves(1);

        expect_ops([] {
            auto l = [](anything&&) {};
            call_ignoring_nothing(l, anything{}, nothing);
        })
            .copies(0)
            .moves(1);

        expect_ops([] {
            auto l = [](anything&&) {};
            call_ignoring_nothing(l, nothing, anything{});
        })
            .copies(0)
            .moves(1);

        expect_ops([] {
            auto l = [](nocopy&&) {};
            call_ignoring_nothing(l, nocopy{});
        })
            .copies(0)
            .moves(1);

        expect_ops([] {
            auto l = [](nocopy&&) {};
            call_ignoring_nothing(l, nocopy{}, nothing);
        })
            .copies(0)
            .moves(1);

        expect_ops([] {
            auto l = [](nocopy&&) {};
            call_ignoring_nothing(l, nothing, nocopy{});
        })
            .copies(0)
            .moves(1);

        expect_ops([] {
            auto l = [](nomove) {};
            call_ignoring_nothing(l, static_cast<const nomove&>(nomove{}));
        })
            .copies(1)
            .moves(0);

        expect_ops([] {
            auto l = [](nomove) {};
            call_ignoring_nothing(
                l, static_cast<const nomove&>(nomove{}), nothing);
        })
            .copies(1)
            .moves(0);

        expect_ops([] {
            auto l = [](nomove) {};
            call_ignoring_nothing(
                l, nothing, static_cast<const nomove&>(nomove{}));
        })
            .copies(1)
            .moves(0);
    }
}
