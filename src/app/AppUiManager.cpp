/**
 * @file AppUiManager.cpp
 * @brief Implementation of the AppUiManager class for managing application UI.
 */

#include "app/AppUiManager.h"

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_vulkan.h>
namespace ImStb {
#include "imstb_textedit.h"
}

#include "res/AppIcon.hpp"
#include "utils/Image.h"

// Font data declarations
extern "C"
{
    extern const unsigned int ForkAwesome_compressed_size;
    extern const unsigned int ForkAwesome_compressed_data[];
    extern const unsigned int SourceSansPro_compressed_size;
    extern const unsigned int SourceSansPro_compressed_data[];
    extern const unsigned int SourceHanSansSC_compressed_size;
    extern const unsigned int SourceHanSansSC_compressed_data[];
}

ImFont* AppUiManager::getNormalIconFont() const {
    return m_normalIconFont;
}

ImFont* AppUiManager::getBoldIconFont() const {
    return m_boldIconFont;
}

int AppUiManager::getAppIcons(std::vector<GuiIcon>& icons) const {
    icons.clear();
    int result = 0;

    GuiIcon icon16{};
    result = ImageRGBA::loadFromMemory(
        AppIcon::ICON16,
        AppIcon::ICON16_SIZE,
        icon16.width,
        icon16.height,
        icon16.data
    );
    if (result)
        return result;
    icons.push_back(icon16);

    GuiIcon icon32{};
    result = ImageRGBA::loadFromMemory(
        AppIcon::ICON32,
        AppIcon::ICON32_SIZE,
        icon32.width,
        icon32.height,
        icon32.data
    );
    if (result)
        return result;
    icons.push_back(icon32);

    GuiIcon icon48{};
    result = ImageRGBA::loadFromMemory(
        AppIcon::ICON48,
        AppIcon::ICON48_SIZE,
        icon48.width,
        icon48.height,
        icon48.data
    );
    if (result)
        return result;
    icons.push_back(icon48);

    GuiIcon icon256{};
    result = ImageRGBA::loadFromMemory(
        AppIcon::ICON256,
        AppIcon::ICON256_SIZE,
        icon256.width,
        icon256.height,
        icon256.data
    );
    if (result)
        return result;
    icons.push_back(icon256);

    return result;
}

void AppUiManager::initUI(float dpiScale) {
    m_dpiScale = dpiScale;

    ImGui::GetIO().IniFilename = 0;

    ImGui::GetIO().Fonts->Clear();

    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
        SourceHanSansSC_compressed_data,
        SourceHanSansSC_compressed_size,
        17.0f * dpiScale
    );

    static const ImWchar icons_ranges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;

    m_normalIconFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
        ForkAwesome_compressed_data,
        ForkAwesome_compressed_size,
        14.0f * dpiScale,
        &icons_config,
        icons_ranges
    );

    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
        SourceHanSansSC_compressed_data,
        SourceHanSansSC_compressed_size,
        17.0f * dpiScale
    );
    icons_config.GlyphOffset.y += (22.0f - 17.0f) * 0.5f;
    m_boldIconFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
        ForkAwesome_compressed_data,
        ForkAwesome_compressed_size,
        22.0f * dpiScale,
        &icons_config,
        icons_ranges
    );

    ImGui::GetIO().FontDefault = m_boldIconFont;

    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowPadding = ImVec2(6.0f, 4.0f);
    style.FramePadding = ImVec2(8.0f, 2.0f);
    style.ItemSpacing = ImVec2(4.0f, 6.0f);
    style.IndentSpacing = 12.0f;
    style.FrameBorderSize = 1.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 2.0f;

    style.Colors[ImGuiCol_Text] = ImVec4(0.94f, 0.94f, 0.94f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 0.4f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.4f, 0.4f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1f, 0.4f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
}

float AppUiManager::getDpiScale() const {
    return m_dpiScale;
}

bool AppUiManager::isInTextEditing() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

AppUiManager::UiSizes& AppUiManager::getUiSizes() {
    return m_uiSizes;
}

ImTextureID AppUiManager::getImGuiTexture(GfxRenderer renderer, GfxImage image) {
    if (m_imguiTextures.find(image) != m_imguiTextures.end())
        return m_imguiTextures[image];
    ImTextureID texture = 0;
    GfxRendererInterface::ImageInfo imageInfo = renderer->getImageInfo(image);
    if (renderer->getBackend() == GfxBackend::Vulkan) {
        auto& vulkanInfo = std::get<GfxRendererInterface::ImageVulkanInfo>(imageInfo);
        VkDescriptorSet tex = ImGui_ImplVulkan_AddTexture(
            reinterpret_cast<VkSampler>(vulkanInfo.sampler),
            reinterpret_cast<VkImageView>(vulkanInfo.imageView),
            static_cast<VkImageLayout>(vulkanInfo.imageLayout)
        );
        texture = reinterpret_cast<ImTextureID>(tex);
    } else
        texture = static_cast<ImTextureID>(std::get<uint32_t>(imageInfo));
    m_imguiTextures[image] = texture;
    return texture;
}

void AppUiManager::destroyImGuiTexture(GfxRenderer renderer, GfxImage image) {
    if (m_imguiTextures.find(image) == m_imguiTextures.end())
        return;
    if (renderer->getBackend() == GfxBackend::Vulkan) {
        ImTextureID texture = m_imguiTextures[image];
        if (texture != 0) {
            renderer->waitDeviceIdle();
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(texture));
        }
    }
    m_imguiTextures.erase(image);
}

void AppUiManager::destroyImGuiTextures(GfxRenderer renderer) {
    if (renderer->getBackend() == GfxBackend::Vulkan) {
        renderer->waitDeviceIdle();
        for (const auto& pair : m_imguiTextures) {
            ImTextureID texture = pair.second;
            if (texture != 0)
                ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(texture));
        }
    }
    m_imguiTextures.clear();
}

int AppUiUtils::initForImGui(GfxRenderer renderer, std::shared_ptr<GuiWindow> window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(reinterpret_cast<GLFWwindow*>(window->getNativeWindow()), true);
    GfxBackend backend = renderer->getBackend();
    if (backend == GfxBackend::Vulkan) {
        return renderer->initForImGui(
            [](void* initInfo) {
                GfxRendererInterface::ImGuiVulkanInitInfo* infoIn =
                    reinterpret_cast<GfxRendererInterface::ImGuiVulkanInitInfo*>(initInfo);
                ImGui_ImplVulkan_InitInfo info = {};
                info.Instance = reinterpret_cast<VkInstance>(infoIn->instance);
                info.PhysicalDevice = reinterpret_cast<VkPhysicalDevice>(infoIn->physicalDevice);
                info.Device = reinterpret_cast<VkDevice>(infoIn->device);
                info.QueueFamily = infoIn->queueFamily;
                info.Queue = reinterpret_cast<VkQueue>(infoIn->queue);
                info.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
                info.MinImageCount = 2;
                info.ImageCount = infoIn->imageCount;
                info.MSAASamples = static_cast<VkSampleCountFlagBits>(infoIn->samples);
                info.RenderPass = reinterpret_cast<VkRenderPass>(infoIn->renderPass);
                ImGui_ImplVulkan_Init(&info);
            }
        );
    }
    // else if (backend == GfxBackend::OpenGL)
    return renderer->initForImGui(
        [](void* glsl_version) {
            ImGui_ImplOpenGL3_Init(reinterpret_cast<const char*>(glsl_version));
        }
    );
}

void AppUiUtils::termForImGui(GfxRenderer renderer) {
    GfxBackend backend = renderer->getBackend();
    if (backend == GfxBackend::Vulkan)
        renderer->termForImGui(ImGui_ImplVulkan_Shutdown);
    else if (backend == GfxBackend::OpenGL)
        renderer->termForImGui(ImGui_ImplOpenGL3_Shutdown);
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void AppUiUtils::newFrameForImGui(GfxRenderer renderer) {
    GfxBackend backend = renderer->getBackend();
    if (backend == GfxBackend::Vulkan)
        ImGui_ImplVulkan_NewFrame();
    else if (backend == GfxBackend::OpenGL)
        ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void AppUiUtils::renderForImGui(GfxRenderer renderer) {
    GfxBackend backend = renderer->getBackend();
    ImGui::Render();
    if (backend == GfxBackend::Vulkan) {
        renderer->renderForImGui(
            [](void* commandBuffer) {
                ImGui_ImplVulkan_RenderDrawData
                (
                    ImGui::GetDrawData(),
                    reinterpret_cast<VkCommandBuffer>(commandBuffer)
                );
            }
        );
    } else if (backend == GfxBackend::OpenGL)
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Math::Vec3 AppUiUtils::arrayToVec3(const std::array<float, 3>& arr) {
    return Math::Vec3(arr[0], arr[1], arr[2]);
}

std::array<float, 3> AppUiUtils::vec3ToArray(const Math::Vec3& vec) {
    return { vec.x, vec.y, vec.z };
}
