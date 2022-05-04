// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

namespace ygg
{
    /**
     * @brief Typesafe opaque handle.
     * @tparam Identifier Identifier used to make this Handle typesafe.
     * @tparam Value_type The underlying value type used for this Handle.
    */
    template<class Identifier, typename Value_type>
    class Handle
    {
    public:
        /**
         * @brief Constructs a Handle with underlying `t`.
        */
        explicit Handle(Value_type t) : m_value(t) {}

        /**
         * @brief Returns the underlying value.
        */
        explicit operator Value_type() const { return m_value; }

        friend bool operator==(Handle a, Handle b) { return a.m_value == b.m_value; };
        friend bool operator!=(Handle a, Handle b) { return !(a.m_value == b.m_value); };
    private:
        Value_type m_value;
    };
}
