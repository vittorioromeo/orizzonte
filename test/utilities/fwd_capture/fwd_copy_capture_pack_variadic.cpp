#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    using namespace test;
    using namespace orizzonte::impl;

    auto fwd_expect_val = [](auto&&... xs) {
        return [fwdx = FWD_COPY_CAPTURE_PACK(xs)]
        {
            for_tuple(
                [](auto&& x) {
                    using x_type = std::decay_t<decltype(x.get())>;
                    SA_DECAY_TYPE_IS(x, fwd_capture_wrapper<x_type>);
                },
                fwdx);
        };
    };

    auto fwd_expect_ref = [](auto&&... xs) {
        return [fwdx = FWD_COPY_CAPTURE_PACK(xs)]
        {
            for_tuple(
                [](auto&& x) {
                    using x_type = std::decay_t<decltype(x.get())>;
                    SA_DECAY_TYPE_IS(x, fwd_capture_wrapper<x_type&>);
                },
                fwdx);
        };
    };

    {
        // literal
        fwd_expect_val(0, 1, 2, 3, 4);

        // copy
        expect_ops([&] {
            // should be copied
            fwd_expect_val(anything{}, anything{});
        })
            .copies(2)
            .moves(2); // TODO: can the moves be avoided?

        // lvalue ref
        expect_ops([&] {
            anything x0, x1;

            fwd_expect_ref(x0, x1);
        })
            .copies(0)
            .moves(0);
    }
}
