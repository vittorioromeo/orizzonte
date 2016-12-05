#pragma once

#include <tuple>
#include <experimental/tuple>
#include <vrm/core/utility_macros.hpp>

namespace orizzonte::impl
{
    using std::experimental::apply;

    template <typename TF, typename TTuple>
    decltype(auto) for_tuple(TF&& f, TTuple&& t)
    {
        return orizzonte::impl::apply(
            [&f](auto&&... xs) { (f(FWD(xs)), ...); }, FWD(t));
    }
}