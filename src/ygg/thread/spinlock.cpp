// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/thread/spinlock.h"

namespace ygg::thread
{
    void spinlock::lock()
    {
        while (true) {
            if (!m_flag.test_and_set(std::memory_order_acquire))
                break;
            while (m_flag.test(std::memory_order::relaxed))
                ;
        }
    }

    void spinlock::unlock()
    {
        m_flag.clear(std::memory_order::release);
    }
}
