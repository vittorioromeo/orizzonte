#include "../include/orizzonte.hpp"
#include <atomic>
#include <boost/thread/thread_pool.hpp>
#include <boost/variant.hpp>
#include <chrono>
#include <experimental/type_traits>
#include <iostream>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <utility>

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY
#include <boost/thread/future.hpp>

namespace ou = orizzonte::utility;

boost::executors::basic_thread_pool pool;

struct S
{
    template <typename F>
    void operator()(F&& f)
    {
        std::thread{std::move(f)}.detach();
    }
};

struct P
{
    template <typename F>
    void operator()(F&& f)
    {
        pool.submit(std::move(f));
    }
};

std::map<std::string, std::vector<double>> g_results;

using hr_clock = std::chrono::high_resolution_clock;

template <typename TF>
void bench(const std::string& id, const std::string& title, TF&& f)
{
    constexpr int times = 1000;
    double acc = 0;

    for(int i(0); i < times; ++i)
    {
        const auto start = hr_clock::now();
        {
            f();
        }

        const auto dur = hr_clock::now() - start;
        const auto cnt =
            std::chrono::duration_cast<std::chrono::microseconds>(dur).count();

        acc += cnt;
    }

    const auto x_ms = ((acc / times) / 1000.0);
    g_results[id].emplace_back(x_ms);
    std::cout << title << " | " << x_ms << " ms\n";
}

#define ENSURE(...)       \
    if(!(__VA_ARGS__))    \
    {                     \
        std::terminate(); \
    }

using namespace orizzonte::node;
using orizzonte::utility::sync_execute;

void sleepus(int x)
{
    usleep(x);
}

template <typename R, typename F>
auto make_bf(F&& f) -> boost::future<R>
{
    return boost::async(boost::launch::async, FWD(f));
}

template <typename F>
void with_ns(F&& f)
{
    f(0);
    for(int i = 0; i < 5; ++i)
    {
        f(std::pow(10, i));
    }

    std::cout << '\n';
}

/*
    (A)
*/
void b0_single_node(int d)
{
    bench("sngl_boostfutu", std::to_string(d) + "\tus - sngl - boostfutu", [&] {
        auto f = make_bf<int>([&] {
            sleepus(d);
            return 42;
        });
        ENSURE(f.get() == 42);
    });

    bench("sngl_orizzonte", std::to_string(d) + "\tus - sngl - orizzonte", [&] {
        auto f = leaf{[&] {
            sleepus(d);
            return 42;
        }};
        sync_execute(S{}, f, [](int x) { ENSURE(x == 42); });
    });

    bench("sngl_orizzpool", std::to_string(d) + "\tus - sngl - orizzpool", [&] {
        auto f = leaf{[&] {
            sleepus(d);
            return 42;
        }};
        sync_execute(P{}, f, [](int x) { ENSURE(x == 42); });
    });
}

/*
    (A) -> (B) -> (C)
*/
void b1_then(int d)
{
    bench("then_boostfutu", std::to_string(d) + "\tus - then - boostfutu", [&] {
        auto g0 = make_bf<int>([&] {
            sleepus(d);
            return 42;
        });

        auto g1 =
            g0.then(boost::launch::async, [&](auto x) { return x.get() + 1; });

        auto f =
            g1.then(boost::launch::async, [&](auto x) { return x.get() + 1; });

        ENSURE(f.get() == 44);
    });

    bench("then_orizzonte", std::to_string(d) + "\tus - then - orizzonte", [&] {
        auto f = seq{seq{leaf{[&] {
                             sleepus(d);
                             return 42;
                         }},
                         leaf{[&](int x) { return x + 1; }}},
            leaf{[&](int x) { return x + 1; }}};

        sync_execute(S{}, f, [](int x) { ENSURE(x == 44); });
    });

    bench("then_orizzpool", std::to_string(d) + "\tus - then - orizzpool", [&] {
        auto f = seq{seq{leaf{[&] {
                             sleepus(d);
                             return 42;
                         }},
                         leaf{[&](int x) { return x + 1; }}},
            leaf{[&](int x) { return x + 1; }}};

        sync_execute(P{}, f, [](int x) { ENSURE(x == 44); });
    });
}

/*
    (A) -> (B) -> (C) -> (D) -> (E) -> (F) -> (G) -> (H)
*/
void b1_then_more(int d)
{
    bench("tmor_boostfutu", std::to_string(d) + "\tus - tmor - boostfutu", [&] {
        auto f = make_bf<int>([&] {
            sleepus(d);
            return 0;
        }).then(boost::launch::async, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::async, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::async, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::async, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::async, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::async, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::async, [&](auto x) { return x.get() + 1; });

        ENSURE(f.get() == 7);
    });

    bench("tmor_bfutdefer", std::to_string(d) + "\tus - tmor - bfutdefer", [&] {
        auto f = make_bf<int>([&] {
            sleepus(d);
            return 0;
        }).then(boost::launch::deferred, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::deferred, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::deferred, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::deferred, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::deferred, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::deferred, [&](auto x) { return x.get() + 1; })
          .then(boost::launch::deferred, [&](auto x) { return x.get() + 1; });

        ENSURE(f.get() == 7);
    });

    bench("tmor_orizzonte", std::to_string(d) + "\tus - tmor - orizzonte", [&] {
        auto f = leaf{[&]{ sleepus(d); return 0; }}
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; });

        sync_execute(S{}, f, [](int x) { ENSURE(x == 7); });
    });

    bench("tmor_orizzpool", std::to_string(d) + "\tus - tmor - orizzpool", [&] {
        auto f = leaf{[&]{ sleepus(d); return 0; }}
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; })
            .then([&](int x) { return x + 1; });

        sync_execute(P{}, f, [](int x) { ENSURE(x == 7); });
    });
}

/*
           -> (B0) \
         /          \
       -> (B1) -----> (C)
         \          /
          -> (B2)  /
*/
void b2_whenall(int d)
{
    bench("wall_boostfutu", std::to_string(d) + "\tus - wall - boostfutu", [&] {
        auto b0 = make_bf<int>([&] {
            sleepus(d);
            return 0;
        });
        auto b1 = make_bf<int>([&] {
            sleepus(d);
            return 1;
        });
        auto b2 = make_bf<int>([&] {
            sleepus(d);
            return 2;
        });

        auto b = boost::when_all(std::move(b0), std::move(b1), std::move(b2));
        auto k = b.then([](auto x) {
            auto r = x.get();
            ENSURE(std::get<0>(r).get() == 0);
            ENSURE(std::get<1>(r).get() == 1);
            ENSURE(std::get<2>(r).get() == 2);

            return 42;
        });

        auto f =
            k.then(boost::launch::async, [&](auto x) { return x.get() + 2; });

        ENSURE(f.get() == 44);
    });

    bench("wall_orizzonte", std::to_string(d) + "\tus - wall - orizzonte", [&] {
        auto f = seq{seq{all{leaf{[&] {
                                 sleepus(d);
                                 return 0;
                             }},
                             leaf{[&] {
                                 sleepus(d);
                                 return 1;
                             }},
                             leaf{[&] {
                                 sleepus(d);
                                 return 2;
                             }}},
                         leaf{[&](ou::cache_aligned_tuple<int, int, int> r) {
                             ENSURE(ou::get<0>(r) == 0);
                             ENSURE(ou::get<1>(r) == 1);
                             ENSURE(ou::get<2>(r) == 2);

                             return 42;
                         }}},
            leaf{[&](int x) { return x + 2; }}};

        sync_execute(S{}, f, [](int x) { ENSURE(x == 44); });
    });

    bench("wall_orizzpool", std::to_string(d) + "\tus - wall - orizzpool", [&] {
        auto f = seq{seq{all{leaf{[&] {
                                 sleepus(d);
                                 return 0;
                             }},
                             leaf{[&] {
                                 sleepus(d);
                                 return 1;
                             }},
                             leaf{[&] {
                                 sleepus(d);
                                 return 2;
                             }}},
                         leaf{[&](ou::cache_aligned_tuple<int, int, int> r) {
                             ENSURE(ou::get<0>(r) == 0);
                             ENSURE(ou::get<1>(r) == 1);
                             ENSURE(ou::get<2>(r) == 2);

                             return 42;
                         }}},
            leaf{[&](int x) { return x + 2; }}};

        sync_execute(P{}, f, [](int x) { ENSURE(x == 44); });
    });
}

/*
           -> (B0)
         /
       -> (B1) -----> (C)
         \
          -> (B2)
*/
void b3_whenany(int d)
{
    bench("wany_boostfutu", std::to_string(d) + "\tus - wany - boostfutu", [&] {
        auto b0 = make_bf<int>([&] {
            sleepus(d);
            return 0;
        });
        auto b1 = make_bf<int>([&] {
            sleepus(d);
            return 1;
        });
        auto b2 = make_bf<int>([&] {
            sleepus(d);
            return 2;
        });

        auto b = boost::when_any(std::move(b0), std::move(b1), std::move(b2));
        auto k = b.then([](auto x) {
            auto r = x.get();
            if(std::get<0>(r).is_ready())
            {
                return 42 + std::get<0>(r).get();
            }
            if(std::get<1>(r).is_ready())
            {
                return 42 + std::get<1>(r).get();
            }
            return 42 + std::get<2>(r).get();
        });

        auto f =
            k.then(boost::launch::async, [&](auto x) { return x.get() + 2; });

        ENSURE(f.get() > 41);
    });

    bench("wany_orizzonte", std::to_string(d) + "\tus - wany - orizzonte", [&] {
        auto f = seq{seq{any{leaf{[&] {
                                 sleepus(d);
                                 return 0;
                             }},
                             leaf{[&] {
                                 sleepus(d);
                                 return 1;
                             }},
                             leaf{[&] {
                                 sleepus(d);
                                 return 2;
                             }}},
                         leaf{[&](orizzonte::variant<int, int, int> r) {
                             return 42 + boost::apply_visitor(
                                             [](int x) { return x; }, r);
                         }}},
            leaf{[&](int x) { return x + 2; }}};

        sync_execute(S{}, f, [](int x) { ENSURE(x > 41); });
    });

    bench("wany_orizzpool", std::to_string(d) + "\tus - wany - orizzpool", [&] {
        auto f = seq{seq{any{leaf{[&] {
                                 sleepus(d);
                                 return 0;
                             }},
                             leaf{[&] {
                                 sleepus(d);
                                 return 1;
                             }},
                             leaf{[&] {
                                 sleepus(d);
                                 return 2;
                             }}},
                         leaf{[&](orizzonte::variant<int, int, int> r) {
                             return 42 + boost::apply_visitor(
                                             [](int x) { return x; }, r);
                         }}},
            leaf{[&](int x) { return x + 2; }}};

        sync_execute(P{}, f, [](int x) { ENSURE(x > 41); });
    });
}

/*
           -> (B0) -> all(D0, D1, D2)   |
         /                              |
       -> (B1)                          |-----> (C)
         \                              |
          -> (B2)                       |
*/
void b4_complex(int d)
{
    bench("cplx_boostfutu", std::to_string(d) + "\tus - cplx - boostfutu", [&] {

        auto b0 = make_bf<int>([&] { sleepus(d); return 0; })
            .then(boost::launch::async, [](auto f){
                f.get();
                auto d0 = make_bf<int>([&] { return 0; });
        auto d1 = make_bf<int>([&] { return 0; });
        auto d2 = make_bf<int>([&] { return 0; });

                boost::when_all(std::move(d0), std::move(d1), std::move(d2)).get();
                return 0; });
        auto b1 = make_bf<int>([&] { sleepus(d); return 1; });
        auto b2 = make_bf<int>([&] { sleepus(d); return 2; });

        auto b = boost::when_any(std::move(b0), std::move(b1), std::move(b2));
        auto k = b.then([](auto x) {
            auto r = x.get();
            if(std::get<0>(r).is_ready())
            {
                return 42 + std::get<0>(r).get();
            }
            if(std::get<1>(r).is_ready())
            {
                return 42 + std::get<1>(r).get();
            }
            return 42 + std::get<2>(r).get();
        });

        auto f =
            k.then(boost::launch::async, [&](auto x) { return x.get() + 2; });

        ENSURE(f.get() > 41);
    });

    bench("cplx_orizzonte", std::to_string(d) + "\tus - cplx - orizzonte", [&] {
        auto f = seq{seq{any{leaf{[&] { sleepus(d); return 0; }}
                                .then(all{
                                    leaf{[](int){ return 0; }},
                                    leaf{[](int){ return 0; }},
                                    leaf{[](int){ return 0; }}
                                }),
                             leaf{[&] { sleepus(d); return 1; }},
                             leaf{[&] { sleepus(d); return 2; }}},
                         leaf{[&](orizzonte::variant<orizzonte::tuple<int, int, int>, int, int> r) {
                             return 42 + boost::apply_visitor(
                                             [](auto x)
                                             {
                                                 if constexpr(std::is_same_v<decltype(x), int>){
                                                    return x;
                                                 } else { return 0; }
                                                }, r);
                         }}},
            leaf{[&](int x) { return x + 2; }}};

        sync_execute(S{}, f, [](int x) { ENSURE(x > 41); });
    });

    bench("cplx_orizzpool", std::to_string(d) + "\tus - cplx - orizzpool", [&] {
        auto f = seq{seq{any{leaf{[&] { sleepus(d); return 0; }}
                                .then(all{
                                    leaf{[](int){ return 0; }},
                                    leaf{[](int){ return 0; }},
                                    leaf{[](int){ return 0; }}
                                }),
                             leaf{[&] { sleepus(d); return 1; }},
                             leaf{[&] { sleepus(d); return 2; }}},
                         leaf{[&](orizzonte::variant<orizzonte::tuple<int, int, int>, int, int> r) {
                             return 42 + boost::apply_visitor(
                                             [](auto x)
                                             {
                                                 if constexpr(std::is_same_v<decltype(x), int>){
                                                    return x;
                                                 } else { return 0; }
                                                }, r);
                         }}},
            leaf{[&](int x) { return x + 2; }}};

        sync_execute(P{}, f, [](int x) { ENSURE(x > 41); });
    });
}

int main()
{
    with_ns(b0_single_node);
    with_ns(b1_then);
    with_ns(b1_then_more);
    with_ns(b2_whenall);
    with_ns(b3_whenany);
    with_ns(b4_complex);

    for(const auto& [k, v] : g_results)
    {
        std::cout << k << " = [";

        if(!v.empty())
        {
            std::cout << v.front();
            for(int i = 1; i < (int)v.size(); ++i) { std::cout << ", " << v[i]; }
        }

        std::cout << "]\n";
    }
}
