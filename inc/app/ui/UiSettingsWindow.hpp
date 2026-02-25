/**
 * @file UiSettingsWindow.hpp
 * @brief UI dialog for application settings.
 */

#pragma once

#include "app/AppUiManager.h"

/*
 * @brief The settings window UI.
 */
class UiSettingsWindow : public GuiDialogView {
public:
    static constexpr const char* LABEL = "settings";

    enum class ID : int {
        LANGUAGE,
        MSAA,
        CAM_NAV_SPEED,
        BACKGROUND_COLOR,
        HOVERED_COLOR,
        PICKED_COLOR,
        RESET_DEFAULTS,
    };

    enum class Language : int {
        EN_US = 0,
        ZH_CN = 1,
    };

    enum class MsaaLevel : int {
        MSAA_1X = 0,
        MSAA_2X = 1,
        MSAA_4X = 2,
        MSAA_8X = 3,
    };

    UiSettingsWindow() {
        /* Default states of the widgets */

        m_widgetStates[static_cast<int>(ID::LANGUAGE)] = {};
        m_widgetStates[static_cast<int>(ID::LANGUAGE)].value =
            static_cast<int>(Language::EN_US);
        m_widgetStates[static_cast<int>(ID::MSAA)] = {};
        m_widgetStates[static_cast<int>(ID::MSAA)].value =
            static_cast<int>(MsaaLevel::MSAA_1X);
        m_widgetStates[static_cast<int>(ID::CAM_NAV_SPEED)] = {};
        m_widgetStates[static_cast<int>(ID::CAM_NAV_SPEED)].value = 3;
        m_widgetStates[static_cast<int>(ID::BACKGROUND_COLOR)] = {};
        m_widgetStates[static_cast<int>(ID::BACKGROUND_COLOR)].value =
            std::array<float, 3>{ 0.0f, 0.0f, 0.0f };
        m_widgetStates[static_cast<int>(ID::HOVERED_COLOR)] = {};
        m_widgetStates[static_cast<int>(ID::HOVERED_COLOR)].value =
            std::array<float, 3>{ 0.9f, 0.9f, 0.1f };
        m_widgetStates[static_cast<int>(ID::PICKED_COLOR)] = {};
        m_widgetStates[static_cast<int>(ID::PICKED_COLOR)].value =
            std::array<float, 3>{ 0.1f, 0.7f, 0.9f };
        m_widgetStates[static_cast<int>(ID::RESET_DEFAULTS)] = {};
    }

    void draw() override {
        bool show = isShown();
        if (!show)
            return;

        float dpiScale = AppUiManager::instance().getDpiScale();

        std::string text;
        bool valueChanged = false;
        int iValue = 0;
        float fValue = 0.0f;
        std::array<float, 3> v3 = {};

        ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Once,
            ImVec2(0.5f, 0.5f)
        );
        ImGui::SetNextWindowSize(ImVec2(500.0f * dpiScale, 500.0f * dpiScale), ImGuiCond_Once);
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(500.0f * dpiScale, 100.0f * dpiScale),
            ImVec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
        );

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 4.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::PushFont(AppUiManager::instance().getNormalIconFont());

        // Title
        text = GuiText::get("settings.title");
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin(text.c_str(), &show, flags);
        if (!show)
            close();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 2.0f));

        // Reset to defaults
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 6.0f));
        float curPosX = ImGui::GetContentRegionMax().x;
        curPosX -= 128.0f * dpiScale + ImGui::GetStyle().WindowPadding.x;
        ImGui::SetCursorPosX(curPosX);
        text = GuiText::get("settings.reset");
        if (ImGui::Button(text.c_str()))
            pushEvent({ LABEL, static_cast<int>(ID::RESET_DEFAULTS) });
        ImGui::PopStyleVar();

        // General settings
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        text = GuiText::get("settings.general");
        if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(8.0f, 8.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

            // Language
            text = std::string("     ") + GuiText::get("settings.language");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(250.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            iValue = getWidgetValue<int>(static_cast<int>(ID::LANGUAGE));
            text = GuiText::get("settings.languages.en_us") + '\0';
            text += GuiText::get("settings.languages.zh_cn") + '\0';
            if (ImGui::Combo("##lang", &iValue, text.c_str())) {
                m_widgetStates[static_cast<int>(ID::LANGUAGE)].value = iValue;
                pushEvent({ LABEL, static_cast<int>(ID::LANGUAGE), iValue });
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::TreePop();
        }

        // Previewer settings
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        text = GuiText::get("settings.previewer");
        if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(8.0f, 8.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

            // MSAA Level
            text = std::string("     ") + GuiText::get("settings.msaa");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(250.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            iValue = getWidgetValue<int>(static_cast<int>(ID::MSAA));
            text = std::string("1x") + '\0';
            text += std::string("2x") + '\0';
            text += std::string("4x") + '\0';
            text += std::string("8x") + '\0';
            if (ImGui::Combo("##msaa", &iValue, text.c_str())) {
                m_widgetStates[static_cast<int>(ID::MSAA)].value = iValue;
                pushEvent({ LABEL, static_cast<int>(ID::MSAA), iValue });
            }

            // Camera navigation speed
            text = std::string("     ") + GuiText::get("settings.cam_move_speed");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(250.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            iValue = getWidgetValue<int>(static_cast<int>(ID::CAM_NAV_SPEED));
            valueChanged = ImGui::SliderInt(
                "##camNavSpeed",
                &iValue,
                1,
                10,
                "%d",
                ImGuiSliderFlags_AlwaysClamp
            );
            if (valueChanged) {
                m_widgetStates[static_cast<int>(ID::CAM_NAV_SPEED)].value = iValue;
                pushEvent({ LABEL, static_cast<int>(ID::CAM_NAV_SPEED), iValue });
            }

            curPosX = ImGui::GetCursorPosX();
            // Background color
            ImGui::SetCursorPosX(250.0f * dpiScale);
            v3 = getWidgetValue<std::array<float, 3>>(static_cast<int>(ID::BACKGROUND_COLOR));
            valueChanged = ImGui::ColorEdit3(
                "##bgCol",
                v3.data(),
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip
            );
            if (valueChanged) {
                m_widgetStates[static_cast<int>(ID::BACKGROUND_COLOR)].value = v3;
                pushEvent({ LABEL, static_cast<int>(ID::BACKGROUND_COLOR), v3 });
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::SameLine();
            ImGui::SetCursorPosX(curPosX);
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            text = GuiText::get("settings.bg_color");
            if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::Text("        R");
                ImGui::SameLine(250.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                fValue = v3[0];
                valueChanged = ImGui::DragFloat
                (
                    "##bgColR",
                    &fValue,
                    0.01f,
                    0.0f,
                    1.0f,
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp
                );
                if (valueChanged) {
                    v3[0] = fValue;
                    m_widgetStates[static_cast<int>(ID::BACKGROUND_COLOR)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::BACKGROUND_COLOR), v3 });
                }

                ImGui::Text("        G");
                ImGui::SameLine(250.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                fValue = v3[1];
                valueChanged = ImGui::DragFloat
                (
                    "##bgColG",
                    &fValue,
                    0.01f,
                    0.0f,
                    1.0f,
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp
                );
                if (valueChanged) {
                    v3[1] = fValue;
                    m_widgetStates[static_cast<int>(ID::BACKGROUND_COLOR)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::BACKGROUND_COLOR), v3 });
                }

                ImGui::Text("        B");
                ImGui::SameLine(250.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                fValue = v3[2];
                valueChanged = ImGui::DragFloat
                (
                    "##bgColB",
                    &fValue,
                    0.01f,
                    0.0f,
                    1.0f,
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp
                );
                if (valueChanged) {
                    v3[2] = fValue;
                    m_widgetStates[static_cast<int>(ID::BACKGROUND_COLOR)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::BACKGROUND_COLOR), v3 });
                }

                ImGui::TreePop();
            }
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(8.0f, 8.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

            // Hovered color
            ImGui::SetCursorPosX(250.0f * dpiScale);
            v3 = getWidgetValue<std::array<float, 3>>(static_cast<int>(ID::HOVERED_COLOR));
            valueChanged = ImGui::ColorEdit3(
                "##hoverCol",
                v3.data(),
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip
            );
            if (valueChanged) {
                m_widgetStates[static_cast<int>(ID::HOVERED_COLOR)].value = v3;
                pushEvent({ LABEL, static_cast<int>(ID::HOVERED_COLOR), v3 });
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::SameLine();
            ImGui::SetCursorPosX(curPosX);
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            text = GuiText::get("settings.hover_color");
            if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::Text("        R");
                ImGui::SameLine(250.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                fValue = v3[0];
                valueChanged = ImGui::DragFloat
                (
                    "##hoverColR",
                    &fValue,
                    0.01f,
                    0.0f,
                    1.0f,
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp
                );
                if (valueChanged) {
                    v3[0] = fValue;
                    m_widgetStates[static_cast<int>(ID::HOVERED_COLOR)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::HOVERED_COLOR), v3 });
                }

                ImGui::Text("        G");
                ImGui::SameLine(250.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                fValue = v3[1];
                valueChanged = ImGui::DragFloat
                (
                    "##hoverColG",
                    &fValue,
                    0.01f,
                    0.0f,
                    1.0f,
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp
                );
                if (valueChanged) {
                    v3[1] = fValue;
                    m_widgetStates[static_cast<int>(ID::HOVERED_COLOR)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::HOVERED_COLOR), v3 });
                }

                ImGui::Text("        B");
                ImGui::SameLine(250.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                fValue = v3[2];
                valueChanged = ImGui::DragFloat
                (
                    "##hoverColB",
                    &fValue,
                    0.01f,
                    0.0f,
                    1.0f,
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp
                );
                if (valueChanged) {
                    v3[2] = fValue;
                    m_widgetStates[static_cast<int>(ID::HOVERED_COLOR)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::HOVERED_COLOR), v3 });
                }

                ImGui::TreePop();
            }
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(8.0f, 8.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

            // Picked color
            ImGui::SetCursorPosX(250.0f * dpiScale);
            v3 = getWidgetValue<std::array<float, 3>>(static_cast<int>(ID::PICKED_COLOR));
            valueChanged = ImGui::ColorEdit3(
                "##pickCol",
                v3.data(),
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip
            );
            if (valueChanged) {
                m_widgetStates[static_cast<int>(ID::PICKED_COLOR)].value = v3;
                pushEvent({ LABEL, static_cast<int>(ID::PICKED_COLOR), v3 });
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::SameLine();
            ImGui::SetCursorPosX(curPosX);
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            text = GuiText::get("settings.picked_color");
            if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::Text("        R");
                ImGui::SameLine(250.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                fValue = v3[0];
                valueChanged = ImGui::DragFloat
                (
                    "##pickColR",
                    &fValue,
                    0.01f,
                    0.0f,
                    1.0f,
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp
                );
                if (valueChanged) {
                    v3[0] = fValue;
                    m_widgetStates[static_cast<int>(ID::PICKED_COLOR)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::PICKED_COLOR), v3 });
                }

                ImGui::Text("        G");
                ImGui::SameLine(250.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                fValue = v3[1];
                valueChanged = ImGui::DragFloat
                (
                    "##pickColG",
                    &fValue,
                    0.01f,
                    0.0f,
                    1.0f,
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp
                );
                if (valueChanged) {
                    v3[1] = fValue;
                    m_widgetStates[static_cast<int>(ID::PICKED_COLOR)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::PICKED_COLOR), v3 });
                }

                ImGui::Text("        B");
                ImGui::SameLine(250.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                fValue = v3[2];
                valueChanged = ImGui::DragFloat
                (
                    "##pickColB",
                    &fValue,
                    0.01f,
                    0.0f,
                    1.0f,
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp
                );
                if (valueChanged) {
                    v3[2] = fValue;
                    m_widgetStates[static_cast<int>(ID::PICKED_COLOR)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::PICKED_COLOR), v3 });
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        ImGui::PopStyleVar();
        ImGui::End();

        ImGui::PopFont();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
};
