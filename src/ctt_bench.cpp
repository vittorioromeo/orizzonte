#include "../include/orizzonte.hpp"
#include <atomic>
#include <boost/thread/thread_pool.hpp>
#include <boost/variant.hpp>
#include <chrono>
#include <experimental/type_traits>
#include <functional>
#include <iostream>
#include <thread>
#include <tuple>
#include <utility>

namespace ou = orizzonte::utility;

using hr_clock = std::chrono::high_resolution_clock;

template <typename TF>
void bench(const std::string& title, int times, TF&& f)
{
    double acc = 0;

    for(int i(0); i < times; ++i)
    {
        auto start = hr_clock::now();
        {
            f();
        }
        auto end = hr_clock::now();

        auto dur = end - start;
        auto cnt =
            std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

        acc += cnt;
    }

    std::cout << title << " | " << (acc / (double)times) << " ms\n";
}

int main()
{
    auto mkt = [](auto& pool, auto& t, auto xi) {
        pool.submit([xi, &t] {
            for(volatile int i = 0; i < 100000; ++i)
            {
                using std::get;
                get<xi>(t) += 1;
            }
        });
    };

    for(int k = 0; k < 2; ++k)
    {
        bench("normal tuple       ", 10, [&] {
            boost::executors::basic_thread_pool pool;
            std::tuple<int, int, int, int> t;

            mkt(pool, t, std::integral_constant<int, 0>{});
            mkt(pool, t, std::integral_constant<int, 1>{});
            mkt(pool, t, std::integral_constant<int, 2>{});
            mkt(pool, t, std::integral_constant<int, 3>{});
        });

        bench("cache-aligned tuple", 10, [&] {
            boost::executors::basic_thread_pool pool;
            ou::cache_aligned_tuple<int, int, int, int> t;

            mkt(pool, t, std::integral_constant<int, 0>{});
            mkt(pool, t, std::integral_constant<int, 1>{});
            mkt(pool, t, std::integral_constant<int, 2>{});
            mkt(pool, t, std::integral_constant<int, 3>{});
        });
    }
}
