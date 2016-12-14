#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    using namespace orizzonte::impl;

    SA_TYPE_IS(fwd_capture(1), fwd_capture_wrapper<int>);
    SA_TYPE_IS(fwd_capture(std::declval<int>()), fwd_capture_wrapper<int>);
    SA_TYPE_IS(fwd_capture(std::declval<int&>()), fwd_capture_wrapper<int&>);
    SA_TYPE_IS(fwd_capture(std::declval<int&&>()), fwd_capture_wrapper<int>);

    {
        int x = 0;
        auto p = fwd_capture(x);
        SA_TYPE_IS(p, fwd_capture_wrapper<int&>);
    }

    {
        int x = 0;
        auto p = fwd_capture(std::move(x));
        SA_TYPE_IS(p, fwd_capture_wrapper<int>);
    }

    {
        auto p = fwd_capture(1);
        SA_TYPE_IS(p, fwd_capture_wrapper<int>);
    }
}
