#include "../../utils.hpp"
#include <orizzonte/utilities/nothing.hpp>

TEST_MAIN()
{
    using namespace orizzonte::impl;
   
    auto expects_int = [](int x){ return x; };
    EXPECT_TRUE( call_ignoring_nothing(expects_int, 0) == 0 );
    EXPECT_TRUE( call_ignoring_nothing(expects_int, nothing, 1) == 1 );
    EXPECT_TRUE( call_ignoring_nothing(expects_int, 2, nothing) == 2 );
    EXPECT_TRUE( call_ignoring_nothing(expects_int, nothing, 3, nothing) == 3 );
    EXPECT_TRUE( call_ignoring_nothing(expects_int, nothing, nothing, 4, nothing) == 4 );

    auto returns_void_nullary = []{};
    auto returns_void = [](auto&&...){};
    // EXPECT_TRUE( is_nothing(with_void_to_nothing(returns_void)) ); // TODO: gcc bug?
    EXPECT_TRUE( is_nothing(with_void_to_nothing(returns_void_nullary)) );
    EXPECT_TRUE( is_nothing(with_void_to_nothing(returns_void, 0)) );
    EXPECT_TRUE( is_nothing(with_void_to_nothing(returns_void, 0, 1)) );
    EXPECT_TRUE( is_nothing(with_void_to_nothing(returns_void, 0, 1, 2)) );
    // EXPECT_TRUE( is_nothing(with_void_to_nothing(returns_void, nothing)) ); // TODO: gcc bug?

    // should be perfectly-captured, so it's a reference to `returns_void`.
    auto wvtn_returs_void_nullary = bind_return_void_to_nothing(returns_void_nullary);
    auto wvtn_returs_void = bind_return_void_to_nothing(returns_void);
    // EXPECT_TRUE( is_nothing(wvtn_returs_void()) ); // TODO: gcc bug?
    EXPECT_TRUE( is_nothing(wvtn_returs_void_nullary()) );
    EXPECT_TRUE( is_nothing(wvtn_returs_void(0)) );
    EXPECT_TRUE( is_nothing(wvtn_returs_void(0, 1)) );
    EXPECT_TRUE( is_nothing(wvtn_returs_void(0, 1, 2)) );
    // EXPECT_TRUE( is_nothing(wvtn_returs_void(nothing)) ); // TODO: gcc bug?


    auto argcnt = [](auto&&... xs){ return sizeof...(xs); };
    EXPECT_TRUE( apply_ignore_nothing(argcnt, std::make_tuple()) == 0 );
    EXPECT_TRUE( apply_ignore_nothing(argcnt, std::make_tuple(0)) == 1 );
    EXPECT_TRUE( apply_ignore_nothing(argcnt, std::make_tuple(0, 1)) == 2 );
    EXPECT_TRUE( apply_ignore_nothing(argcnt, std::make_tuple(nothing, 0)) == 1 );
    EXPECT_TRUE( apply_ignore_nothing(argcnt, std::make_tuple(0, nothing)) == 1 );
    EXPECT_TRUE( apply_ignore_nothing(argcnt, std::make_tuple(nothing, nothing)) == 0 );
    EXPECT_TRUE( apply_ignore_nothing(argcnt, std::make_tuple(nothing, 0, nothing)) == 1 );
}
