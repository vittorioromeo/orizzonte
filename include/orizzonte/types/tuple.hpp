// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "../utility/cache_aligned_tuple.hpp"

namespace orizzonte
{
    template <typename... Ts>
    using tuple = utility::cache_aligned_tuple<Ts...>;

    using orizzonte::utility::get;
}
