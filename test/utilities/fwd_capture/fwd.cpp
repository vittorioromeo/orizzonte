#include "../../utils.hpp"
#include <orizzonte/utilities/fwd_capture.hpp>

TEST_MAIN()
{
    SA_TYPE_IS(FWD(std::declval<int>()), int&&);
    SA_TYPE_IS(FWD(std::declval<int&>()), int&);
    SA_TYPE_IS(FWD(std::declval<int&&>()), int&&);
}
