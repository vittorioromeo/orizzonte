#include "../../utils.hpp"
#include <orizzonte/utilities/nothing.hpp>

TEST_MAIN()
{
    using namespace orizzonte::impl;

    {
        auto r = with_void_to_nothing([] {});
        EXPECT_TRUE(is_nothing(r));
        static_assert(is_nothing(decltype(r){}));
    }

    {
        auto r = with_void_to_nothing([]() -> void {});
        EXPECT_TRUE(is_nothing(r));
        static_assert(is_nothing(decltype(r){}));
    }

    {
        auto r = with_void_to_nothing([]() { return 0; });
        EXPECT_FALSE(is_nothing(r));
        static_assert(!is_nothing(decltype(r){}));
    }

    {
        auto r = with_void_to_nothing([](int) {}, 0);
        EXPECT_TRUE(is_nothing(r));
        static_assert(is_nothing(decltype(r){}));
    }

    {
        auto r = with_void_to_nothing([](int) -> void {}, 0);
        EXPECT_TRUE(is_nothing(r));
        static_assert(is_nothing(decltype(r){}));
    }

    {
        auto r = with_void_to_nothing([](int) { return 0; }, 0);
        EXPECT_FALSE(is_nothing(r));
        static_assert(!is_nothing(decltype(r){}));
    }

    {
        auto r = with_void_to_nothing([](int) {}, nothing, 0, nothing);
        EXPECT_TRUE(is_nothing(r));
        static_assert(is_nothing(decltype(r){}));
    }

    {
        auto r = with_void_to_nothing([](int) -> void {}, nothing, 0, nothing);
        EXPECT_TRUE(is_nothing(r));
        static_assert(is_nothing(decltype(r){}));
    }

    {
        auto r =
            with_void_to_nothing([](int) { return 0; }, nothing, 0, nothing);
        EXPECT_FALSE(is_nothing(r));
        static_assert(!is_nothing(decltype(r){}));
    }

    {
        int i = 0;
        decltype(auto) r = with_void_to_nothing(
            [&i](int) -> int& { return i; }, nothing, 0, nothing);
        EXPECT_FALSE(is_nothing(r));
        static_assert(!is_nothing(std::decay_t<decltype(r)>{}));
        SA_TYPE_IS(r, int&);
    }
}
