// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <boost/variant.hpp>
#include <tuple>

namespace orizzonte
{
    template <typename... Ts>
    using variant = boost::variant<Ts...>;

    namespace detail
    {
        // TODO:
        template <int N, typename... Ts>
        using NthTypeOf =
            typename std::tuple_element<N, std::tuple<Ts...>>::type;
    }

    template <int N, typename... Ts>
    auto& get(variant<Ts...>& v)
    {
        using target = detail::NthTypeOf<N, Ts...>;
        return boost::get<target>(v);
    }

    template <int N, typename... Ts>
    auto& get(const variant<Ts...>& v)
    {
        using target = detail::NthTypeOf<N, Ts...>;
        return boost::get<target>(v);
    }
}
