/**
 * @file AppDataManager.cpp
 * @brief Implementation of the AppDataManager class for managing application data types.
 */

#include "app/AppDataManager.h"

#include "app/Application.h"

AppDataManager::AppDataManager() {}

void AppDataManager::init() {
    registerTypes();
    resetDB();
}

std::shared_ptr<DB> AppDataManager::getDB() {
    return m_db;
}

void AppDataManager::resetDB() {
    const std::vector<uint8_t> fileMagic = { 'S', 'P', 'S' };
    const int fileVersion = AppVersion::MAJOR * 100 + AppVersion::MINOR * 10;
    m_db = std::make_shared<DB>(fileMagic, fileVersion);
    DbObjHandle rootObj = m_db->objCreate(PtScene{});
    m_db->setRootObject(rootObj);
    m_currentDbPath = "";
    
    DbObjHandle hSkyMaterial = m_db->objCreate(SpMaterial{});
    PtScene::setSkyMaterial(rootObj, hSkyMaterial);
}

std::string AppDataManager::getCurrentDbPath() const {
    return m_currentDbPath;
}

int AppDataManager::loadDbFromFile(const std::string& filepath) {
    resetDB();
    if (m_db->loadFromFile(filepath) != DB::Result::SUCCESS)
        return 1;
    m_currentDbPath = filepath;
    return 0;
}

int AppDataManager::saveDbToFile(const std::string& filepath) {
    if (m_db->saveToFile(filepath) != DB::Result::SUCCESS)
        return 1;
    m_currentDbPath = filepath;
    return 0;
}

void AppDataManager::registerTypes() const {
    DbTypeRegistry::instance().registerType<PtScene>();
    DbTypeRegistry::instance().registerType<PtModel>();
    DbTypeRegistry::instance().registerType<PtMesh>();
    DbTypeRegistry::instance().registerType<PtMaterial>();
    DbTypeRegistry::instance().registerType<SpWave>();
    DbTypeRegistry::instance().registerType<SpMaterial>();
}
