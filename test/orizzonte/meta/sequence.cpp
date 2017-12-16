// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include "../../test_utils.hpp"
#include <orizzonte/meta/sequence.hpp>

using namespace orizzonte::meta;

SA_SAME_TYPE_DP(                 //
    (sequence_t<>),              //
    (std::integer_sequence<int>) //
);

SA_SAME_TYPE_DP(                    //
    (sequence_t<0>),                //
    (std::integer_sequence<int, 0>) //
);

SA_SAME_TYPE_DP(                       //
    (sequence_t<0, 1>),                //
    (std::integer_sequence<int, 0, 1>) //
);

SA_SAME_TYPE_DP(                                //
    (sequence_t<0, 1, 2, 1, 0>),                //
    (std::integer_sequence<int, 0, 1, 2, 1, 0>) //
);

SA_SAME_TYPE_DP(                      //
    (sequence_t<0l>),                 //
    (std::integer_sequence<long, 0l>) //
);

SA_SAME_TYPE_DP(                                     //
    (sequence_t<0ul, 1ul>),                          //
    (std::integer_sequence<unsigned long, 0ul, 1ul>) //
);

SA_TYPE_IS_DP(           //
    (sequence_v<>),      //
    (const sequence_t<>) //
);

SA_TYPE_IS_DP(            //
    (sequence_v<0>),      //
    (const sequence_t<0>) //
);

SA_TYPE_IS_DP(                 //
    (sequence_v<1l, 2l>),      //
    (const sequence_t<1l, 2l>) //
);

TEST_MAIN()
{
}
