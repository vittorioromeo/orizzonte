#include "../../utils.hpp"
#include <orizzonte/utilities/nothing.hpp>

TEST_MAIN()
{
    using namespace orizzonte::impl;

    {
        auto l = [] {};
        auto bl = bind_return_void_to_nothing(l);
        static_assert(is_nothing(decltype(bl()){}));
    }

    {
        auto l = [] { return nothing; };
        auto bl = bind_return_void_to_nothing(l);
        static_assert(is_nothing(decltype(bl()){}));
    }

    {
        auto l = [] { return 0; };
        auto bl = bind_return_void_to_nothing(l);
        SA_TYPE_IS(bl(), int);
    }

    {
        int a = 0;
        auto l = [&a]() -> int& { return a; };
        auto bl = bind_return_void_to_nothing(l);
        SA_TYPE_IS(bl(), int&);
    }
}
