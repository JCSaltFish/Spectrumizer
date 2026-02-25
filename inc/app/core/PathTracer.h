/**
 * @file PathTracer.h
 * @brief Header file for the PathTracer class.
 */

#pragma once

#include "utils/Mesh.h"
#include "gfx/GfxPub.h"
#include "app/AppDataManager.h"

/**
 * @brief Class for path tracing rendering.
 */
class PathTracer {
public:
    explicit PathTracer(GfxRenderer& renderer) : m_renderer(renderer) {};

    /**
     * @brief Initialize the path tracer.
     * @return 0 on success, non-zero on failure.
     */
    int init();
    /**
     * @brief Terminate the path tracer.
     */
    void term();

    /**
     * @brief Build the scene for path tracing.
     * @param hScene Handle to the scene object.
     * @return 0 on success, non-zero on failure.
     */
    int buildScene(const DbObjHandle& hScene);
    /**
     * @brief Clear the current scene.
     */
    void clearScene();

    /**
     * @brief Render a frame using the path tracer.
     * @return 0 on success, non-zero on failure.
     */
    int renderFrame();

    /**
     * @brief Get the current sample count.
     * @return Current sample count.
     */
    uint32_t getCurrentSample() const;

    /**
     * @brief Get the current display image (front buffer).
     * @return Current display image.
     */
    GfxBuffer getCurrentDisplayImage() const;
    /**
     * @brief Get the display images (front and back buffers).
     * @return Array containing front and back display images.
     */
    std::array<GfxBuffer, 2> getDisplayImages() const;
    /**
     * @brief Mark the display image as ready for presentation.
     */
    void markDisplayImageReady();
    /**
     * @brief Synchronize the display image by swapping front and back buffers if needed.
     */
    void syncDisplayImage();

    /**
     * @brief Get the image data from the output image.
     * @param[out] pixels Vector to store the pixel data.
     * @param[out] width Output parameter for the image width.
     * @param[out] height Output parameter for the image height.
     * @param[out] nWaves Output parameter for the number of spectral waves.
     * @return 0 on success, non-zero on failure.
     */
    int getImageData(std::vector<float>& pixels, int& width, int& height, int& nWaves) const;

    /* Rendering controls */

    /**
     * @brief Start rendering.
     */
    void render();
    /**
     * @brief Pause rendering.
     */
    void pause();
    /**
     * @brief Stop rendering.
     */
    void stop();
    /**
     * @brief Restart rendering.
     */
    void restart();

    /**
     * @brief Check if rendering is in progress.
     * @return True if rendering, false otherwise.
     */
    bool isRendering() const;
    /**
     * @brief Check if rendering is paused.
     * @return True if paused, false otherwise.
     */
    bool isPaused() const;

    /**
     * @brief Callback function to be called when rendering finishes.
     */
    void renderFinishCallback();
    /**
     * @brief Set the callback function to be called when rendering finishes.
     * @param cb Callback function.
     */
    void setRenderFinishCallback(const std::function<void()>& cb);

private:
    struct BufferData;
    /**
     * @brief Load models from the scene into buffer data.
     * @param hScene Handle to the scene object.
     * @param hSpMaterialIdxMap Map of spectrum material handles to their indices.
     * @param[out] data Reference to buffer data to populate.
     */
    void loadModels(
        const DbObjHandle& hScene,
        const std::unordered_map<DbObjHandle, uint32_t>& hSpMaterialIdxMap,
        BufferData& data
    );
    /**
     * @brief Create GPU buffers from the buffer data.
     * @param data Reference to buffer data.
     * @return 0 on success, non-zero on failure.
     */
    int createBuffers(const BufferData& data);

    /**
     * @brief Build the spectral scene for path tracing.
     * @param hScene Handle to the scene object.
     * @param[out] hSpMaterialIdxMap Map of spectrum material handles to their indices.
     * @return 0 on success, non-zero on failure.
     */
    int buildSpectralScene(
        const DbObjHandle& hScene,
        std::unordered_map<DbObjHandle, uint32_t>& hSpMaterialIdxMap
    );

private:
    GfxRenderer m_renderer = nullptr; // Graphics renderer

    GfxBuffer m_outImage = nullptr; // Output image

    GfxBuffer m_dspImageFront = nullptr; // Display image front buffer
    GfxBuffer m_dspImageBack = nullptr; // Display image back buffer
    std::atomic<bool> m_dspImgSwapPending = false; // Display image swap pending flag

    GfxPipeline m_pipeline = nullptr; // Compute pipeline
    GfxDescriptorSetBinding m_descriptorSetBinding = nullptr; // Descriptor set binding

    GfxBuffer m_uboScene = nullptr; // Scene uniform buffer
    GfxBuffer m_uboCamera = nullptr; // Camera uniform buffer
    GfxBuffer m_uboSpScene = nullptr; // Spectral scene uniform buffer

    GfxBuffer m_ssboVertex = nullptr; // Vertex buffer
    GfxBuffer m_ssboTriangle = nullptr; // Triangle buffer
    GfxBuffer m_ssboMaterial = nullptr; // Material buffer
    GfxBuffer m_ssboBVH = nullptr; // BVH buffer
    GfxBuffer m_ssboWaves = nullptr; // Waves buffer
    GfxBuffer m_ssboSpMaterials = nullptr; // Spectrum materials buffer

    GfxShader m_computeShader = nullptr; // Compute shader
    /**
     * @brief Struct for storing graphics descriptors.
     */
    struct Descriptors {
        GfxDescriptor b_outRadiances = {}; // Output radiances buffer descriptor
        GfxDescriptor u_scene = {}; // Scene descriptor
        GfxDescriptor u_camera = {}; // Camera descriptor
        GfxDescriptor u_textures = {}; // Textures descriptor
        GfxDescriptor b_vertices = {}; // Vertex buffer descriptor
        GfxDescriptor b_triangles = {}; // Triangle buffer descriptor
        GfxDescriptor b_materials = {}; // Material buffer descriptor
        GfxDescriptor b_BVH = {}; // BVH buffer descriptor
        GfxDescriptor u_spScene = {}; // Spectral scene descriptor
        GfxDescriptor b_waves = {}; // Waves buffer descriptor
        GfxDescriptor b_spMaterials = {}; // Spectrum materials descriptor
    } m_descriptors = {}; // Descriptors

    int m_resolutionX = 1024; // Resolution in X
    int m_resolutionY = 768; // Resolution in Y
    uint32_t m_currentSample = 0; // Current sample count

    bool m_rendering = false; // Rendering flag

    std::function<void(void)> m_renderFinishCb = nullptr; // Render finish callback

    int m_nWaves = 0; // Number of waves (for spectral rendering)

    /* Internal structures definitions */
private:
    /* Uniform buffer object structures */

    /**
     * @brief Uniform struct representing the scene parameters.
     */
    struct UScene {
        int resX = 1024; // Resolution in X
        int resY = 768; // Resolution in Y
        int traceDepth = 3; // Trace depth
        int currentSample = 0; // Current sample count
    };
    /**
     * @brief Uniform struct representing the camera parameters.
     */
    struct UCamera {
        Math::Vec4 pos = Math::Vec3(0.0f, 0.0f, -10.0f); // Camera position
        Math::Vec4 dir = Math::Vec3(0.0f, 0.0f, 1.0f); // Camera direction
        Math::Vec4 up = Math::Vec3(0.0f, 1.0f, 0.0f); // Camera up vector
        float focal = PtScene::Camera::FOCAL; // Focal length
        float fov = PtScene::Camera::FOV; // Field of view
        float focusDist = 5.0f; // Focus distance
        float fStop = 32.0f; // F-stop value
    };

    /**
     * @brief Uniform struct representing the spectral scene parameters.
     */
    struct USpScene {
        int nWaves = 0; // Number of waves
        uint32_t idxSkyMaterial = 0; // Index of the sky spectrum material
        float skyTemperature = 0.0f; // Sky temperature in Celsius
    };

    /* GPU buffer structures */

    /**
     * @brief Struct representing a vertex in the mesh.
     */
    struct Vertex {
        Math::Vec4 pos = {}; // Position
        Math::Vec4 normal = {}; // Normal
        Math::Vec4 tangent = {}; // Tangent
        Math::Vec2 texCoord = {}; // Texture Coordinate
        Math::Vec2 padding = {}; // Padding for alignment
    };
    /**
     * @brief Struct representing a material for a mesh.
     */
    struct Material {
        int type = 0; // Material type
        float roughness = 1.0f; // Roughness
        float temperature = 0.0f; // Temperature
        float ior = 1.5f; // Index of Refraction

        uint32_t flags = 0; // Material flags
        uint32_t idxNormalTex = 0; // Index of normal texture
        uint32_t idxRoughnessTex = 0; // Index of roughness texture
        uint32_t idxTemperatureTex = 0; // Index of temperature texture

        uint32_t idxSpMaterial = 0; // Index of associated spectrum material
        uint32_t padding[3] = {}; // Padding for alignment
    };
    /**
     * @brief Struct representing a triangle in the mesh.
     */
    struct Triangle {
        uint32_t v0 = 0; // Index of vertex 0
        uint32_t v1 = 0; // Index of vertex 1
        uint32_t v2 = 0; // Index of vertex 2
        uint32_t idxMaterial = 0; // Index of the material
    };
    /**
     * @brief Struct representing a BVH node in the GPU buffer.
     */
    struct BufferBvhNode {
        uint32_t idx = 0; // Index of this node
        uint32_t rChildOffset = 0; // Offset to the right child node
        uint32_t idxTriangle = 0; // Index of the triangle (if leaf node)
        uint32_t leafFlag = 0; // Flag indicating if this is a leaf node

        Math::Vec4 aabbMin = {}; // Minimum AABB coordinates
        Math::Vec4 aabbMax = {}; // Maximum AABB coordinates
    };

    /**
     * @brief Struct for holding all buffer data.
     */
    struct BufferData {
        std::vector<Vertex> vertices = {}; // Vertices
        std::vector<Triangle> triangles = {}; // Triangles
        std::vector<Material> materials = {}; // Materials
        std::vector<GfxImage> textures = {}; // Textures
        std::vector<BufferBvhNode> bvhBufferData = {}; // BVH buffer data
    };

    /* BVH structures */

    /**
     * @brief Axis-Aligned Bounding Box (AABB) structure.
     */
    class AABB {
    public:
        AABB() = default;
        AABB(const Math::Vec3& min, const Math::Vec3& max) : m_min(min), m_max(max) {};

    public:
        /**
         * @brief Get the minimum coordinates of the AABB.
         * @return Minimum coordinates as a Vec3.
         */
        const Math::Vec3& min() const { return m_min; };
        /**
         * @brief Get the maximum coordinates of the AABB.
         * @return Maximum coordinates as a Vec3.
         */
        const Math::Vec3& max() const { return m_max; };

    public:
        /**
         * @brief Merge a point into the AABB.
         * @param p Point to merge.
         */
        void merge(const Math::Vec3& p) {
            m_min.x = std::min(m_min.x, p.x);
            m_min.y = std::min(m_min.y, p.y);
            m_min.z = std::min(m_min.z, p.z);
            m_max.x = std::max(m_max.x, p.x);
            m_max.y = std::max(m_max.y, p.y);
            m_max.z = std::max(m_max.z, p.z);
        };
        /**
         * @brief Merge another AABB into this AABB.
         * @param aabb AABB to merge.
         */
        void merge(const AABB& aabb) {
            m_min.x = std::min(m_min.x, aabb.m_min.x);
            m_min.y = std::min(m_min.y, aabb.m_min.y);
            m_min.z = std::min(m_min.z, aabb.m_min.z);
            m_max.x = std::max(m_max.x, aabb.m_max.x);
            m_max.y = std::max(m_max.y, aabb.m_max.y);
            m_max.z = std::max(m_max.z, aabb.m_max.z);
        };
        /**
         * @brief Validate the AABB to ensure min is less than max.
         */
        void validate() {
            if (m_min.x >= m_max.x)
                m_max.x = m_min.x + std::numeric_limits<float>::epsilon();
            if (m_min.y >= m_max.y)
                m_max.y = m_min.y + std::numeric_limits<float>::epsilon();
            if (m_min.z >= m_max.z)
                m_max.z = m_min.z + std::numeric_limits<float>::epsilon();
        };
        /**
         * @brief Calculate the surface area of the AABB.
         * @return Surface area as a float.
         */
        float surfaceArea() const {
            const Math::Vec3 e = m_max - m_min;
            return 2.0f * (e.x * e.y + e.y * e.z + e.z * e.x);
        };

    private:
        Math::Vec3 m_min = Math::Vec3(std::numeric_limits<float>::max()); // Minimum coordinates
        Math::Vec3 m_max = Math::Vec3(std::numeric_limits<float>::lowest()); // Maximum coordinates
    };
    /**
     * @brief Struct representing a BVH node.
     */
    struct BvhNode {
        AABB aabb = {}; // Axis-Aligned Bounding Box
        uint32_t idxTriangle = 0; // Index of the triangle (if leaf node)
        std::unique_ptr<BvhNode> left = nullptr; // Left child node
        std::unique_ptr<BvhNode> right = nullptr; // Right child node
    };
    /**
     * @brief Class for building the BVH.
     */
    class BvhBuilder {
    public:
        /**
         * @brief Build the BVH from the given vertices and triangles.
         * @param vertices Vector of vertices.
         * @param triangles Vector of triangles.
         * @return Unique pointer to the root BVH node.
         */
        std::unique_ptr<BvhNode> build(
            const std::vector<Vertex>& vertices,
            const std::vector<Triangle>& triangles
        );

    private:
        /**
         * @brief Recursive function to build the BVH.
         * @param node Current BVH node.
         * @param triListOffset Offset in the triangle list.
         * @param triCount Number of triangles.
         */
        void buildRecursive(BvhNode* node, size_t triListOffset, size_t triCount);

    private:
        std::vector<uint32_t> m_triList = {}; // List of triangle indices
        std::vector<AABB> m_triAABBs = {}; // List of triangle AABBs
    };
    /**
     * @brief Class for bufferizing the BVH for GPU usage.
     */
    class BvhBufferizer {
    public:
        /**
         * @brief Bufferize the BVH starting from the root node.
         * @param root Root BVH node.
         * @return Vector of BufferBvhNode for GPU usage.
         */
        std::vector<BufferBvhNode> bufferize(BvhNode* root);

    private:
        /**
         * @brief Recursive function to bufferize the BVH.
         * @param node Current BVH node.
         */
        void bufferizeRecursive(BvhNode* node);

    private:
        std::vector<BufferBvhNode> m_bufferData = {}; // Buffer data for GPU
    };
};
