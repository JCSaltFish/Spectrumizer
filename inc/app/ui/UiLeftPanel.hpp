/**
 * @file UiLeftPanel.hpp
 * @brief Defines the UI of the left panel.
 */

#pragma once

#include "app/AppUiManager.h"

class UiLeftPanel : public GuiView {
public:
    static constexpr const char* LABEL = "left_panel";

    enum class ID : int {
        OUTPUT_DISP_CHANNEL,

        WAVE_COUNT,
        WAVES_NODE,
        WAVES_CLEAR,
        WAVES_IMPORT,
        WAVES_ADD,
        WAVES_INDEX,
        WAVES_DELETE,
        WAVES_WAVE_NUMBER,

        MATERIAL_LIST,
        MATERIALS_NODE,
        MATERIALS_IMPORT,
        MATERIALS_ADD,

        MATERIAL_ITEM_DELETE,
        MATERIAL_NAME,
        MATERIAL_WAVE_INDEX,
        MATERIAL_EMISSIVITY,

        SKY_NODE,
        SKY_MATERIAL,
        SKY_TEMPERATURE,
    };

    GuiListView materialListView = GuiListView(this, static_cast<int>(ID::MATERIAL_LIST));
    class MaterialListItem;

    UiLeftPanel() {
        /* Default states of the widgets */

        m_widgetStates[static_cast<int>(ID::OUTPUT_DISP_CHANNEL)] = {};
        m_widgetStates[static_cast<int>(ID::OUTPUT_DISP_CHANNEL)].value = -1;

        m_widgetStates[static_cast<int>(ID::WAVE_COUNT)] = {};
        m_widgetStates[static_cast<int>(ID::WAVE_COUNT)].value = 0;
        m_widgetStates[static_cast<int>(ID::WAVES_NODE)] = {};
        m_widgetStates[static_cast<int>(ID::WAVES_CLEAR)] = {};
        m_widgetStates[static_cast<int>(ID::WAVES_IMPORT)] = {};
        m_widgetStates[static_cast<int>(ID::WAVES_ADD)] = {};
        m_widgetStates[static_cast<int>(ID::WAVES_INDEX)] = {};
        m_widgetStates[static_cast<int>(ID::WAVES_INDEX)].value = -1;
        m_widgetStates[static_cast<int>(ID::WAVES_DELETE)] = {};
        m_widgetStates[static_cast<int>(ID::WAVES_WAVE_NUMBER)] = {};
        m_widgetStates[static_cast<int>(ID::WAVES_WAVE_NUMBER)].value = 0.0f;

        m_widgetStates[static_cast<int>(ID::MATERIAL_LIST)] = {};
        m_widgetStates[static_cast<int>(ID::MATERIALS_NODE)] = {};
        m_widgetStates[static_cast<int>(ID::MATERIALS_IMPORT)] = {};
        m_widgetStates[static_cast<int>(ID::MATERIALS_ADD)] = {};

        m_widgetStates[static_cast<int>(ID::SKY_NODE)] = {};
        m_widgetStates[static_cast<int>(ID::SKY_MATERIAL)] = {};
        m_widgetStates[static_cast<int>(ID::SKY_MATERIAL)].value = -1;
        m_widgetStates[static_cast<int>(ID::SKY_TEMPERATURE)] = {};
        m_widgetStates[static_cast<int>(ID::SKY_TEMPERATURE)].value = 0.0f;
    }

    void draw() override {
        float menuBarHeight = AppUiManager::instance().getUiSizes().menuBarHeight;
        float toolBarHeight = AppUiManager::instance().getUiSizes().toolBarHeight;
        float statusBarHeight = AppUiManager::instance().getUiSizes().statusBarHeight;
        int windowWidth = AppUiManager::instance().getUiSizes().windowWidth;
        int windowHeight = AppUiManager::instance().getUiSizes().windowHeight;

        float dpiScale = AppUiManager::instance().getDpiScale();
        float panelWidth = 333.0f * dpiScale;
        AppUiManager::instance().getUiSizes().leftPanelWidth = panelWidth;
        float panelHeight = windowHeight - menuBarHeight - toolBarHeight - statusBarHeight;

        const float clearBtnSize = 25.0f * dpiScale;
        const float clearBtnPadding = 2.5f * dpiScale;
        const float importBtnSize = 80.0f * dpiScale;
        const float importBtnPadding = 5.0f * dpiScale;
        const float addBtnSize = 70.0f * dpiScale;
        const float addBtnPadding = 10.0f * dpiScale;

        float posX = 0.0f, posY = 0.0f;

        ImGui::SetNextWindowPos(ImVec2(0.0f, menuBarHeight + toolBarHeight));
        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

        std::string text;
        bool valueChanged = false;
        int iValue = 0;
        float fValue = 0.0f;

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus;
        text = GuiText::get("left_panel.title");
        ImGui::Begin(text.c_str(), 0, flags);
        
        int nWaves = getWidgetValue<int>(static_cast<int>(ID::WAVE_COUNT));

        // Output display
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        text = GuiText::get("left_panel.output_disp.title");
        if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            // Channel
            text = GuiText::get("left_panel.output_disp.channel");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(160.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            iValue = getWidgetValue<int>(static_cast<int>(ID::OUTPUT_DISP_CHANNEL));
            text = "";
            for (int i = 0; i < nWaves; i++)
                text += std::to_string(i) + '\0';
            if (ImGui::Combo("##dispChannel", &iValue, text.c_str())) {
                m_widgetStates[static_cast<int>(ID::OUTPUT_DISP_CHANNEL)].value = iValue;
                pushEvent({ LABEL, static_cast<int>(ID::OUTPUT_DISP_CHANNEL), iValue });
            }
            ImGui::TreePop();
        }

        // Waves node
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        text = GuiText::get("left_panel.waves_node.title");
        if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            if (!m_widgetStates[static_cast<int>(ID::WAVES_NODE)].enabled)
                ImGui::BeginDisabled();

            // Wave count
            posY = ImGui::GetCursorPosY();
            ImGui::SetCursorPosY(
                posY + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) * 0.5f
            );
            text = GuiText::get("left_panel.waves_node.count");
            text = GuiText::formatString(text, { std::to_string(nWaves) });
            ImGui::Text("%s", text.c_str());

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

            // Clear waves button
            posX = addBtnSize + addBtnPadding;
            posX += importBtnSize + importBtnPadding;
            posX += clearBtnSize + clearBtnPadding;
            ImGui::SameLine(
                ImGui::GetContentRegionMax().x - posX - ImGui::GetStyle().WindowPadding.x
            );
            ImGui::SetCursorPosY(posY);
            ImGui::SetNextItemWidth(clearBtnSize);
            ImGui::PushFont(AppUiManager::instance().getNormalIconFont());
            if (ImGui::Button(ICON_FK_TRASH "##clear_waves"))
                pushEvent({ LABEL, static_cast<int>(ID::WAVES_CLEAR) });
            ImGui::PopFont();

            // Import waves button
            posX -= clearBtnSize + clearBtnPadding;
            ImGui::SameLine(
                ImGui::GetContentRegionMax().x - posX - ImGui::GetStyle().WindowPadding.x
            );
            text = ICON_FK_SIGN_IN;
            text += std::string("   ") + GuiText::get("left_panel.import") + "##wave";
            ImGui::SetNextItemWidth(importBtnSize);
            if (ImGui::Button(text.c_str()))
                pushEvent({ LABEL, static_cast<int>(ID::WAVES_IMPORT) });

            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            // Add wave button
            posX -= importBtnSize + importBtnPadding;
            ImGui::SameLine(
                ImGui::GetContentRegionMax().x - posX - ImGui::GetStyle().WindowPadding.x
            );
            text = ICON_FK_PLUS;
            text += std::string("   ") + GuiText::get("left_panel.add") + "##wave";
            ImGui::SetNextItemWidth(addBtnSize);
            if (ImGui::Button(text.c_str()))
                pushEvent({ LABEL, static_cast<int>(ID::WAVES_ADD) });

            if (nWaves > 0) {
                // Wave index
                text = GuiText::get("left_panel.waves_node.index");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(130.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                iValue = getWidgetValue<int>(static_cast<int>(ID::WAVES_INDEX));
                text = "";
                for (int i = 0; i < nWaves; i++)
                    text += std::to_string(i) + '\0';
                if (ImGui::Combo("##dispChannel", &iValue, text.c_str())) {
                    m_widgetStates[static_cast<int>(ID::WAVES_INDEX)].value = iValue;
                    pushEvent({ LABEL, static_cast<int>(ID::WAVES_INDEX), iValue });
                }

                // Delete wave button
                ImGui::SameLine();
			    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
			    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
			    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
			    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                ImGui::PushFont(AppUiManager::instance().getNormalIconFont());
                if (ImGui::SmallButton(ICON_FK_TRASH "##wave"))
                    pushEvent({ LABEL, static_cast<int>(ID::WAVES_DELETE), iValue });
                ImGui::PopFont();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();

                // Wave number
                text = GuiText::get("left_panel.waves_node.wave_number");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(130.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                fValue = getWidgetValue<float>(static_cast<int>(ID::WAVES_WAVE_NUMBER));
                if (ImGui::InputFloat("##wave_number", &fValue, 0.0f, 0.0f, "%.7f"))
                    m_widgetStates[static_cast<int>(ID::WAVES_WAVE_NUMBER)].value = fValue;
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    m_widgetStates[static_cast<int>(ID::WAVES_WAVE_NUMBER)].value = fValue;
                    pushEvent({ LABEL, static_cast<int>(ID::WAVES_WAVE_NUMBER), fValue });
                }
            }

            if (!m_widgetStates[static_cast<int>(ID::WAVES_NODE)].enabled)
                ImGui::EndDisabled();
            ImGui::TreePop();
        }

        // Materials node
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        text = GuiText::get("left_panel.materials_node.title");
        if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            if (!m_widgetStates[static_cast<int>(ID::MATERIALS_NODE)].enabled)
                ImGui::BeginDisabled();

            // Material count
            posY = ImGui::GetCursorPosY();
            ImGui::SetCursorPosY(
                posY + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) * 0.5f
            );
            text = GuiText::get("left_panel.materials_node.count");
            text = GuiText::formatString(text, { std::to_string(materialListView.size()) });
            ImGui::Text("%s", text.c_str());

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

            // Import materials button
            posX = addBtnSize + addBtnPadding;
            posX += importBtnSize + importBtnPadding;
            ImGui::SameLine(
                ImGui::GetContentRegionMax().x - posX - ImGui::GetStyle().WindowPadding.x
            );
            text = ICON_FK_SIGN_IN;
            text += std::string("   ") + GuiText::get("left_panel.import") + "##material";
            ImGui::SetNextItemWidth(importBtnSize);
            if (ImGui::Button(text.c_str()))
                pushEvent({ LABEL, static_cast<int>(ID::MATERIALS_IMPORT) });

            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            // Add material button
            posX -= importBtnSize + importBtnPadding;
            ImGui::SameLine(
                ImGui::GetContentRegionMax().x - posX - ImGui::GetStyle().WindowPadding.x
            );
            text = ICON_FK_PLUS;
            text += std::string("   ") + GuiText::get("left_panel.add") + "##material";
            ImGui::SetNextItemWidth(addBtnSize);
            if (ImGui::Button(text.c_str()))
                pushEvent({ LABEL, static_cast<int>(ID::MATERIALS_ADD) });

            // Material list
            materialListView.draw();

            if (!m_widgetStates[static_cast<int>(ID::MATERIALS_NODE)].enabled)
                ImGui::EndDisabled();
            ImGui::TreePop();
        }

        // Sky node
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        text = GuiText::get("left_panel.sky_node.title");
        if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            if (!m_widgetStates[static_cast<int>(ID::SKY_NODE)].enabled)
                ImGui::BeginDisabled();

            // Sky material
            text = GuiText::get("left_panel.sky_node.material");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(130.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            iValue = getWidgetValue<int>(static_cast<int>(ID::SKY_MATERIAL));
            text = "";
            for (int i = 0; i < materialListView.size(); i++) {
                auto item = materialListView.getItem<MaterialListItem>(i);
                std::string name =
                    item->getWidgetValue<std::string>(static_cast<int>(ID::MATERIAL_NAME));
                text += name + '\0';
            }
            if (ImGui::Combo("##skyMaterial", &iValue, text.c_str())) {
                m_widgetStates[static_cast<int>(ID::SKY_MATERIAL)].value = iValue;
                pushEvent({ LABEL, static_cast<int>(ID::SKY_MATERIAL), iValue });
            }

            // Sky temperature
            text = GuiText::get("left_panel.sky_node.temperature");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(130.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            fValue = getWidgetValue<float>(static_cast<int>(ID::SKY_TEMPERATURE));
            valueChanged = ImGui::DragFloat(
                "##skyTemperature",
                &fValue,
                0.01f,
                -273.15f,
                std::numeric_limits<float>::max(),
                "%.2f",
                ImGuiSliderFlags_AlwaysClamp
            );
            if (valueChanged)
                m_widgetStates[static_cast<int>(ID::SKY_TEMPERATURE)].value = fValue;
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                m_widgetStates[static_cast<int>(ID::SKY_TEMPERATURE)].value = fValue;
                pushEvent({ LABEL, static_cast<int>(ID::SKY_TEMPERATURE), fValue });
            }

            if (!m_widgetStates[static_cast<int>(ID::SKY_NODE)].enabled)
                ImGui::EndDisabled();
            ImGui::TreePop();
        }

        ImGui::End();
    }

    class MaterialListItem : public GuiListItem {
    public:
        DbObjHandle hMaterial = {};

        MaterialListItem() {
            /* Default states of the widgets */

            m_widgetStates[static_cast<int>(ID::MATERIAL_ITEM_DELETE)] = {};

            m_widgetStates[static_cast<int>(ID::MATERIAL_NAME)] = {};
            m_widgetStates[static_cast<int>(ID::MATERIAL_NAME)].value = "";
            m_widgetStates[static_cast<int>(ID::MATERIAL_WAVE_INDEX)] = {};
            m_widgetStates[static_cast<int>(ID::MATERIAL_WAVE_INDEX)].value = 0;
            m_widgetStates[static_cast<int>(ID::MATERIAL_EMISSIVITY)] = {};
            m_widgetStates[static_cast<int>(ID::MATERIAL_EMISSIVITY)].value = 0.0f;
        }

        void draw() override {
            float dpiScale = AppUiManager::instance().getDpiScale();

            std::string uidStr = getUidString();
            std::string idStr = "##material" + uidStr;
            std::string text = getWidgetValue<std::string>(static_cast<int>(ID::MATERIAL_NAME));
            int iValue = 0;
            float fValue = 0.0f;

            ImGuiTreeNodeFlags treeNodeFlags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                ImGuiTreeNodeFlags_SpanAvailWidth;
            if (isSelected())
                treeNodeFlags |= ImGuiTreeNodeFlags_Selected;

            if (isExpanded())
                ImGui::SetNextItemOpen(true);
            else
                ImGui::SetNextItemOpen(false);

            bool expanded = ImGui::TreeNodeEx(idStr.c_str(), treeNodeFlags, "%s", text.c_str());
            setExpanded(expanded);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 6.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 4.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushFont(AppUiManager::instance().getNormalIconFont());
            if (ImGui::BeginPopupContextItem()) {
                // Delete
                text = GuiText::get("menu_bar.edit_menu.delete");
                if (ImGui::MenuItemEx(text.c_str(), ICON_FK_TRASH))
                    pushEvent({ LABEL, static_cast<int>(ID::MATERIAL_ITEM_DELETE) });
                ImGui::EndPopup();
            }
            ImGui::PopFont();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();

            if (expanded) {
                // Name
                text = GuiText::get("left_panel.material_node.name");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(130.0f * dpiScale);
                ImGui::SetNextItemWidth(183.0f * dpiScale);
                idStr = "##materialName" + uidStr;
                text = getWidgetValue<std::string>(static_cast<int>(ID::MATERIAL_NAME));
                if (ImGui::InputText(idStr.c_str(), &text))
                    m_widgetStates[static_cast<int>(ID::MATERIAL_NAME)].value = text;
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    m_widgetStates[static_cast<int>(ID::MATERIAL_NAME)].value = text;
                    pushEvent({ LABEL, static_cast<int>(ID::MATERIAL_NAME), text });
                }

                // Wave index
                text = GuiText::get("left_panel.material_node.wave_index");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(130.0f * dpiScale);
                ImGui::SetNextItemWidth(183.0f * dpiScale);
                idStr = "##materialWaveIdx" + uidStr;
                iValue = getWidgetValue<int>(static_cast<int>(ID::MATERIAL_WAVE_INDEX));
                text = "";
                int nWaves = 0;
                auto listView = getListView();
                if (listView != nullptr) {
                    auto view = listView->getView();
                    if (view != nullptr)
                        nWaves = view->getWidgetValue<int>(static_cast<int>(ID::WAVE_COUNT));
                }
                for (int i = 0; i < nWaves; i++)
                    text += std::to_string(i) + '\0';
                if (ImGui::Combo(idStr.c_str(), &iValue, text.c_str())) {
                    m_widgetStates[static_cast<int>(ID::MATERIAL_WAVE_INDEX)].value = iValue;
                    pushEvent({ LABEL, static_cast<int>(ID::MATERIAL_WAVE_INDEX), iValue });
                }

                // Emissivity
                text = GuiText::get("left_panel.material_node.emissivity");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(130.0f * dpiScale);
                ImGui::SetNextItemWidth(183.0f * dpiScale);
                idStr = "##materialImissivity" + uidStr;
                fValue = getWidgetValue<float>(static_cast<int>(ID::MATERIAL_EMISSIVITY));
                if (ImGui::InputFloat(idStr.c_str(), &fValue, 0.0f, 0.0f, "%.7f"))
                    m_widgetStates[static_cast<int>(ID::MATERIAL_EMISSIVITY)].value = fValue;
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    m_widgetStates[static_cast<int>(ID::MATERIAL_EMISSIVITY)].value = fValue;
                    pushEvent(
                        { LABEL, static_cast<int>(ID::MATERIAL_EMISSIVITY), fValue }
                    );
                }

                ImGui::TreePop();
            }
        }
    };
};
