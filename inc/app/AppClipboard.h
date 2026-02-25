/**
 * @file AppClipboard.h
 * @brief Declaration of the AppClipboard class for managing clipboard operations.
 */

#pragma once

#include "AppDataManager.h"

/**
 * @brief Singleton class to manage clipboard operations for application objects.
 */
class AppClipboard
{
private:
    AppClipboard() = default;
    ~AppClipboard() = default;
    AppClipboard(const AppClipboard&) = delete;
    AppClipboard& operator=(const AppClipboard&) = delete;
    AppClipboard(AppClipboard&&) = delete;
    AppClipboard& operator=(AppClipboard&&) = delete;

public:
    static AppClipboard& instance()
    {
        static AppClipboard instance;
        return instance;
    };

    /**
     * @brief Copy the specified objects to the clipboard.
     * @param hObjs Vector of object handles to copy.
     */
    void copy(const std::vector<DbObjHandle>& hObjs);
    /**
     * @brief Cut the specified objects from the current scene and store them in the clipboard.
     * @param hObjs Vector of object handles to cut.
     */
    void cut(const std::vector<DbObjHandle>& hObjs);
    /**
     * @brief Paste the data from the clipboard into the current scene.
     * @return Vector of new object handles created from the pasted data.
     */
    std::vector<DbObjHandle> paste() const;
    /**
     * @brief Check if there is data in the clipboard.
     * @return True if there is data, false otherwise.
     */
    bool hasData() const;

private:
    std::shared_ptr<DB> m_db = nullptr; // Internal database for clipboard data
    std::vector<DbObjHandle> m_data = {}; // Stored clipboard data
};
