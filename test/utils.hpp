#pragma once

#include <cassert>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

#define TEST_PURE __attribute__((pure))
#define TEST_CONST __attribute__((const))
#define TEST_MAIN(...) int TEST_CONST main(__VA_ARGS__)
#define TEST_LIKELY(...) __builtin_expect(!!(__VA_ARGS__), 1)
#define TEST_IF_CONSTEXPR \
    if                    \
    constexpr

namespace test_impl
{
    namespace impl
    {
        template <typename...>
        using void_t = void;

        template <typename T, typename = void>
        struct can_print_t : std::false_type
        {
        };

        template <typename T>
        struct can_print_t<T, void_t<decltype(std::declval<std::ostream&>()
                                              << std::declval<T>())>>
            : std::true_type
        {
        };

        template <typename T>
        constexpr can_print_t<T> can_print{};

        inline auto& get_oss() noexcept
        {
            static std::ostringstream oss;
            return oss;
        }

        inline auto& clear_and_get_oss() noexcept
        {
            auto& oss(get_oss());
            oss.str("");
            return oss;
        }

        [[noreturn]] inline void fail() noexcept
        {
            std::cout
                << "\n-----------------------------------------------------\n"
                << get_oss().str()
                << "-----------------------------------------------------\n"
                << std::endl;
            std::exit(1);
        }

        template <typename TStream>
        struct output_t
        {
            TStream& _s;

            output_t(TStream& s) noexcept : _s{s}
            {
            }

            auto& line(int line)
            {
                _s << "    LINE: `" << line << "`\n";
                return *this;
            }

            auto& expr(const char* expr)
            {
                _s << "    EXPR: `" << expr << "`\n";
                return *this;
            }

            template <typename T>
            auto& result(const T& lhs_result)
            {
                TEST_IF_CONSTEXPR(can_print<T>)
                {
                    _s << "  RESULT: `" << lhs_result << "`\n";
                }

                return *this;
            }

            auto& expected(const char* expected)
            {
                _s << "EXPECTED: `" << expected << "`\n";
                return *this;
            }

            template <typename T>
            auto& expected(const char* expected, const T& rhs_result)
            {
                _s << "EXPECTED: `" << expected;

                TEST_IF_CONSTEXPR(can_print<T>)
                {
                    _s << "`, which evaluates to `" << rhs_result << "`\n";
                }

                return *this;
            }
        };

        template <typename TStream>
        inline auto output(TStream& s) noexcept
        {
            return output_t<TStream>{s};
        }

        template <typename TF>
        inline auto do_test(bool x, TF&& f)
        {
            if(TEST_LIKELY(x)) return;

            auto& error(impl::clear_and_get_oss());
            f(output(error));
            fail();
        }
    }

    template <typename T>
    inline auto test_expr(
        int line, bool x, T&& lhs_result, const char* expr) noexcept
    {
        return impl::do_test(
            x, [&](auto&& o) { o.line(line).expr(expr).result(lhs_result); });
    }

    template <typename TLhs, typename TRhs>
    inline auto test_op(int line, bool x, TLhs&& lhs_result, TRhs&& rhs_result,
        const char* expr, const char* expected)
    {
        return impl::do_test(x, [&](auto&& o) {
            o.line(line)
                .expr(expr)
                .result(lhs_result)
                .expected(expected, rhs_result);
        });
    }
}

#define EXPECT(expr)                                       \
    do                                                     \
    {                                                      \
        auto _t_x(expr);                                   \
                                                          \
        test_impl::test_expr(__LINE__, _t_x, expr, #expr); \
    } while(false)

#define EXPECT_OP(lhs, op, rhs)                                         \
    do                                                                  \
    {                                                                   \
        using ct = std::common_type_t<decltype(lhs), decltype(rhs)>;    \
                                                                        \
        auto _t_xl(lhs);                                                \
        auto _t_xr(rhs);                                                \
                                                                        \
        auto _t_x(ct(_t_xl) op ct(_t_xr));                              \
                                                                        \
        test_impl::test_op(                                             \
            __LINE__, _t_x, _t_xl, _t_xr, #lhs " " #op " " #rhs, #rhs); \
    } while(false)

#define EXPECT_EQ(lhs, rhs) EXPECT_OP(lhs, ==, rhs)
#define EXPECT_NEQ(lhs, rhs) EXPECT_OP(lhs, !=, rhs)
#define EXPECT_LT(lhs, rhs) EXPECT_OP(lhs, <, rhs)
#define EXPECT_LTE(lhs, rhs) EXPECT_OP(lhs, <=, rhs)
#define EXPECT_GT(lhs, rhs) EXPECT_OP(lhs, >, rhs)
#define EXPECT_GTE(lhs, rhs) EXPECT_OP(lhs, >=, rhs)

#define EXPECT_TRUE(x) EXPECT_EQ(x, true)
#define EXPECT_FALSE(x) EXPECT_EQ(x, false)

#define SA_SAME_TYPE(T0, T1) static_assert(std::is_same<T0, T1>{})
#define SA_TYPE_IS(a, T) SA_SAME_TYPE(decltype(a), T)
#define SA_DECAY_TYPE_IS(a, T) SA_SAME_TYPE(std::decay_t<decltype(a)>, T)


namespace test
{
    namespace impl
    {
        auto& TEST_CONST ctors() noexcept
        {
            static int res{0};
            return res;
        }

        auto& TEST_CONST dtors() noexcept
        {
            static int res{0};
            return res;
        }

        auto& TEST_CONST copies() noexcept
        {
            static int res{0};
            return res;
        }

        auto& TEST_CONST moves() noexcept
        {
            static int res{0};
            return res;
        }

        class expector
        {
        private:
            int _ctors = -1;
            int _dtors = -1;
            int _copies = -1;
            int _moves = -1;

            void unset()
            {
                _ctors = -1;
                _dtors = -1;
                _copies = -1;
                _moves = -1;
            }

        public:
            ~expector()
            {
                if(_ctors != -1)
                {
                    EXPECT_EQ(impl::ctors(), _ctors);
                }

                if(_dtors != -1)
                {
                    EXPECT_EQ(impl::dtors(), _dtors);
                }

                if(_copies != -1)
                {
                    EXPECT_EQ(impl::copies(), _copies);
                }

                if(_moves != -1)
                {
                    EXPECT_EQ(impl::moves(), _moves);
                }
            }

            auto ctors(int x) noexcept
            {
                auto copy = *this;
                copy._ctors = x;
                unset();
                return copy;
            }

            auto dtors(int x) noexcept
            {
                auto copy = *this;
                copy._dtors = x;
                unset();
                return copy;
            }

            auto copies(int x) noexcept
            {
                auto copy = *this;
                copy._copies = x;
                unset();
                return copy;
            }

            auto moves(int x) noexcept
            {
                auto copy = *this;
                copy._moves = x;
                unset();
                return copy;
            }
        };
    }

    template <typename TF>
    auto expect_ops(TF&& f)
    {
        impl::copies() = 0;
        impl::moves() = 0;

        f();
        return impl::expector{};
    }

    struct nocopy
    {
        nocopy()
        {
            ++(impl::ctors());
        }

        ~nocopy()
        {
            ++(impl::dtors());
        }

        nocopy(const nocopy&) = delete;
        nocopy& operator=(const nocopy&) = delete;

        nocopy(nocopy&&)
        {
            ++(impl::moves());
        }

        nocopy& operator=(nocopy&&)
        {
            ++(impl::moves());
            return *this;
        }
    };

    struct nomove
    {
        nomove()
        {
            ++(impl::ctors());
        }

        ~nomove()
        {
            ++(impl::dtors());
        }

        nomove(const nomove&)
        {
            ++(impl::copies());
        }

        nomove& operator=(const nomove&)
        {
            ++(impl::copies());
            return *this;
        }

        nomove(nomove&&) = delete;
        nomove& operator=(nomove&&) = delete;
    };

    struct anything
    {
        anything()
        {
            std::cout << "anything()\n";
            ++(impl::ctors());
        }

        ~anything()
        {
            std::cout << "~anything()\n";
            ++(impl::dtors());
        }

        anything(const anything&)
        {
            std::cout << "anything(const anything&)\n";
            ++(impl::copies());
        }

        anything& operator=(const anything&)
        {
            std::cout << "anything& operator=(const anything&)\n";
            ++(impl::copies());
            return *this;
        }

        anything(anything&&)
        {
            std::cout << "anything(anything&&)\n";
            ++(impl::moves());
        }

        anything& operator=(anything&&)
        {
            std::cout << "anything& operator=(anything&&)\n";
            ++(impl::moves());
            return *this;
        }
    };
}