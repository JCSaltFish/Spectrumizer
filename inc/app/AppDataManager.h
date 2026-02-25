/**
 * @file AppDataManager.h
 * @brief Declaration of the AppDataManager class for managing application data types.
 */

#pragma once

#include "db/DbPub.h"

/* All data types */
#include "app/data/PtScene.h"
#include "app/data/PtModel.h"
#include "app/data/PtMesh.h"
#include "app/data/PtMaterial.h"
#include "app/data/SpWave.h"
#include "app/data/SpMaterial.h"

/**
 * @brief Manages application data types and their registration.
 */
class AppDataManager {
private:
    AppDataManager();
    ~AppDataManager() = default;
    AppDataManager(const AppDataManager&) = delete;
    AppDataManager& operator=(const AppDataManager&) = delete;
    AppDataManager(AppDataManager&&) = delete;
    AppDataManager& operator=(AppDataManager&&) = delete;

public:
    static AppDataManager& instance() {
        static AppDataManager instance;
        return instance;
    }

    /**
     * @brief Register types and creat a new database instance.
     */
    void init();

    /**
     * @brief Get the database instance.
     * @return Shared pointer to the database.
     */
    std::shared_ptr<DB> getDB();
    /**
     * @brief Reset the database to a new instance.
     * @note Used for application's "New" action.
     *       Make sure to prompt user to save current work before calling this.
     */
    void resetDB();

    /**
     * @brief Get the current database file path.
     * @return The current database file path as a string.
     */
    std::string getCurrentDbPath() const;
    /**
     * @brief Load a database from a file.
     * @param filepath The file path to load the database from.
     * @return 0 on success, non-zero on failure.
     */
    int loadDbFromFile(const std::string& filepath);
    /**
     * @brief Save the current database to a file.
     * @param filepath The file path to save the database to.
     * @return 0 on success, non-zero on failure.
     */
    int saveDbToFile(const std::string& filepath);

private:
    /**
     * @brief Register all application-specific data types.
     */
    void registerTypes() const;

private:
    std::shared_ptr<DB> m_db = nullptr; // Database instance
    std::string m_currentDbPath = ""; // Current database file path
};
