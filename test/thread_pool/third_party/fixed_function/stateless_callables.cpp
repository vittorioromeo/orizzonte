#include "../../../utils.hpp"
#include <orizzonte/thread_pool/third_party/fixed_function.hpp>

int a = 0;

TEST_MAIN()
{
    using namespace orizzonte::thread_pool::third_party;

    {
        a = 0;

        // no move
        fixed_function<void()> f0{[] { a = 1; }};
        f0();
        EXPECT_EQ(a, 1);
    }

    {
        a = 0;

        // move
        fixed_function<void()> f0{[] { a = 1; }};
        auto f1 = std::move(f0);
        f1();
        EXPECT_EQ(a, 1);
    }

    {
        a = 0;

        // from empty, no move
        fixed_function<void()> f0;
        f0 = [] { a = 1; };
        f0();
        EXPECT_EQ(a, 1);
    }

    {
        a = 0;

        // from empty, move
        fixed_function<void()> f0;
        f0 = [] { a = 1; };
        auto f1 = std::move(f0);
        f1();
        EXPECT_EQ(a, 1);
    }
}
