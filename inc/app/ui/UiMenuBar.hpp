/**
 * @file UiMenuBar.hpp
 * @brief Defines the UI of the main menu bar.
 */

#pragma once

#include "app/AppUiManager.h"

/**
 * @brief The main manu bar.
 */
class UiMenuBar : public GuiView {
public:
    static constexpr const char* LABEL = "menu_bar";

    enum class ID : int {
        FILE_NEW_SCENE,
        FILE_OPEN_SCENE,
        FILE_SAVE,
        FILE_SAVE_AS,
        FILE_LOAD_MODEL,
        FILE_EXPORT,
        FILE_SETTINGS,
        FILE_EXIT,

        EDIT_UNDO,
        EDIT_REDO,
        EDIT_CUT,
        EDIT_COPY,
        EDIT_PASTE,
        EDIT_DELETE,
        EDIT_SELECT_ALL,

        VIEW_PREVIEW_MODE,
        VIEW_PATH_TRACER_OUTPUT,

        RENDER_START,
        RENDER_PAUSE,
        RENDER_STOP,
        RENDER_RESTART,

        HELP_ABOUT,
    };

    UiMenuBar() {
        /* Default states of the widgets */

        m_widgetStates[static_cast<int>(ID::FILE_NEW_SCENE)] = {};
        m_widgetStates[static_cast<int>(ID::FILE_OPEN_SCENE)] = {};
        m_widgetStates[static_cast<int>(ID::FILE_SAVE)] = {};
        m_widgetStates[static_cast<int>(ID::FILE_SAVE_AS)] = {};
        m_widgetStates[static_cast<int>(ID::FILE_LOAD_MODEL)] = {};
        m_widgetStates[static_cast<int>(ID::FILE_EXPORT)] = {};
        m_widgetStates[static_cast<int>(ID::FILE_SETTINGS)] = {};
        m_widgetStates[static_cast<int>(ID::FILE_EXIT)] = {};

        m_widgetStates[static_cast<int>(ID::EDIT_UNDO)] = {};
        m_widgetStates[static_cast<int>(ID::EDIT_REDO)] = {};
        m_widgetStates[static_cast<int>(ID::EDIT_CUT)] = {};
        m_widgetStates[static_cast<int>(ID::EDIT_COPY)] = {};
        m_widgetStates[static_cast<int>(ID::EDIT_PASTE)] = {};
        m_widgetStates[static_cast<int>(ID::EDIT_DELETE)] = {};
        m_widgetStates[static_cast<int>(ID::EDIT_SELECT_ALL)] = {};

        m_widgetStates[static_cast<int>(ID::VIEW_PREVIEW_MODE)] = {};
        m_widgetStates[static_cast<int>(ID::VIEW_PATH_TRACER_OUTPUT)] = {};

        m_widgetStates[static_cast<int>(ID::RENDER_START)] = {};
        m_widgetStates[static_cast<int>(ID::RENDER_PAUSE)] = {};
        m_widgetStates[static_cast<int>(ID::RENDER_PAUSE)].enabled = false;
        m_widgetStates[static_cast<int>(ID::RENDER_STOP)] = {};
        m_widgetStates[static_cast<int>(ID::RENDER_STOP)].enabled = false;
        m_widgetStates[static_cast<int>(ID::RENDER_RESTART)] = {};
        m_widgetStates[static_cast<int>(ID::RENDER_RESTART)].enabled = false;

        m_widgetStates[static_cast<int>(ID::HELP_ABOUT)] = {};
    }

    void draw() override {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14.0f, 4.0f));
        ImGui::PushFont(AppUiManager::instance().getNormalIconFont());

        if (ImGui::BeginMainMenuBar()) {
            std::string text;
            bool enabled = true;
            bool clicked = false;

            // File
            text = GuiText::get("menu_bar.file");
            if (ImGui::BeginMenu(text.c_str())) {
                // New Scene
                text = GuiText::get("menu_bar.file_menu.new_scene");
                enabled = m_widgetStates[static_cast<int>(ID::FILE_NEW_SCENE)].enabled;
                if (ImGui::MenuItem(text.c_str(), "CTRL+N", false, enabled))
                    pushEvent({ LABEL, static_cast<int>(ID::FILE_NEW_SCENE), {} });

                // Open Scene
                text = GuiText::get("menu_bar.file_menu.open_scene");
                enabled = m_widgetStates[static_cast<int>(ID::FILE_OPEN_SCENE)].enabled;
                if (ImGui::MenuItem(text.c_str(), "CTRL+O", false, enabled))
                    pushEvent({ LABEL, static_cast<int>(ID::FILE_OPEN_SCENE), {} });

                ImGui::Separator();

                // Save
                text = GuiText::get("menu_bar.file_menu.save");
                enabled = m_widgetStates[static_cast<int>(ID::FILE_SAVE)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_FLOPPY_O,
                    "CTRL+S",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::FILE_SAVE), {} });

                // Save As
                text = GuiText::get("menu_bar.file_menu.save_as");
                enabled = m_widgetStates[static_cast<int>(ID::FILE_SAVE_AS)].enabled;
                if (ImGui::MenuItem(text.c_str(), "CTRL+SHIFT+S", false, enabled))
                    pushEvent({ LABEL, static_cast<int>(ID::FILE_SAVE_AS), {} });

                ImGui::Separator();

                // Load Model
                text = GuiText::get("menu_bar.file_menu.load_model");
                enabled = m_widgetStates[static_cast<int>(ID::FILE_LOAD_MODEL)].enabled;
                if (ImGui::MenuItem(text.c_str(), "CTRL+L", false, enabled))
                    pushEvent({ LABEL, static_cast<int>(ID::FILE_LOAD_MODEL), {} });

                // Export
                text = GuiText::get("menu_bar.file_menu.export");
                enabled = m_widgetStates[static_cast<int>(ID::FILE_EXPORT)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_SIGN_OUT,
                    "CTRL+E",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::FILE_EXPORT), {} });

                ImGui::Separator();

                // Settings
                text = GuiText::get("menu_bar.file_menu.settings");
                if (ImGui::MenuItemEx(text.c_str(), ICON_FK_COGS))
                    pushEvent({ LABEL, static_cast<int>(ID::FILE_SETTINGS), {} });

                ImGui::Separator();

                // Exit
                text = GuiText::get("menu_bar.file_menu.exit");
                if (ImGui::MenuItem(text.c_str(), "ALT+F4"))
                    pushEvent({ LABEL, static_cast<int>(ID::FILE_EXIT), {} });

                ImGui::EndMenu();
            }

            // Edit
            text = GuiText::get("menu_bar.edit");
            if (ImGui::BeginMenu(text.c_str())) {
                // Undo
                text = GuiText::get("menu_bar.edit_menu.undo");
                enabled = m_widgetStates[static_cast<int>(ID::EDIT_UNDO)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_REPLY,
                    "CTRL+Z",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::EDIT_UNDO), {} });

                // Redo
                text = GuiText::get("menu_bar.edit_menu.redo");
                enabled = m_widgetStates[static_cast<int>(ID::EDIT_REDO)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_SHARE,
                    "CTRL+Y",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::EDIT_REDO), {} });

                ImGui::Separator();

                // Cut
                text = GuiText::get("menu_bar.edit_menu.cut");
                enabled = m_widgetStates[static_cast<int>(ID::EDIT_CUT)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_SCISSORS,
                    "CTRL+X",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::EDIT_CUT), {} });

                // Copy
                text = GuiText::get("menu_bar.edit_menu.copy");
                enabled = m_widgetStates[static_cast<int>(ID::EDIT_COPY)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_FILES_O,
                    "CTRL+C",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::EDIT_COPY), {} });

                // Paste
                text = GuiText::get("menu_bar.edit_menu.paste");
                enabled = m_widgetStates[static_cast<int>(ID::EDIT_PASTE)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_CLIPBOARD,
                    "CTRL+V",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::EDIT_PASTE), {} });

                // Delete
                text = GuiText::get("menu_bar.edit_menu.delete");
                enabled = m_widgetStates[static_cast<int>(ID::EDIT_DELETE)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_TRASH_O,
                    "DELETE",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::EDIT_DELETE), {} });

                ImGui::Separator();

                // Select All
                text = GuiText::get("menu_bar.edit_menu.select_all");
                if (ImGui::MenuItem(text.c_str(), "CTRL+A"))
                    pushEvent({ LABEL, static_cast<int>(ID::EDIT_SELECT_ALL), {} });

                ImGui::EndMenu();
            }

            // View
            text = GuiText::get("menu_bar.view");
            if (ImGui::BeginMenu(text.c_str())) {
                // Preview Mode
                text = GuiText::get("menu_bar.view_menu.preview_mode");
                enabled = m_widgetStates[static_cast<int>(ID::VIEW_PREVIEW_MODE)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_DESKTOP,
                    "SHIFT+1",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::VIEW_PREVIEW_MODE), {} });

                // Path Tracer Output
                text = GuiText::get("menu_bar.view_menu.path_tracer_output");
                enabled = m_widgetStates[static_cast<int>(ID::VIEW_PATH_TRACER_OUTPUT)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_PICTURE_O,
                    "SHIFT+2",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::VIEW_PATH_TRACER_OUTPUT), {} });

                ImGui::EndMenu();
            }

            // Render
            text = GuiText::get("menu_bar.render");
            if (ImGui::BeginMenu(text.c_str())) {
                // Start
                text = GuiText::get("menu_bar.render_menu.start");
                enabled = m_widgetStates[static_cast<int>(ID::RENDER_START)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_PLAY,
                    "F5",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::RENDER_START), {} });

                // Pause
                text = GuiText::get("menu_bar.render_menu.pause");
                enabled = m_widgetStates[static_cast<int>(ID::RENDER_PAUSE)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_PAUSE,
                    "F6",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::RENDER_PAUSE), {} });

                // Stop
                text = GuiText::get("menu_bar.render_menu.stop");
                enabled = m_widgetStates[static_cast<int>(ID::RENDER_STOP)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_STOP,
                    "F7",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::RENDER_STOP), {} });

                // Restart
                text = GuiText::get("menu_bar.render_menu.restart");
                enabled = m_widgetStates[static_cast<int>(ID::RENDER_RESTART)].enabled;
                clicked = ImGui::MenuItemEx(
                    text.c_str(),
                    ICON_FK_UNDO,
                    "F8",
                    false,
                    enabled
                );
                if (clicked)
                    pushEvent({ LABEL, static_cast<int>(ID::RENDER_RESTART), {} });

                ImGui::EndMenu();
            }

            // Help
            text = GuiText::get("menu_bar.help");
            if (ImGui::BeginMenu(text.c_str())) {
                // About
                text = GuiText::get("menu_bar.help_menu.about");
                text = GuiText::formatString(text, { GuiText::get("app_name") });
                if (ImGui::MenuItemEx(text.c_str(), ICON_FK_INFO_CIRCLE))
                    pushEvent({ LABEL, static_cast<int>(ID::HELP_ABOUT), {} });

                ImGui::EndMenu();
            }

            AppUiManager::instance().getUiSizes().menuBarHeight = ImGui::GetWindowHeight();
            AppUiManager::instance().getUiSizes().statusBarHeight = ImGui::GetWindowHeight();
            ImGui::EndMainMenuBar();
        }

        ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
    }
};
