/**
 * @file UiTool.hpp
 * @brief Defines the UI of the tool bar.
 */

#pragma once

#include "app/AppUiManager.h"

/**
 * @brief The tool bar.
 */
class UiToolBar : public GuiView {
public:
    static constexpr const char* LABEL = "tool_bar";

    enum class ID : int {
        SAVE,
        EXPORT,

        DISPLAY_MODE,

        LOAD_MODEL,

        START,
        PAUSE,
        STOP,
        RESTART,
        TARGET_SAMPLES,

        SETTINGS,
    };

    enum class DisplayMode : int {
        PREVIEW_MODE = 0,
        PATH_TRACER_OUTPUT = 1,
    };

    UiToolBar() {
        /* Default states of the widgets */

        m_widgetStates[static_cast<int>(ID::SAVE)] = {};
        m_widgetStates[static_cast<int>(ID::EXPORT)] = {};

        m_widgetStates[static_cast<int>(ID::DISPLAY_MODE)] = {};
        m_widgetStates[static_cast<int>(ID::DISPLAY_MODE)].value =
            static_cast<int>(DisplayMode::PREVIEW_MODE);

        m_widgetStates[static_cast<int>(ID::LOAD_MODEL)] = {};

        m_widgetStates[static_cast<int>(ID::START)] = {};
        m_widgetStates[static_cast<int>(ID::PAUSE)] = {};
        m_widgetStates[static_cast<int>(ID::PAUSE)].visible = false;
        m_widgetStates[static_cast<int>(ID::STOP)] = {};
        m_widgetStates[static_cast<int>(ID::STOP)].enabled = false;
        m_widgetStates[static_cast<int>(ID::RESTART)] = {};
        m_widgetStates[static_cast<int>(ID::RESTART)].enabled = false;
        m_widgetStates[static_cast<int>(ID::TARGET_SAMPLES)] = {};
        m_widgetStates[static_cast<int>(ID::TARGET_SAMPLES)].value = 0;

        m_widgetStates[static_cast<int>(ID::SETTINGS)] = {};
    }

    void draw() override {
        float menuBarHeight = AppUiManager::instance().getUiSizes().menuBarHeight;
        int windowWidth = AppUiManager::instance().getUiSizes().windowWidth;

        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.09f, 0.09f, 0.09f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));

        ImGui::SetNextWindowPos(ImVec2(0.0f, menuBarHeight));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0.0f));

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus;
        ImGui::Begin("Toolbar", 0, flags);

        std::string text;
        int value = 0;

        // Save
        text = std::string(ICON_FK_FLOPPY_O) + std::string("   ") + GuiText::get("tool_bar.save");
        if (!m_widgetStates[static_cast<int>(ID::SAVE)].enabled) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::Button(text.c_str());
            ImGui::PopStyleColor();
        } else if (ImGui::Button(text.c_str()))
            pushEvent({ LABEL, static_cast<int>(ID::SAVE), {} });

        // Export
        ImGui::SameLine();
        text = std::string(ICON_FK_SIGN_OUT) + std::string("   ") + GuiText::get("tool_bar.export");
        if (!m_widgetStates[static_cast<int>(ID::EXPORT)].enabled) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::Button(text.c_str());
            ImGui::PopStyleColor();
        } else if (ImGui::Button(text.c_str()))
            pushEvent({ LABEL, static_cast<int>(ID::EXPORT), {} });

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::Text("  ");

        // Display mode
        ImGui::SameLine();
        ImGui::SetNextItemWidth(160.0f * AppUiManager::instance().getDpiScale());
        value = getWidgetValue<int>(static_cast<int>(ID::DISPLAY_MODE));
        text = GuiText::get("tool_bar.display_modes.preview_mode") + '\0';
        text += GuiText::get("tool_bar.display_modes.path_tracer_output") + '\0';
        if (ImGui::Combo("##display_mode", &value, text.c_str())) {
            m_widgetStates[static_cast<int>(ID::DISPLAY_MODE)].value = value;
            pushEvent({ LABEL, static_cast<int>(ID::DISPLAY_MODE), value });
        }
        ImGui::SameLine();
        ImGui::Text("  ");

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

        // Load model
        ImGui::SameLine();
        text = std::string(ICON_FK_PLUS) + std::string("   ") + GuiText::get("tool_bar.load_model");
        if (!m_widgetStates[static_cast<int>(ID::LOAD_MODEL)].enabled) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::Button(text.c_str());
            ImGui::PopStyleColor();
        } else if (ImGui::Button(text.c_str()))
            pushEvent({ LABEL, static_cast<int>(ID::LOAD_MODEL), {} });

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::Text("  ");

        // Path tracer controls
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::SameLine();
            ImGui::BeginChild(
                "PathTracerControls",
                ImVec2(0.0f, 0.0f),
                ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX,
                ImGuiWindowFlags_NoScrollbar
            );

            // Start
            if (m_widgetStates[static_cast<int>(ID::START)].visible) {
                if (!m_widgetStates[static_cast<int>(ID::START)].enabled) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                    ImGui::SmallButton(ICON_FK_PLAY);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 0.5f, 1.0f));
                    if (ImGui::SmallButton(ICON_FK_PLAY))
                        pushEvent({ LABEL, static_cast<int>(ID::START), {} });
                }
                ImGui::PopStyleColor();
            }
            // Pause
            if (m_widgetStates[static_cast<int>(ID::PAUSE)].visible) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                if (ImGui::SmallButton(ICON_FK_PAUSE))
                    pushEvent({ LABEL, static_cast<int>(ID::PAUSE), {} });
                ImGui::PopStyleColor();
            }

            // Stop
            ImGui::SameLine();
            if (m_widgetStates[static_cast<int>(ID::STOP)].visible) {
                if (!m_widgetStates[static_cast<int>(ID::STOP)].enabled) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                    ImGui::SmallButton(ICON_FK_STOP);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                    if (ImGui::SmallButton(ICON_FK_STOP))
                        pushEvent({ LABEL, static_cast<int>(ID::STOP), {} });
                }
                ImGui::PopStyleColor();
            }

            // Restart
            ImGui::SameLine();
            if (m_widgetStates[static_cast<int>(ID::RESTART)].visible) {
                if (!m_widgetStates[static_cast<int>(ID::RESTART)].enabled) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                    ImGui::SmallButton(ICON_FK_UNDO);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                    if (ImGui::SmallButton(ICON_FK_UNDO))
                        pushEvent({ LABEL, static_cast<int>(ID::RESTART), {} });
                }
                ImGui::PopStyleColor();
            }

            // Target samples
            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine();
            if (!m_widgetStates[static_cast<int>(ID::TARGET_SAMPLES)].enabled) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                ImGui::SmallButton(ICON_FK_ELLIPSIS_V);
                ImGui::PopStyleColor();
            } else {
                if (ImGui::SmallButton(ICON_FK_ELLIPSIS_V))
                    ImGui::OpenPopup("targetSampleConfig");
            }
            {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 6.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 2.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 4.0f));
                ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                if (ImGui::BeginPopup("targetSampleConfig")) {
                    ImGui::Text("%s", GuiText::get("tool_bar.target_samples").c_str());
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(100.0f * AppUiManager::instance().getDpiScale());
                    value = getWidgetValue<int>(static_cast<int>(ID::TARGET_SAMPLES));
                    if (ImGui::InputInt("##targetSample", &value, 0)) {
                        value = value < 0 ? 0 : value;
                        m_widgetStates[static_cast<int>(ID::TARGET_SAMPLES)].value = value;
                        pushEvent({ LABEL, static_cast<int>(ID::TARGET_SAMPLES), value });
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
                ImGui::PopStyleVar();
                ImGui::PopStyleVar();
            }

            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
        }

        // Settings
        text = std::string(ICON_FK_COG) + std::string("   ") + GuiText::get("tool_bar.settings");
        float buttonWidth = 100.0f * AppUiManager::instance().getDpiScale();
        float posX =
            ImGui::GetContentRegionMax().x - buttonWidth - ImGui::GetStyle().WindowPadding.x;
        ImGui::SameLine(posX);
        if (ImGui::Button(text.c_str(), ImVec2(buttonWidth, 0.0f)))
            pushEvent({ LABEL, static_cast<int>(ID::SETTINGS), {} });

        AppUiManager::instance().getUiSizes().toolBarHeight = ImGui::GetWindowHeight();

        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        ImGui::PopStyleColor();
    }
};
