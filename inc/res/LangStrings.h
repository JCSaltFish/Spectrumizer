/**
 * @file LangStrings.h
 * @brief Declaration of language string resource retrieval functions.
 */

#pragma once

#include <string>

namespace LangStrings {

/**
 * @brief Enumeration of supported languages.
 */
enum class Lang : int {
    EN_US = 1,
    ZH_CN = 2,
};

/**
 * @brief Retrieve the language string resource for the specified language.
 * @param lang The language identifier.
 * @return The language string resource as a std::string.
 */
std::string get(Lang lang);

}
