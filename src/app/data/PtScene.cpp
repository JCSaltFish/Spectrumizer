/**
 * @file PtScene.cpp
 * @brief Implementation of the PtScene class representing a scene containing multiple 3D models.
 */

#include "app/AppDataManager.h"

void PtScene::serialize(DbSerializer& serializer, const PtScene& scene) {
    serializer.serialize(scene.m_traceDepth);
    serializer.serialize(scene.m_resX);
    serializer.serialize(scene.m_resY);

    serializer.serialize(scene.m_camera.position.x);
    serializer.serialize(scene.m_camera.position.y);
    serializer.serialize(scene.m_camera.position.z);

    serializer.serialize(scene.m_camera.rotation.x);
    serializer.serialize(scene.m_camera.rotation.y);
    serializer.serialize(scene.m_camera.rotation.z);

    serializer.serialize(scene.m_camera.focusDist);
    serializer.serialize(scene.m_camera.fStop);

    serializer.serialize(scene.m_models);
    serializer.serialize(scene.m_skyTemperature);
}

void PtScene::deserialize(DbSerializer& serializer, PtScene& scene) {
    serializer.deserialize(scene.m_traceDepth);
    serializer.deserialize(scene.m_resX);
    serializer.deserialize(scene.m_resY);

    serializer.deserialize(scene.m_camera.position.x);
    serializer.deserialize(scene.m_camera.position.y);
    serializer.deserialize(scene.m_camera.position.z);

    serializer.deserialize(scene.m_camera.rotation.x);
    serializer.deserialize(scene.m_camera.rotation.y);
    serializer.deserialize(scene.m_camera.rotation.z);

    serializer.deserialize(scene.m_camera.focusDist);
    serializer.deserialize(scene.m_camera.fStop);

    serializer.deserialize(scene.m_models);
    serializer.deserialize(scene.m_skyTemperature);
}

void PtScene::migrate(int oldVersion, PtScene& scene) {}

const PtScene* PtScene::view(const DbObjHandle& hScene) {
    if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME)
        return nullptr;
    return hScene.getDB()->objGet<PtScene>(hScene);
}

int PtScene::getTraceDepth(const DbObjHandle& hScene) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return 0;
    return scene->m_traceDepth;
}

DB::Result PtScene::setTraceDepth(const DbObjHandle& hScene, int depth) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    if (depth < 0)
        return DB::Result::FAILURE;
    PtScene newScene = *scene;
    newScene.m_traceDepth = depth;
    return hScene.getDB()->objModify(hScene, newScene);
}

void PtScene::getResolution(const DbObjHandle& hScene, int& resX, int& resY) {
    const PtScene* scene = view(hScene);
    if (!scene) {
        resX = 0;
        resY = 0;
        return;
    }
    resX = scene->m_resX;
    resY = scene->m_resY;
}

DB::Result PtScene::setResolution(const DbObjHandle& hScene, int resX, int resY) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    if (resX <= 0 || resY <= 0)
        return DB::Result::FAILURE;
    PtScene newScene = *scene;
    newScene.m_resX = resX;
    newScene.m_resY = resY;
    return hScene.getDB()->objModify(hScene, newScene);
}

PtScene::Camera PtScene::getCamera(const DbObjHandle& hScene) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return {};
    return scene->m_camera;
}

DB::Result PtScene::setCamera(const DbObjHandle& hScene, const Camera& camera) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    PtScene newScene = *scene;
    newScene.m_camera = camera;
    return hScene.getDB()->objModify(hScene, newScene);
}

std::vector<DbObjHandle> PtScene::getModels(const DbObjHandle& hScene) {
    std::vector<DbObjHandle> models;
    const PtScene* scene = view(hScene);
    if (!scene)
        return models;
    DB* db = hScene.getDB();
    for (DB::ID modelId : scene->m_models) {
        DbObjHandle hModel(db, modelId);
        if (hModel.isValid() && hModel.getType() == PtModel::TYPE_NAME)
            models.push_back(hModel);
    }
    return models;
}

DB::Result PtScene::addModel(const DbObjHandle& hScene, const DbObjHandle& hModel) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    if (!hModel.isValid() || hModel.getType() != PtModel::TYPE_NAME)
        return DB::Result::INVALID_HANDLE;

    if (std::binary_search(scene->m_models.begin(), scene->m_models.end(), hModel.getID()))
        return DB::Result::SUCCESS; // Already in the scene

    PtScene newScene = *scene;
    newScene.m_models.push_back(hModel.getID());
    return hScene.getDB()->objModify(hScene, newScene);
}

DB::Result PtScene::delModel(const DbObjHandle& hScene, const DbObjHandle& hModel) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    if (!hModel.isValid() || hModel.getType() != PtModel::TYPE_NAME)
        return DB::Result::INVALID_HANDLE;

    if (!std::binary_search(scene->m_models.begin(), scene->m_models.end(), hModel.getID()))
        return DB::Result::SUCCESS; // Not in the scene

    PtScene newScene = *scene;
    newScene.m_models.erase
    (
        std::remove(newScene.m_models.begin(), newScene.m_models.end(), hModel.getID()),
        newScene.m_models.end()
    );
    DB::Result result = hScene.getDB()->objModify(hScene, newScene);
    if (result != DB::Result::SUCCESS)
        return result;

    DB* db = hScene.getDB();
    // Also delete the model and its meshes and materials
    for (auto& hMesh : PtModel::getMeshes(hModel)) {
        DbObjHandle hMaterial = PtMesh::getMaterial(hMesh);
        if (hMaterial.isValid()) {
            result = db->objDelete<PtMaterial>(hMaterial);
            if (result != DB::Result::SUCCESS)
                return result;
        }
        result = db->objDelete<PtMesh>(hMesh);
        if (result != DB::Result::SUCCESS)
            return result;
    }
    return db->objDelete<PtModel>(hModel);
}

std::vector<DbObjHandle> PtScene::getWaves(const DbObjHandle &hScene) {
    std::vector<DbObjHandle> waves;
    const PtScene* scene = view(hScene);
    if (!scene)
        return waves;
    DB* db = hScene.getDB();
    for (DB::ID waveId : scene->m_waves) {
        DbObjHandle hWave(db, waveId);
        if (hWave.isValid() && hWave.getType() == SpWave::TYPE_NAME)
            waves.push_back(hWave);
    }
    return waves;
}

DB::Result PtScene::addWave(const DbObjHandle &hScene, const DbObjHandle &hWave) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    if (!hWave.isValid() || hWave.getType() != SpWave::TYPE_NAME)
        return DB::Result::INVALID_HANDLE;

    if (std::binary_search(scene->m_waves.begin(), scene->m_waves.end(), hWave.getID()))
        return DB::Result::SUCCESS; // Already in the scene

    PtScene newScene = *scene;
    newScene.m_waves.push_back(hWave.getID());
    DB::Result result = hScene.getDB()->objModify(hScene, newScene);
    if (result != DB::Result::SUCCESS)
        return result;

    std::vector<DbObjHandle> materialHandles = PtScene::getSpectrumMaterials(hScene);
    for (const auto& hMaterial : materialHandles) {
        std::vector<float> emissivities = SpMaterial::getEmissivities(hMaterial);
        emissivities.push_back(0.0f);
        result = SpMaterial::setEmissivities(hMaterial, emissivities);
        if (result != DB::Result::SUCCESS)
            return result;
    }

    return DB::Result::SUCCESS;
}

DB::Result PtScene::delWave(const DbObjHandle &hScene, const DbObjHandle &hWave) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    if (!hWave.isValid() || hWave.getType() != SpWave::TYPE_NAME)
        return DB::Result::INVALID_HANDLE;

    // Remove associated emissivity data for every spectrum materials
    DB::Result result = DB::Result::SUCCESS;
    auto it = std::lower_bound(scene->m_waves.begin(), scene->m_waves.end(), hWave.getID());
    if (it == scene->m_waves.end() || *it != hWave.getID())
        return DB::Result::SUCCESS;
    size_t index = std::distance(scene->m_waves.begin(), it);
    std::vector<DbObjHandle> materialHandles = PtScene::getSpectrumMaterials(hScene);
    for (const auto& hMaterial : materialHandles) {
        std::vector<float> emissivities = SpMaterial::getEmissivities(hMaterial);
        if (index < emissivities.size()) {
            emissivities.erase(emissivities.begin() + index);
            result = SpMaterial::setEmissivities(hMaterial, emissivities);
            if (result != DB::Result::SUCCESS)
                return result;
        }
    }

    PtScene newScene = *scene;
    newScene.m_waves.erase(
        std::remove(newScene.m_waves.begin(), newScene.m_waves.end(), hWave.getID()),
        newScene.m_waves.end()
    );
    result = hScene.getDB()->objModify(hScene, newScene);
    if (result != DB::Result::SUCCESS)
        return result;

    result = hScene.getDB()->objDelete<SpWave>(hWave);
    if (result != DB::Result::SUCCESS)
        return result;

    return DB::Result::SUCCESS;
}

DB::Result PtScene::clearWaves(const DbObjHandle &hScene) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;

    DB::Result result = DB::Result::SUCCESS;

    std::vector<DbObjHandle> waveHandles = getWaves(hScene);
    for (const auto& hWave : waveHandles) {
        result = hScene.getDB()->objDelete<SpWave>(hWave);
        if (result != DB::Result::SUCCESS)
            return result;
    }

    PtScene newScene = *scene;
    newScene.m_waves = {};
    result = hScene.getDB()->objModify(hScene, newScene);
    if (result != DB::Result::SUCCESS)
        return result;

    std::vector<DbObjHandle> materialHandles = PtScene::getSpectrumMaterials(hScene);
    for (const auto& hMaterial : materialHandles) {
        result = SpMaterial::setEmissivities(hMaterial, {});
        if (result != DB::Result::SUCCESS)
            return result;
    }

    return DB::Result::SUCCESS;
}

std::vector<DbObjHandle> PtScene::getSpectrumMaterials(const DbObjHandle &hScene) {
    std::vector<DbObjHandle> materials;
    const PtScene* scene = view(hScene);
    if (!scene)
        return materials;
    DB* db = hScene.getDB();
    for (DB::ID materialId : scene->m_spectrumMaterials) {
        DbObjHandle hMaterial(db, materialId);
        if (hMaterial.isValid() && hMaterial.getType() == SpMaterial::TYPE_NAME)
            materials.push_back(hMaterial);
    }
    return materials;
}

DB::Result PtScene::addSpectrumMaterial(const DbObjHandle &hScene, const DbObjHandle &hMaterial) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    if (!hMaterial.isValid() || hMaterial.getType() != SpMaterial::TYPE_NAME)
        return DB::Result::INVALID_HANDLE;

    bool found = std::binary_search(
        scene->m_spectrumMaterials.begin(),
        scene->m_spectrumMaterials.end(),
        hMaterial.getID()
    );
    if (found)
        return DB::Result::SUCCESS; // Already in the scene

    std::vector<float> emissivities(getWaves(hScene).size());
    SpMaterial::setEmissivities(hMaterial, emissivities);

    PtScene newScene = *scene;
    newScene.m_spectrumMaterials.push_back(hMaterial.getID());
    return hScene.getDB()->objModify(hScene, newScene);
}

DB::Result PtScene::delSpectrumMaterial(const DbObjHandle &hScene, const DbObjHandle &hMaterial) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    if (!hMaterial.isValid() || hMaterial.getType() != SpMaterial::TYPE_NAME)
        return DB::Result::INVALID_HANDLE;

    bool found = std::binary_search(
        scene->m_spectrumMaterials.begin(),
        scene->m_spectrumMaterials.end(),
        hMaterial.getID()
    );
    if (!found)
        return DB::Result::SUCCESS; // Not in the scene

    PtScene newScene = *scene;
    newScene.m_spectrumMaterials.erase(
        std::remove(
            newScene.m_spectrumMaterials.begin(),
            newScene.m_spectrumMaterials.end(),
            hMaterial.getID()
        ),
        newScene.m_spectrumMaterials.end()
    );
    DB::Result result = hScene.getDB()->objModify(hScene, newScene);
    if (result != DB::Result::SUCCESS)
        return result;
    
    return hScene.getDB()->objDelete<SpMaterial>(hMaterial);
}

DB::Result PtScene::clearSpectrumMaterials(const DbObjHandle &hScene) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;

    DB::Result result = DB::Result::SUCCESS;

    std::vector<DbObjHandle> materialHandles = getSpectrumMaterials(hScene);
    for (const auto& hMaterial : materialHandles) {
        result = hScene.getDB()->objDelete<SpMaterial>(hMaterial);
        if (result != DB::Result::SUCCESS)
            return result;
    }

    PtScene newScene = *scene;
    newScene.m_spectrumMaterials = {};
    return hScene.getDB()->objModify(hScene, newScene);
}

DbObjHandle PtScene::getSkyMaterial(const DbObjHandle &hScene) {
    const PtScene* scene = view(hScene);
    if (!scene || scene->m_skyMaterialId == 0)
        return DbObjHandle();
    DbObjHandle hMaterial(hScene.getDB(), scene->m_skyMaterialId);
    if (hMaterial.isValid() && hMaterial.getType() == SpMaterial::TYPE_NAME)
        return hMaterial;
    return DbObjHandle();
}

DB::Result PtScene::setSkyMaterial(const DbObjHandle &hScene, const DbObjHandle &hMaterial) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    if (hScene.isValid() && hScene.getType() != PtScene::TYPE_NAME)
        return DB::Result::INVALID_HANDLE;
    DB::ID newMaterialId = hMaterial.isValid() ? hMaterial.getID() : 0;
    if (scene->m_skyMaterialId == newMaterialId)
        return DB::Result::SUCCESS;
    PtScene newScene = *scene;
    newScene.m_skyMaterialId = newMaterialId;
    return hScene.getDB()->objModify(hScene, newScene);
}

float PtScene::getSkyTemperature(const DbObjHandle &hScene) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return 0.0f;
    return scene->m_skyTemperature;
}

DB::Result PtScene::setSkyTemperature(const DbObjHandle &hScene, float temperature) {
    const PtScene* scene = view(hScene);
    if (!scene)
        return DB::Result::INVALID_HANDLE;
    if (scene->m_skyTemperature == temperature)
        return DB::Result::SUCCESS;
    PtScene newScene = *scene;
    newScene.m_skyTemperature = temperature;
    return hScene.getDB()->objModify(hScene, newScene);
}
