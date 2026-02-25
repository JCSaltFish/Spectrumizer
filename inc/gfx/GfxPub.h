/*
 * Copyright 2025 Jed Wang
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file GfxPub.h
 * @brief Public header file for the GFX module.
 */

#pragma once

#include "GfxCommon.h"
#include "GfxFormat.h"
#include "GfxPipelineState.h"

  /**
   * @brief Graphics backend enumeration.
   */
enum class GfxBackend {
    OpenGL,
    Vulkan,
};

/**
 * @brief VSync mode enumeration.
 */
enum class GfxVSyncMode {
    IMMEDIATE,
    FIFO,
    MAILBOX,
};

/**
 * @brief Graphics image usage enumeration.
 */
enum class GfxImageUsage {
    STORAGE_IMAGE = 0,
    SAMPLED_TEXTURE = 1 << 0,
    COLOR_ATTACHMENT = 1 << 1,
    DEPTH_ATTACHMENT = 1 << 2,
};
/**
 * @brief Graphics image filter mode enumeration.
 */
enum class GfxImageFilterMode {
    NEAREST,
    LINEAR,
};
/**
 * @brief Graphics image wrap mode enumeration.
 */
enum class GfxImageWrapMode {
    REPEAT,
    MIRRORED_REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER,
};
/**
 * @brief Graphics image information structure.
 */
struct GfxImageInfo {
    int width = 0; // Width of the image.
    int height = 0; // Height of the image.
    int samples = 1; // Number of samples for multisampling.
    int minLod = 0; // Minimum level of detail.
    int maxLod = 0; // Maximum level of detail.
    float lodBias = 0.0f; // Level of detail bias.
    GfxFormat format = GfxFormat::UNDEFINED; // Format of the image.
    GfxFlags<GfxImageUsage> usages = {}; // Usages of the image.
    GfxImageFilterMode minFilter = GfxImageFilterMode::LINEAR; // Minification filter mode.
    GfxImageFilterMode magFilter = GfxImageFilterMode::LINEAR; // Magnification filter mode.
    GfxImageFilterMode mipmapFilter = GfxImageFilterMode::LINEAR; // Mipmap filter mode.
    GfxImageWrapMode wrapS = GfxImageWrapMode::REPEAT; // Wrap mode for the S coordinate.
    GfxImageWrapMode wrapT = GfxImageWrapMode::REPEAT; // Wrap mode for the T coordinate.
};
/**
 * @brief Graphics image class.
 */
class GfxImage_T {
public:
    explicit GfxImage_T(const GfxImageInfo& info) :
        m_width(info.width),
        m_height(info.height),
        m_samples(info.samples),
        m_format(info.format),
        m_usages(info.usages) {
        double levels = std::floor(std::log2(std::max(info.width, info.height)));
        m_levels = info.samples > 1 ? 1 : static_cast<int>(levels) + 1;
    };
    GfxImage_T(const GfxImage_T&) = delete;
    GfxImage_T& operator=(const GfxImage_T&) = delete;

public:
    /**
     * @brief Get the width of the image.
     * @return Width of the image.
     */
    int getWidth() const { return m_width; };
    /**
     * @brief Get the height of the image.
     * @return Height of the image.
     */
    int getHeight() const { return m_height; };
    /**
     * @brief Get the number of samples for multisampling.
     * @return Number of samples.
     */
    int getSamples() const { return m_samples; };
    /**
     * @brief Get the number of mipmap levels.
     * @return Number of mipmap levels.
     */
    int getLevels() const { return m_levels; };
    /**
     * @brief Get the usage of the image.
     * @return Usage of the image.
     */
    GfxFormat getFormat() const { return m_format; };
    /**
     * @brief Get the usages of the image.
     * @return Usages of the image.
     */
    GfxFlags<GfxImageUsage> getUsages() const { return m_usages; };

protected:
    int m_width = 0; // Width of the image.
    int m_height = 0; // Height of the image.
    int m_samples = 1; // Number of samples for multisampling.
    int m_levels = 1; // Number of mipmap levels.
    GfxFormat m_format = GfxFormat::UNDEFINED; // Format of the image.
    GfxFlags<GfxImageUsage> m_usages = {}; // Usages of the image.
};
using GfxImage = std::shared_ptr<GfxImage_T>;

/**
 * @brief Graphics attachment structure.
 * @note Represents an attachment in a render pass, which can be a color or depth attachment.
 */
struct GfxAttachment {
    int samples = 1; // Number of samples for multisampling.
    GfxFormat format = GfxFormat::UNDEFINED; // Format of the attachment.
    GfxFlags<GfxImageUsage> usages = {}; // Usages of the attachment.
    int level = 0; // Mipmap level of the attachment.
};
/**
 * @brief Graphics render pass class.
 */
class GfxRenderPass_T {
public:
    GfxRenderPass_T(
        const std::vector<GfxAttachment>& colorAttachments,
        const GfxAttachment& depthAttachment
    ) :
        m_colorAttachments(colorAttachments),
        m_depthAttachment(depthAttachment) {};
    GfxRenderPass_T(const GfxRenderPass_T&) = delete;
    GfxRenderPass_T& operator=(const GfxRenderPass_T&) = delete;

public:
    /**
     * @brief Get the color attachments of the render pass.
     * @return Vector of color attachments.
     */
    const std::vector<GfxAttachment>& getColorAttachments() const {
        return m_colorAttachments;
    };
    /**
     * @brief Get the depth attachment of the render pass.
     * @return Depth attachment of the render pass.
     */
    GfxAttachment getDepthAttachment() const { return m_depthAttachment; };

protected:
    std::vector<GfxAttachment> m_colorAttachments = {}; // Color attachments of the render pass.
    GfxAttachment m_depthAttachment = {}; // Depth attachment of the render pass.
};
using GfxRenderPass = std::shared_ptr<GfxRenderPass_T>;

/**
 * @brief Graphics framebuffer class.
 */
class GfxFramebuffer_T {
public:
    GfxFramebuffer_T(
        const GfxRenderPass& renderPass,
        const std::vector<GfxImage>& colorImages,
        const GfxImage& depthImage,
        const std::vector<GfxImage>& colorResolveImages
    ) :
        m_renderPass(renderPass),
        m_colorImages(colorImages),
        m_depthImage(depthImage),
        m_colorResolveImages(colorResolveImages) {};
    GfxFramebuffer_T(const GfxFramebuffer_T&) = delete;
    GfxFramebuffer_T& operator=(const GfxFramebuffer_T&) = delete;

public:
    /**
     * @brief Get the render pass associated with the framebuffer object.
     * @return Render pass of the framebuffer object.
     */
    GfxRenderPass getRenderPass() const { return m_renderPass; };
    /**
     * @brief Get the color images of the framebuffer object.
     * @return Vector of color images.
     */
    const std::vector<GfxImage>& getColorImages() const { return m_colorImages; };
    /**
     * @brief Get the depth image of the framebuffer object.
     * @return Depth image.
     */
    GfxImage getDepthImage() const { return m_depthImage; };
    /**
     * @brief Get the color resolve images of the framebuffer object.
     * @return Vector of color resolve images.
     */
    const std::vector<GfxImage>& getColorResolveImages() const {
        return m_colorResolveImages;
    };

protected:
    GfxRenderPass m_renderPass = nullptr; // Render pass associated with the framebuffer object.
    std::vector<GfxImage> m_colorImages = {}; // Color images.
    GfxImage m_depthImage = nullptr; // Depth image.
    std::vector<GfxImage> m_colorResolveImages = {}; // Color resolve images.
};
using GfxFramebuffer = std::shared_ptr<GfxFramebuffer_T>;

/**
 * @brief Graphics buffer usage enumeration.
 * @note Used to specify the intended usage of a buffer.
 */
enum class GfxBufferUsage {
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER,
};
/**
 * @brief Graphics buffer property enumeration.
 * @note Used to specify the properties of a buffer, such as whether it is static or dynamic.
 */
enum class GfxBufferProp {
    STATIC,
    DYNAMIC,
};
/**
 * @brief Graphics buffer class.
 * @note Represents a buffer in the graphics pipeline, which can be used for various purposes
         such as vertex data, index data, uniform data, etc.
 */
class GfxBuffer_T {
public:
    GfxBuffer_T(
        int size,
        GfxBufferUsage usage,
        GfxBufferProp prop
    ) :
        m_size(size),
        m_usage(usage),
        m_prop(prop) {};
    GfxBuffer_T(const GfxBuffer_T&) = delete;
    GfxBuffer_T& operator=(const GfxBuffer_T&) = delete;

public:
    /**
     * @brief Get the size of the buffer.
     * @return Size of the buffer in bytes.
     */
    int getSize() const { return m_size; };
    /**
     * @brief Get the usage of the buffer.
     * @return Usage of the buffer.
     */
    GfxBufferUsage getUsage() const { return m_usage; };
    /**
     * @brief Get the properties of the buffer.
     * @return Properties of the buffer.
     */
    GfxBufferProp getProp() const { return m_prop; };

protected:
    int m_size = 0; // Size of the buffer in bytes.
    GfxBufferUsage m_usage = GfxBufferUsage::VERTEX_BUFFER; // Usage of the buffer.
    GfxBufferProp m_prop = GfxBufferProp::STATIC; // Properties of the buffer.
};
using GfxBuffer = std::shared_ptr<GfxBuffer_T>;

/**
 * @brief Graphics vertex attribute structure.
 * @note Represents a single vertex attribute in a vertex buffer.
 */
struct GfxVertexAttr {
    int location = -1; // Location of the vertex attribute in the shader.
    GfxFormat format = GfxFormat::UNDEFINED; // Format of the vertex attribute.
    int offset = 0; // Offset of the vertex attribute in the vertex buffer.
};
/**
 * @brief Graphics vertex descriptor structure.
 * @note Contains information about the vertex attributes and their layout in a vertex buffer.
 */
struct GfxVertexDesc {
    int stride = -1; // Stride of the vertex buffer, i.e., the size of a single vertex in bytes.
    std::vector<GfxVertexAttr> attributes = {}; // List of vertex attributes in the vertex buffer.
};
/**
 * @brief Graphics vertex array object class.
 * @note Represents a vertex array object (VAO) in the graphics pipeline, which encapsulates
         vertex buffer and index buffer bindings along with vertex attribute configurations.
 */
class GfxVAO_T {
public:
    GfxVAO_T(
        const GfxVertexDesc& vertexDesc,
        const GfxBuffer& vertexBuffer,
        const GfxBuffer& indexBuffer
    ) :
        m_vertexDesc(vertexDesc),
        m_vertexBuffer(vertexBuffer),
        m_indexBuffer(indexBuffer) {};
    GfxVAO_T(const GfxVAO_T&) = delete;
    GfxVAO_T& operator=(const GfxVAO_T&) = delete;

public:
    /**
     * @brief Get the vertex descriptor of the VAO.
     * @return Vertex descriptor containing attribute information.
     */
    GfxVertexDesc getVertexDesc() const { return m_vertexDesc; };
    /**
     * @brief Get the vertex buffer associated with the VAO.
     * @return Vertex buffer used for vertex data.
     */
    GfxBuffer getVertexBuffer() const { return m_vertexBuffer; };
    /**
     * @brief Get the index buffer associated with the VAO.
     * @return Index buffer used for indexed drawing.
     */
    GfxBuffer getIndexBuffer() const { return m_indexBuffer; };

protected:
    GfxVertexDesc m_vertexDesc = {}; // Vertex descriptor containing attribute information.
    GfxBuffer m_vertexBuffer = nullptr; // Vertex buffer used for vertex data.
    GfxBuffer m_indexBuffer = nullptr; // Index buffer used for indexed drawing.
};
using GfxVAO = std::shared_ptr<GfxVAO_T>;

/**
 * @brief Graphics shader stage enumeration.
 * @note Represents different stages of the graphics pipeline where shaders can be applied.
 */
enum class GfxShaderStage {
    VERTEX = 1 << 0,
    TESS_CTRL = 1 << 1,
    TESS_EVAL = 1 << 2,
    GEOMETRY = 1 << 3,
    FRAGMENT = 1 << 4,
    COMPUTE = 1 << 5,
};
/**
 * @brief Graphics shader object class.
 * @note Represents a shader in the graphics pipeline, which can be of different stages
         such as vertex, fragment, etc.
 */
class GfxShader_T {
public:
    explicit GfxShader_T(GfxShaderStage stage) : m_stage(stage) {};
    GfxShader_T(const GfxShader_T&) = delete;
    GfxShader_T& operator=(const GfxShader_T&) = delete;

public:
    /**
     * @brief Get the shader stage of the shader.
     * @return Shader stage of the shader.
     */
    GfxShaderStage getStage() const { return m_stage; };

protected:
    GfxShaderStage m_stage = GfxShaderStage::VERTEX; // Shader stage of the shader.
};
using GfxShader = std::shared_ptr<GfxShader_T>;

/**
 * @brief Graphics shader exception class.
 * @note Represents exceptions that occur during shader compilation.
 */
class GfxShaderException : public std::exception {
public:
    explicit GfxShaderException(const std::string& message) : m_message(message) {};
    virtual const char* what() const noexcept override {
        return m_message.c_str();
    };
private:
    std::string m_message; // Error message
};

/**
 * @brief Graphics descriptor type enumeration.
 * @note Represents different types of descriptors used in the graphics pipeline.
 */
enum class GfxDescriptorType {
    SAMPLER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER,
    STORAGE_IMAGE,
    SAMPLERS, // Bindless textures
};
/**
 * @brief Graphics descriptor structure.
 * @note Represents a descriptor that can be bound to a shader stage in the graphics pipeline.
 */
struct GfxDescriptor {
    GfxDescriptorType type = GfxDescriptorType::SAMPLER; // Type of the descriptor.
    int binding = 0; // Binding point of the descriptor.
    GfxFlags<GfxShaderStage> stages = {}; // Shader stages.
    int size = 1; // Size of the descriptor array (for bindless descriptors).
};
using GfxDescriptorSet = std::vector<GfxDescriptor>;
using GfxBindingResource = std::variant<GfxImage, GfxBuffer, std::vector<GfxImage>>;
/**
 * @brief Graphics descriptor binding structure.
 * @note Represents a binding of a descriptor to an image or buffer in the graphics pipeline.
 */
struct GfxDescriptorBinding {
    GfxDescriptor descriptor = {}; // Descriptor to bind.
    GfxBindingResource resource = {}; // Resource bound to the descriptor.
};
/**
 * @brief Graphics pipeline class.
 * @note Represents a graphics pipeline that encapsulates shader stages, render pass,
         and dynamic states. It is used to configure how rendering is performed in the graphics
         backend.
 */
class GfxPipeline_T {
    friend class GfxPipelineStateController;

public:
    GfxPipeline_T(
        const GfxRenderPass& renderPass,
        const std::vector<GfxDescriptorSet>& descriptorSets,
        const std::vector<GfxPipelineState>& dynamicStates
    ) :
        m_renderPass(renderPass),
        m_descriptorSets(descriptorSets) {
        m_dynamicStates.insert(dynamicStates.begin(), dynamicStates.end());
    };
    GfxPipeline_T(const GfxPipeline_T&) = delete;
    GfxPipeline_T& operator=(const GfxPipeline_T&) = delete;

public:
    /**
     * @brief Get the shader stages used in the pipeline.
     * @return Flags representing the shader stages.
     */
    GfxFlags<GfxShaderStage> getStages() const { return m_stages; };
    /**
     * @brief Get the render pass associated with the pipeline.
     * @return Render pass of the pipeline.
     */
    GfxRenderPass getRenderPass() const { return m_renderPass; };
    /**
     * @brief Get the pipeline state cache.
     * @return Pipeline state cache.
     */
    std::unordered_set<GfxPipelineState> getDynamicStates() const {
        return m_dynamicStates;
    };
    /**
     * @brief Get the pipeline state cache.
     * @return Pipeline state cache.
     */
    bool hasDynamicState(GfxPipelineState state) const {
        return m_dynamicStates.find(state) != m_dynamicStates.end();
    };
    /**
     * @brief Get the descriptor sets used in the pipeline.
     * @return Vector of descriptor sets.
     */
    std::vector<GfxDescriptorSet> getDescriptorSets() const {
        return m_descriptorSets;
    };

protected:
    GfxFlags<GfxShaderStage> m_stages = {}; // Shader stages used in the pipeline.
    std::vector<GfxDescriptorSet> m_descriptorSets = {}; // Descriptor sets used in the pipeline.
    GfxRenderPass m_renderPass = nullptr; // Render pass associated with the pipeline.
    std::unordered_set<GfxPipelineState> m_dynamicStates = {}; // Dynamic states of the pipeline.
    GfxPipelineStateCache m_stateCache = {}; // Pipeline state cache for dynamic states.
};
using GfxPipeline = std::shared_ptr<GfxPipeline_T>;
/**
 * @brief Graphics descriptor set binding class.
 */
class GfxDescriptorSetBinding_T {
public:
    GfxDescriptorSetBinding_T(
        const GfxPipeline& pipeline,
        int descriptorSetIndex,
        const std::vector<GfxDescriptorBinding>& bindings
    ) :
        m_pipeline(pipeline),
        m_descriptorSetIndex(descriptorSetIndex),
        m_bindings(bindings) {};
    GfxDescriptorSetBinding_T(const GfxDescriptorSetBinding_T&) = delete;
    GfxDescriptorSetBinding_T& operator=(const GfxDescriptorSetBinding_T&) = delete;

public:
    /**
     * @brief Get the graphics pipeline associated with the descriptor set binding.
     * @return The graphics pipeline.
     */
    GfxPipeline getPipeline() const {
        return m_pipeline;
    };
    /**
     * @brief Get the index of the descriptor set in the pipeline.
     * @return The descriptor set index.
     */
    int getDescriptorSetIndex() const {
        return m_descriptorSetIndex;
    };
    /**
     * @brief Get the descriptor bindings.
     * @return Vector of descriptor bindings.
     */
    std::vector<GfxDescriptorBinding> getDescriptorBindings() const {
        return m_bindings;
    };

private:
    GfxPipeline m_pipeline = nullptr; // Associated graphics pipeline.
    int m_descriptorSetIndex = 0; // Descriptor set index in the pipeline.
    std::vector<GfxDescriptorBinding> m_bindings = {}; // Descriptor bindings.
};
using GfxDescriptorSetBinding = std::shared_ptr<GfxDescriptorSetBinding_T>;

/**
 * @brief Graphics renderer interface.
 * @note This interface defines the methods that a graphics renderer must implement.
 */
class GfxRendererInterface {
protected:
    GfxRendererInterface() = default;
    GfxRendererInterface(const GfxRendererInterface&) = delete;
    GfxRendererInterface& operator=(const GfxRendererInterface&) = delete;
public:
    virtual ~GfxRendererInterface() = default;

public:
    /**
     * @brief Get the graphics backend used by the renderer.
     * @return The graphics backend (OpenGL or Vulkan).
     */
    GfxBackend getBackend() const;
    /**
     * @brief Get the pipeline state machine associated with the renderer.
     * @return The GfxPipelineStateMachine instance.
     */
    GfxPipelineStateMachine getPipelineStateMachine() const;

    /**
     * @brief [Vulkan specific]
              Set the VSync mode for the renderer.
     * @param mode The VSync mode to set (IMMEDIATE, FIFO, MAILBOX).
     */
    virtual void setVSyncMode(GfxVSyncMode mode) {};

    /**
     * @brief [Vulkan specific]
     *        Get the Vulkan instance associated with the renderer.
     * @return Pointer to the Vulkan instance, or nullptr if not applicable.
     */
    virtual void* getVulkanInstance() const { return nullptr; };
    /**
     * @brief [Vulkan specific]
     *        Set the Vulkan surface for the renderer.
     * @param surface The Vulkan surface to set.
     */
    virtual void setVulkanSurface(void* surface) {};
    /**
     * @brief [Vulkan specific]
     *        Set the size of the swapchain.
     * @param width The width of the swapchain.
     * @param height The height of the swapchain.
     * @return 0 on success, non-zero on failure.
     */
    virtual int setSwapchainSize(int width, int height) { return 0; };
    /**
     * @brief Wait for the device to become idle.
     * @note This is a blocking call that waits until all pending operations on the
     *       device are complete.
     */
    virtual void waitDeviceIdle() const {};

    /**
     * @brief [ImGui specific][Vulkan specific]
     *        Structure to hold ImGui Vulkan initialization information.
     */
    struct ImGuiVulkanInitInfo {
        void* instance; // Vulkan instance.
        void* physicalDevice; // Vulkan physical device.
        void* device; // Vulkan logical device.
        void* queue; // Vulkan queue for graphics operations.
        void* renderPass; // Vulkan render pass for ImGui rendering.
        uint32_t queueFamily; // Queue family index for graphics operations.
        uint32_t imageCount; // Number of images in the swapchain.
        uint32_t samples; // Number of samples for multisampling.
        uint32_t swapchainFormat; // Format of the swapchain images.
    };
    /**
     * @brief [ImGui specific]
     *        Initialize ImGui for the graphics backend.
     *
     *        For Vulkan, the initFunc should receive a pointer to
     *        GfxRendererInterface::ImGuiVulkanInitInfo, convert it to
     *        ImGui_ImplVulkan_InitInfo and pass it to ImGui_ImplVulkan_Init().
     *
     *        For OpenGL, the initFunc should receive a const char* string of GLSL version
     *        and pass it to ImGui_ImplOpenGL3_Init().
     *
     * @param initFunc Function to initialize ImGui.
     * @return 0 on success, non-zero on failure.
     */
    virtual int initForImGui(const std::function<void(void*)>& initFunc) { return 0; };
    /**
     * @brief [ImGui specific]
     *        Terminate ImGui for the graphics backend.
     * @param termFunc Function to terminate ImGui (e.g., ImGui_ImplVulkan_Shutdown).
     */
    virtual void termForImGui(const std::function<void()>& termFunc) {};
    /**
     * @brief [ImGui specific]
     *        Render ImGui draw data using the graphics backend.
     *
     *        For Vulkan, the renderFunc should receive a VkCommandBuffer and pass it to
     *        ImGui_ImplVulkan_RenderDrawData().
     *
     *        For OpenGL, just call the ImGui_ImplOpenGL3_RenderDrawData() and no need to
     *        call this function.
     *
     * @param renderFunc Function to render ImGui draw data.
     */
    virtual void renderForImGui(const std::function<void(void*)>& renderFunc) const {};
    /**
     * @brief [ImGui specific][Vulkan specific]
     *        Structure to hold Vulkan information for a graphics image.
     */
    struct ImageVulkanInfo {
        void* sampler; // Vulkan sampler for the image.
        void* imageView; // Vulkan image view for the image.
        uint32_t imageLayout; // Vulkan image layout for the image.
    };
    using ImageInfo = std::variant<uint32_t, ImageVulkanInfo>;
    /**
     * @brief [ImGui specific]
     *        Get the image information for a graphics image.
     * @param image The GfxImage to get information for.
     * @return ImageVulkanInfo for Vulkan, or OpenGL texture ID for OpenGL.
     */
    virtual ImageInfo getImageInfo(const GfxImage& image) const { return {}; };

    /**
     * @brief Set the number of samples for multisampling.
     * @param samples The number of samples to set.
     */
    virtual void setSamples(int samples) = 0;

    /**
     * @brief Create a graphics image with the specified information.
     * @param info The information about the image to create.
     * @return A shared pointer to the created GfxImage.
     */
    virtual GfxImage createImage(const GfxImageInfo& info) const = 0;
    /**
     * @brief Set the image data for a graphics image.
     * @param image The GfxImage to set data for.
     * @param data Pointer to the image data.
     * @return 0 on success, non-zero on failure.
     */
    virtual int setImageData(const GfxImage& image, void* data) const = 0;
    /**
     * @brief Get the image data from a graphics image.
     * @param image The GfxImage to get data from.
     * @param data[out] Pointer to the buffer to store the image data.
     * @return 0 on success, non-zero on failure.
     */
    virtual int getImageData(const GfxImage& image, void* data) const = 0;
    /**
     * @brief Generate mipmaps for a graphics image.
     * @param image The GfxImage to generate mipmaps for.
     * @return 0 on success, non-zero on failure.
     */
    virtual int generateMipmaps(const GfxImage& image) const = 0;
    /**
     * @brief Copy image data from one graphics image to another.
     * @param src The source GfxImage to copy from.
     * @param dst The destination GfxImage to copy to.
     * @param width The width of the image region to copy.
     * @param height The height of the image region to copy.
     */
    virtual void copyImage(const GfxImage& src, const GfxImage& dst, int width, int height) = 0;
    /**
     * @brief Destroy a graphics image.
     * @param image The GfxImage to destroy.
     */
    virtual void destroyImage(const GfxImage& image) const = 0;

    /**
     * @brief Create a render pass with the specified color and depth attachments.
     * @param colorAttachments The list of color attachments for the render pass.
     * @param depthAttachment The depth attachment for the render pass.
     * @return A shared pointer to the created GfxRenderPass.
     */
    virtual GfxRenderPass createRenderPass(
        const std::vector<GfxAttachment>& colorAttachments,
        const GfxAttachment& depthAttachment = {}
    ) const = 0;
    /**
     * @brief Destroy a render pass.
     * @param renderPass The GfxRenderPass to destroy.
     */
    virtual void destroyRenderPass(const GfxRenderPass& renderPass) const = 0;

    /**
     * @brief Create a framebuffer with the specified render pass and images.
     * @param renderPass The render pass to associate with the framebuffer.
     * @param colorImages The list of color images for the framebuffer.
     * @param depthImage The depth image for the framebuffer (optional).
     * @param colorResolveImages The list of color resolve images (optional).
     * @return A shared pointer to the created GfxFramebuffer.
     */
    virtual GfxFramebuffer createFramebuffer(
        const GfxRenderPass& renderPass,
        const std::vector<GfxImage>& colorImages,
        const GfxImage& depthImage = {},
        const std::vector<GfxImage>& colorResolveImages = {}
    ) const = 0;
    /**
     * @brief Destroy a framebuffer.
     * @param framebuffer The GfxFramebuffer to destroy.
     */
    virtual void destroyFramebuffer(const GfxFramebuffer& framebuffer) const = 0;
    /**
     * @brief Read pixels from a color attachment of a framebuffer.
     * @param framebuffer The framebuffer to read from.
     * @param index The index of the color attachment to read.
     * @param rect The rectangle area to read pixels from.
     * @param pixels[out] Pointer to the buffer to store the read pixels.
     * @return 0 on success, non-zero on failure.
     */
    virtual int readFramebufferColorAttachmentPixels(
        const GfxFramebuffer& framebuffer,
        int index,
        const GfxRect& rect,
        void* pixels
    ) const = 0;

    /**
     * @brief Create a graphics buffer with the specified size, usage, and properties.
     * @param size The size of the buffer in bytes.
     * @param usage The usage of the buffer (e.g., vertex buffer, index buffer).
     * @param prop The properties of the buffer (e.g., static, dynamic).
     * @return A shared pointer to the created GfxBuffer.
     */
    virtual GfxBuffer createBuffer(
        int size,
        GfxBufferUsage usage,
        GfxBufferProp prop
    ) const = 0;
    /**
     * @brief Set the data for a graphics buffer.
     * @param buffer The GfxBuffer to set data for.
     * @param size The size of the data in bytes.
     * @param data Pointer to the data to set.
     * @return 0 on success, non-zero on failure.
     */
    virtual int setBufferData(const GfxBuffer& buffer, int size, const void* data) const = 0;
    /**
     * @brief Update a portion of the data in a graphics buffer.
     * @param buffer The GfxBuffer to update.
     * @param offset The offset in bytes where the update should start.
     * @param size The size of the data to update in bytes.
     * @param data Pointer to the new data to set.
     * @return 0 on success, non-zero on failure.
     */
    virtual int updateBufferData(
        const GfxBuffer& buffer,
        int offset,
        int size,
        const void* data
    ) const = 0;
    /**
     * @brief Destroy a graphics buffer.
     * @param buffer The GfxBuffer to destroy.
     */
    virtual void destroyBuffer(const GfxBuffer& buffer) const = 0;
    /**
     * @brief Read data from a graphics buffer.
     * @param buffer The GfxBuffer to read from.
     * @param offset The offset in bytes where the read should start.
     * @param size The size of the data to read in bytes.
     * @param data[out] Pointer to the buffer to store the read data.
     * @return 0 on success, non-zero on failure.
     */
    virtual int readBufferData(
        const GfxBuffer& buffer,
        int offset,
        int size,
        void* data
    ) const = 0;
    /**
     * @brief Copy data from one graphics buffer to another.
     * @param src The source GfxBuffer to copy from.
     * @param dst The destination GfxBuffer to copy to.
     * @param srcOffset The offset in bytes in the source buffer where the copy should start.
     * @param dstOffset The offset in bytes in the destination buffer where the copy should start.
     * @param size The size of the data to copy in bytes.
     * @return 0 on success, non-zero on failure.
     */
    virtual int copyBuffer(
        const GfxBuffer& src,
        const GfxBuffer& dst,
        int srcOffset,
        int dstOffset,
        int size
    ) const = 0;

    /**
     * @brief Create a vertex array object (VAO) with the specified vertex descriptor,
     *        vertex buffer, and index buffer.
     * @param vertexDesc The vertex descriptor containing attribute information.
     * @param vertexBuffer The vertex buffer to bind to the VAO.
     * @param indexBuffer The index buffer to bind to the VAO.
     * @return A shared pointer to the created GfxVAO.
     */
    virtual GfxVAO createVAO(
        const GfxVertexDesc& vertexDesc,
        const GfxBuffer& vertexBuffer,
        const GfxBuffer& indexBuffer
    ) const = 0;
    /**
     * @brief Destroy a vertex array object (VAO).
     * @param vao The GfxVAO to destroy.
     */
    virtual void destroyVAO(const GfxVAO& vao) const = 0;

    /**
     * @brief Create a shader with the specified stage and source code.
     * @param stage The shader stage (e.g., vertex, fragment).
     * @param source The source code of the shader.
     * @throws GfxShaderException if shader compilation fails.
     * @return A shared pointer to the created GfxShader.
     */
    virtual GfxShader createShader(
        GfxShaderStage stage,
        const std::string& source
    ) const = 0;
    /**
     * @brief Destroy a shader.
     * @param shader The GfxShader to destroy.
     */
    virtual void destroyShader(const GfxShader& shader) const = 0;

    /**
     * @brief Create a graphics pipeline with the specified shaders, descriptors, vertex descriptor,
     *        dynamic states, and render pass.
     * @param shaders The list of shaders to include in the pipeline.
     * @param descriptorSets The list of descriptor sets for the pipeline.
     * @param vertexDesc The vertex descriptor for the pipeline.
     * @param dynamicStates The list of dynamic states for the pipeline.
     * @param renderPass The render pass associated with the pipeline.
              Pass nullptr for the default render pass.
     * @return A shared pointer to the created GfxPipeline.
     */
    virtual GfxPipeline createPipeline(
        const std::vector<GfxShader>& shaders,
        const std::vector<GfxDescriptorSet>& descriptorSets,
        const GfxVertexDesc& vertexDesc = {},
        const std::vector<GfxPipelineState>& dynamicStates = {},
        const GfxRenderPass& renderPass = {}
    ) const = 0;
    /**
     * @brief Destroy a graphics pipeline.
     * @param pipeline The GfxPipeline to destroy.
     */
    virtual void destroyPipeline(const GfxPipeline& pipeline) const = 0;

    /**
     * @brief Create a descriptor set binding for a pipeline.
     * @param pipeline The graphics pipeline.
     * @param descriptorSetIndex The index of the descriptor set in the pipeline.
     * @param bindings The list of descriptor bindings.
     * @return A shared pointer to the created GfxDescriptorSetBinding.
     */
    virtual GfxDescriptorSetBinding createDescriptorSetBinding(
        const GfxPipeline& pipeline,
        int descriptorSetIndex,
        const std::vector<GfxDescriptorBinding>& bindings
    ) const = 0;
    /**
     * @brief Destroy a descriptor set binding.
     * @param binding The GfxDescriptorSetBinding to destroy.
     */
    virtual void destroyDescriptorSetBinding(GfxDescriptorSetBinding& binding) const = 0;

    /**
     * @brief Begin a render pass with the specified framebuffer object.
     * @param framebuffer The framebuffer. Pass nullptr for the default framebuffer.
     * @return 0 on success, non-zero on failure.
     */
    virtual int beginRenderPass(const GfxFramebuffer& framebuffer = {}) = 0;
    /**
     * @brief End the current render pass.
     * @return 0 on success, non-zero on failure.
     */
    virtual void endRenderPass() = 0;

    /**
     * @brief Bind a pipeline for rendering.
     * @param fbo The pipeline object to bind.
     */
    virtual void bindPipeline(const GfxPipeline& pipeline) = 0;
    /**
     * @brief Bind a vertex array object (VAO) for rendering.
     * @param vao The vertex array object to bind.
     */
    virtual void bindVAO(const GfxVAO& vao) = 0;

    /**
     * @brief Clear the color attachment of the current render pass.
     * @param index The index of the color attachment to clear.
     * @param value The color value to clear the attachment with.
     */
    virtual void clearColorAttachment(int index, const std::array<float, 4>& value) = 0;
    /**
     * @brief Clear the depth attachment of the current render pass.
     * @param value The depth value to clear the attachment with.
     */
    virtual void clearDepthAttachment(float value) = 0;
    /**
     * @brief Clear the stencil attachment of the current render pass.
     * @param value The stencil value to clear the attachment with.
     */
    virtual void clearStencilAttachment(int value) = 0;

    /**
     * @brief Bind a descriptor set binding for rendering.
     * @param binding The descriptor set binding to bind.
     */
    virtual void bindDescriptorSetBinding(const GfxDescriptorSetBinding& binding) = 0;

    /**
     * @brief Begin a frame for rendering.
     * @return 0 on success, non-zero on failure.
     */
    virtual int beginFrame() = 0;
    /**
     * @brief End the current frame for rendering.
     * @return 0 on success, non-zero on failure.
     */
    virtual int endFrame() = 0;

    /**
     * @brief Draw commands for rendering.
     * @param nVertices Number of vertices to draw.
     * @param nInstances Number of instances to draw.
     * @param firstVertex First vertex index to start drawing from.
     * @param firstInstance First instance index to start drawing from.
     */
    virtual void draw(
        int nVertices,
        int nInstances = 1,
        int firstVertex = 0,
        int firstInstance = 0
    ) = 0;
    /**
     * @brief Draw indexed commands for rendering.
     * @param nIndices Number of indices to draw.
     * @param nInstances Number of instances to draw.
     * @param firstIndex First index to start drawing from.
     * @param vertexOffset Offset in the vertex buffer.
     * @param firstInstance First instance index to start drawing from.
     */
    virtual void drawIndexed(
        int nIndices,
        int nInstances = 1,
        int firstIndex = 0,
        int vertexOffset = 0,
        int firstInstance = 0
    ) = 0;
    /**
     * @brief Draw indirect commands for rendering.
     * @param buffer The buffer containing draw parameters.
     * @param offset Offset in the buffer where draw parameters start.
     * @param drawCount Number of draws to execute.
     * @param stride Stride between consecutive draw parameters in the buffer.
     */
    virtual void drawIndirect(
        const GfxBuffer& buffer,
        int offset,
        int drawCount,
        int stride
    ) = 0;
    /**
     * @brief Draw indexed indirect commands for rendering.
     * @param buffer The buffer containing indexed draw parameters.
     * @param offset Offset in the buffer where indexed draw parameters start.
     * @param drawCount Number of indexed draws to execute.
     * @param stride Stride between consecutive indexed draw parameters in the buffer.
     */
    virtual void drawIndexedIndirect(
        const GfxBuffer& buffer,
        int offset,
        int drawCount,
        int stride
    ) = 0;
    /**
     * @brief Dispatch compute shader commands.
     * @param nGroupsX Number of groups in the X dimension.
     * @param nGroupsY Number of groups in the Y dimension.
     * @param nGroupsZ Number of groups in the Z dimension.
     */
    virtual void dispatchCompute(int nGroupsX, int nGroupsY, int nGroupsZ) = 0;
    /**
     * @brief Dispatch compute shader commands with indirect parameters.
     * @param buffer The buffer containing compute parameters.
     * @param offset Offset in the buffer where compute parameters start.
     */
    virtual void dispatchComputeIndirect(const GfxBuffer& buffer, int offset) = 0;
    /**
     * @brief Perform a memory barrier to ensure memory visibility and ordering.
     */
    virtual void memoryBarrier() = 0;

protected:
    GfxBackend m_backend = GfxBackend::OpenGL; // Graphics backend used by the renderer.
    GfxPipelineStateMachine m_pipelineStateMachine = nullptr; // Pipeline state machine.
    GfxFramebuffer m_currentFramebuffer = nullptr; // Current framebuffer object.
};
using GfxRenderer = std::shared_ptr<GfxRendererInterface>;

/**
 * @brief Configuration structure for OpenGL renderer.
 * @note Contains a pointer to the OpenGL function loader.
 */
struct GfxGLRendererConfig {
    void* proc = nullptr; // Pointer to the OpenGL function loader.
};
/**
 * @brief Configuration structure for Vulkan renderer.
 * @note Contains the application name and a list of Vulkan extensions to enable.
 */
struct GfxVulkanRendererConfig {
    std::string appName = {}; // Name of the Vulkan application.
    std::vector<const char*> extensions = {}; // List of Vulkan extensions to enable.
};
using GfxRendererConfig = std::variant<GfxGLRendererConfig, GfxVulkanRendererConfig>;
/**
 * @brief Factory class for creating graphics renderers.
 * @note This class provides methods to initialize, terminate, and create graphics renderers
         for different backends (OpenGL, Vulkan).
 */
class GfxRendererFactory {
private:
    GfxRendererFactory() = default;
    GfxRendererFactory(const GfxRendererFactory&) = delete;
    GfxRendererFactory& operator=(const GfxRendererFactory&) = delete;
public:
    ~GfxRendererFactory();

public:
    static GfxRendererFactory& instance() {
        static GfxRendererFactory instance;
        return instance;
    }

    /**
     * @brief Initialize the global graphics renderer with the specified backend and configuration.
     * @param backend The graphics backend to use (OpenGL, Vulkan).
     * @param config The configuration for the renderer.
     * @return 0 on success, non-zero on failure.
     */
    int initGlobal(GfxBackend backend, const GfxRendererConfig& config);
    /**
     * @brief Terminate the global graphics renderer for the specified backend.
     * @param backend The graphics backend to terminate (OpenGL, Vulkan).
     */
    void termGlobal(GfxBackend backend);
    /**
     * @brief Create a graphics renderer for the specified backend.
     * @param backend The graphics backend to create the renderer for (OpenGL, Vulkan).
     * @return A shared pointer to the created GfxRenderer.
     */
    GfxRenderer create(GfxBackend backend);

private:
    std::map<GfxBackend, bool> m_initialized = {}; // Map to track initialized backends.
};
