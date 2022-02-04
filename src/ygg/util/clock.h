// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include <chrono>
#include <cmath>

namespace ygg::util
{
    class clock
    {
    public:
        clock();

        /**
         * @brief Advanced the current frame and calculates the current delta time and time since start.
         * @details This must only be called once per frame.
         * This function must be externally synchronized.
        */
        void next_clock_frame() noexcept;

        /**
         * @brief Returns the current delta time, as calculated by `next_clock_frame()`.
         * @details This will always return 0.0f in the first frame, assuming `next_clock_frame()` was called.
         * @return The current delta time, with `1.0f` representing 1 second.
        */
        [[nodiscard]] float_t dt() const noexcept { return m_dt; };

        /**
         * @brief Returns the time since this clock has been constructed.
         * @return The time since construction of this clock, with `1.0f` representing 1 second.
        */
        [[nodiscard]] float_t time_since_start() const noexcept { return m_time_since_start; };

    private:
        std::chrono::time_point<std::chrono::system_clock> m_current_time;
        std::chrono::time_point<std::chrono::system_clock> m_last_time;
        float_t m_dt;
        float_t m_time_since_start;
    };
}
