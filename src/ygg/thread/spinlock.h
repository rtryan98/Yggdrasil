// Copyright 2022 Robert Ryan. See Licence.md.

#include <atomic>

namespace ygg::thread
{
    class spinlock
    {
    public:
        void lock();
        void unlock();

    private:
        std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
    };
}
