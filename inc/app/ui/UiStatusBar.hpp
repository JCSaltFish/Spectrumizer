/**
 * @file UiStatusBar.hpp
 * @brief Defines the UI of the status bar.
 */

#pragma once

#include "app/AppUiManager.h"

/**
 * @brief The status bar.
 */
class UiStatusBar : public GuiView {
public:
    static constexpr const char* LABEL = "status_bar";

    enum class ID : int {
        // Dynamic informational text
        INFO_TEXT,
        // Number of rendering samples (0 = idle, <0 = pausing)
        RENDERING_SAMPLES,
        // Rendering progress (0.0 to 1.0)
        RENDERING_PROGRESS,
        // Average time per sample in seconds
        EFFICIENCY,
        // Time elapsed since rendering started in seconds
        TIME_ELAPSED,
        // Total number of triangles in the current scene
        TRIANGLE_COUNT,
    };

    UiStatusBar() {
        /* Default states of the widgets */

        m_widgetStates[static_cast<int>(ID::INFO_TEXT)] = {};
        m_widgetStates[static_cast<int>(ID::INFO_TEXT)].value = "";
        m_widgetStates[static_cast<int>(ID::RENDERING_SAMPLES)] = {};
        m_widgetStates[static_cast<int>(ID::RENDERING_SAMPLES)].value = 0;
        m_widgetStates[static_cast<int>(ID::RENDERING_PROGRESS)] = {};
        m_widgetStates[static_cast<int>(ID::RENDERING_PROGRESS)].value = 0.0f;
        m_widgetStates[static_cast<int>(ID::EFFICIENCY)] = {};
        m_widgetStates[static_cast<int>(ID::EFFICIENCY)].value = 0.0f;
        m_widgetStates[static_cast<int>(ID::TIME_ELAPSED)] = {};
        m_widgetStates[static_cast<int>(ID::TIME_ELAPSED)].value = 0.0f;
        m_widgetStates[static_cast<int>(ID::TRIANGLE_COUNT)] = {};
        m_widgetStates[static_cast<int>(ID::TRIANGLE_COUNT)].value = 0;
    }

    void draw() override {
        float statusBarHeight = AppUiManager::instance().getUiSizes().statusBarHeight;
        int windowWidth = AppUiManager::instance().getUiSizes().windowWidth;
        int windowHeight = AppUiManager::instance().getUiSizes().windowHeight;

        float dpiScale = AppUiManager::instance().getDpiScale();
        float renderSegWidth = 210.0f * dpiScale;
        float effSegWidth = 220.0f * dpiScale;
        float timerSegWidth = 200.0f * dpiScale;
        float triCntSegWidth = 210.0f * dpiScale;
        float infoSegWidth = windowWidth - 10.0f;
        infoSegWidth -= renderSegWidth + 10.0f;
        infoSegWidth -= effSegWidth + 10.0f;
        infoSegWidth -= timerSegWidth + 10.0f;
        infoSegWidth -= triCntSegWidth + 10.0f;
        float posX = 0.0f;

        ImGui::SetNextWindowPos(ImVec2(0.0f, windowHeight - statusBarHeight));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, statusBarHeight));

        std::string text;
        int iValue = 0;
        float fValue = 0.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
        ImGui::PushFont(AppUiManager::instance().getNormalIconFont());

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoBringToFrontOnFocus;
        ImGui::Begin("statusbar", 0, flags);

        // Info segment
        ImGui::SetNextItemWidth(infoSegWidth);
        text = getWidgetValue<std::string>(static_cast<int>(ID::INFO_TEXT));
        ImGui::Text("%s", text.c_str());
        posX += infoSegWidth + 10.0f;

        // Render segment
        ImGui::SameLine(posX);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(renderSegWidth);
        iValue = getWidgetValue<int>(static_cast<int>(ID::RENDERING_SAMPLES));
        if (iValue <= 0)
            ImGui::Text(ICON_FK_CIRCLE_O_NOTCH " ");
        else
            ImGui::Text(ICON_FK_PENCIL_SQUARE_O " ");
        ImGui::SameLine();
        fValue = getWidgetValue<float>(static_cast<int>(ID::RENDERING_PROGRESS));
        if (fValue > 0.0f) {
            float progressX = ImGui::GetCursorPosX();
            float progressWidth = renderSegWidth - progressX + posX;
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::ProgressBar(fValue, ImVec2(progressWidth, statusBarHeight - 8.0f));
            ImGui::PopStyleColor();
            ImGui::SameLine(progressX);
        }
        if (iValue == 0)
            text = GuiText::get("status_bar.idle");
        else if (iValue < 0)
            text = GuiText::get("status_bar.paused") + std::to_string(-iValue);
        else
            text = GuiText::get("status_bar.rendering") + std::to_string(iValue);
        ImGui::Text("%s", text.c_str());
        posX += renderSegWidth + 10.0f;

        // Efficiency segment
        ImGui::SameLine(posX);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(effSegWidth);
        ImGui::Text(ICON_FK_TACHOMETER " ");
        ImGui::SameLine();
        fValue = getWidgetValue<float>(static_cast<int>(ID::EFFICIENCY));
        text = GuiText::get("status_bar.efficiency");
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << fValue;
            text = GuiText::formatString(text, { ss.str() });
        }
        ImGui::Text("%s", text.c_str());
        posX += effSegWidth + 10.0f;

        // Timer segment
        ImGui::SameLine(posX);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(timerSegWidth);
        ImGui::Text(ICON_FK_CLOCK_O " ");
        ImGui::SameLine();
        fValue = getWidgetValue<float>(static_cast<int>(ID::TIME_ELAPSED));
        text = GuiText::get("status_bar.time_elapsed");
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << fValue;
            text = GuiText::formatString(text, { ss.str() });
        }
        ImGui::Text("%s", text.c_str());
        posX += timerSegWidth + 10.0f;

        // Triangle count segment
        ImGui::SameLine(posX);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(triCntSegWidth);
        ImGui::Text(ICON_FK_CUBES " ");
        ImGui::SameLine();
        iValue = getWidgetValue<int>(static_cast<int>(ID::TRIANGLE_COUNT));
        text = GuiText::get("status_bar.triangle_count") + std::to_string(iValue);
        ImGui::Text("%s", text.c_str());

        ImGui::End();

        ImGui::PopFont();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
    }
};
