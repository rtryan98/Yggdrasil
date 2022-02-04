// Copyright 2022 Robert Ryan. See Licence.md.

#include <atomic>

namespace ygg::thread
{
    class spinlock
    {
    public:

    private:
        std::atomic<bool> m_val;
    };
}
