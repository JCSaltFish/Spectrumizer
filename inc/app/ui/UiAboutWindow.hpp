/**
 * @file UiAboutWindow.hpp
 * @brief UI dialog for the about window.
 */

#pragma once

#include "app/AppUiManager.h"

/*
 * @brief The about window UI.
 */
class UiAboutWindow : public GuiDialogView {
public:
    static constexpr const char* LABEL = "about";

    enum class ID : int {
        VERSION,
    };

    ImTextureID appIconTexture = 0;

    UiAboutWindow() {
        /* Default states of the widgets */

        m_widgetStates[static_cast<int>(ID::VERSION)] = {};
        m_widgetStates[static_cast<int>(ID::VERSION)].value = "1.0.0";
    }

    void draw() override {
        bool show = isShown();
        if (!show)
            return;

        float dpiScale = AppUiManager::instance().getDpiScale();

        std::string text;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28.0f, 20.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

        ImGui::SetNextWindowPos
        (
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Appearing,
            ImVec2(0.5f, 0.5f)
        );
        ImGui::SetNextWindowSize(ImVec2(450.0f * dpiScale, 250.0f * dpiScale));

        // Title
        text = GuiText::get("about.title") + GuiText::get("app_name");
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoResize;
        ImGui::Begin(text.c_str(), &show, flags);
        if (!show)
            close();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 2.0f));

        // Application icon
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
        ImGui::BeginChild(
            "aboutIcon",
            ImVec2((450.0f - 56.0f) * dpiScale, 140.0f * dpiScale)
        );
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 6.0f));

        ImGui::Image(appIconTexture, ImVec2(120.0f * dpiScale, 120.0f * dpiScale));

        ImGui::SameLine(140.0f * dpiScale);
        ImGui::BeginChild("aboutText", ImVec2(250.0f * dpiScale, 120.0f * dpiScale));

        // App information
        text = GuiText::get("app_name");
        ImGui::Text("%s", text.c_str());
        text = GuiText::get("about.version");
        text += getWidgetValue<std::string>(static_cast<int>(ID::VERSION));
        ImGui::Text("%s", text.c_str());

        ImGui::EndChild();

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::PopStyleVar();

        // OK button
        ImGui::SetCursorPosX((450.0f - 28.0f - 120.0f) * dpiScale);
        text = GuiText::get("about.ok");
        if (ImGui::Button(text.c_str(), ImVec2(120.0f * dpiScale, 25.0f * dpiScale)))
            close();
        ImGui::SetItemDefaultFocus();

        ImGui::PopStyleVar();
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
    }
};
