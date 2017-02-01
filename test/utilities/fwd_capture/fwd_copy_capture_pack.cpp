#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    using namespace test;
    using namespace orizzonte::impl;

    auto fwd_expect_val = [](auto&& x) {
        return [fwdx = fwd_copy_capture_pack(FWD(x))]
        {
            using x_type = std::decay_t<decltype(x)>;
            SA_DECAY_TYPE_IS(std::get<0>(fwdx), fwd_capture_wrapper<x_type>);
        };
    };

    auto fwd_expect_ref = [](auto&& x) {
        return [fwdx = fwd_copy_capture_pack(FWD(x))]
        {
            using x_type = std::decay_t<decltype(x)>;
            SA_DECAY_TYPE_IS(std::get<0>(fwdx), fwd_capture_wrapper<x_type&>);
        };
    };
    expect_ops([&] {
        // should be copied
        fwd_expect_val(anything{});
    })
        .copies(1)
        .moves(1); // TODO: can the move be avoided? is it the tuple being moved
                   // into the lambda?

    expect_ops([&] {
        anything x;

        // should be ref
        fwd_expect_ref(x);
    })
        .copies(0)
        .moves(0);
}
