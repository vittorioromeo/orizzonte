#include "../include/orizzonte.hpp"
#include <atomic>
#include <boost/variant.hpp>
#include <chrono>
#include <experimental/type_traits>
#include <iostream>
#include <thread>
#include <tuple>
#include <functional>
#include <utility>
#include <boost/thread/thread_pool.hpp>

namespace ou = orizzonte::utility;

using hr_clock = std::chrono::high_resolution_clock;

template <typename TF>
void bench(const std::string& title, int times, TF&& f)
{
    std::vector<float> mss;

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

        mss.emplace_back(cnt);
    }

    float mean = 0.f;
    for(auto x : mss)
    {
        mean += x;
    }
    mean /= mss.size();

    std::cout << title << " | " << mean << " ms\n";
}


int main()
{
    bench("normal tuple", 10, [&]
    {
        boost::executors::basic_thread_pool pool;
        std::tuple<int, int, int, int> t;

        for(int i = 0; i < 1000; ++i)
        {
            pool.submit([i, &t] {
                if(i % 4 == 0) { ++std::get<0>(t); } 
                else if(i % 4 == 1) { ++std::get<1>(t); } 
                else if(i % 4 == 2) { ++std::get<2>(t); } 
                else if(i % 4 == 3) { ++std::get<3>(t); } 
            });
        }
    });

    bench("cache-aligned tuple", 10, [&]
    {
        boost::executors::basic_thread_pool pool;
        ou::cache_aligned_tuple<int, int, int, int> t;

        for(int i = 0; i < 1000; ++i)
        {
            pool.submit([i, &t] {
                if(i % 4 == 0) { ++ou::get<0>(t); } 
                else if(i % 4 == 1) { ++ou::get<1>(t); } 
                else if(i % 4 == 2) { ++ou::get<2>(t); } 
                else if(i % 4 == 3) { ++ou::get<3>(t); } 
            });
        }
    });
}