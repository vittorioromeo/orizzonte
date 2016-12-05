#include "../../utils.hpp"
#include <orizzonte/utilities/nothing.hpp>

TEST_MAIN()
{
    using namespace orizzonte::impl;

    SA_SAME_TYPE(void_to_nothing_t<int>, int);
    SA_SAME_TYPE(void_to_nothing_t<int&>, int&);
    SA_SAME_TYPE(void_to_nothing_t<int&&>, int&&);

    SA_SAME_TYPE(void_to_nothing_t<void>, nothing_t);
    SA_SAME_TYPE(void_to_nothing_t<void*>, void*);
}
