/**
 * @file Flags.hpp
 * @brief A header file defining a template class for bitwise operations on enum types.
 */

#pragma once

#include "utils/UtilsCommon.h"

/**
 * @brief A template class for bitwise.
 */
template<typename Enum>
class Flags {
public:
    using UnderlyingType = std::underlying_type_t<Enum>;

    Flags() : m_value(0) {};
    Flags(Enum bit) : m_value(static_cast<UnderlyingType>(bit)) {};
    Flags(UnderlyingType value) : m_value(value) {};

    bool operator==(const Flags& other) const {
        return m_value == other.m_value;
    };
    bool operator==(Enum bit) const {
        return m_value == static_cast<UnderlyingType>(bit);
    };
    bool operator!=(const Flags& other) const {
        return m_value != other.m_value;
    };

    /**
     * @brief Sets a specific bit.
     * @param bit The bit to set.
     */
    void set(Enum bit) {
        m_value |= static_cast<UnderlyingType>(bit);
    };
    /**
     * @brief Unsets a specific bit.
     * @param bit The bit to unset.
     */
    void unset(Enum bit) {
        m_value &= ~static_cast<UnderlyingType>(bit);
    };
    void reset() { m_value = 0; };
    /**
     * @brief Checks if a specific bit is set.
     * @param bit The bit to check.
     * @return True if the bit is set, false otherwise.
     */
    bool check(Enum bit) const {
        return (m_value & static_cast<UnderlyingType>(bit)) != 0;
    };
    /**
     * @brief Returns the underlying value of the flags.
     * @return The underlying value representing the bitwise flags.
     */
    UnderlyingType getValue() const {
        return m_value;
    };

private:
    UnderlyingType m_value = 0; // Stores the bitwise flags as an underlying type
};
