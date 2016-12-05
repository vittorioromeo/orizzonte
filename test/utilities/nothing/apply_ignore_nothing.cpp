#include "../../utils.hpp"
#include <orizzonte/utilities/nothing.hpp>

TEST_MAIN()
{
    using namespace orizzonte::impl;

    {
        int a = 1;
        auto f = [](int& x, int y) { return x + y; };

        EXPECT_EQ(apply_ignore_nothing(f, std::forward_as_tuple(a, 10)), 11);
        EXPECT_EQ(apply_ignore_nothing(f,
                      std::forward_as_tuple(nothing, a, nothing, 10, nothing)),
            11);

        EXPECT_EQ(
            apply_ignore_nothing(f, std::make_tuple(std::ref(a), 10)), 11);
        EXPECT_EQ(apply_ignore_nothing(f, std::make_tuple(nothing, std::ref(a),
                                              nothing, 10, nothing)),
            11);
    }

    {
        int a = 0;
        auto f = [&a]() -> int& { return a; };
        EXPECT_EQ(&apply_ignore_nothing(f, std::make_tuple()), &a);
        EXPECT_EQ(&apply_ignore_nothing(f, std::make_tuple(nothing)), &a);
        EXPECT_EQ(&apply_ignore_nothing(f, std::forward_as_tuple()), &a);
        EXPECT_EQ(&apply_ignore_nothing(f, std::forward_as_tuple(nothing)), &a);
    }
}
