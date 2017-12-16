// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <condition_variable>
#include <mutex>

namespace orizzonte::utility
{
    /// @brief Binary latch that can block the current thread until
    /// `count_down()` is invoked.
    class bool_latch
    {
    private:
        std::condition_variable _cv;
        std::mutex _mtx;
        bool _finished{false};

    public:
        void count_down()
        {
            std::scoped_lock lk{_mtx};
            _finished = true;
            _cv.notify_all();
        }

        void wait()
        {
            std::unique_lock lk{_mtx};
            _cv.wait(lk, [this] { return _finished; });
        }
    };

    /// @brief Wrapper around `bool_latch` that automatically invokes `wait()`
    /// on destruction.
    class scoped_bool_latch : bool_latch
    {
    public:
        scoped_bool_latch() = default;

        // Prevent copies.
        scoped_bool_latch(const scoped_bool_latch&) = delete;
        scoped_bool_latch& operator=(const scoped_bool_latch&) = delete;

        // Prevent moves.
        scoped_bool_latch(scoped_bool_latch&&) = delete;
        scoped_bool_latch& operator=(scoped_bool_latch&&) = delete;

        ~scoped_bool_latch()
        {
            this->wait();
        }

        using bool_latch::count_down;
    };
}
