/**
 * @file AppConfig.h
 * @brief Header file for application configuration singleton.
 */

#pragma once

#include <string>
#include <memory>

#include "utils/Math.h"

/**
 * @brief Singleton class for application configuration.
 */
class AppConfig
{
private:
    AppConfig();
    ~AppConfig();
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;
    AppConfig(AppConfig&&) = delete;
    AppConfig& operator=(AppConfig&&) = delete;

public:
    static AppConfig& instance()
    {
        static AppConfig instance;
        return instance;
    };

    /**
     * @brief Initialize the configuration with application name and config filename.
     * @param appName Name of the application.
     * @param configFilename Name of the configuration file (without extension).
     */
    void init(const std::string& appName, const std::string& configFilename);
    /**
     * @brief Get a configuration value by key.
     * @param key The configuration key.
     * @return The configuration value as a string, or an empty string if the key does not exist.
     */
    std::string getConfig(const std::string& key) const;
    /**
     * @brief Set a configuration value by key.
     * @param key The configuration key.
     * @param value The configuration value to set.
     * @return 0 on success, non-zero on failure.
     */
    int setConfig(const std::string& key, const std::string& value);

private:
    class Impl; // Forward declaration of implementation details
    std::unique_ptr<Impl> m_impl; // Pointer to implementation details
};

namespace AppConfigUitls
{

/**
 * @brief Converts a Vec3 to a comma-separated string.
 * @param vec The Vec3 to convert.
 * @return The comma-separated string representation of the Vec3.
 */
std::string Vec3ToString(const Math::Vec3& vec);
/**
 * @brief Converts a comma-separated string to a Vec3.
 * @param str The comma-separated string to convert.
 * @return The Vec3 representation of the string. Returns a zero vector if conversion fails.
 */
Math::Vec3 StringToVec3(const std::string& str);

} // namespace AppConfigUitls
