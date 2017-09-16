#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    using namespace test;
    using namespace orizzonte;
    using namespace orizzonte::impl;

    auto expect_v_r = [](auto&& x0, auto&& x1) {
        static_assert(std::is_rvalue_reference<decltype(x0)>{});
        static_assert(std::is_lvalue_reference<decltype(x1)>{});

        static_assert(std::is_same<std::decay_t<decltype(x0)>, int>{});
        static_assert(std::is_same<std::decay_t<decltype(x1)>, int>{});
    };

    auto expect_r_v = [](auto&& x0, auto&& x1) {
        static_assert(std::is_lvalue_reference<decltype(x0)>{});
        static_assert(std::is_rvalue_reference<decltype(x1)>{});

        static_assert(std::is_same<std::decay_t<decltype(x0)>, int>{});
        static_assert(std::is_same<std::decay_t<decltype(x1)>, int>{});
    };

    auto l = [](auto&&... xs) {
        return [fwd_xs = FWD_CAPTURE_PACK(xs)](auto&& f)
        {
            apply_fwd_capture(FWD(f), fwd_xs);
        };
    };

    int x;

    // v, r
    expect_v_r(0, x);
    l(0, x)(expect_v_r);

    // r, v
    expect_r_v(x, 0);
    l(x, 0)(expect_r_v);
}
