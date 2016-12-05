#include "../../utils.hpp"
#include <orizzonte/utilities/nothing.hpp>

TEST_MAIN()
{
    using namespace orizzonte::impl;
    SA_DECAY_TYPE_IS(nothing, nothing_t);
    
    static_assert(is_nothing(nothing));
    static_assert(is_nothing(nothing_t{}));

    static_assert(!is_nothing(0));
}
