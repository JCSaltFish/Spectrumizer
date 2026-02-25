/**
 * @file Previewer.h
 * @brief Header file for the Previewer class.
 */

#pragma once

#include "utils/Mesh.h"
#include "utils/Flags.hpp"
#include "gfx/GfxPub.h"
#include "app/AppDataManager.h"

/**
 * @brief Class for previewing 3D models and scenes.
 */
class Previewer {
private:
    /**
     * @brief Struct representing a camera in the scene.
     */
    struct Camera {
        Math::Vec3 pos = {}; // Camera position
        Math::Vec3 rot = {}; // Camera rotation (Euler angles)
        Math::Vec3 dir = {}; // Camera direction
        Math::Vec3 up = {}; // Camera up vector
    };
    /**
     * @brief Struct representing texture maps for a material.
     */
    struct Textures {
        GfxImage normal = nullptr; // Normal texture
        GfxImage roughness = nullptr; // Roughness texture
        GfxImage temperature = nullptr; // temperature texture
    };
    using MaterialFlag = PtMaterial::MaterialFlag;
    /**
     * @brief Struct representing a material for a mesh.
     */
    struct Material {
        DB::ID id = 0; // Unique ID of the material
        float roughness = 0.0f; // Roughness value
        Flags<MaterialFlag> flags = {}; // Material property flags
        Textures textures = {}; // Material texture maps
    };
    /**
     * @brief Struct representing a mesh in a model.
     */
    struct Mesh {
        DB::ID id = 0; // Unique ID of the mesh
        GfxBuffer vertexBuffer = nullptr; // Vertex buffer
        GfxBuffer indexBuffer = nullptr; // Index buffer
        GfxVAO vao = nullptr; // Vertex Array Object
        int indexCount = 0; // Number of indices
        GfxDescriptorSetBinding descriptorSetBinding = nullptr; // Descriptor set binding
        GfxBuffer uboMaterial = nullptr; // Uniform buffer for material properties
        GfxBuffer uboPickInfo = nullptr; // Uniform buffer for picking information
        Material material = {}; // Material of the mesh
    };
    /**
     * @brief Struct representing a 3D model.
     */
    struct Model {
        DB::ID id = 0; // Unique ID of the model
        Math::Vec3 location = {}; // Model location
        Math::Vec3 rotation = {}; // Model rotation (Euler angles)
        Math::Vec3 scale = {}; // Model scale
        GfxDescriptorSetBinding descriptorSetBinding = nullptr; // Descriptor set binding
        GfxBuffer uboXfrom = nullptr; // Uniform buffer for transformation matrices
        std::vector<Mesh> meshes = {}; // Meshes in the model
    };

    /* Uniform buffer object structures */

    /**
     * @brief Uniform struct for model, view, and projection matrices.
     */
    struct UXfrom {
        Math::Mat4 proj; // Projection matrix
        Math::Mat4 view; // View matrix
        Math::Mat4 model; // Model matrix
    };
    /**
     * @brief Uniform struct for camera position.
     */
    struct UCamera {
        Math::Vec3 posW; // Camera position in world space
    };
    /**
     * @brief Uniform struct for material properties.
     */
    struct UMaterial {
        Math::Vec4 diffuse; // Diffuse color
        float roughness; // Roughness value
        uint32_t flags; // Material flags
    };
    /**
     * @brief Uniform struct for object picking information.
     */
    struct UPickInfo {
        uint32_t modelID; // ID of the model
        uint32_t meshID; // ID of the mesh
    };

public:
    explicit Previewer(GfxRenderer& renderer);

    /**
     * @brief Initialize the previewer with given resolution and MSAA samples.
     * @param resX Horizontal resolution.
     * @param resY Vertical resolution.
     * @param samples Number of MSAA samples.
     * @return 0 on success, non-zero on failure.
     */
    int init(int resX, int resY, int samples = 1);
    /**
     * @brief Terminate the previewer and release resources.
     */
    void term();

    /**
     * @brief Get the color frame image.
     * @return The color frame image (GfxImage).
     */
    GfxImage getFrameImage() const;
    /**
     * @brief Set the number of MSAA samples.
     * @param samples The number of samples to set.
     * @return 0 on success, non-zero on failure.
     */
    int setMSAASamples(int samples);

    /**
     * @brief Load a scene from the database.
     * @param hScene Handle to the scene object in the database.
     * @return 0 on success, non-zero on failure.
     */
    int loadScene(const DbObjHandle& hScene);
    /**
     * @brief Update models and materials based on the provided object handles.
     * @param hObjects Vector of object handles to update.
     * @param[out] resolutionChanged Output flag indicating if the resolution has changed.
     */
    void updateObjects(
        const std::vector<DbObjHandle>& hObjects,
        bool& resolutionChanged
    );
    void updateObjects(const std::vector<DbObjHandle>& hObjects);
    /**
     * @brief Clear the current scene and release resources.
     */
    void clearScene();

    /**
     * @brief Quickly set the camera position and rotation.
     * @param position New camera position.
     * @param rotation New camera rotation (Euler angles).
     */
    void setCameraQuick(const Math::Vec3& position, const Math::Vec3& rotation);
    /**
     * @brief Quickly set the rendering resolution.
     * @param resX New horizontal resolution.
     * @param resY New vertical resolution.
     * @return True if the resolution was changed, false otherwise.
     */
    bool setResolutionQuick(int resX, int resY);
    /**
     * @brief Struct for quickly setting model transformation.
     */
    struct ModelXform {
        std::optional<Math::Vec3> location = std::nullopt; // Model location
        std::optional<Math::Vec3> rotation = std::nullopt; // Model rotation (Euler angles)
        std::optional<Math::Vec3> scale = std::nullopt; // Model scale
    };
    /**
     * @brief Quickly set the transformation of a model.
     * @param hModel Handle to the model object in the database.
     * @param info Struct containing transformation information (location, rotation, scale).
     */
    void setModelXformQuick(const DbObjHandle& hModel, const ModelXform& info);
    /**
     * @brief Struct for quickly setting material properties.
     */
    struct MaterialInfo {
        std::optional<float> roughness = std::nullopt; // Roughness value
    };
    /**
     * @brief Quickly set the material properties of a mesh.
     * @param hMesh Handle to the mesh object in the database.
     * @param info Struct containing material information (colors, roughness, flags).
     */
    void setMeshMaterialQuick(const DbObjHandle& hMesh, const MaterialInfo& info);

    /**
     * @brief Get the mesh at the specified pixel coordinates.
     * @param x X coordinate of the pixel.
     * @param y Y coordinate of the pixel.
     * @return Handle to the mesh object at the specified pixel.
     */
    DbObjHandle getMeshAtPixel(int x, int y) const;
    /**
     * @brief Enum for highlight states.
     */
    enum class HightlightState {
        HOVERED,
        PICKED,
    };
    /**
     * @brief Set highlight colors for hovered and picked objects.
     * @param hovered Color for hovered objects.
     * @param picked Color for picked objects.
     */
    void setHighlightColors(const Math::Vec3& hovered, const Math::Vec3& picked);
    /**
     * @brief Highlight or unhighlight an object.
     * @note Highlighted objects are cleared after each render call.
     * @param hObj Handle to the object to highlight.
     * @param state Highlight state (HOVERED, PICKED).
     */
    void hightlightObject(const DbObjHandle& hObj, HightlightState state);

    /**
     * @brief Set the background color of the previewer.
     * @param color New background color.
     */
    void setBackgroundColor(const Math::Vec3& color);

    /**
     * @brief Set the camera movement speed.
     * @param speed New camera movement speed.
     */
    void setCameraMoveSpeed(float speed);

    /**
     * @brief Render a frame in the previewer.
     * @return 0 on success, non-zero on failure.
     */
    int renderFrame();

    /**
     * @brief Count the total number of triangles in the scene.
     * @return Total triangle count.
     */
    int countTriangles() const;

    /**
     * @brief Class for controlling the camera.
     */
    class CameraController {
    public:
        CameraController(Previewer& previewer) : m_previewer(previewer) {};

        /**
         * @brief Set or unset the forward movement flag.
         * @param move True to set the flag, false to unset.
         */
        void moveForward(bool move);
        /**
         * @brief Set or unset the backward movement flag.
         * @param move True to set the flag, false to unset.
         */
        void moveBackward(bool move);
        /**
         * @brief Set or unset the left movement flag.
         * @param move True to set the flag, false to unset.
         */
        void moveLeft(bool move);
        /**
         * @brief Set or unset the right movement flag.
         * @param move True to set the flag, false to unset.
         */
        void moveRight(bool move);
        /**
         * @brief Clear all current movement flags.
         */
        void clearMovement();
        /**
         * @brief Begin camera rotation based on mouse position.
         */
        void beginRotation(const Math::Vec3& rotation, const Math::Vec2& mousePos);
        /**
         * @brief Process camera movement based on current movement flags.
         * @param frameDuration Duration of the current frame in milliseconds.
         * @return Camera position after the movement.
         */
        Math::Vec3 processMovement(float frameDuration);
        /**
         * @brief Process camera rotation based on mouse position.
         * @param mousePos Current mouse position.
         * @return Camera rotation after the rotation.
         */
        Math::Vec3 rotate(const Math::Vec2& mousePos);

    private:
        /**
         * @brief Enum for camera movement directions.
         */
        enum class CamMoveDir {
            NONE = 0,
            FORWARD = 1 << 0,
            BACKWARD = 1 << 1,
            LEFT = 1 << 2,
            RIGHT = 1 << 3,
        };
        Flags<CamMoveDir> m_camMovement = CamMoveDir::NONE; // Current camera movement flags
        Math::Vec2 m_mousePosBeginRot; // Mouse position at the beginning of rotation
        Math::Vec3 m_camRotBeginRot; // Camera rotation at the beginning of rotation
        Previewer& m_previewer; // Reference to the previewer
    };
    /**
     * @brief Get the camera controller.
     * @return Reference to the CameraController instance.
     */
    CameraController& getCameraController();
    /**
     * @brief Get the current camera position.
     * @return Camera position as a Vec3.
     */
    Math::Vec3 getCameraPosition() const;
    /**
     * @brief Get the current camera rotation.
     * @return Camera rotation as a Vec3 (Euler angles).
     */
    Math::Vec3 getCameraRotation() const;

private:
    /**
     * @brief Load a model from the database.
     * @param hModel Handle to the model object in the database.
     * @return 0 on success, non-zero on failure.
     */
    int updateModel(const DbObjHandle& hModel);
    /**
     * @brief Struct for mesh data information.
     */
    struct MeshDataInfo {
        const std::vector<::Mesh::Vertex>& vertices; // Vertex data
        const std::vector<uint32_t>& indices; // Index data
    };
    /**
     * @brief Update a mesh from the database.
     * @param hMesh Handle to the mesh object in the database.
     * @param mesh Reference to the mesh structure to update.
     * @param meshDataInfo Struct containing vertex and index data for the mesh.
     * @return 0 on success, non-zero on failure.
     */
    int updateMesh(const DbObjHandle& hMesh, Mesh& mesh, const MeshDataInfo& meshDataInfo);
    /**
     * @brief Cleanup resources associated with a model.
     * @param model Reference to the model to clean up.
     * @return 0 on success, non-zero on failure.
     */
    int cleanupModel(Model& model);
    /**
     * @brief Remove a model from the previewer.
     * @param hModel Handle to the model object in the database.
     * @return 0 on success, non-zero on failure.
     */
    int removeModel(const DbObjHandle& hModel);
    /**
     * @brief Update a material from the database.
     * @param hMaterial Handle to the material object in the database.
     * @return 0 on success, non-zero on failure.
     */
    int updateMaterial(const DbObjHandle& hMaterial);

    /**
     * @brief Create or recreate the framebuffer with given dimensions and MSAA settings.
     * @param width Framebuffer width.
     * @param height Framebuffer height.
     * @param samplesChanged Whether the MSAA sample count has changed.
     * @return 0 on success, non-zero on failure.
     */
    int initFramebuffer(int width, int height, bool samplesChanged = false);
    /**
     * @brief Handle changes in MSAA sample count.
     * @return 0 on success, non-zero on failure.
     */
    int handleSampleChanged();

private:
    GfxRenderer m_renderer = nullptr; // Reference to the graphics renderer

    int m_MSAAsampleCount = 1; // Number of MSAA samples

    GfxFramebuffer m_framebuffer = nullptr; // Framebuffer for rendering
    GfxImage m_colorFrameImage = nullptr; // Color frame image
    GfxImage m_pickFrameImage = nullptr; // Pick frame image
    GfxImage m_colorFrameResolveImage = nullptr; // Resolved color frame image
    GfxImage m_pickFrameResolveImage = nullptr; // Resolved pick frame image
    GfxImage m_depthFrameImage = nullptr; // Depth frame image
    GfxRenderPass m_renderPass = nullptr; // Render pass for rendering
    GfxPipeline m_pipeline = nullptr; // Graphics pipeline
    GfxDescriptorSetBinding m_descriptorSetBinding = nullptr; // Global descriptor set binding

    GfxBuffer m_uboCamera = nullptr; // Uniform buffer for camera position

    GfxShader m_vertexShader = nullptr; // Vertex shader
    GfxShader m_fragmentShader = nullptr; // Fragment shader
    /**
     * @brief Struct for storing graphics descriptors.
     */
    struct Descriptors {
        GfxDescriptor u_xform = {}; // Descriptor for model/view/projection matrices
        GfxDescriptor u_camera = {}; // Descriptor for camera position
        GfxDescriptor u_material = {}; // Descriptor for material properties
        GfxDescriptor u_normalTex = {}; // Descriptor for normal texture
        GfxDescriptor u_roughnessTex = {}; // Descriptor for roughness texture
        GfxDescriptor u_temperatureTex = {}; // Descriptor for temperature texture
        GfxDescriptor u_pickInfo = {}; // Descriptor for pick information
    };
    Descriptors m_descriptors = {}; // Graphics descriptors
    GfxVertexDesc m_vertexDesc = {}; // Vertex description

    DbObjHandle m_currentScene = {}; // DB Object handle of current scene

    Camera m_camera = {}; // Camera in the scene
    int m_resolutionX = 0; // Horizontal resolution
    int m_resolutionY = 0; // Vertical resolution

    std::unordered_map<DB::ID, Model> m_models = {}; // Loaded models
    std::unordered_map<DB::ID, Mesh*> m_meshLookup = {}; // Lookup for meshes by ID
    std::unordered_map<DB::ID, Material*> m_materialLookup = {}; // Lookup for materials by ID

    Math::Vec3 m_backgroundColor = {}; // Background color
    Math::Vec3 m_highlightColorHovered = {}; // Highlight color for hovered objects
    Math::Vec3 m_highlightColorPicked = {}; // Highlight color for picked objects
    // Map of highlighted objects and their states, cleared after each render call
    std::unordered_map<DB::ID, HightlightState> m_highlightedObjects = {};

    CameraController m_cameraController = { *this }; // Camera controller
    float m_cameraMoveSpeed = 0.3f; // Camera movement speed
};
