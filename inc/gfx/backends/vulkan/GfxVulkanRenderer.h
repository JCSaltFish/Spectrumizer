/**
 * @file GfxVulkanRenderer.h
 * @brief Vulkan implementation of the GfxRenderer interface.
 */

#include <vulkan/vulkan.h>

#include "gfx/GfxPr.h"

/**
 * @brief Vulkan implementation of GfxImage.
 */
class GfxVulkanImage : public GfxImage_T {
public:
    explicit GfxVulkanImage(const GfxImageInfo& info) : GfxImage_T(info) {};

public:
    VkImage m_image = VK_NULL_HANDLE; // Vulkan image object
    VkImageView m_imageView = VK_NULL_HANDLE; // Vulkan image view for the image
    std::vector<VkImageView> m_mipmapViews = {}; // Vulkan image views for each mipmap level
    VkDeviceMemory m_imageMemory = VK_NULL_HANDLE; // Vulkan device memory for the image
    VkSampler m_sampler = VK_NULL_HANDLE; // Vulkan sampler for the image
    VkImageLayout m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Current layout of the image
};

/**
 * @brief Vulkan implementation of GfxRenderPass.
 */
class GfxVulkanRenderPass : public GfxRenderPass_T {
public:
    GfxVulkanRenderPass(
        const std::vector<GfxAttachment>& colorAttachments,
        const GfxAttachment& depthAttachment
    ) :
        GfxRenderPass_T(colorAttachments, depthAttachment)
    {};

public:
    VkRenderPass m_vkRenderPass = VK_NULL_HANDLE; // Vulkan render pass object
    int m_samples = 0; // Number of samples for multisampling
};

/**
 * @brief Vulkan implementation of GfxFramebuffer.
 */
class GfxVulkanFramebuffer : public GfxFramebuffer_T {
public:
    GfxVulkanFramebuffer(
        const GfxRenderPass& renderPass,
        const std::vector<GfxImage>& colorImages,
        const GfxImage& depthImage,
        const std::vector<GfxImage>& colorResolveImages
    ) :
        GfxFramebuffer_T(
            renderPass,
            colorImages,
            depthImage,
            colorResolveImages
        )
    {};

public:
    VkFramebuffer m_vkFramebuffer = VK_NULL_HANDLE; // Vulkan framebuffer object
    int m_width = 0; // Width of the framebuffer
    int m_height = 0; // Height of the framebuffer
};

/**
 * @brief Vulkan implementation of GfxBuffer.
 */
class GfxVulkanBuffer : public GfxBuffer_T {
public:
    GfxVulkanBuffer(
        int size,
        GfxBufferUsage usage,
        GfxBufferProp prop
    ) :
        GfxBuffer_T(size, usage, prop)
    {};

    /**
     * @brief Set the size of the buffer.
     * @param size The new size of the buffer in bytes.
     */
    void setSize(int size) { m_size = size; };

public:
    std::vector<VkBuffer> m_vkBuffers = {}; // Vulkan buffer objects
    std::vector<VkDeviceMemory> m_vkBufferMemories = {}; // Vulkan device memory for the buffers
};

/**
 * @brief Vulkan implementation of GfxShader.
 */
class GfxVulkanShader : public GfxShader_T {
public:
    explicit GfxVulkanShader(GfxShaderStage stage) : GfxShader_T(stage) {};

public:
    VkShaderModule m_vkShaderModule = VK_NULL_HANDLE; // Vulkan shader module object
};

class GfxVulkanDescriptorSetBinding : public GfxDescriptorSetBinding_T {
public:
    GfxVulkanDescriptorSetBinding(
        const GfxPipeline& pipeline,
        int descriptorSetIndex,
        const std::vector<GfxDescriptorBinding>& bindings
    ) :
        GfxDescriptorSetBinding_T(pipeline, descriptorSetIndex, bindings)
    {};

public:
    std::vector<VkDescriptorSet> m_vkDescriptorSets = {}; // Vulkan descriptor sets
    VkDescriptorPool m_vkDescriptorPool = VK_NULL_HANDLE; // Vulkan descriptor pool
};

/**
 * @brief Vulkan implementation of GfxPipeline.
 */
class GfxVulkanPipeline : public GfxPipeline_T {
public:
    GfxVulkanPipeline(
        const GfxRenderPass& renderPass,
        const std::vector<GfxDescriptorSet>& descriptorSets,
        const std::vector<GfxPipelineState>& dynamicStates
    ) :
        GfxPipeline_T(renderPass, descriptorSets, dynamicStates)
    {};

    /**
     * @brief Set the shader stage for the pipeline.
     * @param stage The shader stage to set.
     */
    void setStage(GfxShaderStage stage) { m_stages.set(stage); };
    /**
     * @brief Unset the shader stage for the pipeline.
     * @param stage The shader stage to unset.
     */
    void resetStages() { m_stages.reset(); };

public:
    VkPipeline m_vkPipeline = VK_NULL_HANDLE; // Vulkan pipeline
    VkPipelineLayout m_vkPipelineLayout = VK_NULL_HANDLE; // Vulkan pipeline layout
    std::vector<VkDescriptorSetLayout> m_vkDescriptorSetLayouts = {}; // Vulkan descriptor set layouts
};

/**
 * @brief Vulkan implementation of GfxRenderer.
 */
class GfxVulkanRenderer : public GfxRendererInterface {
public:
    GfxVulkanRenderer();
    ~GfxVulkanRenderer();

public:
    static int initGlobal(const GfxRendererConfig& config);
    static void termGlobal();

    void setVSyncMode(GfxVSyncMode mode) override;

    void* getVulkanInstance() const override;
    void setVulkanSurface(void* surface) override;
    int setSwapchainSize(int width, int height) override;
    void waitDeviceIdle() const override;

    int initForImGui(const std::function<void(void*)>& initFunc) override;
    void termForImGui(const std::function<void()>& termFunc) override;
    void renderForImGui(const std::function<void(void*)>& renderFunc) const override;
    ImageInfo getImageInfo(const GfxImage& image) const override;

    void setSamples(int samples) override;

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
    void destroyFramebuffer(const GfxFramebuffer& framebuffer) const override;
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
    void destroyVAO(const GfxVAO& vao) const override {};

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

    int beginFrame() override;
    int endFrame() override;

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
    /**
     * @brief Structure representing a queue family.
     */
    struct QueueFamily {
        uint32_t index = 0; // Index of the queue family
        uint32_t queueCount = 0; // Number of queues in the queue family
    };
    /**
     * @brief Finds a suitable queue family for graphics operations.
     * @param device The Vulkan physical device to query.
     * @return The found QueueFamily structure.
     */
    static QueueFamily findQueueFamily(const VkPhysicalDevice& device);

    /**
     * @brief Creates the swapchain.
     * @return 0 on success, non-zero on failure.
     */
    int createSwapchain();
    /**
     * @brief Creates the swapchain render pass.
     * @param renderPass Reference to the VkRenderPass to be created.
     * @return 0 on success, non-zero on failure.
     */
    int createSwapchainRenderPass(VkRenderPass& renderPass);
    /**
     * @brief Creates the swapchain framebuffers.
     * @return 0 on success, non-zero on failure.
     */
    int createSwapchainFramebuffers();
    /**
     * @brief Recreates the swapchain and associated resources.
     * @return 0 on success, non-zero on failure.
     */
    int recreateSwapchain();
    /**
     * @brief Cleans up the swapchain and associated resources.
     */
    void cleanupSwapchain();

    /**
     * @brief Creates command buffers for rendering.
     * @return 0 on success, non-zero on failure.
     */
    int createCommandBuffers();

    /**
     * @brief Gets the sample count for multisampling based on the number of samples.
     * @param samples Number of samples for multisampling.
     * @return The corresponding VkSampleCountFlagBits value.
     */
    VkSampleCountFlagBits getSampleCount(int samples) const;
    /**
     * @brief Structure containing parameters for creating a Vulkan image.
     */
    struct CreateVkImageInfo {
        uint32_t width; // Width of the image
        uint32_t height; // Height of the image
        uint32_t mipLevels; // Number of mip levels for the image
        VkSampleCountFlagBits numSamples; // Number of samples for multisampling
        VkFormat format; // Format of the image
        VkImageTiling tiling; // Tiling mode of the image
        VkImageUsageFlags usage; // Usage flags for the image
        VkMemoryPropertyFlags properties; // Memory properties for the image
    };
    /**
     * @brief Creates a Vulkan image with the specified parameters.
     * @param info Structure containing parameters for image creation.
     * @param[out] image Reference to the VkImage to be created.
     * @param[out] imageMemory Reference to the VkDeviceMemory to be allocated for the image.
     * @return 0 on success, non-zero on failure.
     */
    int createVkImage(
        const CreateVkImageInfo& info,
        VkImage& image,
        VkDeviceMemory& imageMemory
    ) const;
    /**
     * @brief Creates a Vulkan image view for the specified image.
     * @param image The VkImage to create the view for.
     * @param format Format of the image view.
     * @param aspectFlags Aspect flags for the image view (e.g., color, depth).
     * @param baseLevel Base mip level for the image view.
     * @param levelCount Number of mip levels in the image.
     * @param[out] imageView Reference to the VkImageView to be created.
     * @return 0 on success, non-zero on failure.
     */
    int createVkImageView(
        const VkImage& image,
        VkFormat format,
        VkImageAspectFlags aspectFlags,
        uint32_t baseLevel,
        uint32_t levelCount,
        VkImageView& imageView
    ) const;
    /**
     * @brief Transitions the layout of a Vulkan image.
     * @param image The VkImage to transition.
     * @param format Format of the image.
     * @param oldLayout Current layout of the image.
     * @param newLayout Desired layout of the image.
     * @param mipLevels Number of mip levels in the image.
     * @param commandBuffer Optional command buffer to record the transition commands.
     * @return 0 on success, non-zero on failure.
     */
    int transitionImageLayout(
        const VkImage& image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        uint32_t mipLevels,
        const VkCommandBuffer& commandBuffer = VK_NULL_HANDLE
    ) const;
    /**
     * @brief Reads image data from a Vulkan image into a CPU-accessible buffer.
     * @param image The GfxImage to read data from.
     * @param rect The rectangle area of the image to read.
     * @param[out] data Pointer to the buffer to store the read image data.
     * @return 0 on success, non-zero on failure.
     */
    int readImageData(const GfxImage& image, const GfxRect& rect, void* data) const;

    /**
     * @brief Clears a specific attachment in the current render pass.
     * @param attachment The VkClearAttachment to clear.
     */
    void clearAttachment(const VkClearAttachment& attachment) const;

    /**
     * @brief Finds a suitable depth format for depth image attachment.
     * @param[out] format Reference to store the found depth format.
     * @return 0 on success, non-zero on failure.
     */
    int findDepthFormat(VkFormat& format) const;

    /**
     * @brief Finds a suitable memory type for the given type filter and properties.
     * @param typeFilter Bitmask of memory types to filter.
     * @param properties Desired memory properties (e.g., device local, host visible).
     * @param[out] typeIndex Reference to store the index of the found memory type.
     * @return 0 on success, non-zero on failure.
     */
    int findMemoryType(
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties,
        uint32_t& typeIndex
    ) const;

    /**
     * @brief Creates a Vulkan buffer with the specified parameters.
     * @param size Size of the buffer in bytes.
     * @param usage Usage flags for the buffer (e.g., vertex buffer, index buffer).
     * @param properties Memory properties for the buffer (e.g., device local, host visible).
     * @param[out] buffer Reference to the VkBuffer to be created.
     * @param[out] bufferMemory Reference to the VkDeviceMemory to be allocated for the buffer.
     * @return 0 on success, non-zero on failure.
     */
    int createVkBuffer(
        const VkDeviceSize& size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory
    ) const;
    /**
     * @brief Resizes a Vulkan buffer to a new size.
     * @param buffer The GfxVulkanBuffer to resize.
     * @param size New size for the buffer in bytes.
     * @return 0 on success, non-zero on failure.
     */
    int resizeVkBuffer(const GfxBuffer& buffer, int size) const;

    /**
     * @brief Creates the Vulkan descriptor set layout, descriptor pool, and descriptor sets based
              on the provided descriptors.
     * @param pipeline The Vulkan pipeline to associate the descriptor sets with.
     * @param descriptors Vector of GfxDescriptor defining the layout.
     * @return 0 on success, non-zero on failure.
     */
    int createVkPipelineDescriptorSets(
        const std::shared_ptr<GfxVulkanPipeline>& pipeline,
        const std::vector<GfxDescriptorSet>& descriptorSets
    ) const;
    /**
     * @brief Updates the Vulkan descriptor sets with the provided descriptor set binding.
     * @param descriptorSetBinding The GfxDescriptorSetBinding with the descriptors to update.
     */
    void updateVkDescriptorSets(const GfxDescriptorSetBinding& descriptorSetBinding) const;

    /**
     * @brief Begins a single-time command buffer for immediate operations.
     * @return A VkCommandBuffer ready for recording commands.
     */
    VkCommandBuffer beginSingleTimeCommands() const;
    /**
     * @brief Ends a single-time command buffer and submits it for execution.
     * @param commandBuffer The command buffer to end and submit.
     */
    void endSingleTimeCommands(const VkCommandBuffer& commandBuffer) const;

private:
    static std::mutex s_mutex; // Mutex for synchronizing access to global Vulkan renderer

    static VkInstance s_vkInstance; // Vulkan instance
    static VkPhysicalDevice s_vkPhysicalDevice; // Vulkan physical device (GPU)
    static VkDevice s_vkDevice; // Vulkan logical device
    static int s_nInstances; // Number of Vulkan renderer instances

    VkQueue m_vkGraphicsQueue = VK_NULL_HANDLE; // Vulkan queue for graphics operations
    VkQueue m_vkPresentQueue = VK_NULL_HANDLE; // Vulkan queue for presentation operations

    VkSampleCountFlagBits m_maxSampleCount = VK_SAMPLE_COUNT_1_BIT; // Maximum sample count
    VkSampleCountFlagBits m_samples = VK_SAMPLE_COUNT_1_BIT; // Number of samples for multisampling

    VkCommandPool m_vkCommandPool = VK_NULL_HANDLE; // Command pool for allocating command buffers
    std::vector<VkCommandBuffer> m_vkCommandBuffers = {}; // Command buffers for recording commands

    GfxVSyncMode m_vsyncMode = GfxVSyncMode::FIFO; // VSync mode for the renderer

    VkSurfaceKHR m_vkSurface = VK_NULL_HANDLE; // Vulkan surface for presentation
    VkSwapchainKHR m_vkSwapchain = VK_NULL_HANDLE; // Vulkan swapchain for presenting images
    VkExtent2D m_swapchainExtent = {}; // Dimensions of the swapchain images
    VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED; // Format of the swapchain images
    std::vector<VkImageView> m_swapchainImageViews; // Views for the swapchain images
    VkImage m_swapchainColorImage = VK_NULL_HANDLE; // Color image for the swapchain
    VkImageView m_swapchainColorImageView = VK_NULL_HANDLE; // View for the swapchain color image
    VkDeviceMemory m_swapchainColorImageMemory = VK_NULL_HANDLE; // Memory for swapchainColorImage
    VkImage m_swapchainDepthImage = VK_NULL_HANDLE; // Depth image for the swapchain
    VkImageView m_swapchainDepthImageView = VK_NULL_HANDLE; // View for the swapchain depth image
    VkDeviceMemory m_swapchainDepthImageMemory = VK_NULL_HANDLE; // Memory for swapchainDepthImage
    VkRenderPass m_swapchainRenderPass = VK_NULL_HANDLE; // Render pass for the swapchain
    std::vector<VkFramebuffer> m_swapchainFramebuffers = {}; // Framebuffers for the swapchain
    uint32_t m_imageIndex = 0; // Index of the current image in the swapchain

    std::vector<VkSemaphore> m_imageAvailableSemaphores = {}; // Semaphores for image availability
    std::vector<VkSemaphore> m_renderFinishedSemaphores = {}; // Semaphores for render completion
    std::vector<VkFence> m_inFlightFences = {}; // Fences for synchronizing frame rendering

    uint32_t m_currentFrame = 0; // Index of the current frame being rendered

    static VkDebugUtilsMessengerEXT s_debugMessenger; // Debug messenger

    VkRenderPass m_ImGuiRenderPass = VK_NULL_HANDLE; // [ImGui specific] Render pass for ImGui
};
