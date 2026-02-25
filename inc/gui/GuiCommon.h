/**
 * @file GuiCommon.h
 * @brief Common definitions and utilities for the GUI module.
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <variant>
#include <functional>
#include <type_traits>
#include <typeindex>
#include <regex>

/**
 * @brief Enumeration for GUI key codes.
 */
enum class GuiKey {
    UNKNOWN = -1,

    // Printable keys
    SPACE = 32,
    APOSTROPHE = 39,  /* ' */
    COMMA = 44,  /* , */
    MINUS = 45,  /* - */
    PERIOD = 46,  /* . */
    SLASH = 47,  /* / */
    ZERO = 48,
    ONE = 49,
    TWO = 50,
    THREE = 51,
    FOUR = 52,
    FIVE = 53,
    SIX = 54,
    SEVEN = 55,
    EIGHT = 56,
    NINE = 57,
    SEMICOLON = 59,  /* ; */
    EQUAL = 61,  /* = */
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LEFT_BRACKET = 91,  /* [ */
    BACKSLASH = 92,  /* \ */
    RIGHT_BRACKET = 93,  /* ] */
    GRAVE_ACCENT = 96,  /* ` */

    // Function keys
    ESCAPE = 256,
    ENTER = 257,
    TAB = 258,
    BACKSPACE = 259,
    INSERT = 260,
    DELETE = 261,
    RIGHT = 262,
    LEFT = 263,
    DOWN = 264,
    UP = 265,
    PAGE_UP = 266,
    PAGE_DOWN = 267,
    HOME = 268,
    END = 269,
    CAPS_LOCK = 280,
    SCROLL_LOCK = 281,
    NUM_LOCK = 282,
    PRINT_SCREEN = 283,
    PAUSE = 284,

    // F keys
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,

    // Keypad
    KP_ZERO = 320,
    KP_ONE = 321,
    KP_TWO = 322,
    KP_THREE = 323,
    KP_FOUR = 324,
    KP_FIVE = 325,
    KP_SIX = 326,
    KP_SEVEN = 327,
    KP_EIGHT = 328,
    KP_NINE = 329,
    KP_DECIMAL = 330,
    KP_DIVIDE = 331,
    KP_MULTIPLY = 332,
    KP_SUBTRACT = 333,
    KP_ADD = 334,
    KP_ENTER = 335,
    KP_EQUAL = 336,

    // Modifier keys
    LEFT_SHIFT = 340,
    LEFT_CONTROL = 341,
    LEFT_ALT = 342,
    LEFT_SUPER = 343,
    RIGHT_SHIFT = 344,
    RIGHT_CONTROL = 345,
    RIGHT_ALT = 346,
    RIGHT_SUPER = 347,
    MENU = 348,
};

/**
 * @brief Enumeration for GUI modifier keys.
 */
enum class GuiModKey {
    SHIFT = 1 << 0,
    CONTROL = 1 << 1,
    ALT = 1 << 2,
    SUPER = 1 << 3,
    CAPS_LOCK = 1 << 4,
    NUM_LOCK = 1 << 5,
};

/**
 * @brief Enumeration for GUI mouse buttons.
 */
enum class GuiMouseButton {
    LEFT = 0,
    RIGHT = 1,
    MIDDLE = 2,
    BUTTON_4 = 3,
    BUTTON_5 = 4,
    BUTTON_6 = 5,
    BUTTON_7 = 6,
    BUTTON_8 = 7,
};

/**
 * @brief A template class for bitwise.
 */
template<typename Enum>
class GuiFlags {
public:
    using UnderlyingType = std::underlying_type_t<Enum>;

    GuiFlags() : m_value(0) {};
    GuiFlags(Enum bit) : m_value(static_cast<UnderlyingType>(bit)) {};

    bool operator==(const GuiFlags& other) const {
        return m_value == other.m_value;
    };
    bool operator==(Enum bit) const {
        return m_value == static_cast<UnderlyingType>(bit);
    };
    bool operator!=(const GuiFlags& other) const {
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
