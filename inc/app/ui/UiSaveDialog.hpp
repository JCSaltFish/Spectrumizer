/**
 * @file UiSaveDialog.hpp
 * @brief UI dialog for saving files.
 */

#pragma once

#include "app/AppUiManager.h"

 /**
  * @brief The save dialog UI.
  */
class UiSaveDialog : public GuiDialogView {
public:
    static constexpr const char* LABEL = "save_dialog";

    enum class ID : int {
        FILENAME,
        YES,
        NO,
        CANCEL,
    };

    UiSaveDialog() {
        /* Default states of the widgets */

        m_widgetStates[static_cast<int>(ID::FILENAME)] = {};
        m_widgetStates[static_cast<int>(ID::FILENAME)].value = "Untitled.pts";
        m_widgetStates[static_cast<int>(ID::YES)] = {};
        m_widgetStates[static_cast<int>(ID::NO)] = {};
        m_widgetStates[static_cast<int>(ID::CANCEL)] = {};
    }

    void draw() override {
        if (!isShown())
            return;

        std::string text;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28.0f, 20.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

        text = GuiText::get("app_name") + "##SaveDialog";
        ImGui::OpenPopup(text.c_str());

        ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)
        );
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoResize;
        if (ImGui::BeginPopupModal(text.c_str(), nullptr, flags)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 2.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 16.0f));

            std::string filename = getWidgetValue<std::string>(static_cast<int>(ID::FILENAME));
            text = GuiText::get("save_dialog.text") + "\n" + filename;
            ImGui::Text("%s", text.c_str());

            ImVec2 btnSize = ImVec2(
                120.0f * AppUiManager::instance().getDpiScale(),
                25.0f * AppUiManager::instance().getDpiScale()
            );

            if (ImGui::Button(GuiText::get("save_dialog.yes").c_str(), btnSize)) {
                pushEvent({ LABEL, static_cast<int>(ID::YES) });
                close();
            }
            ImGui::SetItemDefaultFocus();

            ImGui::SameLine();
            if (ImGui::Button(GuiText::get("save_dialog.no").c_str(), btnSize)) {
                pushEvent({ LABEL, static_cast<int>(ID::NO) });
                close();
            }

            ImGui::SameLine();
            if (ImGui::Button(GuiText::get("save_dialog.cancel").c_str(), btnSize)) {
                pushEvent({ LABEL, static_cast<int>(ID::CANCEL) });
                close();
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::EndPopup();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
    }
};
