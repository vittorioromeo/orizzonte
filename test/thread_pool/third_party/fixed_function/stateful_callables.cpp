#include "../../../utils.hpp"
#include <orizzonte/thread_pool/third_party/fixed_function.hpp>

TEST_MAIN()
{
    using namespace orizzonte::thread_pool::third_party;

    {
        // capture lvalue, no move
        int a = 0;
        fixed_function<void()> f0{[&a] { a = 1; }};
        f0();
        EXPECT_EQ(a, 1);
    }

    {
        // capture lvalue, move
        int a = 0;
        fixed_function<void()> f0{[&a] { a = 1; }};
        auto f1 = std::move(f0);
        f1();
        EXPECT_EQ(a, 1);
    }

    {
        // mutable, no move
        fixed_function<int()> f0{[a = 0]() mutable { a = 1; return a; }};
        auto res = f0();
        EXPECT_EQ(res, 1);
    }

    {
        // mutable, move
        fixed_function<int()> f0{[a = 0]() mutable { a = 1; return a; }};
        auto f1 = std::move(f0);
        auto res = f1();
        EXPECT_EQ(res, 1);
    }

    {
        // from empty, capture lvalue, no move
        int a = 0;
        fixed_function<void()> f0;
        f0 = [&a] { a = 1; };
        f0();
        EXPECT_EQ(a, 1);
    }

    {
        // from empty, capture lvalue, move
        int a = 0;
        fixed_function<void()> f0;
        f0 = [&a] { a = 1; };
        auto f1 = std::move(f0);
        f1();
        EXPECT_EQ(a, 1);
    }

    {
        // from empty, mutable, no move
        fixed_function<int()> f0;
        f0 = [a = 0]() mutable { a = 1; return a; };
        auto res = f0();
        EXPECT_EQ(res, 1);
    }

    {
        // from empty, mutable, move
        fixed_function<int()> f0;
        f0 = [a = 0]() mutable { a = 1; return a; };
        auto f1 = std::move(f0);
        auto res = f1();
        EXPECT_EQ(res, 1);
    }
}
