// Copyright (c) 2017 Vittorio Romeo
// MIT License |  https://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <new>
#include <random>
#include <type_traits>
#include <utility>

#include "../include/orizzonte.hpp"

template <typename T, std::size_t N>
struct aligned
{
    alignas(N) T _x;
};

template <typename... Ts>
struct cache_aligned_tuple
{
    static constexpr auto hardware_destructive_interference_size = 64;
    std::tuple<aligned<Ts, hardware_destructive_interference_size>...> _data;
};

using namespace orizzonte::utility;
using namespace orizzonte::meta;

template <typename Parent, typename F>
class node;

template <typename Parent, typename... Fs>
class when_all;

class root
{
    template <typename>
    friend class node_base;

public:
    // The `root` produces `nothing`.
    using output_type = nothing;

private:
    // When we are at the `root`, we cannot go "up" the chain anymore.
    // Therefore we being going "down".
    template <typename Scheduler, typename Child, typename... Children>
    void walk_up(Scheduler&& s, Child& c, Children&... cs) &
    {
        c.execute(s, nothing{}, cs...);
    }
};

template <typename Parent>
class child_of : public Parent
{
public:
    using input_type = typename Parent::output_type;

protected:
    template <typename ParentFwd>
    child_of(ParentFwd&& p) : Parent{FWD(p)}
    {
    }

    auto& as_parent() noexcept
    {
        return static_cast<Parent&>(*this);
    }
};

template <typename Derived>
class node_base
{
private:
    auto& as_derived() noexcept
    {
        return static_cast<Derived&>(*this);
    }

public:
    template <typename... FThens>
    auto then(FThens&&... f_thens) &&
    {
        if constexpr(sizeof...(FThens) == 1)
        {
            return node{std::move(as_derived()), FWD(f_thens)...};
        }
        else
        {
            return when_all{std::move(as_derived()), FWD(f_thens)...};
        }
    }

    template <typename Scheduler, typename... Children>
    void walk_up(Scheduler&& s, Children&... cs) &
    {
        as_derived().as_parent().walk_up(s, as_derived(), cs...);
    }

    template <typename Scheduler>
    decltype(auto) wait_and_get(Scheduler&& s) &&
    {
        typename Derived::output_type out;

        {
            orizzonte::utility::scoped_bool_latch l;
            auto f = std::move(as_derived()).then([&](auto&&... x) {
                ((out = FWD(x)), ...);
                l.count_down();
            });

            f.walk_up(s);
        }

        return out;
    }
};

template <typename Parent, typename F>
class node : private child_of<Parent>,
             private F,
             public node_base<node<Parent, F>>
{
public:
    using typename child_of<Parent>::input_type;
    using output_type = result_of_ignoring_nothing_t<F&, input_type>;

    template <typename ParentFwd, typename FFwd>
    node(ParentFwd&& p, FFwd&& f) : child_of<Parent>{FWD(p)}, F{FWD(f)}
    {
    }

    using crtp_base_type = node_base<node<Parent, F>>;
    friend crtp_base_type;

    using crtp_base_type::then;
    using crtp_base_type::wait_and_get;
    using crtp_base_type::walk_up;

private:
    auto& as_f() noexcept
    {
        return static_cast<F&>(*this);
    }

public:
    template <typename Scheduler, typename Result>
    void execute(Scheduler&&, Result&& r) &
    {
        call_ignoring_nothing(as_f(), FWD(r));
    }

    template <typename Scheduler, typename Result, typename Child,
        typename... Children>
    void execute(Scheduler&& s, Result&& r, Child& c, Children&... cs) &
    {
        // `r` doesn't need to be stored inside the node here as it is used
        // to synchronously invoke `as_f()`.
        c.execute(s, call_ignoring_nothing(as_f(), FWD(r)), cs...);
    }
};

template <typename ParentFwd, typename FFwd>
node(ParentFwd&&, FFwd &&)->node<std::decay_t<ParentFwd>, std::decay_t<FFwd>>;

template <typename Parent>
class schedule : private child_of<Parent>, public node_base<schedule<Parent>>
{
public:
    using typename child_of<Parent>::input_type;
    using output_type = nothing;

    template <typename ParentFwd>
    schedule(ParentFwd&& p) : child_of<Parent>{FWD(p)}
    {
    }

    using crtp_base_type = node_base<schedule<Parent>>;
    friend crtp_base_type;

    using crtp_base_type::then;
    using crtp_base_type::wait_and_get;
    using crtp_base_type::walk_up;

    template <typename Scheduler, typename Result, typename Child,
        typename... Children>
    void execute(Scheduler&& s, Result&&, Child& c, Children&... cs) &
    {
        s([&] { c.execute(s, nothing{}, cs...); });
    }
};

template <typename ParentFwd>
schedule(ParentFwd &&)->schedule<std::decay_t<ParentFwd>>;

template <typename T>
struct tuple_of_nothing_to_empty;

template <typename T>
struct tuple_of_nothing_to_empty<std::tuple<T>>
{
    using type = std::tuple<T>;
};

template <>
struct tuple_of_nothing_to_empty<std::tuple<nothing>>
{
    using type = std::tuple<>;
};

template <typename T>
using tuple_of_nothing_to_empty_t = typename tuple_of_nothing_to_empty<T>::type;


template <typename T>
struct adapt_tuple_of_nothing;

template <typename... Ts>
struct adapt_tuple_of_nothing<std::tuple<Ts...>>
{
    using type = decltype(std::tuple_cat(
        std::declval<tuple_of_nothing_to_empty_t<std::tuple<Ts>>>()...));
};

template <typename T>
using adapt_tuple_of_nothing_t = typename adapt_tuple_of_nothing<T>::type;

template <typename Parent, typename... Fs>
class when_all : private child_of<Parent>,
                 private Fs...,
                 public node_base<when_all<Parent, Fs...>>
{
public:
    using typename child_of<Parent>::input_type;
    using output_type =
        std::tuple<result_of_ignoring_nothing_t<Fs&, input_type&>...>;

    // TODO: should this be done, or should the tuple be applied to following
    // node? using output_type = adapt_tuple_of_nothing_t<
    //     std::tuple<result_of_ignoring_nothing_t<Fs&, input_type&>...>
    // >;

private:
    // TODO: the size of the entire computation might grow by a lot. Is it
    // possible to reuse this space for multiple nodes?
    movable_atomic<int> _left{sizeof...(Fs)};
    output_type _out;
    aligned_storage_for<input_type> _input_buf;

    template <bool B, typename Scheduler, typename F>
    void schedule_if(Scheduler&& s, F&& f)
    {
        if constexpr(B)
        {
            FWD(f)();
        }
        else
        {
            // The computation has to be moved here as it will die at
            // the end of the `enumerate_args` lambda scope.
            s([g = std::move(f)]() mutable { g(); });
        }
    }

public:
    template <typename ParentFwd, typename... FFwds>
    when_all(ParentFwd&& p, FFwds&&... fs)
        : child_of<Parent>{FWD(p)}, Fs{FWD(fs)}...
    {
    }

    using crtp_base_type = node_base<when_all<Parent, Fs...>>;
    friend crtp_base_type;

    using crtp_base_type::then;
    using crtp_base_type::wait_and_get;
    using crtp_base_type::walk_up;

public:
    template <typename Scheduler, typename Result>
    void execute(Scheduler&& s, Result&& r) &
    {
        // TODO: what if `Result` is an lvalue reference?
        _input_buf.construct(FWD(r));

        enumerate_args(
            [&](auto i, auto t) {
                auto do_computation = [&] {
                    call_ignoring_nothing(
                        static_cast<unwrap<decltype(t)>&>(*this),
                        _input_buf.access());

                    if(_left.fetch_sub(1) == 1)
                    {
                        // TODO: make sure this destruction is correct, probably
                        // should not destroy if storing lvalue reference
                        _input_buf.destroy();
                    }
                };

                constexpr bool is_last = decltype(i){} == sizeof...(Fs) - 1;
                schedule_if<is_last>(s, std::move(do_computation));
            },
            t<Fs>...);
    }

    template <typename Scheduler, typename Result, typename Child,
        typename... Children>
    void execute(Scheduler&& s, Result&& r, Child& c, Children&... cs) &
    {
        // This is necessary as `r` only lives as long as `execute` is active on
        // the call stack. Computations might still be active when `execute`
        // ends, even if the last one is executed on the same thread. This is
        // because the scheduled computations might finish after the last one.
        // TODO: what if `Result` is an lvalue reference?
        _input_buf.construct(FWD(r));

        enumerate_args(
            [&](auto i, auto t) {
                auto do_computation = [&] {
                    std::get<decltype(i){}>(_out) = call_ignoring_nothing(
                        static_cast<unwrap<decltype(t)>&>(*this),
                        _input_buf.access());

                    if(_left.fetch_sub(1) == 1)
                    {
                        // TODO: make sure this destruction is correct, probably
                        // should not destroy if storing lvalue reference
                        _input_buf.destroy();
                        c.execute(s, _out, cs...); // TODO: apply the tuple here
                                                   // to pass multiple arguments
                    }
                };

                constexpr bool is_last = decltype(i){} == sizeof...(Fs) - 1;
                schedule_if<is_last>(s, std::move(do_computation));
            },
            t<Fs>...);
    }
};

template <typename ParentFwd, typename... FFwds>
when_all(ParentFwd&&, FFwds&&...)
    ->when_all<std::decay_t<ParentFwd>, std::decay_t<FFwds>...>;

template <typename... Fs>
auto initiate(Fs&&... fs)
{
    return schedule{root{}}.then(FWD(fs)...);
}
