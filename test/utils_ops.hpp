#include <vrm/core/strong_typedef.hpp>
#include "./utils.hpp"

// TODO: ?

namespace test_impl::opt
{
    VRM_CORE_STRONG_TYPEDEF(int, n_ctors);
    VRM_CORE_STRONG_TYPEDEF(int, n_dtors);
    VRM_CORE_STRONG_TYPEDEF(int, n_copies);
    VRM_CORE_STRONG_TYPEDEF(int, n_moves);

    constexpr n_ctors untracked_ctors{-1};
    constexpr n_dtors untracked_dtors{-1};
    constexpr n_copies untracked_copies{-1};
    constexpr n_moves untracked_moves{-1};

    class context;

    struct anything
    {
        anything()
        {
            ++(impl::ctors());
        }

        ~anything()
        {
            ++(impl::dtors());
        }

        anything(const anything&)
        {
            ++(impl::copies());
        }

        anything& operator=(const anything&)
        {
            ++(impl::copies());
            return *this;
        }

        anything(anything&&)
        {
            ++(impl::moves());
        }

        anything& operator=(anything&&)
        {
            ++(impl::moves());
            return *this;
        }
    };

    class context_aware;

    class context
    {
        friend class context_aware;

    private:
        n_ctors _ctors{untracked_ctors};
        n_dtors _dtors{untracked_dtors};
        n_copies _copies{untracked_copies};
        n_moves _moves{untracked_moves};

    public:
        context() = default;
    };

    class context_aware
    {
    private:
        context& _ctx;

    public:
        context_aware(context& ctx) noexcept : _ctx{ctx}
        {
        }
    };
}