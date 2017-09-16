#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    using namespace test;
    using namespace orizzonte::impl;

    {
        fwd_capture_wrapper<anything> x(anything{});
        expect_ops([&] { // .
            SA_TYPE_IS(x.fwd_get(), anything);
            decltype(auto) y(x.fwd_get());
            (void)y;
        })
            .copies(0)
            .moves(1);
    }
    {
        anything lv;
        fwd_capture_wrapper<anything&> x(lv);
        expect_ops([&] { // .
            SA_TYPE_IS(x.fwd_get(), anything&);
            decltype(auto) y(x.fwd_get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }
    {
        fwd_capture_wrapper<nocopy> x(nocopy{});
        expect_ops([&] { // .
            SA_TYPE_IS(x.fwd_get(), nocopy);
            decltype(auto) y(x.fwd_get());
            (void)y;
        })
            .copies(0)
            .moves(1);
    }
    {
        nocopy lv;
        fwd_capture_wrapper<nocopy&> x(lv);
        expect_ops([&] { // .
            SA_TYPE_IS(x.fwd_get(), nocopy&);
            decltype(auto) y(x.fwd_get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }
}
