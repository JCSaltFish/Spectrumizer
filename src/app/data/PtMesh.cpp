/**
 * @file PtMesh.cpp
 * @brief Implementation of the PtMesh class representing a mesh within a 3D model.
 */

#include "app/AppDataManager.h"

void PtMesh::serialize(DbSerializer& serializer, const PtMesh& mesh) {
    serializer.serialize(mesh.m_modelId);
    serializer.serialize(mesh.m_name);
    serializer.serialize(mesh.m_materialId);
}

void PtMesh::deserialize(DbSerializer& serializer, PtMesh& mesh) {
    serializer.deserialize(mesh.m_modelId);
    serializer.deserialize(mesh.m_name);
    serializer.deserialize(mesh.m_materialId);
}

void PtMesh::migrate(int oldVersion, PtMesh& mesh) {}

const PtMesh* PtMesh::view(const DbObjHandle& hMesh) {
    if (!hMesh.isValid() || hMesh.getType() != PtMesh::TYPE_NAME)
        return nullptr;
    return hMesh.getDB()->objGet<PtMesh>(hMesh);
}

DbObjHandle PtMesh::getModel(const DbObjHandle& hMesh) {
    const PtMesh* mesh = view(hMesh);
    if (!mesh || mesh->m_modelId == 0)
        return DbObjHandle();
    DbObjHandle hModel(hMesh.getDB(), mesh->m_modelId);
    if (hModel.isValid() && hModel.getType() == PtModel::TYPE_NAME)
        return hModel;
    return DbObjHandle();
}

DB::Result PtMesh::setModel(const DbObjHandle& hMesh, const DbObjHandle& hModel) {
    const PtMesh* mesh = view(hMesh);
    if (!mesh)
        return DB::Result::INVALID_HANDLE;
    if (hModel.isValid() && hModel.getType() != PtModel::TYPE_NAME)
        return DB::Result::INVALID_HANDLE;
    DB::ID newModelId = hModel.isValid() ? hModel.getID() : 0;
    if (mesh->m_modelId == newModelId)
        return DB::Result::SUCCESS;
    PtMesh newMesh = *mesh;
    newMesh.m_modelId = newModelId;
    return hMesh.getDB()->objModify(hMesh, newMesh);
}

std::string PtMesh::getName(const DbObjHandle& hMesh) {
    const PtMesh* mesh = view(hMesh);
    if (!mesh)
        return std::string();
    return mesh->m_name;
}

DB::Result PtMesh::setName(const DbObjHandle& hMesh, const std::string& name) {
    const PtMesh* mesh = view(hMesh);
    if (!mesh)
        return DB::Result::INVALID_HANDLE;
    if (mesh->m_name == name)
        return DB::Result::SUCCESS;
    PtMesh newMesh = *mesh;
    newMesh.m_name = name;
    return hMesh.getDB()->objModify(hMesh, newMesh);
}

DbObjHandle PtMesh::getMaterial(const DbObjHandle& hMesh) {
    const PtMesh* mesh = view(hMesh);
    if (!mesh || mesh->m_materialId == 0)
        return DbObjHandle();
    DbObjHandle hMaterial(hMesh.getDB(), mesh->m_materialId);
    if (hMaterial.isValid() && hMaterial.getType() == PtMaterial::TYPE_NAME)
        return hMaterial;
    return DbObjHandle();
}

DB::Result PtMesh::setMaterial(const DbObjHandle& hMesh, const DbObjHandle& hMaterial) {
    const PtMesh* mesh = view(hMesh);
    if (!mesh)
        return DB::Result::INVALID_HANDLE;
    if (hMaterial.isValid() && hMaterial.getType() != PtMaterial::TYPE_NAME)
        return DB::Result::INVALID_HANDLE;

    DB::ID newMaterialId = hMaterial.isValid() ? hMaterial.getID() : 0;
    if (mesh->m_materialId == newMaterialId)
        return DB::Result::SUCCESS;

    // Remove the current material if it exists
    DbObjHandle currentMaterial = getMaterial(hMesh);
    if (currentMaterial.isValid() && currentMaterial.getType() == PtMaterial::TYPE_NAME) {
        DB::Result res = hMesh.getDB()->objDelete<PtMaterial>(currentMaterial);
        if (res != DB::Result::SUCCESS)
            return res;
    }

    PtMesh newMesh = *mesh;
    newMesh.m_materialId = newMaterialId;
    return hMesh.getDB()->objModify(hMesh, newMesh);
}

DbObjHandle PtMesh::copy(const DbObjHandle& hMesh, std::shared_ptr<DB>& dst) {
    const PtMesh* mesh = view(hMesh);
    if (!mesh)
        return {};
    PtMesh newMesh = *mesh;
    newMesh.m_materialId = 0;
    DbObjHandle hMaterial(hMesh.getDB(), mesh->m_materialId);
    DbObjHandle hNewMaterial;
    if (hMaterial.isValid() && hMaterial.getType() == PtMaterial::TYPE_NAME) {
        hNewMaterial = PtMaterial::copy(hMaterial, dst);
        if (!hNewMaterial.isValid())
            return {};
        newMesh.m_materialId = hNewMaterial.getID();
    }
    return dst->objCreate<PtMesh>(newMesh);
}
