// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/util/clock.h"

namespace ygg::util
{
    clock::clock()
        : m_current_time( std::chrono::system_clock::now() ),
        m_last_time( m_current_time ),
        m_dt( 0.0f ),
        m_time_since_start( 0.0f )
    {}

    void clock::next_clock_frame() noexcept
    {
        m_last_time = m_current_time;
        m_current_time = std::chrono::system_clock::now();
        m_dt = std::chrono::duration_cast<std::chrono::duration<float_t>>(m_current_time - m_last_time).count();
        m_time_since_start += m_dt;
    }
}
