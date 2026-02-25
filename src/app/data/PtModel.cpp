/**
 * @file PtModel.cpp
 * @brief Implementation of the PtModel class representing a 3D model.
 */

#include "app/AppDataManager.h"

void PtModel::serialize(DbSerializer& serializer, const PtModel& model) {
    serializer.serialize(model.m_name);
    serializer.serialize(model.m_filePath);
    serializer.serialize(model.m_meshes);

    serializer.serialize(model.m_location.x);
    serializer.serialize(model.m_location.y);
    serializer.serialize(model.m_location.z);

    serializer.serialize(model.m_rotation.x);
    serializer.serialize(model.m_rotation.y);
    serializer.serialize(model.m_rotation.z);

    serializer.serialize(model.m_scale.x);
    serializer.serialize(model.m_scale.y);
    serializer.serialize(model.m_scale.z);
}

void PtModel::deserialize(DbSerializer& serializer, PtModel& model) {
    serializer.deserialize(model.m_name);
    serializer.deserialize(model.m_filePath);
    serializer.deserialize(model.m_meshes);

    serializer.deserialize(model.m_location.x);
    serializer.deserialize(model.m_location.y);
    serializer.deserialize(model.m_location.z);

    serializer.deserialize(model.m_rotation.x);
    serializer.deserialize(model.m_rotation.y);
    serializer.deserialize(model.m_rotation.z);

    serializer.deserialize(model.m_scale.x);
    serializer.deserialize(model.m_scale.y);
    serializer.deserialize(model.m_scale.z);
}

void PtModel::migrate(int oldVersion, PtModel& model) {}

const PtModel* PtModel::view(const DbObjHandle& hModel) {
    if (!hModel.isValid() || hModel.getType() != PtModel::TYPE_NAME)
        return nullptr;
    return hModel.getDB()->objGet<PtModel>(hModel);
}

std::string PtModel::getName(const DbObjHandle& hModel) {
    const PtModel* model = view(hModel);
    if (!model)
        return std::string();
    return model->m_name;
}

DB::Result PtModel::setName(const DbObjHandle& hModel, const std::string& name) {
    const PtModel* model = view(hModel);
    if (!model)
        return DB::Result::INVALID_HANDLE;
    if (model->m_name == name)
        return DB::Result::SUCCESS;
    PtModel newModel = *model;
    newModel.m_name = name;
    return hModel.getDB()->objModify(hModel, newModel);
}

std::string PtModel::getFilePath(const DbObjHandle& hModel) {
    const PtModel* model = view(hModel);
    if (!model)
        return std::string();
    return model->m_filePath.path;
}

DB::Result PtModel::setFilePath(const DbObjHandle& hModel, const std::string& path) {
    const PtModel* model = view(hModel);
    if (!model)
        return DB::Result::INVALID_HANDLE;
    if (model->m_filePath.path == path)
        return DB::Result::SUCCESS;
    PtModel newModel = *model;
    newModel.m_filePath.path = path;
    return hModel.getDB()->objModify(hModel, newModel);
}

std::vector<DbObjHandle> PtModel::getMeshes(const DbObjHandle& hModel) {
    std::vector<DbObjHandle> result;
    const PtModel* model = view(hModel);
    if (!model)
        return result;
    for (DB::ID meshId : model->m_meshes) {
        DbObjHandle hMesh(hModel.getDB(), meshId);
        if (hMesh.isValid() && hMesh.getType() == PtMesh::TYPE_NAME)
            result.push_back(hMesh);
    }
    return result;
}

DB::Result PtModel::setMeshes(
    const DbObjHandle& hModel,
    const std::vector<DbObjHandle>& hMeshes
) {
    const PtModel* model = view(hModel);
    if (!model)
        return DB::Result::INVALID_HANDLE;

    std::vector<DB::ID> newMeshIds;
    newMeshIds.reserve(hMeshes.size());
    for (const DbObjHandle& hMesh : hMeshes) {
        if (!hMesh.isValid() || hMesh.getType() != PtMesh::TYPE_NAME)
            return DB::Result::INVALID_HANDLE;
        newMeshIds.push_back(hMesh.getID());
    }
    if (model->m_meshes == newMeshIds)
        return DB::Result::SUCCESS;

    // Delete old meshes and their materials
    std::vector<DB::ID> currentMeshIds = model->m_meshes;
    if (!currentMeshIds.empty()) {
        for (const auto& meshId : currentMeshIds) {
            DbObjHandle hMesh(hModel.getDB(), meshId);
            if (hMesh.isValid() && hMesh.getType() == PtMesh::TYPE_NAME) {
                DbObjHandle hMaterial = PtMesh::getMaterial(hMesh);
                if (hMaterial.isValid() && hMaterial.getType() == PtMaterial::TYPE_NAME) {
                    DB::Result res = hModel.getDB()->objDelete<PtMaterial>(hMaterial);
                    if (res != DB::Result::SUCCESS)
                        return res;
                }
                DB::Result res = hModel.getDB()->objDelete<PtMesh>(hMesh);
                if (res != DB::Result::SUCCESS)
                    return res;
            }
        }
    }

    PtModel newModel = *model;
    newModel.m_meshes = std::move(newMeshIds);
    return hModel.getDB()->objModify(hModel, newModel);
}

Math::Vec3 PtModel::getLocation(const DbObjHandle& hModel) {
    const PtModel* model = view(hModel);
    if (!model)
        return Math::Vec3();
    return model->m_location;
}

DB::Result PtModel::setLocation(const DbObjHandle& hModel, const Math::Vec3& location) {
    const PtModel* model = view(hModel);
    if (!model)
        return DB::Result::INVALID_HANDLE;
    if (model->m_location == location)
        return DB::Result::SUCCESS;
    PtModel newModel = *model;
    newModel.m_location = location;
    return hModel.getDB()->objModify(hModel, newModel);
}

Math::Vec3 PtModel::getRotation(const DbObjHandle& hModel) {
    const PtModel* model = view(hModel);
    if (!model)
        return Math::Vec3();
    return model->m_rotation;
}

DB::Result PtModel::setRotation(const DbObjHandle& hModel, const Math::Vec3& rotation) {
    const PtModel* model = view(hModel);
    if (!model)
        return DB::Result::INVALID_HANDLE;
    if (model->m_rotation == rotation)
        return DB::Result::SUCCESS;
    PtModel newModel = *model;
    newModel.m_rotation = rotation;
    return hModel.getDB()->objModify(hModel, newModel);
}

Math::Vec3 PtModel::getScale(const DbObjHandle& hModel) {
    const PtModel* model = view(hModel);
    if (!model)
        return Math::Vec3();
    return model->m_scale;
}

DB::Result PtModel::setScale(const DbObjHandle& hModel, const Math::Vec3& scale) {
    const PtModel* model = view(hModel);
    if (!model)
        return DB::Result::INVALID_HANDLE;
    if (model->m_scale == scale)
        return DB::Result::SUCCESS;
    PtModel newModel = *model;
    newModel.m_scale = scale;
    return hModel.getDB()->objModify(hModel, newModel);
}

DbObjHandle PtModel::copy(const DbObjHandle& hModel, std::shared_ptr<DB>& dst) {
    const PtModel* model = view(hModel);
    if (!model)
        return {};
    PtModel newModel = *model;
    newModel.m_meshes.clear();
    for (const auto& meshId : model->m_meshes) {
        DbObjHandle hMesh(hModel.getDB(), meshId);
        DbObjHandle hNewMesh = PtMesh::copy(hMesh, dst);
        if (hNewMesh.isValid())
            newModel.m_meshes.push_back(hNewMesh.getID());
    }
    return dst->objCreate<PtModel>(newModel);
}
