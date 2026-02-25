/**
 * @file AppUiManager.h
 * @brief Declaration of the AppUiManager class for managing application UI.
 */

#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <fonts/IconsForkAwesome.h>

#include "gui/GuiPub.h"
#include "utils/Math.h"

/**
 * @brief Application UI Manager class.
 */
class AppUiManager {
private:
    AppUiManager() = default;
    ~AppUiManager() = default;
    AppUiManager(const AppUiManager&) = delete;
    AppUiManager& operator=(const AppUiManager&) = delete;
    AppUiManager(AppUiManager&&) = delete;
    AppUiManager& operator=(AppUiManager&&) = delete;

public:
    static AppUiManager& instance() {
        static AppUiManager instance;
        return instance;
    }

    /**
     * @brief Get the normal icon fonts.
     * @return Pointers to the normal icon fonts.
     */
    ImFont* getNormalIconFont() const;
    /**
     * @brief Get the bold icon font.
     * @return Pointer to the bold icon font.
     */
    ImFont* getBoldIconFont() const;

    /**
     * @brief Get the application icons in various sizes.
     * @param[out] icons A vector to store the loaded icons.
     * @return 0 on success, non-zero on failure.
     */
    int getAppIcons(std::vector<GuiIcon>& icons) const;

    /**
     * @brief Initialize the UI, including loading fonts and setting styles.
     * @param dpiScale DPI scaling factor (default is 1.0f).
     */
    void initUI(float dpiScale = 1.0f);

    /**
     * @brief Get the DPI scale factor.
     * @return The DPI scale factor.
     */
    float getDpiScale() const;

    /**
     * @brief Check if the user is currently editing text in the UI.
     * @return True if in text editing mode, false otherwise.
     */
    bool isInTextEditing() const;

    /**
     * @brief Structure to hold UI sizes.
     */
    struct UiSizes {
        int windowWidth = 1737; // Width of the main window (dynamic updated on resize)
        int windowHeight = 866; // Height of the main window (dynamic updated on resize)
        float menuBarHeight = 0.0f; // Height of the menu bar (dynamic calculated by font size)
        float toolBarHeight = 0.0f; // Height of the tool bar (dynamic calculated by font size)
        float statusBarHeight = 0.0f; // Height of the status bar (dynamic calculated by font size)
        float leftPanelWidth = 0.0f; // Width of the left panel (fixed)
        float rightPanelWidth = 0.0f; // Width of the right panel (fixed)
    };
    /**
     * @brief Get the UI sizes.
     * @return Reference to the UiSizes structure.
     */
    UiSizes& getUiSizes();

    /**
     * @brief Get the ImGui texture associated with the specified GfxImage.
     *        If the texture does not exist, it will be created.
     * @param renderer The GfxRenderer instance.
     * @param image The GfxImage for which to get the ImGui texture.
     * @return The ImTextureID associated with the GfxImage.
     */
    ImTextureID getImGuiTexture(GfxRenderer renderer, GfxImage image);
    /**
     * @brief Destroy the ImGui texture associated with the specified GfxImage.
     * @param renderer The GfxRenderer instance.
     * @param image The GfxImage whose ImGui texture is to be destroyed.
     */
    void destroyImGuiTexture(GfxRenderer renderer, GfxImage image);
    /**
     * @brief Destroy all ImGui textures associated with the specified GfxRenderer.
     * @param renderer The GfxRenderer instance.
     */
    void destroyImGuiTextures(GfxRenderer renderer);

private:
    ImFont* m_normalIconFont = nullptr; // Normal icon font
    ImFont* m_boldIconFont = nullptr; // Bold icon font

    float m_dpiScale = 1.0f; // DPI scale factor
    UiSizes m_uiSizes; // UI sizes

    // Map of GfxImage to ImTextureID for managing ImGui textures
    std::unordered_map<GfxImage, ImTextureID> m_imguiTextures = {};
};

namespace AppUiUtils {

/**
 * @brief Initializes ImGui for the specified GfxRenderer and GuiWindow.
 * @param renderer The GfxRenderer instance.
 * @param window The GuiWindow instance.
 * @return 0 on success, non-zero on failure.
 */
int initForImGui(GfxRenderer renderer, std::shared_ptr<GuiWindow> window);
/**
 * @brief Terminates ImGui for the specified GfxRenderer.
 * @param renderer The GfxRenderer instance.
 */
void termForImGui(GfxRenderer renderer);

/**
 * @brief Prepares a new frame for ImGui using the specified GfxRenderer.
 * @param renderer The GfxRenderer instance.
 */
void newFrameForImGui(GfxRenderer renderer);
/**
 * @brief Renders the ImGui draw data using the specified GfxRenderer.
 * @param renderer The GfxRenderer instance.
 */
void renderForImGui(GfxRenderer renderer);

/**
 * @brief Converts a std::array<float, 3> to a Math::Vec3.
 * @param arr The std::array<float, 3> to convert.
 * @return The resulting Math::Vec3.
 */
Math::Vec3 arrayToVec3(const std::array<float, 3>& arr);
/**
 * @brief Converts a Math::Vec3 to a std::array<float, 3>.
 * @param vec The Math::Vec3 to convert.
 * @return The resulting std::array<float, 3>.
 */
std::array<float, 3> vec3ToArray(const Math::Vec3& vec);

} // namespace AppUiUtils
