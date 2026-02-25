/**
 * @file PtMaterial.cpp
 * @brief Implementation of the PtMaterial class representing a material with various properties.
 */

#include "app/AppDataManager.h"

void PtMaterial::serialize(DbSerializer& serializer, const PtMaterial& material) {
    serializer.serialize(material.m_meshId);

    serializer.serialize(material.m_type);

    serializer.serialize(material.m_roughness);
    serializer.serialize(material.m_ior);
    serializer.serialize(material.m_temperature);

    serializer.serialize(material.m_flags);

    serializer.serialize(material.m_normalTexPath);
    serializer.serialize(material.m_roughnessTexPath);
    serializer.serialize(material.m_temperatureTexPath);

    serializer.serialize(material.m_spMaterialId);
}

void PtMaterial::deserialize(DbSerializer& serializer, PtMaterial& material) {
    serializer.deserialize(material.m_meshId);

    serializer.deserialize(material.m_type);

    serializer.deserialize(material.m_roughness);
    serializer.deserialize(material.m_ior);
    serializer.deserialize(material.m_temperature);

    serializer.deserialize(material.m_flags);

    serializer.deserialize(material.m_normalTexPath);
    serializer.deserialize(material.m_roughnessTexPath);
    serializer.deserialize(material.m_temperatureTexPath);

    serializer.deserialize(material.m_spMaterialId);
}

void PtMaterial::migrate(int oldVersion, PtMaterial& material) {}

const PtMaterial* PtMaterial::view(const DbObjHandle& hMaterial) {
    if (!hMaterial.isValid() || hMaterial.getType() != PtMaterial::TYPE_NAME)
        return nullptr;
    return hMaterial.getDB()->objGet<PtMaterial>(hMaterial);
}

DbObjHandle PtMaterial::getMesh(const DbObjHandle& hMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material || material->m_meshId == 0)
        return DbObjHandle();
    DbObjHandle hMesh(hMaterial.getDB(), material->m_meshId);
    if (hMesh.isValid() && hMesh.getType() == PtMesh::TYPE_NAME)
        return hMesh;
    return DbObjHandle();
}

DB::Result PtMaterial::setMesh(const DbObjHandle& hMaterial, const DbObjHandle& hMesh) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    DB::ID newMeshId = 0;
    if (hMesh.isValid()) {
        if (hMesh.getType() != PtMesh::TYPE_NAME)
            return DB::Result::INVALID_HANDLE;
        newMeshId = hMesh.getID();
    }
    if (material->m_meshId == newMeshId)
        return DB::Result::SUCCESS;
    PtMaterial newMaterial = *material;
    newMaterial.m_meshId = newMeshId;
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

PtMaterial::MaterialType PtMaterial::getType(const DbObjHandle& hMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return MaterialType::DIFFUSE;
    return static_cast<MaterialType>(material->m_type);
}

DB::Result PtMaterial::setType(const DbObjHandle& hMaterial, MaterialType type) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    if (material->m_type == static_cast<int>(type))
        return DB::Result::SUCCESS;
    PtMaterial newMaterial = *material;
    newMaterial.m_type = static_cast<int>(type);
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

float PtMaterial::getRoughness(const DbObjHandle& hMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return 0.0f;
    return material->m_roughness;
}

DB::Result PtMaterial::setRoughness(const DbObjHandle& hMaterial, float roughness) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    if (material->m_roughness == roughness)
        return DB::Result::SUCCESS;
    PtMaterial newMaterial = *material;
    newMaterial.m_roughness = roughness;
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

float PtMaterial::getIOR(const DbObjHandle& hMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return 0.0f;
    return material->m_ior;
}

DB::Result PtMaterial::setIOR(const DbObjHandle& hMaterial, float ior) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    if (material->m_ior == ior)
        return DB::Result::SUCCESS;
    PtMaterial newMaterial = *material;
    newMaterial.m_ior = ior;
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

float PtMaterial::getTemperature(const DbObjHandle& hMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return 0.0f;
    return material->m_temperature;
}

DB::Result PtMaterial::setTemperature(const DbObjHandle& hMaterial, float temperature) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    if (material->m_temperature == temperature)
        return DB::Result::SUCCESS;
    PtMaterial newMaterial = *material;
    newMaterial.m_temperature = temperature;
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

Flags<PtMaterial::MaterialFlag> PtMaterial::getFlags(const DbObjHandle& hMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return 0;
    return material->m_flags;
}

DB::Result PtMaterial::setFlags
(
    const DbObjHandle& hMaterial,
    Flags<PtMaterial::MaterialFlag> flags
) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    if (flags == material->m_flags)
        return DB::Result::SUCCESS;
    PtMaterial newMaterial = *material;
    newMaterial.m_flags = flags.getValue();
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

std::string PtMaterial::getNormalTexPath(const DbObjHandle& hMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return std::string();
    return material->m_normalTexPath.path;
}

DB::Result PtMaterial::setNormalTexPath(const DbObjHandle& hMaterial, const std::string& path) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    if (material->m_normalTexPath.path == path)
        return DB::Result::SUCCESS;
    PtMaterial newMaterial = *material;
    newMaterial.m_normalTexPath.path = path;
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

std::string PtMaterial::getRoughnessTexPath(const DbObjHandle& hMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return std::string();
    return material->m_roughnessTexPath.path;
}

DB::Result PtMaterial::setRoughnessTexPath(const DbObjHandle& hMaterial, const std::string& path) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    if (material->m_roughnessTexPath.path == path)
        return DB::Result::SUCCESS;
    PtMaterial newMaterial = *material;
    newMaterial.m_roughnessTexPath.path = path;
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

std::string PtMaterial::getTemperatureTexPath(const DbObjHandle& hMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return std::string();
    return material->m_temperatureTexPath.path;
}

DB::Result PtMaterial::setTemperatureTexPath(const DbObjHandle& hMaterial, const std::string& path) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    if (material->m_temperatureTexPath.path == path)
        return DB::Result::SUCCESS;
    PtMaterial newMaterial = *material;
    newMaterial.m_temperatureTexPath.path = path;
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

DbObjHandle PtMaterial::getSpectrumMaterial(const DbObjHandle &hMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material || material->m_meshId == 0)
        return DbObjHandle();
    DbObjHandle hSpMaterial(hMaterial.getDB(), material->m_spMaterialId);
    if (hSpMaterial.isValid() && hSpMaterial.getType() == SpMaterial::TYPE_NAME)
        return hSpMaterial;
    return DbObjHandle();
}

DB::Result PtMaterial::setSpectrumMaterial(const DbObjHandle &hMaterial, const DbObjHandle &hSpMaterial) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return DB::Result::INVALID_HANDLE;
    DB::ID newSpMaterialId = 0;
    if (hSpMaterial.isValid()) {
        if (hSpMaterial.getType() != SpMaterial::TYPE_NAME)
            return DB::Result::INVALID_HANDLE;
        newSpMaterialId = hSpMaterial.getID();
    }
    if (material->m_spMaterialId == newSpMaterialId)
        return DB::Result::SUCCESS;
    PtMaterial newMaterial = *material;
    newMaterial.m_spMaterialId = newSpMaterialId;
    return hMaterial.getDB()->objModify(hMaterial, newMaterial);
}

DbObjHandle PtMaterial::copy(const DbObjHandle& hMaterial, std::shared_ptr<DB>& dst) {
    const PtMaterial* material = view(hMaterial);
    if (!material)
        return {};
    PtMaterial newMaterial = *material;
    return dst->objCreate<PtMaterial>(newMaterial);
}
