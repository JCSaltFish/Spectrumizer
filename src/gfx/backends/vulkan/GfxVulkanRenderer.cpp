/**
 * @file GfxVulkanRenderer.cpp
 * @brief Vulkan implementation of the GfxRenderer interface.
 */

#include "gfx/backends/vulkan/GfxVulkanRenderer.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include "gfx/backends/vulkan/GfxVkTypeConverter.h"
#include "gfx/backends/vulkan/GfxVulkanPipelineState.h"

#ifdef _DEBUG
#include <iostream>
//#define ENABLE_DEBUG_OUTPUT
#endif // _DEBUG

constexpr int MAX_FRAMES_IN_FLIGHT = 2; // Maximum number of frames in flight

std::mutex GfxVulkanRenderer::s_mutex; // Mutex for global Vulkan renderer

VkInstance GfxVulkanRenderer::s_vkInstance = VK_NULL_HANDLE; // Vulkan instance
VkPhysicalDevice GfxVulkanRenderer::s_vkPhysicalDevice = VK_NULL_HANDLE; // Vulkan physical device
VkDevice GfxVulkanRenderer::s_vkDevice = VK_NULL_HANDLE; // Vulkan logical device
int GfxVulkanRenderer::s_nInstances = 0; // Number of Vulkan renderer instances

GfxVulkanRenderer::GfxVulkanRenderer() {
    m_backend = GfxBackend::Vulkan;

    int err = 0;
    GfxScopeGuard errCleaner
    (
        [&]() {
            if (err > 1) {
                for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    vkDestroySemaphore(s_vkDevice, m_imageAvailableSemaphores[i], nullptr);
                    vkDestroySemaphore(s_vkDevice, m_renderFinishedSemaphores[i], nullptr);
                    vkDestroyFence(s_vkDevice, m_inFlightFences[i], nullptr);
                }
            }
            if (err) {
                vkDestroyCommandPool(s_vkDevice, m_vkCommandPool, nullptr);
                m_vkCommandPool = VK_NULL_HANDLE;
            }
        }
    );

    // Get max usable sample count
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(s_vkPhysicalDevice, &physicalDeviceProperties);
    VkSampleCountFlags counts =
        physicalDeviceProperties.limits.framebufferColorSampleCounts &
        physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    const std::vector<VkSampleCountFlagBits> sampleCounts = {
        VK_SAMPLE_COUNT_64_BIT,
        VK_SAMPLE_COUNT_32_BIT,
        VK_SAMPLE_COUNT_16_BIT,
        VK_SAMPLE_COUNT_8_BIT,
        VK_SAMPLE_COUNT_4_BIT,
        VK_SAMPLE_COUNT_2_BIT
    };
    m_maxSampleCount = VK_SAMPLE_COUNT_1_BIT;
    for (auto sampleCount : sampleCounts) {
        if (counts & sampleCount) {
            m_maxSampleCount = sampleCount;
            break;
        }
    }

    // Create command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    QueueFamily family = findQueueFamily(s_vkPhysicalDevice);
    poolInfo.queueFamilyIndex = family.index;
    if (vkCreateCommandPool(s_vkDevice, &poolInfo, nullptr, &m_vkCommandPool)) {
        err = 1;
        return; // Error: Failed to create command pool
    }

    // Create command buffers
    if (createCommandBuffers()) {
        err = 1;
        return; // Error: Failed to create command buffers
    }

    // Initialize pipeline state machine
    m_pipelineStateMachine = std::make_shared<GfxVulkanPipelineStateMachine>();
    std::shared_ptr<GfxVulkanPipelineStateMachine> vulkanPipelineStateMachine
        = std::static_pointer_cast<GfxVulkanPipelineStateMachine>(m_pipelineStateMachine);
    vulkanPipelineStateMachine->loadFunctions(s_vkDevice);

    // Create sync objects
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result = VK_SUCCESS;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        result = vkCreateSemaphore(
            s_vkDevice,
            &semaphoreInfo,
            nullptr,
            &m_imageAvailableSemaphores[i]
        );
        if (result != VK_SUCCESS) {
            err = 2;
            return; // Error: Failed to create image available semaphore
        }
        result = vkCreateSemaphore(
            s_vkDevice,
            &semaphoreInfo,
            nullptr,
            &m_renderFinishedSemaphores[i]
        );
        if (result != VK_SUCCESS) {
            err = 2;
            return; // Error: Failed to create render finished semaphore
        }
        result = vkCreateFence(s_vkDevice, &fenceInfo, nullptr, &m_inFlightFences[i]);
        if (result != VK_SUCCESS) {
            err = 2;
            return; // Error: Failed to create in-flight fence
        }
    }

    vkGetDeviceQueue(s_vkDevice, family.index, s_nInstances, &m_vkGraphicsQueue);
    vkGetDeviceQueue(s_vkDevice, family.index, s_nInstances, &m_vkPresentQueue);

    s_nInstances++;
}

GfxVulkanRenderer::~GfxVulkanRenderer() {
    vkDeviceWaitIdle(s_vkDevice);

    // Cleanup swapchain resources first
    cleanupSwapchain();
    vkDestroyRenderPass(s_vkDevice, m_swapchainRenderPass, nullptr);
    m_swapchainRenderPass = VK_NULL_HANDLE;
    vkDestroySwapchainKHR(s_vkDevice, m_vkSwapchain, nullptr);
    m_vkSwapchain = VK_NULL_HANDLE;

    // Sync objects
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(s_vkDevice, m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(s_vkDevice, m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(s_vkDevice, m_inFlightFences[i], nullptr);
    }

    // Other staff
    vkDestroyCommandPool(s_vkDevice, m_vkCommandPool, nullptr);
    m_vkCommandPool = VK_NULL_HANDLE;

    vkDestroySurfaceKHR(s_vkInstance, m_vkSurface, nullptr);
    m_vkSurface = VK_NULL_HANDLE;

    m_vkGraphicsQueue = VK_NULL_HANDLE;
    m_vkPresentQueue = VK_NULL_HANDLE;

    s_nInstances--;
}

VkDebugUtilsMessengerEXT GfxVulkanRenderer::s_debugMessenger = VK_NULL_HANDLE;
#ifdef ENABLE_DEBUG_OUTPUT
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}
#endif // ENABLE_DEBUG_OUTPUT

int GfxVulkanRenderer::initGlobal(const GfxRendererConfig& config) {
    const GfxVulkanRendererConfig* vulkanConfig = std::get_if<GfxVulkanRendererConfig>(&config);
    if (vulkanConfig == nullptr)
        return 1; // Error: Invalid configuration

    std::lock_guard<std::mutex> lock(s_mutex);

    // Initialize glslang
    glslang::InitializeProcess();

    // Create Vulkan instance
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = vulkanConfig->appName.c_str();
    appInfo.applicationVersion = 0;
    appInfo.pEngineName = nullptr;
    appInfo.engineVersion = 0;
    appInfo.apiVersion = VK_MAKE_VERSION(1, 3, 0);

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.pNext = nullptr;
    std::vector<const char*> extensions = vulkanConfig->extensions;
#ifdef ENABLE_DEBUG_OUTPUT
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    bool validationLayerFound = false;
    for (const auto& layer : availableLayers) {
        if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
            validationLayerFound = true;
            break;
        }
    }
    if (!validationLayerFound) {
        std::cerr << "Validation layer not found!" << std::endl;
        return 1; // Error: Validation layer not found
    }
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    VkDebugUtilsMessengerCreateInfoEXT msgCreateInfo{};
    msgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    msgCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    msgCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    msgCreateInfo.pfnUserCallback = debugCallback;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        s_vkInstance,
        "vkCreateDebugUtilsMessengerEXT"
    );
    if (func != nullptr)
        func(s_vkInstance, &msgCreateInfo, nullptr, &s_debugMessenger);
    static const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&msgCreateInfo;
#endif // ENABLE_DEBUG_OUTPUT
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &s_vkInstance) != VK_SUCCESS)
        return 1; // Error: Failed to create Vulkan instance

    // Create physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(s_vkInstance, &deviceCount, nullptr);
    if (deviceCount == 0)
        return 1; // Error: No Vulkan physical devices found
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(s_vkInstance, &deviceCount, devices.data());
    s_vkPhysicalDevice = devices[0]; // Select the first physical device for simplicity

    // Create logical device
    QueueFamily family = findQueueFamily(s_vkPhysicalDevice);
    std::vector<float> queuePriorities(family.queueCount, 1.0f);
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = family.index;
    queueCreateInfo.queueCount = family.queueCount;
    queueCreateInfo.pQueuePriorities = queuePriorities.data();
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(s_vkPhysicalDevice, &deviceFeatures);
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
    extendedDynamicStateFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
    extendedDynamicStateFeatures.extendedDynamicState = VK_TRUE;
    deviceCreateInfo.pNext = &extendedDynamicStateFeatures;

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features{};
    extendedDynamicState2Features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
    extendedDynamicState2Features.extendedDynamicState2 = VK_TRUE;
    extendedDynamicState2Features.extendedDynamicState2LogicOp = VK_TRUE;
    extendedDynamicStateFeatures.pNext = &extendedDynamicState2Features;

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extendedDynamicState3Features{};
    extendedDynamicState3Features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
    extendedDynamicState3Features.extendedDynamicState3PolygonMode = VK_TRUE;
    extendedDynamicState3Features.extendedDynamicState3LogicOpEnable = VK_TRUE;
    extendedDynamicState3Features.extendedDynamicState3ColorBlendEnable = VK_TRUE;
    extendedDynamicState3Features.extendedDynamicState3ColorBlendEquation = VK_TRUE;
    extendedDynamicState3Features.extendedDynamicState3ColorWriteMask = VK_TRUE;
    extendedDynamicState3Features.extendedDynamicState3LineRasterizationMode = VK_TRUE;
    extendedDynamicState2Features.pNext = &extendedDynamicState3Features;

    // for bindless descriptor support
    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures = {};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    extendedDynamicState3Features.pNext = &indexingFeatures;

    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
        VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
        VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME // for bindless descriptor support
    };
    deviceCreateInfo.enabledExtensionCount = 5;
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
    if (vkCreateDevice(s_vkPhysicalDevice, &deviceCreateInfo, nullptr, &s_vkDevice)) {
        vkDestroyDevice(s_vkDevice, nullptr);
        s_vkDevice = VK_NULL_HANDLE;
        return 1; // Error: Failed to create Vulkan device
    }

    return 0;
}

void GfxVulkanRenderer::termGlobal() {
    std::lock_guard<std::mutex> lock(s_mutex);

    vkDeviceWaitIdle(s_vkDevice);

#ifdef ENABLE_DEBUG_OUTPUT
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        s_vkInstance,
        "vkDestroyDebugUtilsMessengerEXT"
    );
    if (func != nullptr)
        func(s_vkInstance, s_debugMessenger, nullptr);
    s_debugMessenger = VK_NULL_HANDLE;
#endif // ENABLE_DEBUG_OUTPUT

    vkDestroyDevice(s_vkDevice, nullptr);
    s_vkDevice = VK_NULL_HANDLE;
    vkDestroyInstance(s_vkInstance, nullptr);
    s_vkInstance = VK_NULL_HANDLE;

    // Terminate glslang
    glslang::FinalizeProcess();
}

void GfxVulkanRenderer::setVSyncMode(GfxVSyncMode mode) {
    if (m_vkSurface == VK_NULL_HANDLE)
        return; // Error: No surface set
    m_vsyncMode = mode;
    recreateSwapchain();
}

void* GfxVulkanRenderer::getVulkanInstance() const {
    return s_vkInstance;
}

void GfxVulkanRenderer::setVulkanSurface(void* surface) {
    m_vkSurface = static_cast<VkSurfaceKHR>(surface);
}

int GfxVulkanRenderer::setSwapchainSize(int width, int height) {
    if (m_vkSurface == VK_NULL_HANDLE)
        return 1; // Error: No surface set

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_vkPhysicalDevice, m_vkSurface, &surfaceCapabilities);

    VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    extent.width = std::clamp(
        extent.width,
        surfaceCapabilities.minImageExtent.width,
        surfaceCapabilities.maxImageExtent.width
    );
    extent.height = std::clamp(
        extent.height,
        surfaceCapabilities.minImageExtent.height,
        surfaceCapabilities.maxImageExtent.height
    );
    m_swapchainExtent = extent;

    if (recreateSwapchain())
        return 1; // Error: Failed to recreate swapchain

    return 0;
}

void GfxVulkanRenderer::waitDeviceIdle() const {
    vkDeviceWaitIdle(s_vkDevice);
}

int GfxVulkanRenderer::initForImGui(const std::function<void(void*)>& initFunc) {
    ImGuiVulkanInitInfo info{};
    info.instance = s_vkInstance;
    info.physicalDevice = s_vkPhysicalDevice;
    info.device = s_vkDevice;
    info.queue = m_vkGraphicsQueue;
    info.queueFamily = findQueueFamily(s_vkPhysicalDevice).index;
    info.imageCount = static_cast<uint32_t>(m_swapchainImageViews.size());
    info.samples = static_cast<uint32_t>(m_samples);
    info.swapchainFormat = static_cast<uint32_t>(m_swapchainImageFormat);

    // Create the render pass using swapchain specifications
    if (m_ImGuiRenderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(s_vkDevice, m_ImGuiRenderPass, nullptr);
    if (createSwapchainRenderPass(m_ImGuiRenderPass))
        return 1; // Error: Failed to create ImGui render pass

    info.renderPass = m_ImGuiRenderPass;

    initFunc(&info);

    return 0;
}

void GfxVulkanRenderer::termForImGui(const std::function<void()>& termFunc) {
    termFunc();
    vkDestroyRenderPass(s_vkDevice, m_ImGuiRenderPass, nullptr);
    m_ImGuiRenderPass = VK_NULL_HANDLE;
}

void GfxVulkanRenderer::renderForImGui(const std::function<void(void*)>& renderFunc) const {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_ImGuiRenderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[m_imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapchainExtent;
    vkCmdBeginRenderPass(
        m_vkCommandBuffers[m_currentFrame],
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    renderFunc(m_vkCommandBuffers[m_currentFrame]);

    vkCmdEndRenderPass(m_vkCommandBuffers[m_currentFrame]);
}

GfxRendererInterface::ImageInfo GfxVulkanRenderer::getImageInfo(const GfxImage& image) const {
    std::shared_ptr<GfxVulkanImage> vulkanImage =
        std::static_pointer_cast<GfxVulkanImage>(image);
    ImageVulkanInfo info{};
    info.sampler = vulkanImage->m_sampler;
    info.imageView = vulkanImage->m_imageView;
    info.imageLayout = static_cast<uint32_t>(vulkanImage->m_currentLayout);
    return info;
}

void GfxVulkanRenderer::setSamples(int samples) {
    if (m_vkSurface == VK_NULL_HANDLE)
        return; // Error: No surface set
    m_samples = getSampleCount(samples);
    vkDestroyRenderPass(s_vkDevice, m_swapchainRenderPass, nullptr);
    m_swapchainRenderPass = VK_NULL_HANDLE;
    recreateSwapchain();
}

GfxImage GfxVulkanRenderer::createImage(const GfxImageInfo& info) const {
    GfxVulkanImage* image_ptr = new GfxVulkanImage(info);
    GfxImage image(
        image_ptr,
        [this](GfxImage_T* img) {
            if (img)
                this->destroyImage(GfxImage(img, [](GfxImage_T*) {}));
        }
    );
    std::shared_ptr<GfxVulkanImage> vulkanImage =
        std::static_pointer_cast<GfxVulkanImage>(image);
    vulkanImage->m_mipmapViews.resize(image->getLevels());

    // Type specific parameters
    VkImageUsageFlags vkUsage = 0;
    VkFormat vkFormat = GfxVkTypeConverter::toVkFormat(info.format);
    VkImageAspectFlagBits aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    uint32_t levels = image->getLevels();
    if (info.usages.check(GfxImageUsage::COLOR_ATTACHMENT)) {
        if (vkFormat == VK_FORMAT_UNDEFINED)
            vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
        vkUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (info.usages.check(GfxImageUsage::DEPTH_ATTACHMENT)) {
        if (vkFormat == VK_FORMAT_UNDEFINED && findDepthFormat(vkFormat))
            return nullptr; // Error: Failed to find a suitable depth format
        vkUsage |=
            VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (info.usages.check(GfxImageUsage::SAMPLED_TEXTURE)) {
        if (vkFormat == VK_FORMAT_UNDEFINED)
            vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
        vkUsage |=
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (info.usages == GfxImageUsage::STORAGE_IMAGE) {
        if (vkFormat == VK_FORMAT_UNDEFINED)
            vkFormat = VK_FORMAT_R8G8B8A8_SNORM;
        vkUsage =
            VK_IMAGE_USAGE_STORAGE_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    int err = 0;
    GfxScopeGuard errCleaner(
        [&]() {
            if (err)
                destroyImage(image);
        }
    );

    // Create image
    CreateVkImageInfo createImageInfo{};
    createImageInfo.width = static_cast<uint32_t>(info.width);
    createImageInfo.height = static_cast<uint32_t>(info.height);
    createImageInfo.mipLevels = levels;
    createImageInfo.numSamples = getSampleCount(info.samples);
    createImageInfo.format = vkFormat;
    createImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createImageInfo.usage = vkUsage;
    createImageInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    err = createVkImage(
        createImageInfo,
        vulkanImage->m_image,
        vulkanImage->m_imageMemory
    );
    if (err)
        return nullptr;

    // Transition image layout for sampled textures and storage images
    if (info.usages.check(GfxImageUsage::SAMPLED_TEXTURE)) {
        err = transitionImageLayout(
            vulkanImage->m_image,
            vkFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            levels
        );
        if (err)
            return nullptr;
        vulkanImage->m_currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } else if (info.usages == GfxImageUsage::STORAGE_IMAGE) {
        err = transitionImageLayout(
            vulkanImage->m_image,
            vkFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL,
            levels
        );
        if (err)
            return nullptr;
        vulkanImage->m_currentLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    // Create image views
    err = createVkImageView(
        vulkanImage->m_image,
        vkFormat,
        aspectFlags,
        0,
        levels,
        vulkanImage->m_imageView
    );
    if (err)
        return nullptr; // Error: Failed to create image view
    for (int i = 0; i < levels; i++) {
        err = createVkImageView(
            vulkanImage->m_image,
            vkFormat,
            aspectFlags,
            static_cast<uint32_t>(i),
            1,
            vulkanImage->m_mipmapViews[i]
        );
        if (err)
            return nullptr; // Error: Failed to create mipmap view
    }

    // Create texture sampler
    VkFilter magFilter = static_cast<VkFilter>(info.magFilter);
    VkFilter minFilter = static_cast<VkFilter>(info.minFilter);

    VkSamplerAddressMode addressModeU = static_cast<VkSamplerAddressMode>(info.wrapS);
    VkSamplerAddressMode addressModeV = static_cast<VkSamplerAddressMode>(info.wrapT);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = magFilter;
    samplerInfo.minFilter = minFilter;

    samplerInfo.addressModeU = addressModeU;
    samplerInfo.addressModeV = addressModeV;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(s_vkPhysicalDevice, &properties);
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = static_cast<VkSamplerMipmapMode>(info.mipmapFilter);
    samplerInfo.mipLodBias = info.lodBias;
    samplerInfo.minLod = static_cast<float>(info.minLod);
    samplerInfo.maxLod = static_cast<float>(info.maxLod);

    if (vkCreateSampler(s_vkDevice, &samplerInfo, nullptr, &vulkanImage->m_sampler)) {
        err = 1;
        return nullptr; // Error: Failed to create texture sampler
    }

    return image;
}

int GfxVulkanRenderer::setImageData(const GfxImage& image, void* data) const {
    std::shared_ptr<GfxVulkanImage> vulkanImage =
        std::static_pointer_cast<GfxVulkanImage>(image);
    int width = image->getWidth();
    int height = image->getHeight();

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

    GfxScopeGuard cleaner(
        [&]() {
            vkDestroyBuffer(s_vkDevice, stagingBuffer, nullptr);
            vkFreeMemory(s_vkDevice, stagingBufferMemory, nullptr);
        }
    );

    // Create buffer
    VkDeviceSize imageSize =
        static_cast<VkDeviceSize>(width) *
        static_cast<VkDeviceSize>(height) * 4;
    int err = createVkBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );
    if (err)
        return err; // Error: Failed to create staging buffer

    void* dstData;
    VkResult result = vkMapMemory(
        s_vkDevice,
        stagingBufferMemory,
        0,
        imageSize,
        0,
        &dstData
    );
    if (result != VK_SUCCESS)
        return 1; // Error: Failed to map memory for staging buffer
    memcpy(dstData, data, static_cast<int>(imageSize));
    vkUnmapMemory(s_vkDevice, stagingBufferMemory);

    // Copy buffer to image
    {
        VkFormat format = GfxVkTypeConverter::toVkFormat(vulkanImage->getFormat());

        err = transitionImageLayout(
            vulkanImage->m_image,
            format,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            image->getLevels()
        );
        if (err)
            return 1; // Error: Failed to transition image layout

        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            stagingBuffer,
            vulkanImage->m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endSingleTimeCommands(commandBuffer);

        err = transitionImageLayout(
            vulkanImage->m_image,
            format,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            image->getLevels()
        );
        if (err)
            return 1; // Error: Failed to transition image layout
    }

    return 0;
}

int GfxVulkanRenderer::getImageData(const GfxImage& image, void* data) const {
    GfxRect rect{ 0, 0, image->getWidth(), image->getHeight() };
    return readImageData(image, rect, data);
}

int GfxVulkanRenderer::generateMipmaps(const GfxImage& image) const {
    std::shared_ptr<GfxVulkanImage> vulkanImage =
        std::static_pointer_cast<GfxVulkanImage>(image);
    int width = image->getWidth();
    int height = image->getHeight();

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(
        s_vkPhysicalDevice,
        GfxVkTypeConverter::toVkFormat(vulkanImage->getFormat()),
        &formatProperties
    );
    if (!(formatProperties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        return 1; // Error: Linear blitting not supported for this format
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = vulkanImage->m_image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = image->getLevels();
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );

    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = static_cast<uint32_t>(width);
    int32_t mipHeight = static_cast<uint32_t>(height);
    uint32_t mipLevels = static_cast<uint32_t>(image->getLevels());

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = {
            mipWidth > 1 ? mipWidth / 2 : 1,
            mipHeight > 1 ? mipHeight / 2 : 1,
            1
        };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(
            commandBuffer,
            vulkanImage->m_image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            vulkanImage->m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blit,
            VK_FILTER_LINEAR
        );

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );

    endSingleTimeCommands(commandBuffer);

    return 0;
}

void GfxVulkanRenderer::copyImage(
    const GfxImage& src,
    const GfxImage& dst,
    int width,
    int height
) {
    std::shared_ptr<GfxVulkanImage> vulkanImageSrc =
        std::static_pointer_cast<GfxVulkanImage>(src);
    std::shared_ptr<GfxVulkanImage> vulkanImageDst =
        std::static_pointer_cast<GfxVulkanImage>(dst);

    VkImageCopy copyRegion{};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.extent.width = static_cast<uint32_t>(width);
    copyRegion.extent.height = static_cast<uint32_t>(height);
    copyRegion.extent.depth = 1;

    std::shared_ptr<GfxVulkanImage> dstImage =
        std::static_pointer_cast<GfxVulkanImage>(dst);
    VkFormat dstFormat = GfxVkTypeConverter::toVkFormat(dst->getFormat());
    int err = transitionImageLayout(
        dstImage->m_image,
        dstFormat,
        dstImage->m_currentLayout,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        dst->getLevels(),
        m_vkCommandBuffers[m_currentFrame]
    );
    if (err)
        return; // Error: Failed to transition image layout

    vkCmdCopyImage(
        m_vkCommandBuffers[m_currentFrame],
        vulkanImageSrc->m_image,
        vulkanImageSrc->m_currentLayout,
        vulkanImageDst->m_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &copyRegion
    );

    err = transitionImageLayout(
        dstImage->m_image,
        dstFormat,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        dstImage->m_currentLayout,
        dst->getLevels(),
        m_vkCommandBuffers[m_currentFrame]
    );
    if (err)
        return; // Error: Failed to transition image layout
}

void GfxVulkanRenderer::destroyImage(const GfxImage& image) const {
    std::shared_ptr<GfxVulkanImage> vulkanImage =
        std::static_pointer_cast<GfxVulkanImage>(image);
    vkDestroySampler(s_vkDevice, vulkanImage->m_sampler, nullptr);
    vulkanImage->m_sampler = VK_NULL_HANDLE;
    vkDestroyImageView(s_vkDevice, vulkanImage->m_imageView, nullptr);
    vulkanImage->m_imageView = VK_NULL_HANDLE;
    for (auto& view : vulkanImage->m_mipmapViews) {
        vkDestroyImageView(s_vkDevice, view, nullptr);
        view = VK_NULL_HANDLE;
    }
    vkDestroyImage(s_vkDevice, vulkanImage->m_image, nullptr);
    vulkanImage->m_image = VK_NULL_HANDLE;
    vkFreeMemory(s_vkDevice, vulkanImage->m_imageMemory, nullptr);
    vulkanImage->m_imageMemory = VK_NULL_HANDLE;
}

GfxRenderPass GfxVulkanRenderer::createRenderPass(
    const std::vector<GfxAttachment>& colorAttachments,
    const GfxAttachment& depthAttachment
) const {
    GfxRenderPass renderPass = std::make_shared<GfxVulkanRenderPass>(
            colorAttachments,
            depthAttachment
        );
    std::shared_ptr<GfxVulkanRenderPass> vulkanRenderPass =
        std::static_pointer_cast<GfxVulkanRenderPass>(renderPass);

    std::vector<VkAttachmentDescription> attachments{};
    int attachmentCount = 0;

    std::vector<VkAttachmentReference> colorAttachmentReferences{};
    colorAttachmentReferences.reserve(colorAttachments.size());
    for (const auto& colorAttachment : vulkanRenderPass->getColorAttachments()) {
        if (vulkanRenderPass->m_samples == 0)
            vulkanRenderPass->m_samples = colorAttachment.samples;
        else if (vulkanRenderPass->m_samples != colorAttachment.samples)
            return nullptr; // Error: Inconsistent sample counts in color attachments

        VkAttachmentDescription attachment{};
        attachment.format = GfxVkTypeConverter::toVkFormat(colorAttachment.format);
        attachment.samples = getSampleCount(colorAttachment.samples);
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments.push_back(attachment);

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = attachmentCount++;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentReferences.push_back(colorAttachmentRef);
    }

    VkAttachmentReference depthAttachmentRef{};
    if (depthAttachment.usages.check(GfxImageUsage::DEPTH_ATTACHMENT)) {
        if (vulkanRenderPass->m_samples == 0)
            vulkanRenderPass->m_samples = depthAttachment.samples;
        else if (vulkanRenderPass->m_samples != depthAttachment.samples)
            return nullptr; // Error: Inconsistent sample counts in depth attachment

        VkAttachmentDescription attachment{};
        attachment.format = GfxVkTypeConverter::toVkFormat(depthAttachment.format);
        attachment.samples = getSampleCount(depthAttachment.samples);
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments.push_back(attachment);

        depthAttachmentRef.attachment = attachmentCount++;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    std::vector<VkAttachmentReference> resolveAttachmentRefs{};
    if (vulkanRenderPass->m_samples > 1) {
        resolveAttachmentRefs.reserve(colorAttachments.size());
        for (const auto& colorAttachment : vulkanRenderPass->getColorAttachments()) {
            VkAttachmentDescription resolveAttachment{};
            resolveAttachment.format =
                GfxVkTypeConverter::toVkFormat(colorAttachment.format);
            resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            attachments.push_back(resolveAttachment);

            VkAttachmentReference resolveAttachmentRef{};
            resolveAttachmentRef.attachment = attachmentCount++;
            resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            resolveAttachmentRefs.push_back(resolveAttachmentRef);
        }
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());
    subpass.pColorAttachments = colorAttachmentReferences.data();
    subpass.pDepthStencilAttachment = nullptr;
    if (depthAttachment.usages.check(GfxImageUsage::DEPTH_ATTACHMENT))
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    if (vulkanRenderPass->m_samples > 1)
        subpass.pResolveAttachments = resolveAttachmentRefs.data();

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkResult result = vkCreateRenderPass(
        s_vkDevice,
        &renderPassInfo,
        nullptr,
        &vulkanRenderPass->m_vkRenderPass
    );
    if (result != VK_SUCCESS)
        return nullptr; // Error: Failed to create render pass

    return renderPass;
}

void GfxVulkanRenderer::destroyRenderPass(const GfxRenderPass& renderPass) const {
    std::shared_ptr<GfxVulkanRenderPass> vulkanRenderPass =
        std::static_pointer_cast<GfxVulkanRenderPass>(renderPass);
    vkDestroyRenderPass(s_vkDevice, vulkanRenderPass->m_vkRenderPass, nullptr);
    vulkanRenderPass->m_vkRenderPass = VK_NULL_HANDLE;
    vulkanRenderPass->m_samples = 0;
}

GfxFramebuffer GfxVulkanRenderer::createFramebuffer(
    const GfxRenderPass& renderPass,
    const std::vector<GfxImage>& colorImages,
    const GfxImage& depthImage,
    const std::vector<GfxImage>& colorResolveImages
) const {
    GfxFramebuffer framebuffer = std::make_shared<GfxVulkanFramebuffer>(
            renderPass,
            colorImages,
            depthImage,
            colorResolveImages
        );
    std::shared_ptr<GfxVulkanFramebuffer> vulkanFramebuffer =
        std::static_pointer_cast<GfxVulkanFramebuffer>(framebuffer);
    std::shared_ptr<GfxVulkanRenderPass> vulkanRenderPass =
        std::static_pointer_cast<GfxVulkanRenderPass>(renderPass);

    std::vector<VkImageView> attachments{};

    if (colorImages.size() != renderPass->getColorAttachments().size())
        return nullptr; // Error: Mismatched number of color images
    for (int i = 0; i < colorImages.size(); i++) {
        std::shared_ptr<GfxVulkanImage> image =
            std::static_pointer_cast<GfxVulkanImage>(colorImages[i]);

        if (vulkanFramebuffer->m_width == 0 || vulkanFramebuffer->m_height == 0) {
            vulkanFramebuffer->m_width = image->getWidth();
            vulkanFramebuffer->m_height = image->getHeight();
        } else if (vulkanFramebuffer->m_width != image->getWidth() ||
            vulkanFramebuffer->m_height != image->getHeight()) {
            return nullptr; // Error: Inconsistent dimensions in color images
        }

        int level = renderPass->getColorAttachments()[i].level;
        attachments.push_back(image->m_mipmapViews[level]);
    }
    if (depthImage) {
        GfxAttachment depthAttachment = renderPass->getDepthAttachment();
        if (!depthAttachment.usages.check(GfxImageUsage::DEPTH_ATTACHMENT))
            return nullptr; // Error: Render pass has no depth attachment

        std::shared_ptr<GfxVulkanImage> vulkanImage =
            std::static_pointer_cast<GfxVulkanImage>(depthImage);

        if (vulkanFramebuffer->m_width == 0 || vulkanFramebuffer->m_height == 0) {
            vulkanFramebuffer->m_width = vulkanImage->getWidth();
            vulkanFramebuffer->m_height = vulkanImage->getHeight();
        } else if (vulkanFramebuffer->m_width != vulkanImage->getWidth() ||
            vulkanFramebuffer->m_height != vulkanImage->getHeight()) {
            return nullptr; // Error: Inconsistent dimensions in depth image
        }

        attachments.push_back(vulkanImage->m_mipmapViews[depthAttachment.level]);
    }
    if (vulkanRenderPass->m_samples > 1) {
        if (colorResolveImages.size() != renderPass->getColorAttachments().size())
            return nullptr; // Error: Mismatched number of resolve images
        for (int i = 0; i < colorResolveImages.size(); i++) {
            std::shared_ptr<GfxVulkanImage> vulkanImage =
                std::static_pointer_cast<GfxVulkanImage>(colorResolveImages[i]);
            if (vulkanFramebuffer->m_width == 0 || vulkanFramebuffer->m_height == 0) {
                vulkanFramebuffer->m_width = vulkanImage->getWidth();
                vulkanFramebuffer->m_height = vulkanImage->getHeight();
            } else if (vulkanFramebuffer->m_width != vulkanImage->getWidth() ||
                vulkanFramebuffer->m_height != vulkanImage->getHeight()) {
                return nullptr; // Error: Inconsistent dimensions in resolve images
            }
            int level = renderPass->getColorAttachments()[i].level;
            attachments.push_back(vulkanImage->m_mipmapViews[level]);
        }
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vulkanRenderPass->m_vkRenderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = static_cast<uint32_t>(vulkanFramebuffer->m_width);
    framebufferInfo.height = static_cast<uint32_t>(vulkanFramebuffer->m_height);
    framebufferInfo.layers = 1;

    int err = vkCreateFramebuffer(
        s_vkDevice,
        &framebufferInfo,
        nullptr,
        &vulkanFramebuffer->m_vkFramebuffer
    );
    if (err)
        return nullptr; // Error: Failed to create framebuffer

    return framebuffer;
}

void GfxVulkanRenderer::destroyFramebuffer(const GfxFramebuffer& framebuffer) const {
    std::shared_ptr<GfxVulkanFramebuffer> vulkanFramebuffer =
        std::static_pointer_cast<GfxVulkanFramebuffer>(framebuffer);
    vkDestroyFramebuffer(s_vkDevice, vulkanFramebuffer->m_vkFramebuffer, nullptr);
    vulkanFramebuffer->m_vkFramebuffer = VK_NULL_HANDLE;
}

int GfxVulkanRenderer::readFramebufferColorAttachmentPixels(
    const GfxFramebuffer& framebuffer,
    int index,
    const GfxRect& rect,
    void* pixels
) const {
    if (!framebuffer || index < 0 || index >= framebuffer->getColorImages().size() || !pixels)
        return 1; // Error: Invalid arguments
    GfxImage image = framebuffer->getColorImages()[index];
    if (!framebuffer->getColorResolveImages().empty())
        image = framebuffer->getColorResolveImages()[index];
    return readImageData(image, rect, pixels);
}

GfxBuffer GfxVulkanRenderer::createBuffer(
    int size,
    GfxBufferUsage usage,
    GfxBufferProp prop
) const {
    if (size <= 0)
        return nullptr; // Error: Invalid buffer size

    GfxBuffer buffer = std::make_shared<GfxVulkanBuffer>(size, usage, prop);
    std::shared_ptr<GfxVulkanBuffer> vulkanBuffer =
        std::static_pointer_cast<GfxVulkanBuffer>(buffer);

    if (resizeVkBuffer(buffer, size))
        return nullptr; // Error: Failed to create Vulkan buffer

    return vulkanBuffer;
}

int GfxVulkanRenderer::setBufferData(const GfxBuffer& buffer, int size, const void* data) const {
    std::shared_ptr<GfxVulkanBuffer> vulkanBuffer =
        std::static_pointer_cast<GfxVulkanBuffer>(buffer);

    // Resize the buffer if necessary
    if (vulkanBuffer->getSize() != size) {
        if (resizeVkBuffer(buffer, size))
            return 1; // Error: Failed to recreate Vulkan buffer
        vulkanBuffer->setSize(size);
    }

    if (updateBufferData(buffer, 0, size, data))
        return 1; // Error: Failed to update buffer data

    return 0;
}

int GfxVulkanRenderer::updateBufferData(
    const GfxBuffer& buffer,
    int offset,
    int size,
    const void* data
) const {
    std::shared_ptr<GfxVulkanBuffer> vulkanBuffer =
        std::static_pointer_cast<GfxVulkanBuffer>(buffer);

    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(vulkanBuffer->getSize());
    VkDeviceSize updateSize = static_cast<VkDeviceSize>(size);
    VkDeviceSize offsetSize = static_cast<VkDeviceSize>(offset);

    if (updateSize > bufferSize - offsetSize)
        return 1; // Error: Update size exceeds buffer size

    GfxBufferUsage usage = vulkanBuffer->getUsage();
    GfxBufferProp prop = vulkanBuffer->getProp();

    VkBuffer vkBuffer = vulkanBuffer->m_vkBuffers[0];
    VkDeviceMemory vkBufferMemory = vulkanBuffer->m_vkBufferMemories[0];
    if (usage == GfxBufferUsage::UNIFORM_BUFFER || usage == GfxBufferUsage::STORAGE_BUFFER) {
        vkBuffer = vulkanBuffer->m_vkBuffers[m_currentFrame];
        vkBufferMemory = vulkanBuffer->m_vkBufferMemories[m_currentFrame];
    }

    if (prop == GfxBufferProp::STATIC) {
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

        GfxScopeGuard cleaner(
            [&]() {
                vkDestroyBuffer(s_vkDevice, stagingBuffer, nullptr);
                vkFreeMemory(s_vkDevice, stagingBufferMemory, nullptr);
            }
        );

        int err = createVkBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );
        if (err)
            return err; // Error: Failed to create staging buffer

        void* dstData;
        VkResult result = vkMapMemory(
            s_vkDevice,
            stagingBufferMemory,
            0,
            bufferSize,
            0,
            &dstData
        );
        if (result != VK_SUCCESS)
            return 1; // Error: Failed to map memory for staging buffer
        memcpy(static_cast<char*>(dstData) + offsetSize, data, static_cast<int>(updateSize));
        vkUnmapMemory(s_vkDevice, stagingBufferMemory);

        // Copy staging buffer to the Vulkan buffer
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = offsetSize;
            copyRegion.dstOffset = offsetSize;
            copyRegion.size = size;
            vkCmdCopyBuffer(
                commandBuffer,
                stagingBuffer,
                vkBuffer,
                1,
                &copyRegion
            );

            VkAccessFlags dstAccessMask = 0;
            VkPipelineStageFlags dstStageMask = 0;
            if (usage == GfxBufferUsage::VERTEX_BUFFER) {
                dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            } else if (usage == GfxBufferUsage::INDEX_BUFFER) {
                dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
                dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            } else if (usage == GfxBufferUsage::UNIFORM_BUFFER) {
                dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
                dstStageMask =
                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else if (usage == GfxBufferUsage::STORAGE_BUFFER) {
                dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                dstStageMask =
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }

            VkBufferMemoryBarrier bufferBarrier{};
            bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            bufferBarrier.dstAccessMask = dstAccessMask;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.buffer = vkBuffer;
            bufferBarrier.offset = offsetSize;
            bufferBarrier.size = updateSize;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                dstStageMask,
                0,
                0,
                nullptr,
                1,
                &bufferBarrier,
                0,
                nullptr
            );

            endSingleTimeCommands(commandBuffer);
        }
    } else if (prop == GfxBufferProp::DYNAMIC) {
        void* dstData;
        VkResult result = vkMapMemory(
            s_vkDevice,
            vkBufferMemory,
            offsetSize,
            updateSize,
            0,
            &dstData
        );
        if (result != VK_SUCCESS)
            return 1; // Error: Failed to map memory for Vulkan buffer
        memcpy(dstData, data, static_cast<int>(updateSize));
        vkUnmapMemory(s_vkDevice, vkBufferMemory);
    }

    return 0;
}

void GfxVulkanRenderer::destroyBuffer(const GfxBuffer& buffer) const {
    std::shared_ptr<GfxVulkanBuffer> vulkanBuffer =
        std::static_pointer_cast<GfxVulkanBuffer>(buffer);
    for (auto& vkBuffer : vulkanBuffer->m_vkBuffers)
        vkDestroyBuffer(s_vkDevice, vkBuffer, nullptr);
    for (auto& vkBufferMemory : vulkanBuffer->m_vkBufferMemories)
        vkFreeMemory(s_vkDevice, vkBufferMemory, nullptr);
}

int GfxVulkanRenderer::readBufferData(
    const GfxBuffer& buffer,
    int offset,
    int size,
    void* data
) const {
    std::shared_ptr<GfxVulkanBuffer> vulkanBuffer =
        std::static_pointer_cast<GfxVulkanBuffer>(buffer);

    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(vulkanBuffer->getSize());
    VkDeviceSize readSize = static_cast<VkDeviceSize>(size);
    VkDeviceSize offsetSize = static_cast<VkDeviceSize>(offset);

    GfxBufferUsage usage = vulkanBuffer->getUsage();
    GfxBufferProp prop = vulkanBuffer->getProp();

    VkBuffer vkBuffer = vulkanBuffer->m_vkBuffers[0];
    VkDeviceMemory vkBufferMemory = vulkanBuffer->m_vkBufferMemories[0];
    if (usage == GfxBufferUsage::UNIFORM_BUFFER || usage == GfxBufferUsage::STORAGE_BUFFER) {
        vkBuffer = vulkanBuffer->m_vkBuffers[m_currentFrame];
        vkBufferMemory = vulkanBuffer->m_vkBufferMemories[m_currentFrame];
    }

    if (prop == GfxBufferProp::STATIC) {
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

        GfxScopeGuard cleaner(
            [&]() {
                vkDestroyBuffer(s_vkDevice, stagingBuffer, nullptr);
                vkFreeMemory(s_vkDevice, stagingBufferMemory, nullptr);
            }
        );

        int err = createVkBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );
        if (err)
            return 1; // Error: Failed to create staging buffer

        VkBufferCopy copyRegion = {};
        copyRegion.size = bufferSize;
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();
            vkCmdCopyBuffer(
                commandBuffer,
                vkBuffer,
                stagingBuffer,
                1,
                &copyRegion
            );
            endSingleTimeCommands(commandBuffer);
        }

        void* srcData;
        VkResult result = vkMapMemory(
            s_vkDevice,
            stagingBufferMemory,
            offsetSize,
            readSize,
            0,
            &srcData
        );
        if (result != VK_SUCCESS)
            return 1; // Error: Failed to map memory for staging buffer
        memcpy(data, static_cast<char*>(srcData), static_cast<int>(readSize));
    } else if (prop == GfxBufferProp::DYNAMIC) {
        void* srcData;
        VkResult result = vkMapMemory(
            s_vkDevice,
            vkBufferMemory,
            offsetSize,
            readSize,
            0,
            &srcData
        );
        if (result != VK_SUCCESS)
            return 1; // Error: Failed to map memory for Vulkan buffer
        memcpy(data, static_cast<char*>(srcData), static_cast<int>(readSize));
    }

    return 0;
}

int GfxVulkanRenderer::copyBuffer(
    const GfxBuffer& src,
    const GfxBuffer& dst,
    int srcOffset,
    int dstOffset,
    int size
) const {
    std::shared_ptr<GfxVulkanBuffer> vulkanBufferSrc =
        std::static_pointer_cast<GfxVulkanBuffer>(src);
    std::shared_ptr<GfxVulkanBuffer> vulkanBufferDst =
        std::static_pointer_cast<GfxVulkanBuffer>(dst);
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = static_cast<VkDeviceSize>(srcOffset);
    copyRegion.dstOffset = static_cast<VkDeviceSize>(dstOffset);
    copyRegion.size = static_cast<VkDeviceSize>(size);
    VkBuffer vkBufferSrc = vulkanBufferSrc->m_vkBuffers[0];
    VkBuffer vkBufferDst = vulkanBufferDst->m_vkBuffers[0];
    if (vulkanBufferSrc->getUsage() == GfxBufferUsage::UNIFORM_BUFFER ||
        vulkanBufferSrc->getUsage() == GfxBufferUsage::STORAGE_BUFFER) {
        vkBufferSrc = vulkanBufferSrc->m_vkBuffers[m_currentFrame];
    }
    if (vulkanBufferDst->getUsage() == GfxBufferUsage::UNIFORM_BUFFER ||
        vulkanBufferDst->getUsage() == GfxBufferUsage::STORAGE_BUFFER) {
        vkBufferDst = vulkanBufferDst->m_vkBuffers[m_currentFrame];
    }
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        vkCmdCopyBuffer(
            commandBuffer,
            vkBufferSrc,
            vkBufferDst,
            1,
            &copyRegion
        );
        endSingleTimeCommands(commandBuffer);
    }
    return 0;
}

GfxVAO GfxVulkanRenderer::createVAO(
    const GfxVertexDesc& vertexDesc,
    const GfxBuffer& vertexBuffer,
    const GfxBuffer& indexBuffer
) const {
    return std::make_shared<GfxVAO_T>(vertexDesc, vertexBuffer, indexBuffer);
}

GfxShader GfxVulkanRenderer::createShader(
    GfxShaderStage stage,
    const std::string& source
) const {
    GfxShader shader = std::make_shared<GfxVulkanShader>(stage);
    std::shared_ptr<GfxVulkanShader> vulkanShader =
        std::static_pointer_cast<GfxVulkanShader>(shader);

    EShLanguage shaderStage = EShLangVertex;
    switch (stage) {
    case GfxShaderStage::VERTEX:
        shaderStage = EShLangVertex;
        break;
    case GfxShaderStage::TESS_CTRL:
        shaderStage = EShLangTessControl;
        break;
    case GfxShaderStage::TESS_EVAL:
        shaderStage = EShLangTessEvaluation;
        break;
    case GfxShaderStage::GEOMETRY:
        shaderStage = EShLangGeometry;
        break;
    case GfxShaderStage::FRAGMENT:
        shaderStage = EShLangFragment;
        break;
    case GfxShaderStage::COMPUTE:
        shaderStage = EShLangCompute;
        break;
    default:
        return nullptr; // Error: Invalid shader stage
    }

    glslang::TShader tShader(shaderStage);
    const char* string = source.c_str();
    const char* const* strings = &string;
    tShader.setStrings(strings, 1);
    tShader.setEnvInput(glslang::EShSourceGlsl, shaderStage, glslang::EShClientVulkan, 100);
    tShader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    tShader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);

    if (!tShader.parse(GetDefaultResources(), 100, ENoProfile, false, false, EShMsgDefault))
        throw GfxShaderException(tShader.getInfoLog()); // Error: Failed to parse shader

    glslang::TProgram program;
    program.addShader(&tShader);
    if (!program.link(EShMsgDefault))
        return nullptr; // Error: Failed to link shader program
    const auto intermediate = program.getIntermediate(shaderStage);

    std::vector<uint32_t> spvCode;
    glslang::GlslangToSpv(*intermediate, spvCode);

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = spvCode.size() * sizeof(unsigned int);
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(spvCode.data());

    int err = vkCreateShaderModule(
        s_vkDevice,
        &shaderModuleCreateInfo,
        nullptr,
        &vulkanShader->m_vkShaderModule
    );
    if (err)
        return nullptr; // Error: Failed to create shader module

    return shader;
}

void GfxVulkanRenderer::destroyShader(const GfxShader& shader) const {
    std::shared_ptr<GfxVulkanShader> vulkanShader =
        std::static_pointer_cast<GfxVulkanShader>(shader);
    vkDestroyShaderModule(s_vkDevice, vulkanShader->m_vkShaderModule, nullptr);
    vulkanShader->m_vkShaderModule = VK_NULL_HANDLE;
}

GfxPipeline GfxVulkanRenderer::createPipeline(
    const std::vector<GfxShader>& shaders,
    const std::vector<GfxDescriptorSet>& descriptorSets,
    const GfxVertexDesc& vertexDesc,
    const std::vector<GfxPipelineState>& dynamicStates,
    const GfxRenderPass& renderPass
) const {
    GfxPipeline pipeline = std::make_shared<GfxVulkanPipeline>(
            renderPass,
            descriptorSets,
            dynamicStates
        );
    std::shared_ptr<GfxVulkanPipeline> vulkanPipeline =
        std::static_pointer_cast<GfxVulkanPipeline>(pipeline);

    int err = 0;
    GfxScopeGuard errCleaner(
        [&]() {
            if (err)
                destroyPipeline(pipeline);
        }
    );

    // Gather shader stages
    bool computePipeline = false;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(shaders.size());
    for (auto& shader : shaders) {
        vulkanPipeline->setStage(shader->getStage());

        std::shared_ptr<GfxVulkanShader> vulkanShader =
            std::static_pointer_cast<GfxVulkanShader>(shader);
        VkShaderStageFlagBits stageFlag =
            static_cast<VkShaderStageFlagBits>(shader->getStage());
        computePipeline = computePipeline || (stageFlag == VK_SHADER_STAGE_COMPUTE_BIT);

        VkPipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = stageFlag;
        shaderStageInfo.module = vulkanShader->m_vkShaderModule;
        shaderStageInfo.pName = "main";

        shaderStages.push_back(shaderStageInfo);
    }

    // Create pipeline descriptor sets
    if (err = createVkPipelineDescriptorSets(vulkanPipeline, descriptorSets))
        return nullptr; // Error: Failed to create pipeline descriptor sets

    // Gather vertex input information
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = vertexDesc.stride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.reserve(vertexDesc.attributes.size());
    for (auto& attribute : vertexDesc.attributes) {
        VkVertexInputAttributeDescription attributeDescription{};
        attributeDescription.binding = 0;
        attributeDescription.location = attribute.location;
        attributeDescription.format = GfxVkTypeConverter::toVkFormat(attribute.format);
        attributeDescription.offset = attribute.offset;
        attributeDescriptions.push_back(attributeDescription);
    }
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    if (vertexDesc.stride < 0) {
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
    } else {
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    }

    // Set states
    GfxPipelineStateController::cachePipelineState(m_pipelineStateMachine, pipeline);
    GfxPipelineStateCache stateCache =
        GfxPipelineStateController::getStateCache(m_pipelineStateMachine);
    std::shared_ptr<GfxVulkanRenderPass> vulkanRenderPass = nullptr;
    int colorAttachmentCount = 1;
    VkSampleCountFlagBits samples = m_samples;
    if (renderPass != nullptr) {
        colorAttachmentCount = renderPass->getColorAttachments().size();
        vulkanRenderPass = std::static_pointer_cast<GfxVulkanRenderPass>(renderPass);
        samples = getSampleCount(vulkanRenderPass->m_samples);
    }
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology =
        static_cast<VkPrimitiveTopology>(stateCache.primitiveTopo);
    inputAssembly.primitiveRestartEnable = stateCache.primitiveRestartEnabled;
    VkExtent2D extent = {
        static_cast<uint32_t>(stateCache.viewport.width),
        static_cast<uint32_t>(stateCache.viewport.height)
    };
    VkViewport viewport{};
    viewport.x = stateCache.viewport.x;
    viewport.y = stateCache.viewport.y;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = stateCache.viewport.minDepth;
    viewport.maxDepth = stateCache.viewport.maxDepth;
    VkRect2D scissor{};
    scissor.offset = { stateCache.scissor.x, stateCache.scissor.y };
    scissor.extent = {
        static_cast<uint32_t>(stateCache.scissor.width),
        static_cast<uint32_t>(stateCache.scissor.height)
    };
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = static_cast<VkPolygonMode>(stateCache.polygonMode);
    rasterizer.lineWidth = stateCache.lineWidth;
    rasterizer.cullMode = static_cast<VkCullModeFlags>(stateCache.cullMode);
    rasterizer.frontFace = static_cast<VkFrontFace>(stateCache.frontFace);
    rasterizer.depthBiasEnable = stateCache.depthBiasEnabled;
    rasterizer.depthBiasConstantFactor = stateCache.depthBiasParams.constantFactor;
    rasterizer.depthBiasClamp = stateCache.depthBiasParams.clamp;
    rasterizer.depthBiasSlopeFactor = stateCache.depthBiasParams.slopeFactor;
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = samples & VK_SAMPLE_COUNT_1_BIT ? VK_FALSE : VK_TRUE;
    multisampling.rasterizationSamples = samples;
    multisampling.minSampleShading = 0.2f;
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    colorBlendAttachments.reserve(colorAttachmentCount);
    for (int i = 0; i < colorAttachmentCount; ++i) {
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = stateCache.colorWriteMask;
        colorBlendAttachment.blendEnable = stateCache.colorBlendEnabled;
        colorBlendAttachment.srcColorBlendFactor =
            static_cast<VkBlendFactor>(stateCache.colorBlendEquation.srcColorFactor);
        colorBlendAttachment.dstColorBlendFactor =
            static_cast<VkBlendFactor>(stateCache.colorBlendEquation.dstColorFactor);
        colorBlendAttachment.colorBlendOp =
            static_cast<VkBlendOp>(stateCache.colorBlendEquation.colorBlendOp);
        colorBlendAttachment.srcAlphaBlendFactor =
            static_cast<VkBlendFactor>(stateCache.colorBlendEquation.srcAlphaFactor);
        colorBlendAttachment.dstAlphaBlendFactor =
            static_cast<VkBlendFactor>(stateCache.colorBlendEquation.dstAlphaFactor);
        colorBlendAttachment.alphaBlendOp =
            static_cast<VkBlendOp>(stateCache.colorBlendEquation.alphaBlendOp);
        colorBlendAttachments.push_back(colorBlendAttachment);
    }
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = stateCache.logicOpEnabled;
    colorBlending.logicOp = static_cast<VkLogicOp>(stateCache.logicOp);
    colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = stateCache.blendConstants[0];
    colorBlending.blendConstants[1] = stateCache.blendConstants[1];
    colorBlending.blendConstants[2] = stateCache.blendConstants[2];
    colorBlending.blendConstants[3] = stateCache.blendConstants[3];
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = stateCache.depthTestEnabled;
    depthStencil.depthWriteEnable = stateCache.depthWriteEnabled;
    depthStencil.depthCompareOp = static_cast<VkCompareOp>(stateCache.depthCompareOp);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = stateCache.stencilTestEnabled;
    depthStencil.front.failOp =
        static_cast<VkStencilOp>(stateCache.frontFaceStencilOpParams.failOp);
    depthStencil.front.passOp =
        static_cast<VkStencilOp>(stateCache.frontFaceStencilOpParams.passOp);
    depthStencil.front.depthFailOp =
        static_cast<VkStencilOp>(stateCache.frontFaceStencilOpParams.depthFailOp);
    depthStencil.front.compareOp =
        static_cast<VkCompareOp>(stateCache.frontFaceStencilOpParams.compareOp);
    depthStencil.front.compareMask = stateCache.frontFaceStencilOpParams.compareMask;
    depthStencil.front.writeMask = stateCache.frontFaceStencilOpParams.writeMask;
    depthStencil.front.reference = stateCache.frontFaceStencilOpParams.reference;
    depthStencil.back.failOp =
        static_cast<VkStencilOp>(stateCache.backFaceStencilOpParams.failOp);
    depthStencil.back.passOp =
        static_cast<VkStencilOp>(stateCache.backFaceStencilOpParams.passOp);
    depthStencil.back.depthFailOp =
        static_cast<VkStencilOp>(stateCache.backFaceStencilOpParams.depthFailOp);
    depthStencil.back.compareOp =
        static_cast<VkCompareOp>(stateCache.backFaceStencilOpParams.compareOp);
    depthStencil.back.compareMask = stateCache.backFaceStencilOpParams.compareMask;
    depthStencil.back.writeMask = stateCache.backFaceStencilOpParams.writeMask;
    depthStencil.back.reference = stateCache.backFaceStencilOpParams.reference;

    std::vector<VkDynamicState> vkDynamicStates{};
    vkDynamicStates.reserve(dynamicStates.size());
    for (const auto& state : dynamicStates)
        vkDynamicStates.push_back(GfxVkTypeConverter::toVkDynamicState(state));
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(vkDynamicStates.size());
    dynamicState.pDynamicStates = vkDynamicStates.data();

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = vulkanPipeline->m_vkDescriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = vulkanPipeline->m_vkDescriptorSetLayouts.data();
    VkResult result = vkCreatePipelineLayout(
        s_vkDevice,
        &pipelineLayoutInfo,
        nullptr,
        &vulkanPipeline->m_vkPipelineLayout
    );
    if (result != VK_SUCCESS) {
        err = 1;
        return nullptr; // Error: Failed to create pipeline layout
    }

    // Everything looks great, create the pipeline now
    if (!computePipeline) {
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = vulkanPipeline->m_vkPipelineLayout;
        pipelineInfo.renderPass = vulkanRenderPass == nullptr ?
            m_swapchainRenderPass : vulkanRenderPass->m_vkRenderPass;
        pipelineInfo.subpass = 0;
        result = vkCreateGraphicsPipelines
        (
            s_vkDevice,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &vulkanPipeline->m_vkPipeline
        );
    } else {
        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.stage = shaderStages[0];
        pipelineInfo.layout = vulkanPipeline->m_vkPipelineLayout;
        result = vkCreateComputePipelines
        (
            s_vkDevice,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &vulkanPipeline->m_vkPipeline
        );
    }
    if (result != VK_SUCCESS) {
        err = 1;
        return nullptr; // Error: Failed to create graphics pipeline
    }

    return pipeline;
}

void GfxVulkanRenderer::destroyPipeline(const GfxPipeline& pipeline) const {
    std::shared_ptr<GfxVulkanPipeline> vulkanPipeline =
        std::static_pointer_cast<GfxVulkanPipeline>(pipeline);
    vkDestroyPipeline(s_vkDevice, vulkanPipeline->m_vkPipeline, nullptr);
    vulkanPipeline->m_vkPipeline = VK_NULL_HANDLE;
    vkDestroyPipelineLayout(s_vkDevice, vulkanPipeline->m_vkPipelineLayout, nullptr);
    vulkanPipeline->m_vkPipelineLayout = VK_NULL_HANDLE;
    for (auto& layout : vulkanPipeline->m_vkDescriptorSetLayouts)
        vkDestroyDescriptorSetLayout(s_vkDevice, layout, nullptr);
    vulkanPipeline->m_vkDescriptorSetLayouts.clear();
}

GfxDescriptorSetBinding GfxVulkanRenderer::createDescriptorSetBinding(
    const GfxPipeline& pipeline,
    int descriptorSetIndex,
    const std::vector<GfxDescriptorBinding>& bindings
) const {
    std::shared_ptr<GfxVulkanPipeline> vulkanPipeline =
        std::static_pointer_cast<GfxVulkanPipeline>(pipeline);

    GfxDescriptorSetBinding descriptorSetBinding =
        std::make_shared<GfxVulkanDescriptorSetBinding>(pipeline, descriptorSetIndex, bindings);
    std::shared_ptr<GfxVulkanDescriptorSetBinding> vulkanDescriptorSetBinding =
        std::static_pointer_cast<GfxVulkanDescriptorSetBinding>(descriptorSetBinding);

    VkDescriptorSetLayout vkDescriptorSetLayout =
        vulkanPipeline->m_vkDescriptorSetLayouts[descriptorSetIndex];
    GfxDescriptorSet descriptorSet = pipeline->getDescriptorSets()[descriptorSetIndex];

    int samplersCount = 0, uniformBuffersCount = 0;
    int storageBuffersCount = 0, storageImagesCount = 0;
    for (const auto& descriptor : descriptorSet) {
        if (descriptor.type == GfxDescriptorType::SAMPLER)
            samplersCount++;
        else if (descriptor.type == GfxDescriptorType::SAMPLERS)
            samplersCount += descriptor.size;
        else if (descriptor.type == GfxDescriptorType::UNIFORM_BUFFER)
            uniformBuffersCount++;
        else if (descriptor.type == GfxDescriptorType::STORAGE_BUFFER)
            storageBuffersCount++;
        else if (descriptor.type == GfxDescriptorType::STORAGE_IMAGE)
            storageImagesCount++;
    }

    std::vector<VkDescriptorPoolSize> poolSizes;
    if (samplersCount > 0) {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount =
            static_cast<uint32_t>(samplersCount * MAX_FRAMES_IN_FLIGHT);
        poolSizes.push_back(poolSize);
    }
    if (uniformBuffersCount > 0) {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount =
            static_cast<uint32_t>(uniformBuffersCount * MAX_FRAMES_IN_FLIGHT);
        poolSizes.push_back(poolSize);
    }
    if (storageBuffersCount > 0) {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSize.descriptorCount =
            static_cast<uint32_t>(storageBuffersCount * MAX_FRAMES_IN_FLIGHT);
        poolSizes.push_back(poolSize);
    }
    if (storageImagesCount > 0) {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        poolSize.descriptorCount =
            static_cast<uint32_t>(storageImagesCount * MAX_FRAMES_IN_FLIGHT);
        poolSizes.push_back(poolSize);
    }
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    VkResult result = vkCreateDescriptorPool
    (
        s_vkDevice,
        &poolInfo,
        nullptr,
        &vulkanDescriptorSetBinding->m_vkDescriptorPool
    );
    if (result != VK_SUCCESS)
        return nullptr; // Error: Failed to create descriptor pool

    // Allocate descriptor sets
    std::vector<VkDescriptorSetLayout> layouts(
        MAX_FRAMES_IN_FLIGHT,
        vkDescriptorSetLayout
    );
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vulkanDescriptorSetBinding->m_vkDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();
    vulkanDescriptorSetBinding->m_vkDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    result = vkAllocateDescriptorSets(
        s_vkDevice,
        &allocInfo,
        vulkanDescriptorSetBinding->m_vkDescriptorSets.data()
    );
    if (result != VK_SUCCESS) {
        destroyDescriptorSetBinding(descriptorSetBinding);
        return nullptr; // Error: Failed to allocate descriptor sets
    }

    // update descriptor sets
    updateVkDescriptorSets(descriptorSetBinding);

    return descriptorSetBinding;
}

void GfxVulkanRenderer::destroyDescriptorSetBinding(GfxDescriptorSetBinding& binding) const {
    std::shared_ptr<GfxVulkanDescriptorSetBinding> vulkanDescriptorSetBinding =
        std::static_pointer_cast<GfxVulkanDescriptorSetBinding>(binding);
    vkDestroyDescriptorPool
    (
        s_vkDevice,
        vulkanDescriptorSetBinding->m_vkDescriptorPool,
        nullptr
    );
}

int GfxVulkanRenderer::beginRenderPass(const GfxFramebuffer& framebuffer) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    if (framebuffer) {
        std::shared_ptr<GfxVulkanFramebuffer> vulkanFramebuffer =
            std::static_pointer_cast<GfxVulkanFramebuffer>(framebuffer);
        std::shared_ptr<GfxVulkanRenderPass> vulkanRenderPass =
            std::static_pointer_cast<GfxVulkanRenderPass>(framebuffer->getRenderPass());

        renderPassInfo.renderPass = vulkanRenderPass->m_vkRenderPass;
        renderPassInfo.framebuffer = vulkanFramebuffer->m_vkFramebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = {
            static_cast<uint32_t>(vulkanFramebuffer->m_width),
            static_cast<uint32_t>(vulkanFramebuffer->m_height)
        };

        for (auto& colorImage : framebuffer->getColorImages()) {
            std::shared_ptr<GfxVulkanImage> vulkanImage =
                std::static_pointer_cast<GfxVulkanImage>(colorImage);
            if (vulkanImage->m_currentLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                int err = transitionImageLayout(
                    vulkanImage->m_image,
                    GfxVkTypeConverter::toVkFormat(vulkanImage->getFormat()),
                    vulkanImage->m_currentLayout,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    1,
                    m_vkCommandBuffers[m_currentFrame]
                );
                if (err)
                    return err; // Error: Failed to transition image layout
                vulkanImage->m_currentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
        }

        if (framebuffer->getDepthImage()) {
            std::shared_ptr<GfxVulkanImage> vulkanImage =
                std::static_pointer_cast<GfxVulkanImage>(framebuffer->getDepthImage());
            if (vulkanImage->m_currentLayout !=
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                int err = transitionImageLayout(
                    vulkanImage->m_image,
                    GfxVkTypeConverter::toVkFormat(vulkanImage->getFormat()),
                    vulkanImage->m_currentLayout,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    1,
                    m_vkCommandBuffers[m_currentFrame]
                );
                if (err)
                    return err; // Error: Failed to transition depth image layout
                vulkanImage->m_currentLayout =
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
        }

        for (auto& resolveImage : framebuffer->getColorResolveImages()) {
            std::shared_ptr<GfxVulkanImage> vulkanImage =
                std::static_pointer_cast<GfxVulkanImage>(resolveImage);
            if (vulkanImage->m_currentLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                int err = transitionImageLayout(
                    vulkanImage->m_image,
                    GfxVkTypeConverter::toVkFormat(vulkanImage->getFormat()),
                    vulkanImage->m_currentLayout,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    1,
                    m_vkCommandBuffers[m_currentFrame]
                );
                if (err)
                    return err; // Error: Failed to transition resolve image layout
                vulkanImage->m_currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
        }

        m_currentFramebuffer = framebuffer;
    } else {
        // Bind swapchain framebuffer
        if (m_vkSwapchain == VK_NULL_HANDLE)
            return 1; // Error: Vulkan swapchain not set

        renderPassInfo.renderPass = m_swapchainRenderPass;
        renderPassInfo.framebuffer = m_swapchainFramebuffers[m_imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapchainExtent;
    }

    VkMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(
        m_vkCommandBuffers[m_currentFrame],
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        1,
        &barrier,
        0,
        nullptr,
        0,
        nullptr
    );

    vkCmdBeginRenderPass(
        m_vkCommandBuffers[m_currentFrame],
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    return 0;
}

void GfxVulkanRenderer::endRenderPass() {
    vkCmdEndRenderPass(m_vkCommandBuffers[m_currentFrame]);

    if (m_currentFramebuffer) {
        for (const auto& colorImage : m_currentFramebuffer->getColorImages()) {
            std::shared_ptr<GfxVulkanImage> vulkanImage =
                std::static_pointer_cast<GfxVulkanImage>(colorImage);
            vulkanImage->m_currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        if (m_currentFramebuffer->getDepthImage()) {
            GfxImage depthImage = m_currentFramebuffer->getDepthImage();
            std::shared_ptr<GfxVulkanImage> vulkanImage =
                std::static_pointer_cast<GfxVulkanImage>(depthImage);
            vulkanImage->m_currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        for (const auto& resolveImage : m_currentFramebuffer->getColorResolveImages()) {
            std::shared_ptr<GfxVulkanImage> vulkanImage =
                std::static_pointer_cast<GfxVulkanImage>(resolveImage);
            vulkanImage->m_currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }

    m_currentFramebuffer = nullptr;
}

void GfxVulkanRenderer::bindPipeline(const GfxPipeline& pipeline) {
    std::shared_ptr<GfxVulkanPipeline> vulkanPipeline =
        std::static_pointer_cast<GfxVulkanPipeline>(pipeline);

    VkPipeline vkPipeline = vulkanPipeline->m_vkPipeline;
    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    if (pipeline->getStages().check(GfxShaderStage::COMPUTE))
        bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

    vkCmdBindPipeline(m_vkCommandBuffers[m_currentFrame], bindPoint, vkPipeline);

    GfxPipelineStateController::bindPipeline(m_pipelineStateMachine, pipeline);
}

void GfxVulkanRenderer::bindVAO(const GfxVAO& vao) {
    if (vao == nullptr)
        return; // Error: Invalid VAO

    if (vao->getVertexBuffer()) {
        std::shared_ptr<GfxVulkanBuffer> vulkanVertexBuffer =
            std::static_pointer_cast<GfxVulkanBuffer>(vao->getVertexBuffer());
        VkBuffer vertexBuffer = vulkanVertexBuffer->m_vkBuffers[0];
        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(
            m_vkCommandBuffers[m_currentFrame],
            0,
            1,
            vertexBuffers,
            offsets
        );
    }

    if (vao->getIndexBuffer()) {
        std::shared_ptr<GfxVulkanBuffer> vulkanIndexBuffer =
            std::static_pointer_cast<GfxVulkanBuffer>(vao->getIndexBuffer());
        VkBuffer indexBuffer = vulkanIndexBuffer->m_vkBuffers[0];
        vkCmdBindIndexBuffer(
            m_vkCommandBuffers[m_currentFrame],
            indexBuffer,
            0,
            VK_INDEX_TYPE_UINT32
        );
    }
}

void GfxVulkanRenderer::clearColorAttachment(int index, const std::array<float, 4>& value) {
    VkClearAttachment attachment{};
    attachment.colorAttachment = index;
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.clearValue.color = { value[0], value[1], value[2], value[3] };
    clearAttachment(attachment);
}

void GfxVulkanRenderer::clearDepthAttachment(float value) {
    VkClearAttachment attachment{};
    attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    attachment.clearValue.depthStencil = { value, 0 };
    clearAttachment(attachment);
}

void GfxVulkanRenderer::clearStencilAttachment(int value) {
    VkClearAttachment attachment{};
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    attachment.clearValue.depthStencil = { 1.0f, static_cast<uint32_t>(value) };
    clearAttachment(attachment);
}

void GfxVulkanRenderer::bindDescriptorSetBinding(const GfxDescriptorSetBinding& binding) {
    std::shared_ptr<GfxVulkanDescriptorSetBinding> vulkanDescriptorSetBinding =
        std::static_pointer_cast<GfxVulkanDescriptorSetBinding>(binding);
    GfxPipeline pipeline = binding->getPipeline();
    std::shared_ptr<GfxVulkanPipeline> vulkanPipeline =
        std::static_pointer_cast<GfxVulkanPipeline>(pipeline);

    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    if (pipeline->getStages().check(GfxShaderStage::COMPUTE))
        bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    VkDescriptorSet descriptorSet = vulkanDescriptorSetBinding->m_vkDescriptorSets[m_currentFrame];

    vkCmdBindDescriptorSets(
        m_vkCommandBuffers[m_currentFrame],
        bindPoint,
        vulkanPipeline->m_vkPipelineLayout,
        binding->getDescriptorSetIndex(),
        1,
        &descriptorSet,
        0,
        nullptr
    );
}

int GfxVulkanRenderer::beginFrame() {
    if (m_vkSurface != VK_NULL_HANDLE) {
        if (m_vkSwapchain == VK_NULL_HANDLE) {
            if (recreateSwapchain())
                return 1; // Error: Failed to recreate swapchain
            return -1;
        }
        VkResult result = vkAcquireNextImageKHR
        (
            s_vkDevice,
            m_vkSwapchain,
            UINT64_MAX,
            m_imageAvailableSemaphores[m_currentFrame],
            VK_NULL_HANDLE,
            &m_imageIndex
        );
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            if (recreateSwapchain())
                return 1; // Error: Failed to recreate swapchain
            return -1;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            return 1; // Error: Failed to acquire next image
    }

    // Only reset the fence if we are submitting work
    vkResetFences(s_vkDevice, 1, &m_inFlightFences[m_currentFrame]);

    vkResetCommandBuffer(m_vkCommandBuffers[m_currentFrame], 0);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(m_vkCommandBuffers[m_currentFrame], &beginInfo) != VK_SUCCESS)
        return 1; // Error: Failed to begin command buffer

    std::shared_ptr<GfxVulkanPipelineStateMachine> vulkanPipelineStateMachine
        = std::static_pointer_cast<GfxVulkanPipelineStateMachine>(m_pipelineStateMachine);
    vulkanPipelineStateMachine->m_commandBuffer = m_vkCommandBuffers[m_currentFrame];

    return 0;
}

int GfxVulkanRenderer::endFrame() {
    VkResult result = vkEndCommandBuffer(m_vkCommandBuffers[m_currentFrame]);
    if (result != VK_SUCCESS)
        return 1; // Error: Failed to end command buffer

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_vkCommandBuffers[m_currentFrame];

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_imageIndex] };
    if (m_vkSwapchain != VK_NULL_HANDLE) {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
    }

    result = vkQueueSubmit(m_vkGraphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]);
    if (result != VK_SUCCESS)
        return 1; // Error: Failed to submit command buffer

    if (m_vkSwapchain != VK_NULL_HANDLE) {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_vkSwapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &m_imageIndex;

        result = vkQueuePresentKHR(m_vkPresentQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            if (recreateSwapchain())
                return 1; // Error: Failed to recreate swapchain
        } else if (result != VK_SUCCESS)
            return 1; // Error: Failed to present swapchain image
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    vkWaitForFences(s_vkDevice, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    return 0;
}

void GfxVulkanRenderer::draw(int nVertices, int nInstances, int firstVertex, int firstInstance) {
    vkCmdDraw(
        m_vkCommandBuffers[m_currentFrame],
        static_cast<uint32_t>(nVertices),
        static_cast<uint32_t>(nInstances),
        static_cast<uint32_t>(firstVertex),
        static_cast<uint32_t>(firstInstance)
    );
}

void GfxVulkanRenderer::drawIndexed(
    int nIndices,
    int nInstances,
    int firstIndex,
    int vertexOffset,
    int firstInstance
) {
    vkCmdDrawIndexed(
        m_vkCommandBuffers[m_currentFrame],
        static_cast<uint32_t>(nIndices),
        static_cast<uint32_t>(nInstances),
        static_cast<uint32_t>(firstIndex),
        static_cast<int32_t>(vertexOffset),
        static_cast<uint32_t>(firstInstance)
    );
}

void GfxVulkanRenderer::drawIndirect(
    const GfxBuffer& buffer,
    int offset,
    int drawCount,
    int stride
) {
    std::shared_ptr<GfxVulkanBuffer> vulkanBuffer =
        std::static_pointer_cast<GfxVulkanBuffer>(buffer);
    VkBuffer vkBuffer = vulkanBuffer->m_vkBuffers[m_currentFrame];
    vkCmdDrawIndirect(
        m_vkCommandBuffers[m_currentFrame],
        vkBuffer,
        static_cast<VkDeviceSize>(offset),
        static_cast<uint32_t>(drawCount),
        static_cast<uint32_t>(stride)
    );
}

void GfxVulkanRenderer::drawIndexedIndirect(
    const GfxBuffer& buffer,
    int offset,
    int drawCount,
    int stride
) {
    std::shared_ptr<GfxVulkanBuffer> vulkanBuffer =
        std::static_pointer_cast<GfxVulkanBuffer>(buffer);
    VkBuffer vkBuffer = vulkanBuffer->m_vkBuffers[m_currentFrame];
    vkCmdDrawIndexedIndirect(
        m_vkCommandBuffers[m_currentFrame],
        vkBuffer,
        static_cast<VkDeviceSize>(offset),
        static_cast<uint32_t>(drawCount),
        static_cast<uint32_t>(stride)
    );
}

void GfxVulkanRenderer::dispatchCompute(int nGroupsX, int nGroupsY, int nGroupsZ) {
    vkCmdDispatch(
        m_vkCommandBuffers[m_currentFrame],
        static_cast<uint32_t>(nGroupsX),
        static_cast<uint32_t>(nGroupsY),
        static_cast<uint32_t>(nGroupsZ)
    );
}

void GfxVulkanRenderer::dispatchComputeIndirect(const GfxBuffer& buffer, int offset) {
    std::shared_ptr<GfxVulkanBuffer> vulkanBuffer =
        std::static_pointer_cast<GfxVulkanBuffer>(buffer);
    VkBuffer vkBuffer = vulkanBuffer->m_vkBuffers[m_currentFrame];
    vkCmdDispatchIndirect(
        m_vkCommandBuffers[m_currentFrame],
        vkBuffer,
        static_cast<VkDeviceSize>(offset)
    );
}

void GfxVulkanRenderer::memoryBarrier() {
    VkMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(
        m_vkCommandBuffers[m_currentFrame],
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        1,
        &memoryBarrier,
        0,
        nullptr,
        0,
        nullptr
    );
}

GfxVulkanRenderer::QueueFamily GfxVulkanRenderer::findQueueFamily(
    const VkPhysicalDevice& device
) {
    QueueFamily family{};

    // Assign index to queue families that could be found
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            family.index = i;
            family.queueCount = queueFamily.queueCount;
            break;
        }
        i++;
    }

    return family;
}

int GfxVulkanRenderer::createSwapchain() {
    if (m_vkSurface == VK_NULL_HANDLE)
        return 1;

    int err = 0;
    GfxScopeGuard errCleaner(
        [&]() {
            if (err)
                cleanupSwapchain();
        }
    );

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        s_vkPhysicalDevice,
        m_vkSurface,
        &formatCount,
        nullptr
    );
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        s_vkPhysicalDevice,
        m_vkSurface,
        &formatCount,
        surfaceFormats.data()
    );
    VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];
    for (const auto& format : surfaceFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = format;
            break;
        }
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        s_vkPhysicalDevice,
        m_vkSurface,
        &presentModeCount,
        nullptr
    );
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        s_vkPhysicalDevice,
        m_vkSurface,
        &presentModeCount,
        presentModes.data()
    );
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    switch (m_vsyncMode) {
    case GfxVSyncMode::IMMEDIATE:
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        break;
    case GfxVSyncMode::FIFO:
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
        break;
    case GfxVSyncMode::MAILBOX:
        presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        if (std::find(presentModes.begin(), presentModes.end(), presentMode) == presentModes.end())
            presentMode = VK_PRESENT_MODE_FIFO_KHR; // fallback to FIFO if MAILBOX is not available
        break;
    default:
        break; // Use FIFO by default
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_vkPhysicalDevice, m_vkSurface, &surfaceCapabilities);

    uint32_t imageCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    if (surfaceCapabilities.maxImageCount > 0 &&
        imageCount > surfaceCapabilities.maxImageCount) {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_vkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = m_swapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;

    createInfo.preTransform = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = m_vkSwapchain;

    VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
    if (err = vkCreateSwapchainKHR(s_vkDevice, &createInfo, nullptr, &newSwapchain))
        return 1; // Error: Failed to create swapchain
    if (newSwapchain == VK_NULL_HANDLE)
        return 1; // Error: Failed to create swapchain
    if (m_vkSwapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(s_vkDevice, m_vkSwapchain, nullptr);
    m_vkSwapchain = newSwapchain;

    // Create swapchain images
    std::vector<VkImage> swapchainImages;
    vkGetSwapchainImagesKHR(s_vkDevice, m_vkSwapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(s_vkDevice, m_vkSwapchain, &imageCount, swapchainImages.data());

    m_swapchainImageFormat = surfaceFormat.format;

    m_swapchainImageViews.resize(swapchainImages.size(), VK_NULL_HANDLE);
    for (int i = 0; i < swapchainImages.size(); i++) {
        err = createVkImageView
        (
            swapchainImages[i],
            m_swapchainImageFormat,
            VK_IMAGE_ASPECT_COLOR_BIT,
            0, // baseMipLevel
            1, // mipLevels
            m_swapchainImageViews[i]
        );
        if (err)
            return err; // Error: Failed to create swapchain image view
    }

    CreateVkImageInfo colorImageInfo{};
    colorImageInfo.width = m_swapchainExtent.width;
    colorImageInfo.height = m_swapchainExtent.height;
    colorImageInfo.mipLevels = 1;
    colorImageInfo.numSamples = m_samples;
    colorImageInfo.format = m_swapchainImageFormat;
    colorImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    colorImageInfo.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    colorImageInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    err = createVkImage(
        colorImageInfo,
        m_swapchainColorImage,
        m_swapchainColorImageMemory
    );
    if (err)
        return err; // Error: Failed to create swapchain color image
    err = createVkImageView(
        m_swapchainColorImage,
        m_swapchainImageFormat,
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, // baseMipLevel
        1, // mipLevels
        m_swapchainColorImageView
    );
    if (err)
        return err; // Error: Failed to create swapchain color image view

    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    err = findDepthFormat(depthFormat);
    if (err)
        return err; // Error: Failed to find a suitable depth format

    CreateVkImageInfo depthImageInfo{};
    depthImageInfo.width = m_swapchainExtent.width;
    depthImageInfo.height = m_swapchainExtent.height;
    depthImageInfo.mipLevels = 1;
    depthImageInfo.numSamples = m_samples;
    depthImageInfo.format = depthFormat;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthImageInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    err = createVkImage(
        depthImageInfo,
        m_swapchainDepthImage,
        m_swapchainDepthImageMemory
    );
    if (err)
        return err; // Error: Failed to create swapchain depth image
    err = createVkImageView(
        m_swapchainDepthImage,
        depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        0, // baseMipLevel
        1, // mipLevels
        m_swapchainDepthImageView
    );
    if (err)
        return err; // Error: Failed to create swapchain depth image view

    return 0;
}

int GfxVulkanRenderer::createSwapchainRenderPass(VkRenderPass& renderPass) {
    int err = 0;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainImageFormat;
    colorAttachment.samples = m_samples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    err = findDepthFormat(depthFormat);

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = m_samples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = m_swapchainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    if (m_samples != VK_SAMPLE_COUNT_1_BIT)
        subpass.pResolveAttachments = &colorAttachmentResolveRef;

    std::vector<VkAttachmentDescription> attachments = {
        colorAttachment,
        depthAttachment,
    };
    if (m_samples != VK_SAMPLE_COUNT_1_BIT)
        attachments.push_back(colorAttachmentResolve);

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (err = vkCreateRenderPass(s_vkDevice, &renderPassInfo, nullptr, &renderPass))
        return 1; // Error: Failed to create swapchain render pass

    return 0;
}

int GfxVulkanRenderer::createSwapchainFramebuffers() {
    if (m_vkSwapchain == VK_NULL_HANDLE ||
        m_swapchainRenderPass == VK_NULL_HANDLE) {
        return 1;
    }

    int err = 0;
    GfxScopeGuard errCleaner(
        [&]() {
            if (err) {
                for (auto& framebuffer : m_swapchainFramebuffers)
                    vkDestroyFramebuffer(s_vkDevice, framebuffer, nullptr);
            }
        }
    );

    m_swapchainFramebuffers.resize(m_swapchainImageViews.size(), VK_NULL_HANDLE);

    for (int i = 0; i < m_swapchainImageViews.size(); i++) {
        std::vector<VkImageView> attachments{};
        if (m_samples != VK_SAMPLE_COUNT_1_BIT) {
            attachments = {
                m_swapchainColorImageView,
                m_swapchainDepthImageView,
                m_swapchainImageViews[i],
            };
        } else {
            attachments = {
                m_swapchainImageViews[i],
                m_swapchainDepthImageView,
            };
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_swapchainRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapchainExtent.width;
        framebufferInfo.height = m_swapchainExtent.height;
        framebufferInfo.layers = 1;

        err = vkCreateFramebuffer(
            s_vkDevice,
            &framebufferInfo,
            nullptr,
            &m_swapchainFramebuffers[i]
        );
        if (err)
            return err; // Error: Failed to create swapchain framebuffer
    }

    return 0;
}

int GfxVulkanRenderer::recreateSwapchain() {
    vkQueueWaitIdle(m_vkPresentQueue);
    cleanupSwapchain();

    if (createSwapchain())
        return 1;
    if (!m_swapchainRenderPass && createSwapchainRenderPass(m_swapchainRenderPass))
        return 1;
    if (createSwapchainFramebuffers())
        return 1;

    return 0;
}

void GfxVulkanRenderer::cleanupSwapchain() {
    if (m_swapchainColorImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(s_vkDevice, m_swapchainColorImageView, nullptr);
        m_swapchainColorImageView = VK_NULL_HANDLE;
    }
    if (m_swapchainColorImage != VK_NULL_HANDLE) {
        vkDestroyImage(s_vkDevice, m_swapchainColorImage, nullptr);
        m_swapchainColorImage = VK_NULL_HANDLE;
    }
    if (m_swapchainColorImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(s_vkDevice, m_swapchainColorImageMemory, nullptr);
        m_swapchainColorImageMemory = VK_NULL_HANDLE;
    }
    if (m_swapchainDepthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(s_vkDevice, m_swapchainDepthImageView, nullptr);
        m_swapchainDepthImageView = VK_NULL_HANDLE;
    }
    if (m_swapchainDepthImage != VK_NULL_HANDLE) {
        vkDestroyImage(s_vkDevice, m_swapchainDepthImage, nullptr);
        m_swapchainDepthImage = VK_NULL_HANDLE;
    }
    if (m_swapchainDepthImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(s_vkDevice, m_swapchainDepthImageMemory, nullptr);
        m_swapchainDepthImageMemory = VK_NULL_HANDLE;
    }
    for (auto framebuffer : m_swapchainFramebuffers) {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(s_vkDevice, framebuffer, nullptr);
            framebuffer = VK_NULL_HANDLE;
        }
    }
    for (auto imageView : m_swapchainImageViews) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(s_vkDevice, imageView, nullptr);
            imageView = VK_NULL_HANDLE;
        }
    }
}

int GfxVulkanRenderer::createCommandBuffers() {
    m_vkCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_vkCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_vkCommandBuffers.size());

    if (vkAllocateCommandBuffers(s_vkDevice, &allocInfo, m_vkCommandBuffers.data()))
        return 1; // Error: Failed to allocate command buffers

    return 0;
}

VkSampleCountFlagBits GfxVulkanRenderer::getSampleCount(int samples) const {
    samples = std::max(1, samples);
    int clamped = 1;
    while (clamped * 2 <= samples && clamped * 2 <= 64)
        clamped *= 2;
    while (clamped > static_cast<int>(m_maxSampleCount))
        clamped /= 2;
    return static_cast<VkSampleCountFlagBits>(clamped);
}

int GfxVulkanRenderer::createVkImage(
    const CreateVkImageInfo& info,
    VkImage& image,
    VkDeviceMemory& imageMemory
) const {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(info.width);
    imageInfo.extent.height = static_cast<uint32_t>(info.height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = info.mipLevels;
    imageInfo.arrayLayers = 1;

    imageInfo.format = info.format;
    imageInfo.tiling = info.tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    imageInfo.usage = info.usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    imageInfo.samples = info.numSamples;
    imageInfo.flags = 0;

    if (vkCreateImage(s_vkDevice, &imageInfo, nullptr, &image))
        return 1;

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(s_vkDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    uint32_t typeIndex = 0;
    if (findMemoryType(memRequirements.memoryTypeBits, info.properties, typeIndex))
        return 1; // Error: Failed to find suitable memory type
    allocInfo.memoryTypeIndex = typeIndex;

    if (vkAllocateMemory(s_vkDevice, &allocInfo, nullptr, &imageMemory))
        return 1;

    vkBindImageMemory(s_vkDevice, image, imageMemory, 0);

    return 0;
}

int GfxVulkanRenderer::createVkImageView(
    const VkImage& image,
    VkFormat format,
    VkImageAspectFlags aspectFlags,
    uint32_t baseLevel,
    uint32_t levelCount,
    VkImageView& imageView
) const {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = baseLevel;
    viewInfo.subresourceRange.levelCount = levelCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(s_vkDevice, &viewInfo, nullptr, &imageView))
        return 1;

    return 0;
}

int GfxVulkanRenderer::transitionImageLayout(
    const VkImage& image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    uint32_t mipLevels,
    const VkCommandBuffer& commandBuffer
) const {
    if (oldLayout == newLayout)
        return 0; // No transition needed

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            format == VK_FORMAT_D24_UNORM_S8_UINT) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }

    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }

    else
        return 0; // No transition needed

    VkCommandBuffer cmdBuffer = commandBuffer;
    if (commandBuffer == VK_NULL_HANDLE)
        cmdBuffer = beginSingleTimeCommands();

    vkCmdPipelineBarrier(
        cmdBuffer,
        sourceStage,
        destinationStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );

    if (commandBuffer == VK_NULL_HANDLE)
        endSingleTimeCommands(cmdBuffer);

    return 0;
}

int GfxVulkanRenderer::readImageData(
    const GfxImage& image,
    const GfxRect& rect,
    void* data
) const {
    std::shared_ptr<GfxVulkanImage> vulkanImage =
        std::static_pointer_cast<GfxVulkanImage>(image);

    VkImage srcImage = vulkanImage->m_image;
    VkFormat format = GfxVkTypeConverter::toVkFormat(vulkanImage->getFormat());
    int pixelSize = GfxVkTypeConverter::formatSize(vulkanImage->getFormat());

    VkDeviceSize imageSize = static_cast<VkDeviceSize>(rect.width) * rect.height * pixelSize;

    // Create staging buffer
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    int err = createVkBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );
    if (err)
        return 1; // Error: Failed to create staging buffer

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // Transition image to transfer src
    err = transitionImageLayout(
        srcImage,
        format,
        vulkanImage->m_currentLayout,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        1,
        commandBuffer
    );
    if (err)
        return 1; // Error: Failed to transition image layout

    // Copy image region to buffer
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { rect.x, rect.y, 0 };
    region.imageExtent = {
        static_cast<uint32_t>(rect.width),
        static_cast<uint32_t>(rect.height),
        1
    };
    vkCmdCopyImageToBuffer(
        commandBuffer,
        srcImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        stagingBuffer,
        1,
        &region
    );

    // Restore image layout
    transitionImageLayout(
        srcImage,
        format,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        vulkanImage->m_currentLayout,
        1,
        commandBuffer
    );

    endSingleTimeCommands(commandBuffer);

    // Map buffer and copy to output
    void* mappedData = nullptr;
    VkResult result = vkMapMemory(s_vkDevice, stagingBufferMemory, 0, imageSize, 0, &mappedData);
    if (result != VK_SUCCESS) {
        vkDestroyBuffer(s_vkDevice, stagingBuffer, nullptr);
        vkFreeMemory(s_vkDevice, stagingBufferMemory, nullptr);
        return 1; // Error: Failed to map memory for staging buffer
    }
    memcpy(data, mappedData, static_cast<size_t>(imageSize));
    vkUnmapMemory(s_vkDevice, stagingBufferMemory);

    // Cleanup
    vkDestroyBuffer(s_vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(s_vkDevice, stagingBufferMemory, nullptr);

    return 0;
}

void GfxVulkanRenderer::clearAttachment(const VkClearAttachment& attachment) const {
    VkExtent2D extent = {};
    if (m_currentFramebuffer != nullptr) {
        std::shared_ptr<GfxVulkanFramebuffer> vulkanFramebuffer =
            std::static_pointer_cast<GfxVulkanFramebuffer>(m_currentFramebuffer);
        extent.width = vulkanFramebuffer->m_width;
        extent.height = vulkanFramebuffer->m_height;
    } else
        extent = m_swapchainExtent;

    VkClearRect clearRect = {};
    clearRect.rect.offset = { 0, 0 };
    clearRect.rect.extent = extent;
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    vkCmdClearAttachments(
        m_vkCommandBuffers[m_currentFrame],
        1,
        &attachment,
        1,
        &clearRect
    );
}

int GfxVulkanRenderer::findDepthFormat(VkFormat& format) const {
    format = VK_FORMAT_UNDEFINED;

    std::vector<VkFormat> depthFormatCandidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    for (VkFormat depthFormat : depthFormatCandidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(s_vkPhysicalDevice, depthFormat, &props);
        if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) ==
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            format = depthFormat;
            break;
        }
    }
    if (format == VK_FORMAT_UNDEFINED)
        return 1; // Error: No suitable depth format found

    return 0;
}

int GfxVulkanRenderer::findMemoryType(
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties,
    uint32_t& typeIndex
) const {
    if (s_vkPhysicalDevice == VK_NULL_HANDLE)
        return 1;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(s_vkPhysicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            typeIndex = i;
            return 0; // Found suitable memory type
        }
    }
    return 1;
}

int GfxVulkanRenderer::createVkBuffer(
    const VkDeviceSize& size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory
) const {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(s_vkDevice, &bufferInfo, nullptr, &buffer))
        return 1;

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(s_vkDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    uint32_t typeIndex = 0;
    if (findMemoryType(memRequirements.memoryTypeBits, properties, typeIndex))
        return 1; // Error: Failed to find suitable memory type
    allocInfo.memoryTypeIndex = typeIndex;

    if (vkAllocateMemory(s_vkDevice, &allocInfo, nullptr, &bufferMemory))
        return 1;

    vkBindBufferMemory(s_vkDevice, buffer, bufferMemory, 0);

    return 0;
}

int GfxVulkanRenderer::resizeVkBuffer(const GfxBuffer& buffer, int size) const {
    std::shared_ptr<GfxVulkanBuffer> vulkanBuffer =
        std::static_pointer_cast<GfxVulkanBuffer>(buffer);

    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(size);
    GfxBufferUsage usage = buffer->getUsage();
    GfxBufferProp prop = buffer->getProp();

    int nBuffers = 1;

    VkBufferUsageFlags vkUsage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags vkProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (usage == GfxBufferUsage::VERTEX_BUFFER)
        vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    else if (usage == GfxBufferUsage::INDEX_BUFFER)
        vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    else {
        nBuffers = MAX_FRAMES_IN_FLIGHT;
        if (usage == GfxBufferUsage::UNIFORM_BUFFER)
            vkUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        else if (usage == GfxBufferUsage::STORAGE_BUFFER)
            vkUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (prop == GfxBufferProp::STATIC)
        vkProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    else if (prop == GfxBufferProp::DYNAMIC) {
        vkProperties =
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    destroyBuffer(buffer);
    vulkanBuffer->m_vkBuffers.resize(nBuffers, VK_NULL_HANDLE);
    vulkanBuffer->m_vkBufferMemories.resize(nBuffers, VK_NULL_HANDLE);

    for (int i = 0; i < nBuffers; i++) {
        VkBuffer& vkBuffer = vulkanBuffer->m_vkBuffers[i];
        VkDeviceMemory& vkBufferMemory = vulkanBuffer->m_vkBufferMemories[i];

        int err = createVkBuffer(
            bufferSize,
            vkUsage,
            vkProperties,
            vkBuffer,
            vkBufferMemory
        );
        if (err) {
            destroyBuffer(buffer);
            return err; // Error: Failed to recreate Vulkan buffer
        }
    }

    vulkanBuffer->setSize(size);

    return 0;
}

int GfxVulkanRenderer::createVkPipelineDescriptorSets(
    const std::shared_ptr<GfxVulkanPipeline>& pipeline,
    const std::vector<GfxDescriptorSet>& descriptorSets
) const {
    for (const auto& descriptorSet : descriptorSets) {
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(descriptorSet.size());
        std::vector<VkDescriptorBindingFlags> bindingFlags;
        bindingFlags.resize(descriptorSet.size(), 0);
        for (int i = 0; i < descriptorSet.size(); i++) {
            const GfxDescriptor& descriptor = descriptorSet[i];

            VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            if (descriptor.type == GfxDescriptorType::SAMPLER)
                descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            else if (descriptor.type == GfxDescriptorType::SAMPLERS)
                descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            else if (descriptor.type == GfxDescriptorType::UNIFORM_BUFFER)
                descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            else if (descriptor.type == GfxDescriptorType::STORAGE_BUFFER)
                descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            else if (descriptor.type == GfxDescriptorType::STORAGE_IMAGE)
                descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

            VkShaderStageFlags stageFlags =
                static_cast<VkShaderStageFlags>(descriptor.stages.getValue());

            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = descriptor.binding;
            layoutBinding.descriptorType = descriptorType;
            layoutBinding.descriptorCount = 1;
            if (descriptor.type == GfxDescriptorType::SAMPLERS)
                layoutBinding.descriptorCount = static_cast<uint32_t>(descriptor.size);
            layoutBinding.stageFlags = stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBindings.push_back(layoutBinding);
        }
        VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
        flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        flagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
        flagsInfo.pBindingFlags = bindingFlags.data();
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();
        layoutInfo.pNext = &flagsInfo;
        layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        VkResult result = vkCreateDescriptorSetLayout(
            s_vkDevice,
            &layoutInfo,
            nullptr,
            &layout
        );
        if (result != VK_SUCCESS) {
            for (auto layout : pipeline->m_vkDescriptorSetLayouts)
                vkDestroyDescriptorSetLayout(s_vkDevice, layout, nullptr);
            pipeline->m_vkDescriptorSetLayouts.clear();
            return 1; // Error: Failed to create descriptor set layout
        }
        pipeline->m_vkDescriptorSetLayouts.push_back(layout);
    }

    return 0;
}

void GfxVulkanRenderer::updateVkDescriptorSets(
    const GfxDescriptorSetBinding& descriptorSetBinding
) const {
    std::shared_ptr<GfxVulkanDescriptorSetBinding> vulkanDescriptorSetBinding =
        std::static_pointer_cast<GfxVulkanDescriptorSetBinding>(descriptorSetBinding);
    auto bindings = vulkanDescriptorSetBinding->getDescriptorBindings();

    for (int i = 0; i < vulkanDescriptorSetBinding->m_vkDescriptorSets.size(); i++) {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(bindings.size());

        size_t samplerCount = 0, samplerArraysCount = 0, bufferCount = 0;
        for (const auto& binding : bindings) {
            if (binding.descriptor.type == GfxDescriptorType::SAMPLER)
                samplerCount++;
            else if (binding.descriptor.type == GfxDescriptorType::SAMPLERS)
                samplerArraysCount++;
            else
                bufferCount++;
        }
        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(samplerCount);
        std::vector<std::vector<VkDescriptorImageInfo>> samplerArrays;
        samplerArrays.reserve(samplerArraysCount);
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(bufferCount);

        for (const auto& binding : bindings) {
            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = vulkanDescriptorSetBinding->m_vkDescriptorSets[i];
            descriptorWrite.dstBinding = binding.descriptor.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;
            if (binding.descriptor.type == GfxDescriptorType::SAMPLER) {
                const GfxImage* image = std::get_if<GfxImage>(&binding.resource);
                if (image == nullptr)
                    continue; // Invalid binding, skip it
                std::shared_ptr<GfxVulkanImage> vulkanImage =
                    std::static_pointer_cast<GfxVulkanImage>(*image);
                imageInfos.emplace_back();
                VkDescriptorImageInfo& imageInfo = imageInfos.back();
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = vulkanImage->m_imageView;
                imageInfo.sampler = vulkanImage->m_sampler;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.pImageInfo = &imageInfos.back();
            } else if (binding.descriptor.type == GfxDescriptorType::SAMPLERS) {
                const std::vector<GfxImage>* images =
                    std::get_if<std::vector<GfxImage>>(&binding.resource);
                if (images == nullptr || images->empty())
                    continue; // Invalid binding, skip it
                samplerArrays.emplace_back();
                std::vector<VkDescriptorImageInfo>& imageInfosArray = samplerArrays.back();
                for (const auto& image : *images) {
                    std::shared_ptr<GfxVulkanImage> vulkanImage =
                        std::static_pointer_cast<GfxVulkanImage>(image);
                    imageInfosArray.emplace_back();
                    VkDescriptorImageInfo& imageInfo = imageInfosArray.back();
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = vulkanImage->m_imageView;
                    imageInfo.sampler = vulkanImage->m_sampler;
                }
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.descriptorCount = static_cast<uint32_t>(imageInfosArray.size());
                descriptorWrite.pImageInfo = imageInfosArray.data();
            } else if (binding.descriptor.type == GfxDescriptorType::STORAGE_IMAGE) {
                const GfxImage* image = std::get_if<GfxImage>(&binding.resource);
                if (image == nullptr)
                    continue; // Invalid binding, skip it
                std::shared_ptr<GfxVulkanImage> vulkanImage =
                    std::static_pointer_cast<GfxVulkanImage>(*image);
                imageInfos.emplace_back();
                VkDescriptorImageInfo& imageInfo = imageInfos.back();
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                imageInfo.imageView = vulkanImage->m_imageView;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                descriptorWrite.pImageInfo = &imageInfos.back();
            } else {
                const GfxBuffer* buffer = std::get_if<GfxBuffer>(&binding.resource);
                if (buffer == nullptr)
                    continue; // Invalid binding, skip it
                std::shared_ptr<GfxVulkanBuffer> vulkanBuffer =
                    std::static_pointer_cast<GfxVulkanBuffer>(*buffer);
                bufferInfos.emplace_back();
                VkDescriptorBufferInfo& bufferInfo = bufferInfos.back();
                bufferInfo.buffer = vulkanBuffer->m_vkBuffers[i];
                bufferInfo.offset = 0;
                bufferInfo.range = static_cast<VkDeviceSize>((*buffer)->getSize());
                if (binding.descriptor.type == GfxDescriptorType::UNIFORM_BUFFER)
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                else
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrite.pBufferInfo = &bufferInfos.back();
            }
            descriptorWrites.push_back(descriptorWrite);
        }

        vkUpdateDescriptorSets(
            s_vkDevice,
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0,
            nullptr
        );
    }
}

VkCommandBuffer GfxVulkanRenderer::beginSingleTimeCommands() const {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_vkCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(s_vkDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void GfxVulkanRenderer::endSingleTimeCommands(const VkCommandBuffer& commandBuffer) const {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vkGraphicsQueue);

    vkFreeCommandBuffers(s_vkDevice, m_vkCommandPool, 1, &commandBuffer);
}
