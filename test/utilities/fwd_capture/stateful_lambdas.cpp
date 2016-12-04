#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    using namespace test;
    using namespace orizzonte::impl;

    {
        auto test = [](auto&& x) { return [x = FWD(x)]{}; };
        test(nocopy{});
        nocopy j;
        (void)j;
        // test(j); // does not compile as intended
    }

    {
        auto test2 = [](auto&& x) { return [x = FWD_CAPTURE(x)]{}; };
        test2(nocopy{});
        nocopy j;
        test2(j); // compiles (stores a reference wrapper)
    }

    {
        // A copy of l is made

        auto l = [i = 0]() mutable
        {
            ++i;
            return i;
        };
        auto test = [](auto&& x) {
            return [x = FWD(x)]() mutable
            {
                return x();
            };
        };

        auto bl = test(l);
        EXPECT_EQ(bl(), 1);
        EXPECT_EQ(l(), 1);
        EXPECT_EQ(bl(), 2);
        EXPECT_EQ(l(), 2);
        EXPECT_EQ(bl(), 3);
        EXPECT_EQ(l(), 3);
    }

    {
        // Reference is preserved

        auto l = [i = 0]() mutable
        {
            ++i;
            return i;
        };
        auto test = [](auto&& x) {
            return [x = FWD_CAPTURE(x)]() mutable
            {
                return x.get()();
            };
        };

        auto bl = test(l);
        EXPECT_EQ(bl(), 1);
        EXPECT_EQ(l(), 2);
        EXPECT_EQ(bl(), 3);
        EXPECT_EQ(l(), 4);
        EXPECT_EQ(bl(), 5);
        EXPECT_EQ(l(), 6);
    }

    {
        auto copy = [](auto&& x) -> auto
        {
            return x;
        };

        // A copy is made

        auto l = [i = 0]() mutable
        {
            ++i;
            return i;
        };
        auto test = [](auto&& x) {
            return [x = FWD_CAPTURE(x)]() mutable
            {
                return x.get()();
            };
        };

        auto bl = test(copy(l));
        EXPECT_EQ(bl(), 1);
        EXPECT_EQ(l(), 1);
        EXPECT_EQ(bl(), 2);
        EXPECT_EQ(l(), 2);
        EXPECT_EQ(bl(), 3);
        EXPECT_EQ(l(), 3);
    }

    {
        // Constness
        int a = 0;
        const int& aref = a;
        auto test = [aa = FWD_CAPTURE(aref)]() mutable {/*++(aa.get());*/};
        test();
        EXPECT_EQ(a, 0);
    }
}
