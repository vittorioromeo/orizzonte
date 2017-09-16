#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    using namespace test;
    using namespace orizzonte::impl;

    // `get()` should always return a ref (no copies/moves)
    {
        fwd_capture_wrapper<anything> x(anything{});
        expect_ops([&] { // .
            SA_TYPE_IS(x.get(), anything&);
            decltype(auto) y(x.get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }
    {
        const fwd_capture_wrapper<anything> x(anything{});
        expect_ops([&] { // .
            SA_TYPE_IS(x.get(), const anything&);
            decltype(auto) y(x.get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }
    {
        fwd_capture_wrapper<nocopy> x(nocopy{});
        expect_ops([&] { // .
            SA_TYPE_IS(x.get(), nocopy&);
            decltype(auto) y(x.get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }
    {
        const fwd_capture_wrapper<nocopy> x(nocopy{});
        expect_ops([&] { // .
            SA_TYPE_IS(x.get(), const nocopy&);
            decltype(auto) y(x.get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }

    // lvalues return references
    // a const reference_wrapper still returns a non-const reference
    {
        anything lv;
        fwd_capture_wrapper<anything&> x(lv);
        expect_ops([&] { // .
            SA_TYPE_IS(x.get(), anything&);
            decltype(auto) y(x.get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }
    {
        anything lv;
        const fwd_capture_wrapper<anything&> x(lv);
        expect_ops([&] { // .
            SA_TYPE_IS(x.get(), anything&);
            decltype(auto) y(x.get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }
    {
        nocopy lv;
        fwd_capture_wrapper<nocopy&> x(lv);
        expect_ops([&] { // .
            SA_TYPE_IS(x.get(), nocopy&);
            decltype(auto) y(x.get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }
    {
        nocopy lv;
        const fwd_capture_wrapper<nocopy&> x(lv);
        expect_ops([&] { // .
            SA_TYPE_IS(x.get(), nocopy&);
            decltype(auto) y(x.get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }

    // moves occur when the wrapper itself is an rvalue
    {
        fwd_capture_wrapper<anything> x(anything{});
        expect_ops([&] { // .
            SA_TYPE_IS(std::move(x).get(), anything);
            decltype(auto) y(std::move(x).get());
            (void)y;
        })
            .copies(0)
            .moves(1);
    }
    {
        fwd_capture_wrapper<nocopy> x(nocopy{});
        expect_ops([&] { // .
            SA_TYPE_IS(std::move(x).get(), nocopy);
            decltype(auto) y(std::move(x).get());
            (void)y;
        })
            .copies(0)
            .moves(1);
    }

    // not with lvalues though
    {
        anything lv;
        fwd_capture_wrapper<anything&> x(lv);
        expect_ops([&] { // .
            SA_TYPE_IS(std::move(x).get(), anything&);
            decltype(auto) y(std::move(x).get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }
    {
        nocopy lv;
        fwd_capture_wrapper<nocopy&> x(lv);
        expect_ops([&] { // .
            SA_TYPE_IS(std::move(x).get(), nocopy&);
            decltype(auto) y(std::move(x).get());
            (void)y;
        })
            .copies(0)
            .moves(0);
    }
}
