// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include "../../test_utils.hpp"
#include <orizzonte/meta/constant.hpp>

using namespace orizzonte::meta;

template <auto X>
struct sanity_check
{
    using type = decltype(X);
};

SA_SAME_TYPE(decltype(1u), unsigned int);

SA_SAME_TYPE(typename sanity_check<1>::type, int);
SA_SAME_TYPE(typename sanity_check<1l>::type, long);
SA_SAME_TYPE(typename sanity_check<1u>::type, unsigned int);
SA_SAME_TYPE(typename sanity_check<1ul>::type, unsigned long);

SA_SAME_TYPE_DP(                     //
    (constant_t<0>),                 //
    (std::integral_constant<int, 0>) //
);

SA_SAME_TYPE_DP(                     //
    (constant_t<1>),                 //
    (std::integral_constant<int, 1>) //
);

SA_SAME_TYPE_DP(                               //
    (constant_t<1u>),                          //
    (std::integral_constant<unsigned int, 1u>) //
);

SA_SAME_TYPE_DP(                                 //
    (constant_t<1ul>),                           //
    (std::integral_constant<unsigned long, 1ul>) //
);

SA_SAME_TYPE_DP(                       //
    (constant_t<1l>),                  //
    (std::integral_constant<long, 1l>) //
);

SA_TYPE_IS(constant_v<0>, const constant_t<0>);
SA_TYPE_IS(c<0>, const constant_t<0>);

TEST_MAIN()
{
}
