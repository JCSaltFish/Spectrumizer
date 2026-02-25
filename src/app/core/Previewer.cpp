/**
 * @file Previewer.cpp
 * @brief Implementation of the Previewer class.
 */

#include "app/core/Previewer.h"

#include "app/AppTextureManager.h"
#include "utils/Logger.hpp"
#include "res/ShaderStrings.hpp"

constexpr float DRAW_DIST = 100.0f; // Far clipping plane distance

Previewer::Previewer(GfxRenderer& renderer) :
    m_renderer(renderer) {
    m_camera.dir = Math::Vec3(0.0f, 0.0f, 1.0f);
    m_camera.up = Math::Vec3(0.0f, 1.0f, 0.0f);
}

int Previewer::init(int resX, int resY, int samples) {
    m_resolutionX = resX;
    m_resolutionY = resY;
    m_MSAAsampleCount = samples;

    if (!m_renderer) {
        Logger() << "Renderer is null in Previewer::init";
        return 1;
    }

    // Load shaders
    try {
        m_vertexShader = m_renderer->createShader(
            GfxShaderStage::VERTEX,
            ShaderStrings::PREVIEW_VERT
        );
    } catch (GfxShaderException& e) {
        Logger() << "Failed to create vertex shader in Previewer::init: " << e.what();
        return 1;
    }
    try {
        m_fragmentShader = m_renderer->createShader(
            GfxShaderStage::FRAGMENT,
            ShaderStrings::PREVIEW_FRAG
        );
    } catch (GfxShaderException& e) {
        Logger() << "Failed to create fragment shader in Previewer::init: " << e.what();
        return 1;
    }

    // Initialize descriptors and UBOs
    m_descriptors.u_xform.binding = 0;
    m_descriptors.u_xform.type = GfxDescriptorType::UNIFORM_BUFFER;
    m_descriptors.u_xform.stages.set(GfxShaderStage::VERTEX);

    m_descriptors.u_camera.binding = 1;
    m_descriptors.u_camera.type = GfxDescriptorType::UNIFORM_BUFFER;
    m_descriptors.u_camera.stages.set(GfxShaderStage::FRAGMENT);
    m_uboCamera = m_renderer->createBuffer(
        sizeof(UCamera),
        GfxBufferUsage::UNIFORM_BUFFER,
        GfxBufferProp::DYNAMIC
    );
    if (m_uboCamera == nullptr) {
        Logger() << "Failed to create UBO for camera in Previewer::init";
        return 1;
    }

    m_descriptors.u_material.binding = 2;
    m_descriptors.u_material.type = GfxDescriptorType::UNIFORM_BUFFER;
    m_descriptors.u_material.stages.set(GfxShaderStage::FRAGMENT);

    m_descriptors.u_normalTex.binding = 3;
    m_descriptors.u_normalTex.type = GfxDescriptorType::SAMPLER;
    m_descriptors.u_normalTex.stages.set(GfxShaderStage::FRAGMENT);

    m_descriptors.u_roughnessTex.binding = 4;
    m_descriptors.u_roughnessTex.type = GfxDescriptorType::SAMPLER;
    m_descriptors.u_roughnessTex.stages.set(GfxShaderStage::FRAGMENT);

    m_descriptors.u_temperatureTex.binding = 5;
    m_descriptors.u_temperatureTex.type = GfxDescriptorType::SAMPLER;
    m_descriptors.u_temperatureTex.stages.set(GfxShaderStage::FRAGMENT);

    m_descriptors.u_pickInfo.binding = 6;
    m_descriptors.u_pickInfo.type = GfxDescriptorType::UNIFORM_BUFFER;
    m_descriptors.u_pickInfo.stages.set(GfxShaderStage::FRAGMENT);

    // Vertex attributes: pos(3), normal(3), tangent(3), texCoord(2)
    std::vector<GfxVertexAttr> vtxAttrs(4);
    int offset = 0;
    vtxAttrs[0] = { 0, GfxFormat::R32G32B32_SFLOAT, offset }; // vec3 iPos
    offset += 3 * sizeof(float);
    vtxAttrs[1] = { 1, GfxFormat::R32G32B32_SFLOAT, offset }; // vec3 iNormal
    offset += 3 * sizeof(float);
    vtxAttrs[2] = { 2, GfxFormat::R32G32B32_SFLOAT, offset }; // vec3 iTangent
    offset += 3 * sizeof(float);
    vtxAttrs[3] = { 3, GfxFormat::R32G32_SFLOAT, offset }; // vec2 iTexCoord
    offset += 2 * sizeof(float);
    m_vertexDesc.attributes = vtxAttrs;
    m_vertexDesc.stride = offset;

    // Create framebuffer and related resources
    if (initFramebuffer(resX, resY, true)) {
        Logger() << "Failed to create framebuffer in Previewer::init";
        return 1;
    }

    return 0;
}

void Previewer::term() {
    if (!m_renderer)
        return;

    clearScene();

    m_renderer->waitDeviceIdle();

    m_renderer->destroyShader(m_vertexShader);
    m_vertexShader = nullptr;
    m_renderer->destroyShader(m_fragmentShader);
    m_fragmentShader = nullptr;

    m_renderer->destroyBuffer(m_uboCamera);
    m_uboCamera = nullptr;

    m_renderer->destroyImage(m_colorFrameImage);
    m_colorFrameImage = nullptr;
    m_renderer->destroyImage(m_pickFrameImage);
    m_pickFrameImage = nullptr;
    m_renderer->destroyImage(m_depthFrameImage);
    m_depthFrameImage = nullptr;

    if (m_colorFrameResolveImage) {
        m_renderer->destroyImage(m_colorFrameResolveImage);
        m_colorFrameResolveImage = nullptr;
    }
    if (m_pickFrameResolveImage) {
        m_renderer->destroyImage(m_pickFrameResolveImage);
        m_pickFrameResolveImage = nullptr;
    }

    m_renderer->destroyFramebuffer(m_framebuffer);
    m_framebuffer = nullptr;
    m_renderer->destroyRenderPass(m_renderPass);
    m_renderPass = nullptr;
    m_renderer->destroyPipeline(m_pipeline);
    m_pipeline = nullptr;

    m_renderer->destroyDescriptorSetBinding(m_descriptorSetBinding);
    m_descriptorSetBinding = nullptr;
}

GfxImage Previewer::getFrameImage() const {
    return m_colorFrameResolveImage ? m_colorFrameResolveImage : m_colorFrameImage;
}

int Previewer::setMSAASamples(int samples) {
    m_MSAAsampleCount = samples;
    if (initFramebuffer(m_resolutionX, m_resolutionY, true))
        return 1;
    return 0;
}

int Previewer::loadScene(const DbObjHandle& hScene) {
    PtScene::getResolution(hScene, m_resolutionX, m_resolutionY);
    if (initFramebuffer(m_resolutionX, m_resolutionY))
        return 1;

    PtScene::Camera sceneCam = PtScene::getCamera(hScene);
    setCameraQuick(sceneCam.position, sceneCam.rotation);

    for (const auto& hModel : PtScene::getModels(hScene)) {
        if (updateModel(hModel))
            return 1;
    }

    m_currentScene = hScene;
    return 0;
}

void Previewer::updateObjects
(
    const std::vector<DbObjHandle>& hObjects,
    bool& resolutionChanged
) {
    for (const auto& hObj : hObjects) {
        const std::string type = hObj.getType();
        if (type == PtScene::TYPE_NAME) {
            int resX = 0, resY = 0;
            PtScene::getResolution(hObj, resX, resY);
            resolutionChanged = setResolutionQuick(resX, resY);
            PtScene::Camera sceneCam = PtScene::getCamera(hObj);
            setCameraQuick(sceneCam.position, sceneCam.rotation);
        } else if (type == PtModel::TYPE_NAME) {
            if (hObj.isValid()) {
                if (m_models.find(hObj.getID()) != m_models.end()) {
                    Model& model = m_models[hObj.getID()];
                    model.location = PtModel::getLocation(hObj);
                    // Invert the X axis for previewer coordinate system
                    model.location = Math::Vec3(
                        -model.location.x,
                        model.location.y,
                        model.location.z
                    );
                    model.rotation = PtModel::getRotation(hObj);
                    // Invert Y and Z rotation for previewer coordinate system
                    model.rotation = Math::Vec3(
                        model.rotation.x,
                        -model.rotation.y,
                        -model.rotation.z
                    );
                    model.scale = PtModel::getScale(hObj);
                }
                updateModel(hObj);
            } else
                removeModel(hObj);
        } else if (type == PtMaterial::TYPE_NAME) {
            if (hObj.isValid())
                updateMaterial(hObj);
        }
    }
}

void Previewer::updateObjects(const std::vector<DbObjHandle>& hObjects) {
    bool resolutionChanged = false;
    updateObjects(hObjects, resolutionChanged);
}

void Previewer::clearScene() {
    if (m_currentScene.isValid()) {
        for (auto& [id, model] : m_models)
            cleanupModel(model);
        m_models.clear();
        m_meshLookup.clear();
        m_currentScene = DbObjHandle();
    }
}

void Previewer::setCameraQuick(const Math::Vec3& position, const Math::Vec3& rotation) {
    using namespace Math;

    m_camera.pos = position;

    float pitch = std::fmod(rotation.x, 360.0f);
    if (pitch < 0.0f)
        pitch += 360.0f;
    float yaw = std::fmod(rotation.y, 360.0f);
    if (yaw < 0.0f)
        yaw += 360.0f;
    float roll = std::fmod(rotation.z, 360.0f);
    if (roll < 0.0f)
        roll += 360.0f;
    m_camera.rot = Vec3(pitch, yaw, roll);

    Mat4 rotX = rotate(Mat4(1.0f), pitch, Vec3(1.0f, 0.0f, 0.0f));
    Mat4 rotY = rotate(Mat4(1.0f), yaw, Vec3(0.0f, 1.0f, 0.0f));
    Mat4 rotZ = rotate(Mat4(1.0f), roll, Vec3(0.0f, 0.0f, 1.0f));
    Mat4 rot = rotZ * rotY * rotX;

    m_camera.dir = normalize(Vec3(rot * Vec4(0.0f, 0.0f, 1.0f, 1.0f)));
    m_camera.up = normalize(Vec3(rot * Vec4(0.0f, 1.0f, 0.0f, 1.0f)));
}

bool Previewer::setResolutionQuick(int resX, int resY) {
    if (resX == m_resolutionX && resY == m_resolutionY)
        return false;
    m_resolutionX = resX;
    m_resolutionY = resY;
    initFramebuffer(resX, resY);
    return true;
}

void Previewer::setModelXformQuick(const DbObjHandle& hModel, const ModelXform& xform) {
    if (hModel.isValid() && hModel.getType() != PtModel::TYPE_NAME)
        return;
    if (m_models.find(hModel.getID()) == m_models.end())
        return;
    Model& model = m_models[hModel.getID()];
    if (xform.location.has_value()) {
        model.location = xform.location.value();
        // Invert the X axis for previewer coordinate system
        model.location = Math::Vec3(
            -model.location.x,
            model.location.y,
            model.location.z
        );
    }
    if (xform.rotation.has_value()) {
        model.rotation = xform.rotation.value();
        // Invert Y and Z rotation for previewer coordinate system
        model.rotation = Math::Vec3(
            model.rotation.x,
            -model.rotation.y,
            -model.rotation.z
        );
    }
    if (xform.scale.has_value())
        model.scale = xform.scale.value();
}

void Previewer::setMeshMaterialQuick(const DbObjHandle& hMesh, const MaterialInfo& info) {
    if (hMesh.isValid() && hMesh.getType() != PtMesh::TYPE_NAME)
        return;
    if (m_meshLookup.find(hMesh.getID()) == m_meshLookup.end())
        return;
    Mesh* mesh = m_meshLookup[hMesh.getID()];
    if (info.roughness.has_value())
        mesh->material.roughness = info.roughness.value();
}

DbObjHandle Previewer::getMeshAtPixel(int x, int y) const {
    Math::Vec4 pixel;
    m_renderer->readFramebufferColorAttachmentPixels(m_framebuffer, 1, { x, y }, &pixel);
    if (pixel.b == 0.0f)
        return {};
    return DbObjHandle(AppDataManager::instance().getDB().get(), pixel.g);
}

void Previewer::setHighlightColors(const Math::Vec3& hovered, const Math::Vec3& picked) {
    m_highlightColorHovered = hovered;
    m_highlightColorPicked = picked;
}

void Previewer::hightlightObject(const DbObjHandle& hObj, HightlightState state) {
    if (state == HightlightState::PICKED) {
        if (m_highlightedObjects.count(hObj.getID()) > 0) {
            if (m_highlightedObjects[hObj.getID()] == HightlightState::HOVERED)
                return;
        }
    }
    m_highlightedObjects[hObj.getID()] = state;
}

void Previewer::setBackgroundColor(const Math::Vec3& color) {
    m_backgroundColor = color;
}

void Previewer::setCameraMoveSpeed(float speed) {
    m_cameraMoveSpeed = speed;
}

int Previewer::renderFrame() {
    using namespace Math;

    UXfrom u_xform = {};
    UCamera u_camera = {};
    UMaterial u_material = {};
    UPickInfo u_pickInfo = {};

    m_renderer->beginRenderPass(m_framebuffer);
    m_renderer->bindPipeline(m_pipeline);

    m_renderer->getPipelineStateMachine()->setViewport({ 0, 0, m_resolutionX, m_resolutionY });
    m_renderer->getPipelineStateMachine()->setScissor({ 0, 0, m_resolutionX, m_resolutionY });
    m_renderer->getPipelineStateMachine()->setDepthTestEnabled(true);

    m_renderer->clearColorAttachment(
        0,
        { m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, 1.0f }
    );
    m_renderer->clearColorAttachment(1, { 0.0f, 0.0f, 0.0f, 1.0f });
    m_renderer->clearDepthAttachment(1.0f);

    m_renderer->getPipelineStateMachine()->setPrimitiveTopo(GfxPrimitiveTopo::TRIANGLE_LIST);

    u_camera.posW = m_camera.pos;
    if (m_renderer->updateBufferData(m_uboCamera, 0, sizeof(u_camera), &u_camera))
        return 1;
    // Calculate view matrix
    u_xform.view = lookAt(m_camera.pos, m_camera.pos + m_camera.dir, m_camera.up);
    // Calculate projection matrix
    {
        DepthRange depthRange = DepthRange::MINUS_ONE_TO_ONE;
        if (m_renderer->getBackend() == GfxBackend::Vulkan)
            depthRange = DepthRange::ZERO_TO_ONE;
        u_xform.proj = perspective
        (
            PtScene::Camera::FOV * Math::PI / 180.0f,
            float(m_resolutionX) / float(m_resolutionY),
            PtScene::Camera::FOCAL,
            DRAW_DIST,
            depthRange
        );
    }

    // Traverse models
    for (auto& [modelID, model] : m_models) {
        u_pickInfo.modelID = modelID;

        // Calculate model matrix
        {
            Mat4 t = translate(Mat4(1.0f), model.location);
            Mat4 rx = rotate(Mat4(1.0f), model.rotation.x, Vec3(1.0f, 0.0f, 0.0f));
            Mat4 ry = rotate(Mat4(1.0f), model.rotation.y, Vec3(0.0f, 1.0f, 0.0f));
            Mat4 rz = rotate(Mat4(1.0f), model.rotation.z, Vec3(0.0f, 0.0f, 1.0f));
            Mat4 r = rz * ry * rx;
            Mat4 s = scale(Mat4(1.0f), model.scale);

            u_xform.model = t * r * s;
        }
        if (m_renderer->updateBufferData(model.uboXfrom, 0, sizeof(UXfrom), &u_xform))
            return 1;
        m_renderer->bindDescriptorSetBinding(model.descriptorSetBinding);

        // Traverse meshes
        for (auto& mesh : model.meshes) {
            u_pickInfo.meshID = mesh.id;
            if (m_renderer->updateBufferData(mesh.uboPickInfo, 0, sizeof(UPickInfo), &u_pickInfo))
                return 1;

            // Prepare material
            Flags<MaterialFlag> matFlags = mesh.material.flags;
            Vec4 diffuseColor = Vec4(1.0f);
            if (m_highlightedObjects.find(mesh.id) != m_highlightedObjects.end()) {
                matFlags.set(MaterialFlag::HIGHLIGHT);
                if (m_highlightedObjects.at(mesh.id) == HightlightState::HOVERED)
                    diffuseColor = Vec4(m_highlightColorHovered, 1.0f);
            } else if (m_highlightedObjects.find(modelID) != m_highlightedObjects.end()) {
                matFlags.set(MaterialFlag::HIGHLIGHT);
                if (m_highlightedObjects.at(modelID) == HightlightState::PICKED)
                    diffuseColor = Vec4(m_highlightColorPicked, 1.0f);
                else if (m_highlightedObjects.at(modelID) == HightlightState::HOVERED)
                    diffuseColor = Vec4(m_highlightColorHovered, 1.0f);
            }
            u_material.diffuse = diffuseColor;
            u_material.roughness = mesh.material.roughness;
            u_material.flags = static_cast<uint32_t>(matFlags.getValue());
            if (m_renderer->updateBufferData(mesh.uboMaterial, 0, sizeof(u_material), &u_material))
                return 1;

            // bind UBOs and textures
            m_renderer->bindDescriptorSetBinding(m_descriptorSetBinding);
            m_renderer->bindDescriptorSetBinding(mesh.descriptorSetBinding);

            m_renderer->bindVAO(mesh.vao);
            // Draw call
            m_renderer->drawIndexed(mesh.indexCount);
        }
    }

    m_renderer->endRenderPass();

    // Clear highlighted objects after rendering
    m_highlightedObjects.clear();

    return 0;
}

int Previewer::countTriangles() const {
    int result = 0;
    for (const auto& [id, model] : m_models) {
        for (const auto& mesh : model.meshes)
            result += mesh.indexCount / 3;
    }
    return result;
}

int Previewer::updateModel(const DbObjHandle& hModel) {
    if (hModel.isValid() && hModel.getType() != PtModel::TYPE_NAME)
        return 1;

    Model* model = nullptr;
    if (m_models.find(hModel.getID()) == m_models.end()) {
        // New model
        model = &m_models[hModel.getID()];
        model->id = hModel.getID();
    } else {
        // Existing model, clean up previous data
        model = &m_models[hModel.getID()];
        if (cleanupModel(*model))
            return 1;
    }
    if (!model)
        return 1; // Should not happen

    // Get model info
    model->location = PtModel::getLocation(hModel);
    model->location = Math::Vec3(
        -model->location.x, // Invert X axis for previewer coordinate system
        model->location.y,
        model->location.z
    );
    model->rotation = PtModel::getRotation(hModel);
    model->rotation = Math::Vec3(
        model->rotation.x,
        -model->rotation.y, // Invert Y axis for previewer coordinate system
        -model->rotation.z  // Invert Z axis for previewer coordinate system
    );
    model->scale = PtModel::getScale(hModel);

    // Create UBO for model transform
    model->uboXfrom = m_renderer->createBuffer(
        sizeof(UXfrom),
        GfxBufferUsage::UNIFORM_BUFFER,
        GfxBufferProp::DYNAMIC
    );
    if (model->uboXfrom == nullptr) {
        Logger() << "Failed to create UBO for model transform in model ID: " << hModel.getID();
        return 1;
    }
    // Create descriptor set binding
    model->descriptorSetBinding = m_renderer->createDescriptorSetBinding(
        m_pipeline,
        1,
        { { m_descriptors.u_xform, model->uboXfrom } }
    );

    // Load model data from file
    std::string filename = PtModel::getFilePath(hModel);
    if (filename.empty()) {
        Logger() << "Model file path is empty for model ID: " << hModel.getID();
        return 1;
    }
    ::Mesh::Model modelData = {};
    if (MeshLoader::loadOBJ(filename, modelData)) {
        Logger() << "Failed to load model file: " << filename;
        return 1;
    }

    // Prepare mesh data info
    std::vector<MeshDataInfo> meshDataInfos;
    for (const auto& meshData : modelData.meshes) {
        for (const auto& submeshData : meshData.submeshes)
            meshDataInfos.push_back({ meshData.vertices, submeshData.indices });
    }

    // Populate mesh data
    std::vector<DbObjHandle> meshHandles = PtModel::getMeshes(hModel);
    if (meshHandles.size() != meshDataInfos.size()) {
        Logger() << "Mesh count mismatch for model: " << filename;
        return 1;
    }
    model->meshes.reserve(meshHandles.size());
    for (int i = 0; i < meshHandles.size(); i++) {
        const DbObjHandle& hMesh = meshHandles[i];
        if (hMesh.isValid() && hMesh.getType() != PtMesh::TYPE_NAME) {
            Logger() << "Invalid mesh handle in model: " << filename;
            return 1;
        }

        model->meshes.push_back({});
        Mesh& mesh = model->meshes.back();
        mesh.id = hMesh.getID();
        m_meshLookup[hMesh.getID()] = &model->meshes.back();

        const auto& meshDataInfo = meshDataInfos[i];

        updateMesh(hMesh, mesh, meshDataInfo);
    }

    return 0;
}

int Previewer::updateMesh(const DbObjHandle& hMesh, Mesh& mesh, const MeshDataInfo& meshDataInfo) {
    int vtxBufferSize = meshDataInfo.vertices.size() * m_vertexDesc.stride;

    // Create and fill vertex buffer
    mesh.vertexBuffer = m_renderer->createBuffer(
        vtxBufferSize,
        GfxBufferUsage::VERTEX_BUFFER,
        GfxBufferProp::STATIC
    );
    if (!mesh.vertexBuffer) {
        Logger() << "Failed to create vertex buffer for mesh ID: " << hMesh.getID();
        return 1;
    }
    std::vector<float> vertexData{};
    vertexData.reserve(meshDataInfo.vertices.size() * (m_vertexDesc.stride / sizeof(float)));
    for (const auto& vtx : meshDataInfo.vertices) {
        vertexData.push_back(vtx.pos.x);
        vertexData.push_back(vtx.pos.y);
        vertexData.push_back(vtx.pos.z);

        vertexData.push_back(vtx.normal.x);
        vertexData.push_back(vtx.normal.y);
        vertexData.push_back(vtx.normal.z);

        vertexData.push_back(vtx.tangent.x);
        vertexData.push_back(vtx.tangent.y);
        vertexData.push_back(vtx.tangent.z);

        vertexData.push_back(vtx.texCoord.x);
        vertexData.push_back(vtx.texCoord.y);
    }
    if (m_renderer->setBufferData(mesh.vertexBuffer, vtxBufferSize, vertexData.data())) {
        Logger() << "Failed to upload vertex data for mesh ID: " << hMesh.getID();
        return 1;
    }

    // Create and fill index buffer
    int idxBufferSize = meshDataInfo.indices.size() * sizeof(uint32_t);
    mesh.indexBuffer = m_renderer->createBuffer(
        idxBufferSize,
        GfxBufferUsage::INDEX_BUFFER,
        GfxBufferProp::STATIC
    );
    if (!mesh.indexBuffer) {
        Logger() << "Failed to create index buffer for mesh ID: " << hMesh.getID();
        return 1;
    }
    std::vector<uint32_t> indexData = meshDataInfo.indices;
    if (m_renderer->setBufferData(mesh.indexBuffer, idxBufferSize, indexData.data())) {
        Logger() << "Failed to upload index data for mesh ID: " << hMesh.getID();
        return 1;
    }
    mesh.indexCount = static_cast<int>(meshDataInfo.indices.size());

    // Create VAO
    mesh.vao = m_renderer->createVAO(m_vertexDesc, mesh.vertexBuffer, mesh.indexBuffer);
    if (!mesh.vao) {
        Logger() << "Failed to create VAO for mesh ID: " << hMesh.getID();
        return 1;
    }

    // Create UBOs
    mesh.uboMaterial = m_renderer->createBuffer(
        sizeof(UMaterial),
        GfxBufferUsage::UNIFORM_BUFFER,
        GfxBufferProp::DYNAMIC
    );
    if (mesh.uboMaterial == nullptr) {
        Logger() << "Failed to create UBO for mesh material in mesh ID: " << hMesh.getID();
        return 1;
    }
    mesh.uboPickInfo = m_renderer->createBuffer(
        sizeof(UPickInfo),
        GfxBufferUsage::UNIFORM_BUFFER,
        GfxBufferProp::DYNAMIC
    );
    if (mesh.uboPickInfo == nullptr) {
        Logger() << "Failed to create UBO for pick info in mesh ID: " << hMesh.getID();
        return 1;
    }

    // Material
    DbObjHandle hMaterial = PtMesh::getMaterial(hMesh);
    if (hMaterial.isValid() && hMaterial.getType() == PtMaterial::TYPE_NAME) {
        mesh.material.id = hMaterial.getID();
        m_materialLookup[hMaterial.getID()] = &mesh.material;
        if (updateMaterial(hMaterial))
            return 1;
    }

    // Descriptor set binding
    Flags<MaterialFlag> matFlags = mesh.material.flags;
    GfxImage defaultTexture = AppTextureManager::instance().getDefaultTexture();
    std::vector<GfxDescriptorBinding> bindings = {};
    bindings.reserve(5);
    bindings.push_back({ m_descriptors.u_material, mesh.uboMaterial });
    if (matFlags.check(MaterialFlag::NORMAL_MAP) && mesh.material.textures.normal) {
        bindings.push_back(
            { m_descriptors.u_normalTex, mesh.material.textures.normal }
        );
    } else
        bindings.push_back({ m_descriptors.u_normalTex, defaultTexture });
    if (matFlags.check(MaterialFlag::ROUGHNESS_MAP) && mesh.material.textures.roughness) {
        bindings.push_back(
            { m_descriptors.u_roughnessTex, mesh.material.textures.roughness }
        );
    } else
        bindings.push_back({ m_descriptors.u_roughnessTex, defaultTexture });
    if (matFlags.check(MaterialFlag::TEMPERATURE_MAP) && mesh.material.textures.temperature) {
        bindings.push_back(
            { m_descriptors.u_temperatureTex, mesh.material.textures.temperature }
        );
    } else
        bindings.push_back({ m_descriptors.u_temperatureTex, defaultTexture });
    bindings.push_back({ m_descriptors.u_pickInfo, mesh.uboPickInfo });
    mesh.descriptorSetBinding =
        m_renderer->createDescriptorSetBinding(m_pipeline, 0, bindings);

    return 0;
}

int Previewer::cleanupModel(Model& model) {
    m_renderer->waitDeviceIdle();

    if (model.uboXfrom)
        m_renderer->destroyBuffer(model.uboXfrom);
    if (model.descriptorSetBinding)
        m_renderer->destroyDescriptorSetBinding(model.descriptorSetBinding);

    for (auto& mesh : model.meshes) {
        if (mesh.vao)
            m_renderer->destroyVAO(mesh.vao);
        if (mesh.vertexBuffer)
            m_renderer->destroyBuffer(mesh.vertexBuffer);
        if (mesh.indexBuffer)
            m_renderer->destroyBuffer(mesh.indexBuffer);

        if (mesh.uboMaterial)
            m_renderer->destroyBuffer(mesh.uboMaterial);
        if (mesh.uboPickInfo)
            m_renderer->destroyBuffer(mesh.uboPickInfo);

        m_meshLookup.erase(mesh.id);
        m_materialLookup.erase(mesh.material.id);

        if (mesh.descriptorSetBinding)
            m_renderer->destroyDescriptorSetBinding(mesh.descriptorSetBinding);
    }

    m_models[model.id].meshes.clear();
    return 0;
}

int Previewer::removeModel(const DbObjHandle& hModel) {
    if (hModel.getType() != PtModel::TYPE_NAME)
        return 1;
    if (m_models.find(hModel.getID()) == m_models.end())
        return 0;
    if (cleanupModel(m_models[hModel.getID()]))
        return 1;
    m_models.erase(hModel.getID());
    return 0;
}

int Previewer::updateMaterial(const DbObjHandle& hMaterial) {
    if (hMaterial.isValid() && hMaterial.getType() != PtMaterial::TYPE_NAME)
        return 1;
    if (m_materialLookup.find(hMaterial.getID()) == m_materialLookup.end())
        return 1;
    Material* material = m_materialLookup[hMaterial.getID()];

    // Basic material info
    material->roughness = PtMaterial::getRoughness(hMaterial);
    material->flags = PtMaterial::getFlags(hMaterial);
    // Textures
    if (material->flags.check(MaterialFlag::NORMAL_MAP)) {
        std::string texFile = PtMaterial::getNormalTexPath(hMaterial);
        material->textures.normal = AppTextureManager::instance().getTexture(texFile);
    }
    if (material->flags.check(MaterialFlag::ROUGHNESS_MAP)) {
        std::string texFile = PtMaterial::getRoughnessTexPath(hMaterial);
        material->textures.roughness = AppTextureManager::instance().getTexture(texFile);
    }
    if (material->flags.check(MaterialFlag::TEMPERATURE_MAP)) {
        std::string texFile = PtMaterial::getTemperatureTexPath(hMaterial);
        material->textures.temperature =
            AppTextureManager::instance().getIntensityPreviewTexture(texFile);
    }

    return 0;
}

int Previewer::initFramebuffer(int width, int height, bool samplesChanged) {
    m_renderer->waitDeviceIdle();

    // Check if we need to recreate the render pass and pipeline
    if (samplesChanged)
        handleSampleChanged();

    // Create images and framebuffer
    if (m_colorFrameImage)
        m_renderer->destroyImage(m_colorFrameImage);
    GfxImageInfo colorInfo = {};
    colorInfo.width = width;
    colorInfo.height = height;
    colorInfo.samples = m_MSAAsampleCount;
    colorInfo.format = GfxFormat::R8G8B8A8_UNORM;
    colorInfo.usages.set(GfxImageUsage::COLOR_ATTACHMENT);
    colorInfo.usages.set(GfxImageUsage::SAMPLED_TEXTURE);
    m_colorFrameImage = m_renderer->createImage(colorInfo);
    if (!m_colorFrameImage) {
        Logger() << "Failed to create color frame image in Previewer::initFramebuffer";
        return 1;
    }

    // Pick image (for object ID rendering)
    if (m_pickFrameImage)
        m_renderer->destroyImage(m_pickFrameImage);
    GfxImageInfo pickInfo = {};
    pickInfo.width = width;
    pickInfo.height = height;
    pickInfo.samples = m_MSAAsampleCount;
    pickInfo.format = GfxFormat::R32G32B32A32_SFLOAT;
    pickInfo.usages.set(GfxImageUsage::COLOR_ATTACHMENT);
    pickInfo.usages.set(GfxImageUsage::SAMPLED_TEXTURE);
    m_pickFrameImage = m_renderer->createImage(pickInfo);
    if (!m_pickFrameImage) {
        Logger() << "Failed to create pick frame image in Previewer::initFramebuffer";
        return 1;
    }

    // Resolve images for MSAA
    std::vector<GfxImage> resolveImages{};
    if (m_MSAAsampleCount > 1) {
        // Resolve images
        if (m_colorFrameResolveImage)
            m_renderer->destroyImage(m_colorFrameResolveImage);
        GfxImageInfo colorResolveInfo = {};
        colorResolveInfo.width = width;
        colorResolveInfo.height = height;
        colorResolveInfo.samples = 1;
        colorResolveInfo.format = GfxFormat::R8G8B8A8_UNORM;
        colorResolveInfo.usages.set(GfxImageUsage::COLOR_ATTACHMENT);
        colorResolveInfo.usages.set(GfxImageUsage::SAMPLED_TEXTURE);
        m_colorFrameResolveImage = m_renderer->createImage(colorResolveInfo);
        if (!m_colorFrameResolveImage) {
            Logger() << "Failed to create color frame resolve image";
            return 1;
        }
        resolveImages.push_back(m_colorFrameResolveImage);
        if (m_pickFrameResolveImage)
            m_renderer->destroyImage(m_pickFrameResolveImage);
        GfxImageInfo pickResolveInfo = {};
        pickResolveInfo.width = width;
        pickResolveInfo.height = height;
        pickResolveInfo.samples = 1;
        pickResolveInfo.format = GfxFormat::R32G32B32A32_SFLOAT;
        pickResolveInfo.usages.set(GfxImageUsage::COLOR_ATTACHMENT);
        pickResolveInfo.usages.set(GfxImageUsage::SAMPLED_TEXTURE);
        m_pickFrameResolveImage = m_renderer->createImage(pickResolveInfo);
        if (!m_pickFrameResolveImage) {
            Logger() << "Failed to create pick frame resolve image";
            return 1;
        }
        resolveImages.push_back(m_pickFrameResolveImage);
    }

    // Depth image
    if (m_depthFrameImage)
        m_renderer->destroyImage(m_depthFrameImage);
    GfxImageInfo depthInfo = {};
    depthInfo.width = width;
    depthInfo.height = height;
    depthInfo.samples = m_MSAAsampleCount;
    depthInfo.format = GfxFormat::D32_SFLOAT;
    depthInfo.usages.set(GfxImageUsage::DEPTH_ATTACHMENT);
    m_depthFrameImage = m_renderer->createImage(depthInfo);
    if (!m_depthFrameImage) {
        Logger() << "Failed to create depth frame image in Previewer::initFramebuffer";
        return 1;
    }

    // Framebuffer
    if (m_framebuffer)
        m_renderer->destroyFramebuffer(m_framebuffer);
    m_framebuffer = m_renderer->createFramebuffer(
        m_renderPass,
        { m_colorFrameImage, m_pickFrameImage },
        m_depthFrameImage,
        resolveImages
    );
    if (!m_framebuffer) {
        Logger() << "Failed to create framebuffer in Previewer::initFramebuffer";
        return 1;
    }

    return 0;
}

int Previewer::handleSampleChanged() {
    // Create render pass
    GfxAttachment colorAttachment = {};
    colorAttachment.samples = m_MSAAsampleCount;
    colorAttachment.format = GfxFormat::R8G8B8A8_UNORM;
    colorAttachment.usages.set(GfxImageUsage::COLOR_ATTACHMENT);

    GfxAttachment pickAttachment = {};
    pickAttachment.samples = m_MSAAsampleCount;
    pickAttachment.format = GfxFormat::R32G32B32A32_SFLOAT;
    pickAttachment.usages.set(GfxImageUsage::COLOR_ATTACHMENT);

    GfxAttachment depthAttachment = {};
    depthAttachment.samples = m_MSAAsampleCount;
    depthAttachment.format = GfxFormat::D32_SFLOAT;
    depthAttachment.usages.set(GfxImageUsage::DEPTH_ATTACHMENT);

    if (m_renderPass)
        m_renderer->destroyRenderPass(m_renderPass);
    m_renderPass = m_renderer->createRenderPass(
        { colorAttachment, pickAttachment },
        depthAttachment
    );
    if (!m_renderPass) {
        Logger() << "Failed to create render pass in Previewer::initFramebuffer";
        return 1;
    }

    // Create pipeline
    std::vector<GfxPipelineState> dynamicStates = {
        GfxPipelineState::VIEWPORT,
        GfxPipelineState::SCISSOR,
        GfxPipelineState::DEPTH_TEST_ENABLE,
        GfxPipelineState::PRIMITIVE_TOPOLOGY,
    };

    if (m_pipeline)
        m_renderer->destroyPipeline(m_pipeline);
    m_pipeline = m_renderer->createPipeline(
        { m_vertexShader, m_fragmentShader },
        {
            {
                m_descriptors.u_material,
                m_descriptors.u_normalTex,
                m_descriptors.u_roughnessTex,
                m_descriptors.u_temperatureTex,
                m_descriptors.u_pickInfo,
            },
            {
                m_descriptors.u_xform,
            },
            {
                m_descriptors.u_camera,
            }
        },
        m_vertexDesc,
        dynamicStates,
        m_renderPass
    );
    if (!m_pipeline) {
        Logger() << "Failed to create pipeline in Previewer::initFramebuffer";
        return 1;
    }

    // Create global descriptor set binding
    if (m_descriptorSetBinding)
        m_renderer->destroyDescriptorSetBinding(m_descriptorSetBinding);
    m_descriptorSetBinding = m_renderer->createDescriptorSetBinding(
        m_pipeline,
        2,
        { { m_descriptors.u_camera, m_uboCamera } }
    );

    // Reload current scene
    if (m_currentScene.isValid()) {
        DbObjHandle hScene = m_currentScene;
        m_currentScene = DbObjHandle(); // Clear current scene to avoid issues
        loadScene(hScene);
    }

    return 0;
}

Previewer::CameraController& Previewer::getCameraController() {
    return m_cameraController;
}

Math::Vec3 Previewer::getCameraPosition() const {
    return m_camera.pos;
}

Math::Vec3 Previewer::getCameraRotation() const {
    return m_camera.rot;
}

void Previewer::CameraController::moveForward(bool move) {
    if (move)
        m_camMovement.set(CamMoveDir::FORWARD);
    else
        m_camMovement.unset(CamMoveDir::FORWARD);
}

void Previewer::CameraController::moveBackward(bool move) {
    if (move)
        m_camMovement.set(CamMoveDir::BACKWARD);
    else
        m_camMovement.unset(CamMoveDir::BACKWARD);
}

void Previewer::CameraController::moveLeft(bool move) {
    if (move)
        m_camMovement.set(CamMoveDir::LEFT);
    else
        m_camMovement.unset(CamMoveDir::LEFT);
}

void Previewer::CameraController::moveRight(bool move) {
    if (move)
        m_camMovement.set(CamMoveDir::RIGHT);
    else
        m_camMovement.unset(CamMoveDir::RIGHT);
}

void Previewer::CameraController::clearMovement() {
    m_camMovement = CamMoveDir::NONE;
}

void Previewer::CameraController::beginRotation
(
    const Math::Vec3& rotation,
    const Math::Vec2& mousePos
) {
    m_mousePosBeginRot = mousePos;
    m_camRotBeginRot = rotation;
}

Math::Vec3 Previewer::CameraController::processMovement(float frameDuration) {
    float speed = m_previewer.m_cameraMoveSpeed * 10.0f;
    float moveDelta = speed * speed * frameDuration * 0.001f;
    Math::Vec3 camRight = normalize(cross(m_previewer.m_camera.dir, m_previewer.m_camera.up));
    if (m_camMovement.check(CamMoveDir::FORWARD))
        m_previewer.m_camera.pos += m_previewer.m_camera.dir * moveDelta;
    if (m_camMovement.check(CamMoveDir::BACKWARD))
        m_previewer.m_camera.pos -= m_previewer.m_camera.dir * moveDelta;
    if (m_camMovement.check(CamMoveDir::LEFT))
        m_previewer.m_camera.pos -= camRight * moveDelta;
    if (m_camMovement.check(CamMoveDir::RIGHT))
        m_previewer.m_camera.pos += camRight * moveDelta;
    return m_previewer.m_camera.pos;
}

Math::Vec3 Previewer::CameraController::rotate(const Math::Vec2& mousePos) {
    constexpr float ROTATE_SPEED = 0.5f;
    Math::Vec3 offset = Math::Vec3(mousePos - m_mousePosBeginRot, 0.0f) * ROTATE_SPEED;
    offset = Math::Vec3(offset.y, -offset.x, 0.0f);
    Math::Vec3 rotation = m_camRotBeginRot + offset;
    m_previewer.setCameraQuick(m_previewer.m_camera.pos, rotation);
    return rotation;
}
