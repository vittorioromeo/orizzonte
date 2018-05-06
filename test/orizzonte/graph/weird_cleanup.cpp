// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include "../../test_utils.hpp"
#include <boost/variant.hpp>
#include <orizzonte/node.hpp>
#include <orizzonte/utility.hpp>
#include <string>
#include <thread>

struct S
{
    template <typename F>
    void operator()(F&& f)
    {
        std::thread{std::move(f)}.detach();
    }
};

using namespace orizzonte::node;
using orizzonte::get;
using orizzonte::utility::sync_execute;

TEST_MAIN()
{
    namespace ou = orizzonte::utility;

    auto graph =
            any
            {
                seq
                {
                    any
                    {
                        seq{leaf{[]{ std::cout << "B"; }}, leaf{[]{ std::cout << "E"; }}},
                        leaf{[]{ std::cout << "A"; }}
                    },
                    leaf{[](orizzonte::variant<ou::nothing, ou::nothing>){ std::cout << "D"; }}
                },
                leaf{[]{ std::cout << "C"; }}
            };

        sync_execute(S{}, graph, [](auto&&) {});
}
