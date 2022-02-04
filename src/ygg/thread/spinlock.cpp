// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/thread/spinlock.h"

namespace ygg::thread
{
    void spinlock::lock()
    {
        while (m_flag.test_and_set(std::memory_order::acquire))
            ;
    }

    void spinlock::unlock()
    {
        m_flag.clear(std::memory_order::release);
    }
}
