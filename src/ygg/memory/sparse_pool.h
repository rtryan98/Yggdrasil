// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include <type_traits>
#include <vector>

namespace ygg::memory
{
    /**
     * @brief A non-iterable object store, to provide faster allocation.
     * @tparam T The type that will be stored.
    */
    template<typename T>
    class Sparse_pool
    {
        static_assert(sizeof(T) >= sizeof(std::size_t),
            "This implementation of Sparse_pool stores the linked list inside the data store itself. "
            "The index size (sizeof(std::size_t)) must be less than or equal to sizeof(T).");
        static_assert(std::is_trivial(T)::value && std::is_standard_layout(T)::value,
            "T must be POD.");
    public:
        /**
         * @brief Emplaces an element into this pool, returning the index.
         * @details The order of insertion after removal is defined by a linked list.
         * This function must be externally synchronized.
         * @tparam ...Args The forward argument types of T's constructor.
         * @param ...args The forward arguments of T's constructor.
         * @return The inserted element.
        */
        template<class... Args>
        std::size_t emplace(Args&&... args)
        {
            std::size_t idx = m_head;
            if (idx != m_no_head) {
                m_head = *reinterpret_cast<std::size_t*>(&m_vector[idx]);
                m_vector[idx] = T(std::forward<Args>(args)...);
                return idx;
            } else {
                m_vector.emplace_back(std::forward<Args>(args)...);
                return m_vector.size() - 1ull;
            }
        }

        /**
         * @brief Removes an element from this pool.
         * @details Removes an element and changes this pools head to idx.
         * This function must be externally synchronized.
         * @param idx The index of the element.
        */
        void remove(std::size_t idx)
        {
            memset(&m_vector[idx], 0, sizeof(T));
            *reinterpret_cast<std::size_t*> (&m_vector[idx]) = m_head;
            m_head = idx;
        }

        /**
         * @brief Returns the element associated with the provided index `idx`.
         * @param idx The index of the element.
         * @return The element associated with idx.
        */
        [[nodiscard]] T& at(std::size_t idx) noexcept { return m_vector.at(idx); }

        /**
         * @brief Returns the element associated with the provided index `idx`.
         * @param idx The index of the element.
         * @return The element associated with idx.
        */
        [[nodiscard]] const T& at(std::size_t idx) const noexcept { return m_vector.at(idx); }

        /**
         * @brief Returns the element associated with the provided index `idx`.
         * @param idx The index of the element.
         * @return The element associated with idx.
        */
        [[nodiscard]] T& operator[](std::size_t idx) noexcept { return m_vector[idx]; }

        /**
         * @brief Returns the element associated with the provided index `idx`.
         * @param idx The index of the element.
         * @return The element associated with idx.
        */
        [[nodiscard]] const T& operator[](std::size_t idx) const noexcept { return m_vector[idx]; }

    private:
        static constexpr inline uint64_t m_no_head = ~0ull;
        std::vector<T> m_vector;
        std::size_t m_head = m_no_head;
    };
}
