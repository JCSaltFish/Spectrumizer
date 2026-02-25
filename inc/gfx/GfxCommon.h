/**
 * @file GfxCommon.h
 * @brief Common definitions and utilities for the gfx module.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <exception>
#include <optional>
#include <algorithm>
#include <functional>
#include <variant>
#include <cmath>
#include <cstring>
#include <type_traits>
#include <mutex>

/**
 * @brief Structure representing a rectangle.
 */
struct GfxRect {
    int x = 0; // X coordinate of the rectangle
    int y = 0; // Y coordinate of the rectangle
    int width = 1; // Width of the rectangle
    int height = 1; // Height of the rectangle
};

/**
 * @brief A template class for bitwise.
 */
template<typename Enum>
class GfxFlags {
public:
    using UnderlyingType = std::underlying_type_t<Enum>;

    GfxFlags() : m_value(0) {};
    GfxFlags(Enum bit) : m_value(static_cast<UnderlyingType>(bit)) {};

    bool operator==(const GfxFlags& other) const {
        return m_value == other.m_value;
    };
    bool operator==(Enum bit) const {
        return m_value == static_cast<UnderlyingType>(bit);
    };
    bool operator!=(const GfxFlags& other) const {
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
