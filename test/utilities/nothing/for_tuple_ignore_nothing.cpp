#include "../../utils.hpp"
#include <orizzonte/utilities/nothing.hpp>

TEST_MAIN()
{
    using namespace orizzonte::impl;

    // TODO: gcc bug?
    /*
        {
            int acc = 0;
            for_tuple([&acc](auto&&...) { ++acc; },
                std::make_tuple(0, nothing, 1, nothing, 2));
            EXPECT_EQ(acc, 5);
        }

        {
            int acc = 0;
            for_tuple_ignore_nothing([&acc](auto&&...) { ++acc; },
                std::make_tuple(0, nothing, 1, nothing, 2));
            EXPECT_EQ(acc, 3);
        }
    */
}
