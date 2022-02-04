// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/thread/spinlock.h"

namespace ygg::thread
{
    void spinlock::lock() noexcept
    {
        while (true) {
            if (!m_flag.test_and_set(std::memory_order::acquire))
                break;
            while (m_flag.test(std::memory_order::relaxed))
                ;
        }
    }

    void spinlock::unlock() noexcept
    {
        m_flag.clear(std::memory_order::release);
    }
}
