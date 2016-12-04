#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    using namespace test;
    using namespace orizzonte::impl;

    auto fwd_expect_val = [](auto&& x) {
        return [fwdx = FWD_COPY_CAPTURE(x)]
        {
            using x_type = std::decay_t<decltype(x)>;
            SA_DECAY_TYPE_IS(fwdx, fwd_copy_capture_wrapper<x_type>);
        };
    };

    auto fwd_expect_ref = [](auto&& x) {
        return [fwdx = FWD_COPY_CAPTURE(x)]
        {
            using x_type = std::decay_t<decltype(x)>;
            SA_DECAY_TYPE_IS(fwdx, fwd_copy_capture_wrapper<x_type&>);
        };
    };

    {
        // literal
        fwd_expect_val(0);

        // explicit move (will copy)
        int a = 0;
        fwd_expect_val(std::move(a));

        // lvalue ref
        int b = 0;
        fwd_expect_ref(b);
    }

    // won't compile as intended
    /*
    expect_ops([&] {
        fwd_expect_val(nocopy{});
    })
        .copies(0)
        .moves(1);
    */

    expect_ops([&] {
        // should be copied
        fwd_expect_val(anything{});
    })
        .copies(1)
        .moves(0);


    expect_ops([&] {
        nocopy x;

        // should be ref
        fwd_expect_ref(x);
    })
        .copies(0)
        .moves(0);

    expect_ops([&] {
        anything x;

        // should be ref
        fwd_expect_ref(x);
    })
        .copies(0)
        .moves(0);
}
