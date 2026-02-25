/**
 * @file GfxGLRenderer.h
 * @brief OpenGL implementation of the GfxRenderer interface.
 */

#pragma once

#include <glad/glad.h>

#include "gfx/GfxPr.h"

 /**
  * @brief OpenGL implementation of GfxImage.
  */
class GfxGLImage : public GfxImage_T {
public:
    explicit GfxGLImage(const GfxImageInfo& info) :
        GfxImage_T(info)
    {};

public:
    GLuint m_texture = 0; // OpenGL texture object
    int m_samples = 1; // Number of samples for multisampling
};

/**
 * @brief OpenGL implementation of GfxRenderPass.
 */
class GfxGLRenderPass : public GfxRenderPass_T {
public:
    GfxGLRenderPass(
        const std::vector<GfxAttachment>& colorAttachments,
        const GfxAttachment& depthAttachment
    ) :
        GfxRenderPass_T(colorAttachments, depthAttachment)
    {};

public:
    GLuint m_fbo = 0; // OpenGL framebuffer object
    GLuint m_msaaFbo = 0; // OpenGL multisample framebuffer object
    int m_samples = 0; // Number of samples for multisampling
};

/**
 * @brief OpenGL implementation of GfxBuffer.
 */
class GfxGLBuffer : public GfxBuffer_T {
public:
    GfxGLBuffer(
        int size,
        GfxBufferUsage usage,
        GfxBufferProp prop
    ) :
        GfxBuffer_T(size, usage, prop)
    {};

    void setSize(int size) { m_size = size; };

public:
    GLuint m_buffer = 0; // OpenGL buffer object
};

/**
 * @brief OpenGL implementation of GfxVAO.
 */
class GfxGLVAO : public GfxVAO_T {
public:
    GfxGLVAO(
        const GfxVertexDesc& vertexDesc,
        const GfxBuffer& vertexBuffer,
        const GfxBuffer& indexBuffer
    ) :
        GfxVAO_T(vertexDesc, vertexBuffer, indexBuffer)
    {};

public:
    GLuint m_vao = 0; // OpenGL Vertex Array Object
};

/**
 * @brief OpenGL implementation of GfxShader.
 */
class GfxGLShader : public GfxShader_T {
public:
    explicit GfxGLShader(GfxShaderStage stage) : GfxShader_T(stage) {};

public:
    GLuint m_shader = 0; // OpenGL shader object
};

/**
 * @brief OpenGL implementation of GfxDescriptorSetBinding.
 */
class GfxGLDescriptorSetBinding : public GfxDescriptorSetBinding_T {
public:
    GfxGLDescriptorSetBinding(
        const GfxPipeline& pipeline,
        int descriptorSetIndex,
        const std::vector<GfxDescriptorBinding>& bindings
    ) :
        GfxDescriptorSetBinding_T(pipeline, descriptorSetIndex, bindings)
    {};

public:
    // Map of binding index to OpenGL SSBO for bindless textures
    std::map<int, GLuint> m_bindingSamplesSsboMap;
};

/**
 * @brief OpenGL implementation of GfxPipeline.
 */
class GfxGLPipeline : public GfxPipeline_T {
public:
    GfxGLPipeline(
        const GfxRenderPass& renderPass,
        const std::vector<GfxDescriptorSet>& descriptorSets,
        const std::vector<GfxPipelineState>& dynamicStates
    ) :
        GfxPipeline_T(renderPass, descriptorSets, dynamicStates)
    {};

public:
    GLuint m_program = 0; // OpenGL program object
};

/**
 * @brief OpenGL implementation of GfxRenderer.
 */
class GfxGLRenderer : public GfxRendererInterface {
public:
    GfxGLRenderer();
    ~GfxGLRenderer() = default;

public:
    static int initGlobal(const GfxRendererConfig& config);
    static void termGlobal() {};

    void waitDeviceIdle() const override;

    int initForImGui(const std::function<void(void*)>& initFunc) override;
    void termForImGui(const std::function<void()>& termFunc) override;
    void renderForImGui(const std::function<void(void*)>& renderFunc) const override;
    ImageInfo getImageInfo(const GfxImage& image) const override;

    void setSamples(int samples) override {};

    GfxImage createImage(const GfxImageInfo& info) const override;
    int setImageData(const GfxImage& image, void* data) const override;
    int getImageData(const GfxImage& image, void* data) const override;
    int generateMipmaps(const GfxImage& image) const override;
    void copyImage(const GfxImage& src, const GfxImage& dst, int width, int height) override;
    void destroyImage(const GfxImage& image) const override;

    GfxRenderPass createRenderPass(
        const std::vector<GfxAttachment>& colorAttachments,
        const GfxAttachment& depthAttachment
    ) const override;
    void destroyRenderPass(const GfxRenderPass& renderPass) const override;

    GfxFramebuffer createFramebuffer(
        const GfxRenderPass& renderPass,
        const std::vector<GfxImage>& colorImages,
        const GfxImage& depthImage,
        const std::vector<GfxImage>& colorResolveImages
    ) const override;
    void destroyFramebuffer(const GfxFramebuffer& framebuffer) const override {};
    int readFramebufferColorAttachmentPixels(
        const GfxFramebuffer& framebuffer,
        int index,
        const GfxRect& rect,
        void* pixels
    ) const override;

    GfxBuffer createBuffer(
        int size,
        GfxBufferUsage usage,
        GfxBufferProp prop
    ) const override;
    int setBufferData(const GfxBuffer& buffer, int size, const void* data) const override;
    int updateBufferData(
        const GfxBuffer& buffer,
        int offset,
        int size,
        const void* data
    ) const override;
    void destroyBuffer(const GfxBuffer& buffer) const override;
    int readBufferData(
        const GfxBuffer& buffer,
        int offset,
        int size,
        void* data
    ) const override;
    int copyBuffer(
        const GfxBuffer& src,
        const GfxBuffer& dst,
        int srcOffset,
        int dstOffset,
        int size
    ) const override;

    GfxVAO createVAO(
        const GfxVertexDesc& vertexDesc,
        const GfxBuffer& vertexBuffer,
        const GfxBuffer& indexBuffer
    ) const override;
    void destroyVAO(const GfxVAO& vao) const override;

    GfxShader createShader(
        GfxShaderStage stage,
        const std::string& source
    ) const override;
    void destroyShader(const GfxShader& shader) const override;

    GfxPipeline createPipeline(
        const std::vector<GfxShader>& shaders,
        const std::vector<GfxDescriptorSet>& descriptorSets,
        const GfxVertexDesc& vertexDesc,
        const std::vector<GfxPipelineState>& dynamicStates,
        const GfxRenderPass& renderPass
    ) const override;
    void destroyPipeline(const GfxPipeline& pipeline) const override;

    GfxDescriptorSetBinding createDescriptorSetBinding(
        const GfxPipeline& pipeline,
        int descriptorSetIndex,
        const std::vector<GfxDescriptorBinding>& bindings
    ) const override;
    void destroyDescriptorSetBinding(GfxDescriptorSetBinding& binding) const override;

    int beginRenderPass(const GfxFramebuffer& framebuffer) override;
    void endRenderPass() override;

    void bindPipeline(const GfxPipeline& pipeline) override;
    void bindVAO(const GfxVAO& vao) override;

    void clearColorAttachment(int index, const std::array<float, 4>& value) override;
    void clearDepthAttachment(float value) override;
    void clearStencilAttachment(int value) override;

    void bindDescriptorSetBinding(const GfxDescriptorSetBinding& binding) override;

    int beginFrame() override { return 0; };
    int endFrame() override { return 0; };

    void draw(int nVertices, int nInstances, int firstVertex, int firstInstance) override;
    void drawIndexed(
        int nIndices,
        int nInstances,
        int firstIndex,
        int vertexOffset,
        int firstInstance
    ) override;
    void drawIndirect(
        const GfxBuffer& buffer,
        int offset,
        int drawCount,
        int stride
    ) override;
    void drawIndexedIndirect(
        const GfxBuffer& buffer,
        int offset,
        int drawCount,
        int stride
    ) override;
    void dispatchCompute(int nGroupsX, int nGroupsY, int nGroupsZ) override;
    void dispatchComputeIndirect(const GfxBuffer& buffer, int offset) override;
    void memoryBarrier() override;

private:
    static std::mutex s_mutex; // Mutex for synchronizing access to global OpenGL renderer
};
