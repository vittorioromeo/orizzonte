// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <condition_variable>
#include <mutex>

namespace orizzonte::utility
{
    namespace detail
    {
        template <typename T>
        void decrement(T& x)
        {
            --x;
        }

        void decrement(bool& x)
        {
            x = true;
        }

        template <typename T>
        bool done(const T& x)
        {
            return x == 0;
        }

        bool done(const bool& x)
        {
            return x == true;
        }


        template <typename T>
        class latch_impl
        {
        private:
            std::condition_variable _cv;
            std::mutex _mtx;
            T _ctr{};

        public:
            latch_impl(T ctr = T{}) : _ctr{ctr}
            {
            }

            void count_down()
            {
                std::scoped_lock lk{_mtx};
                decrement(_ctr);
                _cv.notify_all();
            }

            void wait()
            {
                std::unique_lock lk{_mtx};
                _cv.wait(lk, [this] { return done(_ctr); });
            }
        };

        template <typename T>
        class scoped_latch_impl : latch_impl<T>
        {
        public:
            using latch_impl<T>::latch_impl;
            using latch_impl<T>::count_down;

            // Prevent copies.
            scoped_latch_impl(const scoped_latch_impl&) = delete;
            scoped_latch_impl& operator=(const scoped_latch_impl&) = delete;

            // Prevent moves.
            scoped_latch_impl(scoped_latch_impl&&) = delete;
            scoped_latch_impl& operator=(scoped_latch_impl&&) = delete;

            ~scoped_latch_impl()
            {
                this->wait();
            }
        };
    }

    /// @brief Binary latch that can block the current thread until
    /// `count_down()` is invoked.
    using bool_latch = detail::latch_impl<bool>;

    /// @brief Wrapper around `bool_latch` that automatically invokes `wait()`
    /// on destruction.
    using scoped_bool_latch = detail::scoped_latch_impl<bool>;

    /// @brief TODO
    using int_latch = detail::latch_impl<int>;

    /// @brief TODO
    using scoped_int_latch = detail::scoped_latch_impl<int>;
}
