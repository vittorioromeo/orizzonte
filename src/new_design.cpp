#include <utility>
#include <iostream>
#include <experimental/type_traits>

#define FWD(x) ::std::forward<decltype(x)>(x)

template <class T, class... Ts>
using executable = decltype(std::declval<T>().execute(std::declval<Ts>()...));

template <typename Scheduler, typename F, typename... Ts>
decltype(auto) activate(Scheduler& s, F& f, Ts&&... xs)
{
    if constexpr(std::experimental::is_detected_v<executable, F&, Scheduler&, Ts&&...>)
    {
        return f.execute(s, FWD(xs)...);
    }
    else
    {
        return f(FWD(xs)...);
    }
}


/*template <typename F>
struct node_one : F
{
    node_one(F&& f) : F{std::move(f)} { }

    void execute()
    {
        activate(f);
    }
};*/

template <typename A, typename B>
struct node_two : A, B
{
    node_two(A&& a, B&& b) : A{std::move(a)}, B{std::move(b)} { }

    template <typename Scheduler, typename... Ts>
    void execute(Scheduler& s, Ts&&... xs)
    {
        auto a_result = activate(s, static_cast<A&>(*this), FWD(xs)...);
        activate(s, static_cast<B&>(*this), std::move(a_result));

    }
};


template <typename... Fs>
struct node_and : Fs...
{
    node_and(Fs&&... fs) : Fs{std::move(fs)}... { }

    template <typename Scheduler, typename... Ts>
    void execute(Scheduler& s, Ts&&... xs)
    {
        // TODO: xs... must be emplaced in node storage

        auto run = [&](auto& f)
        {
            activate(s, f, xs...);
        };

        (run(static_cast<Fs&>(*this)), ...);
    }
};

template <typename... Fs>
struct node_any : Fs...
{
    node_any(Fs&&... fs) : Fs{std::move(fs)}... { }

    template <typename Scheduler, typename... Ts>
    void execute(Scheduler& s, Ts&&... xs)
    {
        // TODO: xs... must be emplaced in node storage

        auto run = [&](auto& f)
        {
            activate(s, f, xs...);
        };

        (run(static_cast<Fs&>(*this)), ...);
    }
};

struct S { };

int main()
{
        auto scheduler = S{};

    auto n0 = node_two{[]{ return 42; }, [](int x){ std::cout << "got " << x << '\n'; }};
    n0.execute(scheduler);

    auto nx = node_two{[]{ return 42; }, node_two{[](int x){ std::cout << "got " << x << '\n'; return x + 1; },
                                                  [](int x){ std::cout << "finalres: " << x << '\n'; }}};
    nx.execute(scheduler);

    auto n1 = node_two{[]{ return 42; }, node_and{[](int x){ std::cout << "got " << x << '\n'; },
                                                  [](int x){ std::cout << "me too " << x << '\n'; }}};
    n1.execute(scheduler);

    auto l0 = []{ std::cout << "l0\n"; return 0; };
    auto l1 = []{ std::cout << "l1\n"; return 'a'; };

    auto and01 = node_and{std::move(l0), std::move(l1)};

    auto l2 = []{ std::cout << "l2\n"; return 1; };
    auto l3 = []{ std::cout << "l3\n"; return 'b'; };

    auto and23 = node_and{std::move(l2), std::move(l3)};

    auto total = node_any{std::move(and01), std::move(and23)};

    total.execute(scheduler);
}
