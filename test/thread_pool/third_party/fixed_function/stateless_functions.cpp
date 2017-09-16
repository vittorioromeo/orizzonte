#include "../../../utils.hpp"
#include <orizzonte/thread_pool/third_party/fixed_function.hpp>

int a = 0;

void fn0(int x)
{
    a = x;
}

TEST_MAIN()
{
    using namespace orizzonte::thread_pool::third_party;

    {
        a = 0;

        // no move
        fixed_function<void(int)> f0{&fn0};

        f0(1);
        EXPECT_EQ(a, 1);

        f0(2);
        EXPECT_EQ(a, 2);
    }

    {
        a = 0;

        // move
        fixed_function<void(int)> f0{&fn0};
        auto f1 = std::move(f0);

        f1(1);
        EXPECT_EQ(a, 1);

        f1(2);
        EXPECT_EQ(a, 2);
    }

    {
        a = 0;

        // from empty, no move
        fixed_function<void(int)> f0;
        f0 = &fn0;

        f0(1);
        EXPECT_EQ(a, 1);

        f0(2);
        EXPECT_EQ(a, 2);
    }

    {
        a = 0;

        // from empty, move
        fixed_function<void(int)> f0;
        f0 = &fn0;
        auto f1 = std::move(f0);

        f1(1);
        EXPECT_EQ(a, 1);

        f1(2);
        EXPECT_EQ(a, 2);
    }
}
