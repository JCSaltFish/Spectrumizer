/**
 * @file SpMaterial.cpp
 * @brief Implementation of the SpMaterial class representing a material for spectral rendering.
 */

#include "app/AppDataManager.h"

void SpMaterial::serialize(DbSerializer &serializer, const SpMaterial &material) {
    serializer.serialize(material.m_name);
    serializer.serialize(material.m_emissivities);
}

void SpMaterial::deserialize(DbSerializer &serializer, SpMaterial &material) {
    serializer.deserialize(material.m_name);
    serializer.deserialize(material.m_emissivities);
}

void SpMaterial::migrate(int oldVersion, SpMaterial &material) {}

const SpMaterial *SpMaterial::view(const DbObjHandle &hMaterial) {
    if (!hMaterial.isValid() || hMaterial.getType() != SpMaterial::TYPE_NAME)
        return nullptr;
    return hMaterial.getDB()->objGet<SpMaterial>(hMaterial);
}

std::string SpMaterial::getName(const DbObjHandle &hMaterial) {
    const SpMaterial* material = view(hMaterial);
    if (!material)
        return std::string();
    return material->m_name;
}

DB::Result SpMaterial::setName(const DbObjHandle &hMaterial, const std::string &name) {
    const SpMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    if (material->m_name == name)
        return DB::Result::SUCCESS;
    SpMaterial newMaterial = *material;
    newMaterial.m_name = name;
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

std::vector<float> SpMaterial::getEmissivities(const DbObjHandle &hMaterial) {
    const SpMaterial* material = view(hMaterial);
    if (!material)
        return {};
    return material->m_emissivities;
}

DB::Result SpMaterial::setEmissivities(
    const DbObjHandle &hMaterial,
    const std::vector<float>& emissivities
) {
    const SpMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    SpMaterial newMaterial = *material;
    newMaterial.m_emissivities = emissivities;
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}
