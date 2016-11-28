#pragma once

#include <cassert>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define TEST_CONST __attribute__((const))
#define TEST_MAIN(...) int TEST_CONST main(__VA_ARGS__)
#define TEST_LIKELY(...) __builtin_expect(!!(__VA_ARGS__), 1)

namespace test_impl
{
    namespace impl
    {
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
                // TODO: if contexpr(can print)
                _s << "  RESULT: `" << lhs_result << "`\n";

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

                // TODO: if contexpr(can print)
                _s << "`, which evaluates to `" << rhs_result << "`\n";


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
