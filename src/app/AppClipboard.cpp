/**
 * @file AppClipboard.cpp
 * @brief Implementation of the AppClipboard class for managing clipboard operations.
 */

#include "app/AppClipboard.h"

void AppClipboard::copy(const std::vector<DbObjHandle>& hObjs) {
    m_data.clear();
    m_db = std::make_shared<DB>();
    DbUtils::TxnGuard txnGuard(m_db);
    DbObjHandle hScene = m_db->objCreate(PtScene{});
    m_db->setRootObject(hScene);
    for (const auto hObj : hObjs) {
        DbObjHandle hModel = PtModel::copy(hObj, m_db);
        if (!hModel.isValid())
            continue;
        if (PtScene::addModel(hScene, hModel) != DB::Result::SUCCESS)
            continue;
        m_data.push_back(hModel);
    }
    txnGuard.commit();
}

void AppClipboard::cut(const std::vector<DbObjHandle>& hObjs) {
    copy(hObjs);
    auto db = AppDataManager::instance().getDB();
    DbObjHandle hScene = db->getRootObject();
    DbUtils::TxnGuard txnGuard(db);
    for (const auto hObj : hObjs) {
        if (PtScene::delModel(hScene, hObj) != DB::Result::SUCCESS)
            continue;
    }
    txnGuard.commit();
}

std::vector<DbObjHandle> AppClipboard::paste() const {
    if (!hasData())
        return {};
    auto db = AppDataManager::instance().getDB();
    DbObjHandle hScene = db->getRootObject();
    DbUtils::TxnGuard txnGuard(db);
    std::vector<DbObjHandle> newObjHandles;
    for (const auto hObj : m_data) {
        DbObjHandle hModel = PtModel::copy(hObj, db);
        if (!hModel.isValid())
            continue;
        if (PtScene::addModel(hScene, hModel) != DB::Result::SUCCESS)
            continue;
        newObjHandles.push_back(hModel);
    }
    txnGuard.commit();
    return newObjHandles;
}

bool AppClipboard::hasData() const {
    return !m_data.empty();
}
