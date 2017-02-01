#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    using namespace test;
    using namespace orizzonte::impl;

    auto fwd_expect_val = [](auto&& x) {
        return [fwdx = fwd_capture_pack(FWD(x))]
        {
            using x_type = std::decay_t<decltype(x)>;
            SA_DECAY_TYPE_IS(std::get<0>(fwdx), fwd_capture_wrapper<x_type>);
        };
    };

    auto fwd_expect_ref = [](auto&& x) {
        return [fwdx = fwd_capture_pack(FWD(x))]
        {
            using x_type = std::decay_t<decltype(x)>;
            SA_DECAY_TYPE_IS(std::get<0>(fwdx), fwd_capture_wrapper<x_type&>);
        };
    };

    {
        // literal
        fwd_expect_val(0);

        // explicit move
        int a = 0;
        fwd_expect_val(std::move(a));

        // lvalue ref
        int b = 0;
        fwd_expect_ref(b);
    }

    expect_ops([&] {
        // should be moved
        fwd_expect_val(nocopy{});
    })
        .copies(0)
        .moves(2); // TODO: can the second move be avoided?

    expect_ops([&] {
        // should be moved
        fwd_expect_val(anything{});
    })
        .copies(0)
        .moves(2); // TODO: can the second move be avoided?

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
