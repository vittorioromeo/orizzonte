#include "utils.hpp"

struct unprintable
{
    auto operator==(unprintable)
    {
        return true;
    }
};

TEST_MAIN()
{
    int a = 5;
    EXPECT_EQ(5, a);

    EXPECT_EQ(unprintable{}, unprintable{});
}
