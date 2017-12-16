// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <atomic>

namespace orizzonte::utility
{
    /// @brief Wrapper around `std::atomic` that allows move operations.
    /// @details Moves are implemented as `store(load())`.
    template <typename T>
    struct movable_atomic : std::atomic<T>
    {
        using std::atomic<T>::atomic;

        movable_atomic(movable_atomic&& rhs) : std::atomic<T>{rhs.load()}
        {
        }

        movable_atomic& operator=(movable_atomic&& rhs)
        {
            this->store(rhs.load());
            return *this;
        }
    };
}
