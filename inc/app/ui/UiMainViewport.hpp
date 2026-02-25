/**
 * @file UiMainViewport.hpp
 * @brief Defines the UI of the main viewport area.
 */

#pragma once

#include "app/AppUiManager.h"

/**
 * @brief The main viewport area.
 */
class UiMainViewport : public GuiView {
public:
    static constexpr const char* LABEL = "main_viewport";

    enum class ID : int {
        MOUSE_COORD,
    };

    ImTextureID frameTexture = 0;
    int frameWidth = 0;
    int frameHeight = 0;

    UiMainViewport() {
        /* Default states of the widgets */

        m_widgetStates[static_cast<int>(ID::MOUSE_COORD)] = {};
        m_widgetStates[static_cast<int>(ID::MOUSE_COORD)].value =
            std::array<float, 3>{ 0.0f, 0.0f, 0.0f };
    }

    void draw() override {
        float menuBarHeight = AppUiManager::instance().getUiSizes().menuBarHeight;
        float toolBarHeight = AppUiManager::instance().getUiSizes().toolBarHeight;
        float statusBarHeight = AppUiManager::instance().getUiSizes().statusBarHeight;
        int windowWidth = AppUiManager::instance().getUiSizes().windowWidth;
        int windowHeight = AppUiManager::instance().getUiSizes().windowHeight;
        float leftPanelWidth = AppUiManager::instance().getUiSizes().leftPanelWidth;
        float rightPanelWidth = AppUiManager::instance().getUiSizes().rightPanelWidth;

        float viewportX = leftPanelWidth;
        float viewportY = menuBarHeight + toolBarHeight;
        float viewportWidth = windowWidth - leftPanelWidth - rightPanelWidth;
        float viewportHeight = windowHeight - menuBarHeight - toolBarHeight - statusBarHeight;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowPos(ImVec2(viewportX, viewportY));
        ImGui::SetNextWindowSize(ImVec2(viewportWidth, viewportHeight));

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus;
        ImGui::Begin("viewport", 0, flags);
        ImGui::BeginChild("ViewportRect");

        ImVec2 frameSize(viewportWidth, viewportHeight);
        float viewportAspect = viewportWidth / viewportHeight;
        ImVec2 framePos = ImGui::GetCursorScreenPos();
        if (frameWidth > 0 && frameHeight > 0) {
            float frameAspect =
                static_cast<float>(frameWidth) / static_cast<float>(frameHeight);
            if (frameAspect > 0.0f) {
                if (viewportAspect > frameAspect) {
                    frameSize.x = viewportHeight * frameAspect;
                    float xOffset = (viewportWidth - frameSize.x) * 0.5f;
                    ImGui::SetCursorPos(
                        ImVec2((viewportWidth - frameSize.x) * 0.5f, ImGui::GetCursorPosY())
                    );
                    framePos = ImVec2(framePos.x + xOffset, framePos.y);
                } else {
                    frameSize.y = viewportWidth / frameAspect;
                    float yOffset = (viewportHeight - frameSize.y) * 0.5f;
                    ImGui::SetCursorPos(
                        ImVec2(ImGui::GetCursorPosX(), (viewportHeight - frameSize.y) * 0.5f)
                    );
                    framePos = ImVec2(framePos.x, framePos.y + yOffset);
                }
            }
        }

        ImGui::Image(frameTexture, frameSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

        ImVec2 mouseCoord(0.0f, 0.0f);
        if (frameWidth > 0 && frameHeight > 0 && ImGui::IsWindowHovered()) {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 frameMin = framePos;
            ImVec2 frameMax = ImVec2(framePos.x + frameSize.x, framePos.y + frameSize.y);

            bool hovered = mousePos.x >= frameMin.x && mousePos.x <= frameMax.x;
            hovered = hovered && mousePos.y >= frameMin.y && mousePos.y <= frameMax.y;
            if (hovered) {
                float normalizedX = (mousePos.x - frameMin.x) / frameSize.x;
                float normalizedY = (mousePos.y - frameMin.y) / frameSize.y;
                normalizedY = 1.0f - normalizedY;
                mouseCoord.x = normalizedX * frameWidth;
                mouseCoord.y = normalizedY * frameHeight;
                std::array<float, 3> v3 = { mouseCoord.x, mouseCoord.y, 1.0f };
                m_widgetStates[static_cast<int>(ID::MOUSE_COORD)].value = v3;
                pushEvent({ LABEL, static_cast<int>(ID::MOUSE_COORD), v3 });
            }
        } else {
            m_widgetStates[static_cast<int>(ID::MOUSE_COORD)].value =
                std::array<float, 3>{ 0.0f, 0.0f, 0.0f };
        }

        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleVar();
    }
};
