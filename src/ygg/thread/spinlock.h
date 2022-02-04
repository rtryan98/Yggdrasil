// Copyright 2022 Robert Ryan. See Licence.md.

#include <atomic>

namespace ygg::thread
{
    /**
     * @brief Basic lock for synchronizing low-contention work.
    */
    class spinlock
    {
    public:
        spinlock() noexcept = default;

        /**
         * @brief Locks the spinlock until it has been unlocked via unlock().
        */
        void lock() noexcept;

        /**
         * @brief Unlocks this spinlock, making it available for other threads.
        */
        void unlock() noexcept;

    private:
        std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
    };
}
