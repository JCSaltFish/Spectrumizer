/**
 * @file UiRightPanel.hpp
 * @brief Defines the UI of the right panel.
 */

#pragma once

#include "app/AppUiManager.h"

/**
 * @brief The right panel.
 */
class UiRightPanel : public GuiView {
public:
    static constexpr const char* LABEL = "right_panel";

    enum class ID : int {
        IMAGE_NODE,
        TRACE_DEPTH,
        RES_X,
        RES_Y,

        CAMERA_NODE,
        FOCUS_DIST,
        F_STOP,
        CAM_POS,
        CAM_ROT,

        SCENE_NODE,
        ADD_MODEL,

        MODEL_LIST,
        MODEL_ITEM_HOVERED,
        MODEL_ITEM_SELECTED,
        MODEL_ITEM_CONTEXT,
        MODEL_ITEM_CUT,
        MODEL_ITEM_COPY,
        MODEL_ITEM_PASTE,
        MODEL_ITEM_DELETE,
        MODEL_ITEM_SELECT_ALL,

        MODEL_NAME,
        MODEL_LOCATION,
        MODEL_ROTATION,
        MODEL_SCALE,
        MODEL_SCALE_LOCK,

        MESH_LIST,
        MESH_ITEM_HOVERED,
        MESH_NAME,

        MESH_MATERIAL_TYPE,
        MESH_ROUGHNESS,
        MESH_MATERIAL,
        MESH_TEMPERATURE,
        MESH_IOR,

        MESH_NORMAL_TEX_LOAD,
        MESH_NORMAL_TEX_REMOVE,
        MESH_ROUGHNESS_TEX_LOAD,
        MESH_ROUGHNESS_TEX_REMOVE,
        MESH_TEMPERATURE_TEX_LOAD,
        MESH_TEMPERATURE_TEX_REMOVE,
    };

    enum class MaterialType : int {
        DIFFUSE,
        SPECULAR,
        GLOSSY,
        TRANSLUCENT,
    };

    GuiListView modelListView = GuiListView(this, static_cast<int>(ID::MODEL_LIST));

    UiRightPanel() {
        /* Default states of the widgets */

        m_widgetStates[static_cast<int>(ID::IMAGE_NODE)] = {};
        m_widgetStates[static_cast<int>(ID::TRACE_DEPTH)] = {};
        m_widgetStates[static_cast<int>(ID::TRACE_DEPTH)].value = 3;
        m_widgetStates[static_cast<int>(ID::RES_X)] = {};
        m_widgetStates[static_cast<int>(ID::RES_X)].value = 1024;
        m_widgetStates[static_cast<int>(ID::RES_Y)] = {};
        m_widgetStates[static_cast<int>(ID::RES_Y)].value = 768;

        m_widgetStates[static_cast<int>(ID::CAMERA_NODE)] = {};
        m_widgetStates[static_cast<int>(ID::FOCUS_DIST)] = {};
        m_widgetStates[static_cast<int>(ID::FOCUS_DIST)].value = 5.0f;
        m_widgetStates[static_cast<int>(ID::F_STOP)] = {};
        m_widgetStates[static_cast<int>(ID::F_STOP)].value = 32.0f;
        m_widgetStates[static_cast<int>(ID::CAM_POS)] = {};
        m_widgetStates[static_cast<int>(ID::CAM_POS)].value =
            std::array<float, 3>{ 0.0f, 0.0f, -10.0f };
        m_widgetStates[static_cast<int>(ID::CAM_ROT)] = {};
        m_widgetStates[static_cast<int>(ID::CAM_ROT)].value =
            std::array<float, 3>{ 0.0f, 0.0f, 0.0f };

        m_widgetStates[static_cast<int>(ID::SCENE_NODE)] = {};
        m_widgetStates[static_cast<int>(ID::MODEL_ITEM_PASTE)] = {};
        m_widgetStates[static_cast<int>(ID::MODEL_ITEM_SELECT_ALL)] = {};
        m_widgetStates[static_cast<int>(ID::ADD_MODEL)] = {};

        m_widgetStates[static_cast<int>(ID::MESH_MATERIAL)] = {};
    }

    void draw() override {
        float menuBarHeight = AppUiManager::instance().getUiSizes().menuBarHeight;
        float toolBarHeight = AppUiManager::instance().getUiSizes().toolBarHeight;
        float statusBarHeight = AppUiManager::instance().getUiSizes().statusBarHeight;
        int windowWidth = AppUiManager::instance().getUiSizes().windowWidth;
        int windowHeight = AppUiManager::instance().getUiSizes().windowHeight;

        float dpiScale = AppUiManager::instance().getDpiScale();
        float panelWidth = 380.0f * dpiScale;
        AppUiManager::instance().getUiSizes().rightPanelWidth = panelWidth;
        float panelHeight = windowHeight - menuBarHeight - toolBarHeight - statusBarHeight;

        ImGui::SetNextWindowPos(ImVec2(windowWidth - panelWidth, menuBarHeight + toolBarHeight));
        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

        std::string text;
        bool valueChanged = false;
        int iValue = 0;
        float fValue = 0.0f;
        std::array<float, 3> v3 = {};

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus;
        text = GuiText::get("right_panel.title");
        ImGui::Begin(text.c_str(), 0, flags);

        // Image node
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        text = GuiText::get("right_panel.image_node.title");
        if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            if (!m_widgetStates[static_cast<int>(ID::IMAGE_NODE)].enabled)
                ImGui::BeginDisabled();

            // Trace depth
            text = GuiText::get("right_panel.image_node.trace_depth");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(160.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            iValue = getWidgetValue<int>(static_cast<int>(ID::TRACE_DEPTH));
            valueChanged = ImGui::SliderInt(
                "##traceDepth",
                &iValue,
                1,
                10,
                "%d",
                ImGuiSliderFlags_AlwaysClamp
            );
            if (valueChanged) {
                m_widgetStates[static_cast<int>(ID::TRACE_DEPTH)].value = iValue;
                pushEvent(
                    {
                        LABEL,
                        static_cast<int>(ID::TRACE_DEPTH),
                        iValue,
                        GuiEventType::MODIFY
                    }
                );
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                m_widgetStates[static_cast<int>(ID::TRACE_DEPTH)].value = iValue;
                pushEvent({ LABEL, static_cast<int>(ID::TRACE_DEPTH), iValue });
            }

            // Resolution X
            text = GuiText::get("right_panel.image_node.resolution_x");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(160.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            iValue = getWidgetValue<int>(static_cast<int>(ID::RES_X));
            valueChanged = ImGui::DragInt(
                "##resolutionX",
                &iValue,
                1,
                1,
                5000,
                "%d",
                ImGuiSliderFlags_AlwaysClamp
            );
            if (valueChanged) {
                m_widgetStates[static_cast<int>(ID::RES_X)].value = iValue;
                pushEvent({ LABEL, static_cast<int>(ID::RES_X), iValue, GuiEventType::MODIFY });
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                m_widgetStates[static_cast<int>(ID::RES_X)].value = iValue;
                pushEvent({ LABEL, static_cast<int>(ID::RES_X), iValue });
            }

            // Resolution Y
            text = GuiText::get("right_panel.image_node.resolution_y");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(160.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            iValue = getWidgetValue<int>(static_cast<int>(ID::RES_Y));
            valueChanged = ImGui::DragInt(
                "##resolutionY",
                &iValue,
                1,
                1,
                5000,
                "%d",
                ImGuiSliderFlags_AlwaysClamp
            );
            if (valueChanged) {
                m_widgetStates[static_cast<int>(ID::RES_Y)].value = iValue;
                pushEvent({ LABEL, static_cast<int>(ID::RES_Y), iValue, GuiEventType::MODIFY });
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                m_widgetStates[static_cast<int>(ID::RES_Y)].value = iValue;
                pushEvent({ LABEL, static_cast<int>(ID::RES_Y), iValue });
            }

            if (!m_widgetStates[static_cast<int>(ID::IMAGE_NODE)].enabled)
                ImGui::EndDisabled();

            ImGui::TreePop();
        }

        // Camera node
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        text = GuiText::get("right_panel.camera_node.title");
        if (ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            if (!m_widgetStates[static_cast<int>(ID::CAMERA_NODE)].enabled)
                ImGui::BeginDisabled();

            // Focus distance
            text = GuiText::get("right_panel.camera_node.focus_dist");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(160.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            fValue = getWidgetValue<float>(static_cast<int>(ID::FOCUS_DIST));
            valueChanged = ImGui::SliderFloat(
                "##focusDist",
                &fValue,
                0.1f,
                5.0f,
                "%.2f",
                ImGuiSliderFlags_AlwaysClamp
            );
            if (valueChanged) {
                m_widgetStates[static_cast<int>(ID::FOCUS_DIST)].value = fValue;
                pushEvent(
                    { LABEL, static_cast<int>(ID::FOCUS_DIST), fValue, GuiEventType::MODIFY }
                );
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                m_widgetStates[static_cast<int>(ID::FOCUS_DIST)].value = fValue;
                pushEvent({ LABEL, static_cast<int>(ID::FOCUS_DIST), fValue });
            }

            // F-Stop
            text = GuiText::get("right_panel.camera_node.f_stop");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(160.0f * dpiScale);
            ImGui::SetNextItemWidth(150.0f * dpiScale);
            fValue = getWidgetValue<float>(static_cast<int>(ID::F_STOP));
            valueChanged = ImGui::SliderFloat(
                "##fStop",
                &fValue,
                1.0f,
                32.0f,
                "%.2f",
                ImGuiSliderFlags_AlwaysClamp
            );
            if (valueChanged) {
                m_widgetStates[static_cast<int>(ID::F_STOP)].value = fValue;
                pushEvent({ LABEL, static_cast<int>(ID::F_STOP), fValue, GuiEventType::MODIFY });
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                m_widgetStates[static_cast<int>(ID::F_STOP)].value = fValue;
                pushEvent({ LABEL, static_cast<int>(ID::F_STOP), fValue });
            }

            // Camera Position
            text = GuiText::get("right_panel.camera_node.position");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(160.0f * dpiScale);
            ImGui::SetNextItemWidth(200.0f * dpiScale);
            v3 = getWidgetValue<std::array<float, 3>>(static_cast<int>(ID::CAM_POS));
            if (ImGui::DragFloat3("##cameraPos", v3.data(), 0.01f, 0.0f, 0.0f, "%.2f")) {
                m_widgetStates[static_cast<int>(ID::CAM_POS)].value = v3;
                pushEvent({ LABEL, static_cast<int>(ID::CAM_POS), v3, GuiEventType::MODIFY });
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                m_widgetStates[static_cast<int>(ID::CAM_POS)].value = v3;
                pushEvent({ LABEL, static_cast<int>(ID::CAM_POS), v3 });
            }

            // Camera Rotation
            text = GuiText::get("right_panel.camera_node.rotation");
            ImGui::Text("%s", text.c_str());
            ImGui::SameLine(160.0f * dpiScale);
            ImGui::SetNextItemWidth(200.0f * dpiScale);
            v3 = getWidgetValue<std::array<float, 3>>(static_cast<int>(ID::CAM_ROT));
            if (ImGui::DragFloat3("##cameraRot", v3.data(), 0.01f, 0.0f, 0.0f, "%.2f")) {
                m_widgetStates[static_cast<int>(ID::CAM_ROT)].value = v3;
                pushEvent({ LABEL, static_cast<int>(ID::CAM_ROT), v3, GuiEventType::MODIFY });
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                m_widgetStates[static_cast<int>(ID::CAM_ROT)].value = v3;
                pushEvent({ LABEL, static_cast<int>(ID::CAM_ROT), v3 });
            }

            if (!m_widgetStates[static_cast<int>(ID::CAMERA_NODE)].enabled)
                ImGui::EndDisabled();

            ImGui::TreePop();
        }

        // Scene node
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        text = GuiText::get("right_panel.scene_node.title");
        bool expanded = ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);

        // Context menu
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 4.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushFont(AppUiManager::instance().getNormalIconFont());
        if (ImGui::BeginPopupContextItem()) {
            // Paste
            if (!m_widgetStates[static_cast<int>(ID::MODEL_ITEM_PASTE)].enabled)
                ImGui::BeginDisabled();
            text = GuiText::get("menu_bar.edit_menu.paste");
            if (ImGui::MenuItemEx(text.c_str(), ICON_FK_SCISSORS, "CTRL+V"))
                pushEvent({ LABEL, static_cast<int>(ID::MODEL_ITEM_PASTE) });
            if (!m_widgetStates[static_cast<int>(ID::MODEL_ITEM_PASTE)].enabled)
                ImGui::EndDisabled();

            ImGui::Separator();

            // Select All
            text = GuiText::get("menu_bar.edit_menu.select_all");
            if (ImGui::MenuItem(text.c_str(), "CTRL+A"))
                pushEvent({ LABEL, static_cast<int>(ID::MODEL_ITEM_SELECT_ALL) });

            ImGui::EndPopup();
        }
        ImGui::PopFont();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        if (expanded) {
            if (!m_widgetStates[static_cast<int>(ID::SCENE_NODE)].enabled)
                ImGui::BeginDisabled();

            // Model count
            text = GuiText::get("right_panel.scene_node.models_loaded");
            text = GuiText::formatString(text, { std::to_string(modelListView.size()) });
            ImGui::Text("%s", text.c_str());

            // Add model
            text = std::string(ICON_FK_PLUS) + std::string("  ");
            text += GuiText::get("right_panel.scene_node.add");
            float xPos = ImGui::GetContentRegionMax().x;
            xPos -= 80.0f * dpiScale + ImGui::GetStyle().WindowPadding.x;
            ImGui::SameLine(xPos);
            if (ImGui::Button(text.c_str()))
                pushEvent({ LABEL, static_cast<int>(ID::ADD_MODEL) });

            // Model list
            modelListView.draw();

            if (!m_widgetStates[static_cast<int>(ID::SCENE_NODE)].enabled)
                ImGui::EndDisabled();

            ImGui::TreePop();
        }

        ImGui::End();
    }

    class ModelListItem : public GuiListItem {
    public:
        DbObjHandle hModel = {};
        GuiListView meshListView = GuiListView(this, static_cast<int>(ID::MESH_LIST));

        ModelListItem() {
            /* Default states of the widgets */

            m_widgetStates[static_cast<int>(ID::MODEL_ITEM_HOVERED)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_ITEM_SELECTED)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_ITEM_CONTEXT)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_ITEM_CUT)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_ITEM_COPY)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_ITEM_PASTE)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_ITEM_DELETE)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_ITEM_SELECT_ALL)] = {};

            m_widgetStates[static_cast<int>(ID::MODEL_NAME)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_NAME)].value = "";
            m_widgetStates[static_cast<int>(ID::MODEL_LOCATION)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_LOCATION)].value =
                std::array<float, 3>{ 0.0f, 0.0f, 0.0f };
            m_widgetStates[static_cast<int>(ID::MODEL_ROTATION)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_ROTATION)].value =
                std::array<float, 3>{ 0.0f, 0.0f, 0.0f };
            m_widgetStates[static_cast<int>(ID::MODEL_SCALE)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_SCALE)].value =
                std::array<float, 3>{ 1.0f, 1.0f, 1.0f };
            m_widgetStates[static_cast<int>(ID::MODEL_SCALE_LOCK)] = {};
            m_widgetStates[static_cast<int>(ID::MODEL_SCALE_LOCK)].value = 1;
        }

        void draw() override {
            float dpiScale = AppUiManager::instance().getDpiScale();

            std::string uidStr = getUidString();
            std::string idStr = "##model" + uidStr;
            std::string text = getWidgetValue<std::string>(static_cast<int>(ID::MODEL_NAME));
            std::array<float, 3> v3 = {};

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
            if (ImGui::IsItemHovered())
                pushEvent({ LABEL, static_cast<int>(ID::MODEL_ITEM_HOVERED) });

            // Context menu
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 6.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 4.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushFont(AppUiManager::instance().getNormalIconFont());
            if (ImGui::BeginPopupContextItem()) {
                pushEvent({ LABEL, static_cast<int>(ID::MODEL_ITEM_CONTEXT) });

                // Cut
                text = GuiText::get("menu_bar.edit_menu.cut");
                if (ImGui::MenuItemEx(text.c_str(), ICON_FK_SCISSORS, "CTRL+X"))
                    pushEvent({ LABEL, static_cast<int>(ID::MODEL_ITEM_CUT) });

                // Copy
                text = GuiText::get("menu_bar.edit_menu.copy");
                if (ImGui::MenuItemEx(text.c_str(), ICON_FK_FILES_O, "CTRL+C"))
                    pushEvent({ LABEL, static_cast<int>(ID::MODEL_ITEM_COPY) });

                // Delete
                text = GuiText::get("menu_bar.edit_menu.delete");
                if (ImGui::MenuItemEx(text.c_str(), ICON_FK_TRASH, "DELETE"))
                    pushEvent({ LABEL, static_cast<int>(ID::MODEL_ITEM_DELETE) });

                ImGui::Separator();

                // Select All
                text = GuiText::get("menu_bar.edit_menu.select_all");
                if (ImGui::MenuItem(text.c_str(), "CTRL+A")) {
                    pushEvent({ LABEL, static_cast<int>(ID::MODEL_ITEM_SELECT_ALL) });
                }

                ImGui::EndPopup();
            }
            ImGui::PopFont();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                pushEvent({ LABEL, static_cast<int>(ID::MODEL_ITEM_SELECTED) });
            if (expanded) {
                // Name
                text = GuiText::get("right_panel.model_node.name");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetNextItemWidth(200.0f * dpiScale);
                idStr = "##modelName" + uidStr;
                text = getWidgetValue<std::string>(static_cast<int>(ID::MODEL_NAME));
                if (ImGui::InputText(idStr.c_str(), &text)) {
                    m_widgetStates[static_cast<int>(ID::MODEL_NAME)].value = text;
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MODEL_NAME),
                            text,
                            GuiEventType::MODIFY
                        }
                    );
                }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    m_widgetStates[static_cast<int>(ID::MODEL_NAME)].value = text;
                    pushEvent({ LABEL, static_cast<int>(ID::MODEL_NAME), text });
                }

                // Location
                text = GuiText::get("right_panel.model_node.location");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetNextItemWidth(200.0f * dpiScale);
                idStr = "##modelLoc" + uidStr;
                v3 = getWidgetValue<std::array<float, 3>>(static_cast<int>(ID::MODEL_LOCATION));
                if (ImGui::DragFloat3(idStr.c_str(), v3.data(), 0.01f, 0.0f, 0.0f, "%.2f")) {
                    m_widgetStates[static_cast<int>(ID::MODEL_LOCATION)].value = v3;
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MODEL_LOCATION),
                            v3,
                            GuiEventType::MODIFY
                        }
                    );
                }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    m_widgetStates[static_cast<int>(ID::MODEL_LOCATION)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::MODEL_LOCATION), v3 });
                }

                // Rotation
                text = GuiText::get("right_panel.model_node.rotation");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetNextItemWidth(200.0f * dpiScale);
                idStr = "##modelRot" + uidStr;
                v3 = getWidgetValue<std::array<float, 3>>(static_cast<int>(ID::MODEL_ROTATION));
                if (ImGui::DragFloat3(idStr.c_str(), v3.data(), 1.0f, 0.0f, 0.0f, "%.2f")) {
                    v3[0] = std::fmod(std::fmod(v3[0], 360.0f) + 360.0f, 360.0f);
                    v3[1] = std::fmod(std::fmod(v3[1], 360.0f) + 360.0f, 360.0f);
                    v3[2] = std::fmod(std::fmod(v3[2], 360.0f) + 360.0f, 360.0f);
                    m_widgetStates[static_cast<int>(ID::MODEL_ROTATION)].value = v3;
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MODEL_ROTATION),
                            v3,
                            GuiEventType::MODIFY
                        }
                    );
                }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    m_widgetStates[static_cast<int>(ID::MODEL_ROTATION)].value = v3;
                    pushEvent({ LABEL, static_cast<int>(ID::MODEL_ROTATION), v3 });
                }

                // Scale
                text = GuiText::get("right_panel.model_node.scale");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine();
                ImGui::PushFont(AppUiManager::instance().getNormalIconFont());
                bool scaleLocked =
                    getWidgetValue<int>(static_cast<int>(ID::MODEL_SCALE_LOCK)) == 1;
                if (scaleLocked)
                    ImGui::Text(" " ICON_FK_LOCK);
                else
                    ImGui::Text(" " ICON_FK_UNLOCK);
                ImGui::PopFont();
                if (ImGui::IsItemClicked()) {
                    scaleLocked = !scaleLocked;
                    m_widgetStates[static_cast<int>(ID::MODEL_SCALE_LOCK)].value =
                        scaleLocked ? 1 : 0;
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MODEL_SCALE_LOCK),
                            scaleLocked ? 1 : 0
                        }
                    );
                }
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetNextItemWidth(200.0f * dpiScale);
                idStr = "##modelScale" + uidStr;
                v3 = getWidgetValue<std::array<float, 3>>(static_cast<int>(ID::MODEL_SCALE));
                if (ImGui::DragFloat3(idStr.c_str(), v3.data(), 0.01f, 0.0f, 0.0f, "%.2f")) {
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MODEL_SCALE),
                            v3,
                            GuiEventType::MODIFY
                        }
                    );
                }
                if (ImGui::IsItemDeactivatedAfterEdit())
                    pushEvent({ LABEL, static_cast<int>(ID::MODEL_SCALE), v3 });

                // Mesh list
                meshListView.draw();

                ImGui::TreePop();
            }
        }
    };

    class MeshListItem : public GuiListItem {
    public:
        DbObjHandle hMesh = {};

        ImTextureID normalTexture = 0;
        ImTextureID roughnessTexture = 0;
        ImTextureID temperatureTexture = 0;

        MeshListItem() {
            /* Default states of the widgets */

            m_widgetStates[static_cast<int>(ID::MESH_ITEM_HOVERED)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_NAME)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_NAME)].value = "";

            m_widgetStates[static_cast<int>(ID::MESH_MATERIAL_TYPE)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_MATERIAL_TYPE)].value =
                static_cast<int>(MaterialType::DIFFUSE);
            m_widgetStates[static_cast<int>(ID::MESH_ROUGHNESS)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_ROUGHNESS)].value = 1.0f;
            m_widgetStates[static_cast<int>(ID::MESH_MATERIAL)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_MATERIAL)].value = -1;
            m_widgetStates[static_cast<int>(ID::MESH_TEMPERATURE)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_TEMPERATURE)].value = 0.0f;
            m_widgetStates[static_cast<int>(ID::MESH_IOR)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_IOR)].value = 1.5f;

            m_widgetStates[static_cast<int>(ID::MESH_NORMAL_TEX_LOAD)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_NORMAL_TEX_REMOVE)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_ROUGHNESS_TEX_LOAD)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_ROUGHNESS_TEX_REMOVE)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_TEMPERATURE_TEX_LOAD)] = {};
            m_widgetStates[static_cast<int>(ID::MESH_TEMPERATURE_TEX_REMOVE)] = {};
        }

        void draw() override {
            float dpiScale = AppUiManager::instance().getDpiScale();

            std::string uidStr = getUidString();
            std::string idStr = "##mesh" + uidStr;
            std::string text = getWidgetValue<std::string>(static_cast<int>(ID::MESH_NAME));
            bool valueChanged = false;
            int iValue = 0;
            float fValue = 0.0f;
            std::array<float, 3> v3 = {};

            if (isExpanded())
                ImGui::SetNextItemOpen(true);
            else
                ImGui::SetNextItemOpen(false);

            ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;
            bool expanded = ImGui::TreeNodeEx(idStr.c_str(), treeNodeFlags, "%s", text.c_str());
            setExpanded(expanded);
            if (ImGui::IsItemHovered())
                pushEvent({ LABEL, static_cast<int>(ID::MESH_ITEM_HOVERED) });
            if (expanded) {
                // Name
                text = GuiText::get("right_panel.mesh_node.name");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetNextItemWidth(200.0f * dpiScale);
                idStr = "##meshName" + uidStr;
                text = getWidgetValue<std::string>(static_cast<int>(ID::MESH_NAME));
                if (ImGui::InputText(idStr.c_str(), &text)) {
                    m_widgetStates[static_cast<int>(ID::MESH_NAME)].value = text;
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MESH_NAME),
                            text,
                            GuiEventType::MODIFY
                        }
                    );
                }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    m_widgetStates[static_cast<int>(ID::MESH_NAME)].value = text;
                    pushEvent({ LABEL, static_cast<int>(ID::MESH_NAME), text });
                }

                ImGuiColorEditFlags colorEditFlags =
                    ImGuiColorEditFlags_NoInputs |
                    ImGuiColorEditFlags_NoTooltip;

                // Material Type
                text = GuiText::get("right_panel.mesh_node.material_type");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                idStr = "##meshMaterialType" + uidStr;
                iValue = getWidgetValue<int>(static_cast<int>(ID::MESH_MATERIAL_TYPE));
                text = GuiText::get("right_panel.mesh_node.material_types.diffuse") + '\0';
                text += GuiText::get("right_panel.mesh_node.material_types.specular") + '\0';
                text += GuiText::get("right_panel.mesh_node.material_types.glossy") + '\0';
                text += GuiText::get("right_panel.mesh_node.material_types.translucent") + '\0';
                if (ImGui::Combo(idStr.c_str(), &iValue, text.c_str())) {
                    m_widgetStates[static_cast<int>(ID::MESH_MATERIAL_TYPE)].value = iValue;
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MESH_MATERIAL_TYPE),
                            iValue
                        }
                    );
                }
                MaterialType materialType = static_cast<MaterialType>(iValue);


                if (materialType == MaterialType::GLOSSY) {
                    // Roughness
                    text = GuiText::get("right_panel.mesh_node.roughness");
                    ImGui::Text("%s", text.c_str());
                    ImGui::SameLine(160.0f * dpiScale);
                    ImGui::SetNextItemWidth(150.0f * dpiScale);
                    idStr = "##meshRoughness" + uidStr;
                    fValue = getWidgetValue<float>(static_cast<int>(ID::MESH_ROUGHNESS));
                    valueChanged = ImGui::SliderFloat(
                        idStr.c_str(),
                        &fValue,
                        0.0f,
                        1.0f,
                        "%.2f",
                        ImGuiSliderFlags_AlwaysClamp
                    );
                    if (valueChanged) {
                        m_widgetStates[static_cast<int>(ID::MESH_ROUGHNESS)].value = fValue;
                        pushEvent(
                            {
                                LABEL,
                                static_cast<int>(ID::MESH_ROUGHNESS),
                                fValue,
                                GuiEventType::MODIFY
                            }
                        );
                    }
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        m_widgetStates[static_cast<int>(ID::MESH_ROUGHNESS)].value = fValue;
                        pushEvent(
                            {
                                LABEL,
                                static_cast<int>(ID::MESH_ROUGHNESS),
                                fValue
                            }
                        );
                    }
                }

                // Spectrum material
                text = GuiText::get("right_panel.mesh_node.material");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                idStr = "##meshMaterial" + uidStr;
                iValue = getWidgetValue<int>(static_cast<int>(ID::MESH_MATERIAL));
                GuiView* view = nullptr;
                auto meshListView = getListView();
                if (meshListView != nullptr) {
                    auto modelListItem = meshListView->getParent();
                    if (modelListItem != nullptr) {
                        auto modelListView = modelListItem->getListView();
                        if (modelListView != nullptr)
                            view = modelListView->getView();
                    }
                }
                text = "";
                std::vector<std::string> strList = {};
                if (view != nullptr)
                    strList = view->getWidgetComboItems(static_cast<int>(ID::MESH_MATERIAL));
                for (const auto& itemStr : strList)
                    text += itemStr + '\0';
                if (ImGui::Combo(idStr.c_str(), &iValue, text.c_str())) {
                    m_widgetStates[static_cast<int>(ID::MESH_MATERIAL)].value = iValue;
                    pushEvent({ LABEL, static_cast<int>(ID::MESH_MATERIAL), iValue });
                }

                // Temperature
                text = GuiText::get("right_panel.mesh_node.temperature");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                idStr = "##meshTemperature" + uidStr;
                fValue = getWidgetValue<float>(static_cast<int>(ID::MESH_TEMPERATURE));
                valueChanged = ImGui::DragFloat(
                    idStr.c_str(),
                    &fValue,
                    0.01f,
                    -273.15f,
                    std::numeric_limits<float>::max(),
                    "%.2f",
                    ImGuiSliderFlags_AlwaysClamp
                );
                if (valueChanged) {
                    m_widgetStates[static_cast<int>(ID::MESH_TEMPERATURE)].value = fValue;
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MESH_TEMPERATURE),
                            fValue,
                            GuiEventType::MODIFY
                        }
                    );
                }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    m_widgetStates[static_cast<int>(ID::MESH_TEMPERATURE)].value = fValue;
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MESH_TEMPERATURE),
                            fValue
                        }
                    );
                }

                // IOR
                text = GuiText::get("right_panel.mesh_node.ior");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetNextItemWidth(150.0f * dpiScale);
                idStr = "##meshIOR" + uidStr;
                fValue = getWidgetValue<float>(static_cast<int>(ID::MESH_IOR));
                if (ImGui::InputFloat(idStr.c_str(), &fValue, 0.0f, 0.0f, "%.7f")) {
                    m_widgetStates[static_cast<int>(ID::MESH_IOR)].value = fValue;
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MESH_IOR),
                            fValue,
                            GuiEventType::MODIFY
                        }
                    );
                }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    m_widgetStates[static_cast<int>(ID::MESH_IOR)].value = fValue;
                    pushEvent(
                        {
                            LABEL,
                            static_cast<int>(ID::MESH_IOR),
                            fValue
                        }
                    );
                }

                float height = ImGui::GetTextLineHeightWithSpacing() * 2;
                float textHeight = ImGui::GetTextLineHeight();
                float posX = 0.0f, posY = 0.0f;

                // Normal Texture
                posY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(posY + (height - textHeight) * 0.5f);
                text = GuiText::get("right_panel.mesh_node.normal_texture");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetCursorPosY(posY);
                if (normalTexture != 0)
                    ImGui::Image(normalTexture, ImVec2(height, height));
                else
                    ImGui::Dummy(ImVec2(height, height));
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
                ImGui::PushFont(AppUiManager::instance().getNormalIconFont());
                ImGui::SameLine();
                posX = ImGui::GetCursorPosX();
                ImGui::SetCursorPosY(posY);
                text = GuiText::get("right_panel.mesh_node.load");
                text += "##meshNormalTexLoad" + uidStr;
                if (ImGui::Button(text.c_str(), ImVec2(65.0f * dpiScale, 0.0f))) {
                    pushEvent(
                        { LABEL, static_cast<int>(ID::MESH_NORMAL_TEX_LOAD) }
                    );
                }
                ImGui::SetCursorPosX(posX);
                ImGui::SetCursorPosY(posY + height * 0.5f);
                if (normalTexture == 0)
                    ImGui::BeginDisabled();
                text = GuiText::get("right_panel.mesh_node.remove");
                text += "##meshNormalTexRemove" + uidStr;
                if (ImGui::Button(text.c_str(), ImVec2(65.0f * dpiScale, 0.0f))) {
                    pushEvent(
                        { LABEL, static_cast<int>(ID::MESH_NORMAL_TEX_REMOVE) }
                    );
                }
                if (normalTexture == 0)
                    ImGui::EndDisabled();
                ImGui::PopFont();
                ImGui::PopStyleVar();

                // Roughness Texture
                posY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(posY + (height - textHeight) * 0.5f);
                text = GuiText::get("right_panel.mesh_node.roughness_texture");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetCursorPosY(posY);
                if (roughnessTexture != 0)
                    ImGui::Image(roughnessTexture, ImVec2(height, height));
                else
                    ImGui::Dummy(ImVec2(height, height));
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
                ImGui::PushFont(AppUiManager::instance().getNormalIconFont());
                ImGui::SameLine();
                posX = ImGui::GetCursorPosX();
                ImGui::SetCursorPosY(posY);
                text = GuiText::get("right_panel.mesh_node.load");
                text += "##meshRoughnessTexLoad" + uidStr;
                if (ImGui::Button(text.c_str(), ImVec2(65.0f * dpiScale, 0.0f))) {
                    pushEvent(
                        { LABEL, static_cast<int>(ID::MESH_ROUGHNESS_TEX_LOAD) }
                    );
                }
                ImGui::SetCursorPosX(posX);
                ImGui::SetCursorPosY(posY + height * 0.5f);
                if (roughnessTexture == 0)
                    ImGui::BeginDisabled();
                text = GuiText::get("right_panel.mesh_node.remove");
                text += "##meshRoughnessTexRemove" + uidStr;
                if (ImGui::Button(text.c_str(), ImVec2(65.0f * dpiScale, 0.0f))) {
                    pushEvent(
                        { LABEL, static_cast<int>(ID::MESH_ROUGHNESS_TEX_REMOVE) }
                    );
                }
                if (roughnessTexture == 0)
                    ImGui::EndDisabled();
                ImGui::PopFont();
                ImGui::PopStyleVar();

                // Temperature Texture
                posY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(posY + (height - textHeight) * 0.5f);
                text = GuiText::get("right_panel.mesh_node.temperature_texture");
                ImGui::Text("%s", text.c_str());
                ImGui::SameLine(160.0f * dpiScale);
                ImGui::SetCursorPosY(posY);
                if (temperatureTexture != 0)
                    ImGui::Image(temperatureTexture, ImVec2(height, height));
                else
                    ImGui::Dummy(ImVec2(height, height));
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
                ImGui::PushFont(AppUiManager::instance().getNormalIconFont());
                ImGui::SameLine();
                posX = ImGui::GetCursorPosX();
                ImGui::SetCursorPosY(posY);
                text = GuiText::get("right_panel.mesh_node.load");
                text += "##meshTemperatureTexLoad" + uidStr;
                if (ImGui::Button(text.c_str(), ImVec2(65.0f * dpiScale, 0.0f))) {
                    pushEvent(
                        { LABEL, static_cast<int>(ID::MESH_TEMPERATURE_TEX_LOAD) }
                    );
                }
                ImGui::SetCursorPosX(posX);
                ImGui::SetCursorPosY(posY + height * 0.5f);
                if (temperatureTexture == 0)
                    ImGui::BeginDisabled();
                text = GuiText::get("right_panel.mesh_node.remove");
                text += "##meshTemperatureTexRemove" + uidStr;
                if (ImGui::Button(text.c_str(), ImVec2(65.0f * dpiScale, 0.0f))) {
                    pushEvent(
                        { LABEL, static_cast<int>(ID::MESH_TEMPERATURE_TEX_REMOVE) }
                    );
                }
                if (temperatureTexture == 0)
                    ImGui::EndDisabled();
                ImGui::PopFont();
                ImGui::PopStyleVar();

                ImGui::TreePop();
            }
        }
    };
};
