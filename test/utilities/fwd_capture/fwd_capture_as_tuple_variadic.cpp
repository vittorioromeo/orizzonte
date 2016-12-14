#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    using namespace test;
    using namespace orizzonte::impl;

    auto fwd_expect_val = [](auto&&... xs) {
        return [fwdx = FWD_CAPTURE_PACK_AS_TUPLE(xs)]
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
        return [fwdx = FWD_CAPTURE_PACK_AS_TUPLE(xs)]
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

        // explicit move
        int a0 = 0;
        int a1 = 1;
        fwd_expect_val(std::move(a0), std::move(a1));

        // lvalue ref
        int b0 = 0;
        int b1 = 0;
        fwd_expect_ref(b0, b1);
    }

    {
        auto fwd_no_expectations = [](auto&&... xs) {
            return [fwdx = fwd_capture_as_tuple(FWD(xs)...)]{};
        };

        nomove nm0;
        nomove nm1;
        nomove nm2;

        // test that these compile
        fwd_no_expectations(nocopy{}, nocopy{}, nocopy{});
        fwd_no_expectations(nm0, nm1, nm2);
        fwd_no_expectations(nocopy{}, nm0, nm1, nocopy{}, nm2, nocopy{});
    }
}
