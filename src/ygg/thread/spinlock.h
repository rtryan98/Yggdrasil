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
        /**
         * @brief Locks the spinlock until it has been unlocked via unlock().
        */
        void lock();

        /**
         * @brief Unlocks this spinlock, making it available for other threads.
        */
        void unlock();

    private:
        std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
    };
}
