#include "./temp.hpp"

struct world_s_best_thread_pool
{
    template <typename F>
    void operator()(F&& f)
    {
        std::thread{FWD(f)}.detach();
    }
};

std::atomic<int> ctr = 0;
struct sctr
{
    sctr()
    {
        ++ctr;
    }
    ~sctr()
    {
        --ctr;
    }
};

void fuzzy()
{
    std::mutex mtx;
    std::random_device rd;
    std::default_random_engine re(rd());

    const auto rndint = [&](int min, int max) {
        std::scoped_lock l{mtx};
        return std::uniform_int_distribution<int>{min, max - 1}(re);
    };

    const auto rndsleep = [&] {
        std::this_thread::sleep_for(std::chrono::nanoseconds{rndint(0, 100)});
    };

    for(int i = 0; i < 1000; ++i)
    {
        auto f = initiate(
            [&] {
                rndsleep();
                return 1;
            },
            [&] {
                rndsleep();
                return 2;
            },
            [&] {
                rndsleep();
                return 3;
            })
                     .then(
                         [&](auto t) {
                             rndsleep();
                             auto [a, b, c] = t;
                             assert(a + b + c == 1 + 2 + 3);
                             return 0;
                         },
                         [&](auto t) {
                             rndsleep();
                             auto [a, b, c] = t;
                             assert(a + b + c == 1 + 2 + 3);
                             return 1;
                         })
                     .then(
                         [](auto t) {
                             auto [a, b] = t;
                             assert(a + b == 1);
                             return std::string{"hello"};
                         },
                         [](auto t) {
                             auto [a, b] = t;
                             assert(a + b == 1);
                             return std::string{"world"};
                         })
                     .then([](auto y) {
                         auto [s0, s1] = y;
                         assert(s0 + s1 == "helloworld");
                     });

        std::move(f).wait_and_get(world_s_best_thread_pool{});
    }


    for(int i = 0; i < 1000; ++i)
    {
        assert(ctr == 0);
        auto f = initiate([] { return std::make_shared<sctr>(); })
                     .then([](auto&) { assert(ctr == 1); },
                         [](auto&) { assert(ctr == 1); })
                     .then([](auto) { assert(ctr == 0); });

        std::move(f).wait_and_get(world_s_best_thread_pool{});
        assert(ctr == 0);
    }
}



int main()
{
  std::cout << sizeof(std::tuple<int>) << " " << sizeof(orizzonte::utility::cache_aligned_tuple<int>) << '\n';

  return 0;

    fuzzy();

    {
        auto f = initiate([] { return 1; });
        assert(std::move(f).wait_and_get(world_s_best_thread_pool{}) == 1);
    }


    {
        auto f = initiate([] { return 1; }).then([](int x) { return x + 1; });

        assert(std::move(f).wait_and_get(world_s_best_thread_pool{}) == 2);
    }

    {
        auto f = initiate([] { return 1; })
                     .then([](int x) { return x + 1; })
                     .then([](int x) { return x + 1; });

        assert(std::move(f).wait_and_get(world_s_best_thread_pool{}) == 3);
    }

    {
        auto f = initiate([] { return 1; })
                     .then([](int x) { return x + 1; })
                     .then([](int) { std::cout << "void!\n"; });

        std::move(f).wait_and_get(world_s_best_thread_pool{});
    }

    {
        auto f = initiate([] { return 1; }, [] { return 2; });

        assert(std::move(f).wait_and_get(world_s_best_thread_pool{}) ==
               (std::tuple{1, 2}));
    }

    {
        auto f = initiate([] { return 1; }, [] { return 1; }).then([](auto t) {
            auto [a, b] = t;
            return a + b;
        });

        assert(std::move(f).wait_and_get(world_s_best_thread_pool{}) == 2);
    }

    auto f2 = initiate(
        [] {
            std::cout << "A0\n";
            return 1;
        },
        [] {
            std::cout << "A1\n";
            return 2;
        })
                  .then([](auto t) {
                      auto [a, b] = t;
                      assert(a + b == 3);
                      return a + b;
                  })
                  .then(
                      [](auto x) {
                          assert(x == 3);
                          std::cout << x << " C0\n";
                          return std::string{"hello"};
                      },
                      [](auto x) {
                          assert(x == 3);
                          std::cout << x << " C1\n";
                          return std::string{"world"};
                      })
                  .then([](auto y) {
                      auto [s0, s1] = y;
                      assert(s0 == "hello");
                      assert(s1 == "world");
                      std::cout << s0 << " " << s1 << "\n";
                  });

    std::move(f2).wait_and_get(world_s_best_thread_pool{});

    auto f3 = initiate([] {}).then([] {});
    std::move(f3).wait_and_get(world_s_best_thread_pool{});

    auto f4 = initiate([] {}, [] {}).then([](auto) {});
    std::move(f4).wait_and_get(world_s_best_thread_pool{});

    auto f5 = initiate(
        [] {
            std::cout << "A0\n";
            return 1;
        },
        [] {
            std::cout << "A1\n";
            return 2;
        })
                  .then([](auto t) {
                      auto [a, b] = t;
                      assert(a + b == 3);
                      return a + b;
                  })
                  .then(
                      [](auto x) {
                          assert(x == 3);
                          std::cout << x << " C0\n";
                          return 2;
                      },
                      [](auto x) {
                          assert(x == 3);
                          std::cout << x << " C1\n";
                          return 3;
                      })
                  .then(
                      [](auto t) {
                          auto [a, b] = t;
                          auto x = a + b;
                          assert(x == 5);
                          std::cout << x << " C2\n";
                          return 4;
                      },
                      [](auto t) {
                          auto [a, b] = t;
                          auto x = a + b;
                          assert(x == 5);
                          std::cout << x << " C3\n";
                          return 5;
                      })
                  .then([](auto y) {
                      auto [a, b] = y;
                      assert(a + b == 9);
                  });

    std::move(f5).wait_and_get(world_s_best_thread_pool{});

    std::cout.flush();
    return 0;
}
