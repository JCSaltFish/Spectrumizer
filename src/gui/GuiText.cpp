/**
 * @file GuiText.cpp
 * @brief Implementation of the GUI text resource management.
 */

#include "gui/GuiPub.h"

#include <nlohmann/json.hpp>

/**
 * @brief Implementation details for the GuiText class.
 */
class GuiText::Impl {
public:
    ~Impl() = default;

    void load(const std::string& lang) {
        m_cache.clear();
        m_strings = nlohmann::json::parse(lang, nullptr, false);
    };
    std::string get(const std::string& key) {
        // Check if the key is already cached
        if (auto it = m_cache.find(key); it != m_cache.end())
            return it->second;
        // If not cached, retrieve from JSON and cache it
        try {
            auto pJson = nlohmann::json::json_pointer("/" + replace(key, ".", "/"));
            std::string value = m_strings.at(pJson).get<std::string>();
            m_cache[key] = value;
            return value;
        } catch (...) {
            m_cache[key] = "";
        }
        return "";
    };

private:
    std::string replace(std::string str, const std::string& from, const std::string& to) const {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        return str;
    };

private:
    nlohmann::json m_strings; // JSON object to hold language strings
    std::unordered_map<std::string, std::string> m_cache; // Cache for string resources
};

std::unique_ptr<GuiText::Impl> GuiText::s_impl = std::make_unique<GuiText::Impl>();

GuiText::~GuiText() = default; // Destructor must be defined after the Impl definition

void GuiText::load(const std::string& lang) {
    s_impl->load(lang);
}

std::string GuiText::get(const std::string& key) {
    return s_impl->get(key);
}

std::string GuiText::formatString(
    const std::string& format,
    const std::vector<std::string>& args
) {
    std::string result = format;
    for (int i = args.size() - 1; i >= 0; --i) {
        std::string placeholder = "{" + std::to_string(i) + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), args[i]);
            pos += args[i].length();
        }
    }
    return result;
}
