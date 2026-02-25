/**
 * @file AppConfig.cpp
 * @brief Implementation of the AppConfig singleton.
 */

#include "app/AppConfig.h"

#include <fstream>
#include <mutex>
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <nlohmann/json.hpp>

/**
 * @brief Implementation details for the AppConfig class.
 */
class AppConfig::Impl {
public:
    /**
     * @brief Initialize the configuration with application name and config filename.
     * @param appName Name of the application.
     * @param configFilename Name of the configuration file (without extension).
     */
    void init(const std::string& appName, const std::string& configFilename) {
        std::filesystem::path configDir = getAppConfigPath(appName);
        m_configPath = (configDir / (configFilename + ".json")).string();

        // Load existing configuration if the file exists
        std::ifstream configFile(m_configPath);
        if (configFile.is_open()) {
            try {
                configFile >> m_configData;
            } catch (const nlohmann::json::parse_error&) {
                m_configData = nlohmann::json::object(); // Reset to empty object on error
            }
            configFile.close();
        } else {
            // If the file doesn't exist, start with an empty configuration
            m_configData = nlohmann::json::object();
        }
    }
    /**
     * @brief Get a configuration value by key.
     * @param key The configuration key.
     * @return The configuration value as a string, or an empty string if the key does not exist.
     */
    std::string getConfig(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_configData.contains(key))
            return m_configData[key].get<std::string>();
        return ""; // Return empty string if key not found
    }
    /**
     * @brief Set a configuration value by key.
     * @param key The configuration key.
     * @param value The configuration value to set.
     * @return 0 on success, non-zero on failure.
     */
    int setConfig(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_configData[key] = value;
        return saveConfigFile();
    }

private:
    /**
     * @brief Get the application configuration path based on the OS.
     * @param appName Name of the application.
     * @return The path to the application configuration directory.
     */
    std::filesystem::path getAppConfigPath(const std::string& appName) const {
#ifdef _WIN32
        PWSTR pathPtr = nullptr;
        REFKNOWNFOLDERID folderId = FOLDERID_RoamingAppData;

        // Get the path to the roaming app data folder
        if (SUCCEEDED(SHGetKnownFolderPath(folderId, KF_FLAG_CREATE, nullptr, &pathPtr))) {
            // Convert the wide string (UTF-16) to a UTF-8 string
            int size = WideCharToMultiByte(CP_UTF8, 0, pathPtr, -1, nullptr, 0, nullptr, nullptr);
            std::string utf8Path(size - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, pathPtr, -1, &utf8Path[0], size, nullptr, nullptr);

            // Free the memory allocated for the path
            CoTaskMemFree(pathPtr);

            // Create the full path for the application configuration
            std::filesystem::path fullPath = std::filesystem::path(utf8Path) / appName;
            std::filesystem::create_directories(fullPath);
            return fullPath;
        }
#else
        const char* envPath = std::getenv("XDG_CONFIG_HOME");
        std::filesystem::path basePath;

        if (envPath)
            basePath = envPath;
        else {
            // Fallback to ~/.config if XDG_CONFIG_HOME is not set
            const char* homeDir = std::getenv("HOME");
            if (!homeDir) {
                // Fallback to current directory if HOME is not set
                return std::filesystem::current_path();
            }
            basePath = homeDir;
            basePath /= ".config";
        }

        std::filesystem::path fullPath = basePath / appName;
        std::filesystem::create_directories(fullPath);
        return fullPath;
#endif

        // Fallback to current directory if all else fails
        return std::filesystem::current_path();
    }

    /**
     * @brief Save the current configuration to the file.
     * @return 0 on success, non-zero on failure.
     */
    int saveConfigFile() const {
        std::ofstream configFile(m_configPath);
        if (!configFile.is_open())
            return 1; // Error opening file
        configFile << m_configData.dump(4); // Pretty print with 4 spaces
        configFile.close();
        return 0; // Success
    }

private:
    std::string m_configPath; // Path to the configuration file
    nlohmann::json m_configData; // JSON object to hold configuration data
    mutable std::mutex m_mutex; // Mutex for thread-safe access
};

AppConfig::AppConfig() :
    m_impl(std::make_unique<Impl>()) {}

AppConfig::~AppConfig() = default;

void AppConfig::init(const std::string& appName, const std::string& configFilename) {
    if (m_impl)
        m_impl->init(appName, configFilename);
}

std::string AppConfig::getConfig(const std::string& key) const {
    if (m_impl)
        return m_impl->getConfig(key);
    return "";
}

int AppConfig::setConfig(const std::string& key, const std::string& value) {
    if (m_impl)
        return m_impl->setConfig(key, value);
    return 1;
}

std::string AppConfigUitls::Vec3ToString(const Math::Vec3& vec) {
    return std::to_string(vec.x) + "," + std::to_string(vec.y) + "," + std::to_string(vec.z);
}

Math::Vec3 AppConfigUitls::StringToVec3(const std::string& str) {
    Math::Vec3 vec = {};
    size_t pos1 = str.find(',');
    if (pos1 == std::string::npos)
        return vec;
    size_t pos2 = str.find(',', pos1 + 1);
    if (pos2 == std::string::npos)
        return vec;
    try {
        vec.x = std::stof(str.substr(0, pos1));
        vec.y = std::stof(str.substr(pos1 + 1, pos2 - pos1 - 1));
        vec.z = std::stof(str.substr(pos2 + 1));
    } catch (const std::exception&) {
        vec = Math::Vec3();
    }
    return vec;
}
