/**
 * @file PathTracerApp.h
 * @brief Header file for the PathTracerApp class.
 */
#pragma once

#include "application.h"

#include "core/Previewer.h"
#include "core/PathTracer.h"
#include "core/PostProcesser.h"

#include "utils/FrameTimer.h"
#include "utils/Stopwatch.h"

#include "ui/UiMenuBar.hpp"
#include "ui/UiToolBar.hpp"
#include "ui/UiMainViewport.hpp"
#include "ui/UiRightPanel.hpp"
#include "ui/UiStatusBar.hpp"
#include "ui/UiSaveDialog.hpp"
#include "ui/UiSettingsWindow.hpp"
#include "ui/UiAboutWindow.hpp"
#include "ui/UiLeftPanel.hpp"

/**
 * @brief PathTracer application.
 */
class PathTracerApp : public BaseApp, public GuiEventListener {
public:
    PathTracerApp(int argc, char** argv);
    ~PathTracerApp() override = default;

    /**
     * @brief Initializes the path tracer application.
     * @return 0 on success, non-zero on failure.
     */
    int init() override;
    /**
     * @brief Runs the path tracer application.
     * @return 0 on success, non-zero on failure.
     */
    int run() override;
    /**
     * @brief Terminates the path tracer application.
     */
    void term() override;

    /**
     * @brief Handles a GUI event.
     * @param event The GUI event to handle.
     */
    void onGuiEvent(const GuiEvent& event) override;

private:
    /**
     * @brief Callback function for window resize events.
     * @param width The new width of the window.
     * @param height The new height of the window.
     */
    void onResizeWindow(int width, int height);
    /**
     * @brief Callback function for drawing events.
     */
    void onDrawWindow();
    /**
     * @brief Callback function called after drawing is finished.
     */
    void onDrawWindowFinished();
    /**
     * @brief Callback function for mouse button events.
     * @param button The mouse button.
     * @param pressed True if the button is pressed, false if released.
     * @param mod Modifier keys pressed during the event.
     */
    void onMouseButton(GuiMouseButton button, bool pressed, GuiFlags<GuiModKey> mod);
    /**
     * @brief Callback function for mouse move events.
     * @param x The x-coordinate of the mouse.
     * @param y The y-coordinate of the mouse.
     */
    void onMouseMove(double x, double y);
    /**
     * @brief Callback function for keyboard action events.
     * @param key The key that was pressed or released.
     * @param pressed True if the key was pressed, false if released.
     * @param mod Modifier keys pressed during the event.
     */
    void onKeyboardAction(GuiKey key, bool pressed, GuiFlags<GuiModKey> mod);

    /**
     * @brief Callback function for rendering the path tracer context.
     */
    void onPathTracerRender();

    /**
     * @brief Callback function for file drop events.
     * @param paths Vector of file paths that were dropped.
     */
    void onFileDrop(const std::vector<std::string>& paths);

    /**
     * @brief Callback function for window close events.
     * @return True to allow the window to close, false to prevent it.
     */
    bool onCloseWindow();

private:
    /**
     * @brief Initializes the main application window.
     * @return 0 on success, non-zero on failure.
     */
    int initWindow();
    /**
     * @brief Synchronizes dirty objects with the database.
     * @param hObjects Set of object handles to synchronize.
     */
    void syncDirtyObjects(const std::unordered_set<DbObjHandle>& hObjects);
    /**
     * @brief Updates the previewer frame image.
     * @param image The new frame image to display.
     */
    void updatePreviewerFrameImage(GfxImage image);
    /**
     * @brief Updates the left panel UI with the given objects.
     * @param hObjects Vector of object handles to update in the UI.
     */
    void updateUiLeftPanel(const std::vector<DbObjHandle>& hObjects);
    /**
     * @brief Updates the left panel UI for wave objects.
     */
    void updateUiLeftPanelWaves();
    /**
     * @brief Update the spectrum material combo boxes in the left and right panel
     *        based on the current spectrum materials in the scene.
     */
    void updateUiSpectrumMaterialCombos();
    /**
     * @brief Updates the UI list item for a material.
     * @param hMaterial Handle to the material object.
     */
    void updateUiMaterialListItem(const DbObjHandle& hMaterial);
    /**
     * @brief Updates the right panel UI with the given objects.
     * @param hObjects Vector of object handles to update in the UI.
     */
    void updateUiRightPanel(const std::vector<DbObjHandle>& hObjects);
    /**
     * @brief Updates the UI list item for a model.
     * @param hModel Handle to the model object.
     */
    void updateUiModelListItem(const DbObjHandle& hModel);
    /**
     * @brief Updates the UI list item for a mesh.
     * @param hObj Handle to the mesh or its material object.
     */
    void updateUiMeshListItem(const DbObjHandle& hObj);
    /**
     * @brief Updates the status bar UI.
     */
    void updateUiStatusBar();

    /**
     * @brief Selects a model in the application.
     * @param hModel Handle to the model object to select.
     */
    void selectModel(const DbObjHandle& hModel);

    /**
     * @brief Opens a dialog to ask the user to save the current scene.
     */
    void saveFileDialog();
    /**
     * @brief Loads a new scene.
     * @param filename The filename of the scene to load. Pass empty to create a new scene.
     * @return 0 on success, non-zero on failure.
     */
    int loadNewScene(const std::string& filename = {});

    /**
     * @brief Creates a new scene.
     */
    void newScene();
    /**
     * @brief Opens an existing scene from a file.
     */
    void openScene();
    /**
     * @brief Saves the current scene to the existing file.
     */
    void saveScene() const;
    /**
     * @brief Saves the current scene to a new file.
     */
    void saveAsScene() const;

    /**
     * @brief Loads a model from a file.
     */
    void loadModel();
    /**
     * @brief Exports the rendered image to a file.
     */
    void exportImage() const;

    /**
     * @brief Undoes the last action.
     */
    void undo();
    /**
     * @brief Redoes the last undone action.
     */
    void redo();

    /**
     * @brief Cuts the selected models in the application.
     */
    void cutSeletedModels();
    /**
     * @brief Copies the selected models in the application.
     */
    void copySeletedModels();
    /**
     * @brief Pastes copied models into the application.
     */
    void pasteCopiedModels();
    /**
     * @brief Deletes the selected models from the application.
     */
    void deleteSeletedModels();
    /**
     * @brief Selects all models in the application.
     */
    void selectAllModels();

    /**
     * @brief Starts the rendering process.
     */
    void startRendering();
    /**
     * @brief Pauses the rendering process.
     */
    void pauseRendering();
    /**
     * @brief Stops the rendering process.
     */
    void stopRendering();
    /**
     * @brief Restarts the rendering process.
     */
    void restartRendering();

    /**
     * @brief Handles a menu bar event.
     * @param event The GUI event to handle.
     */
    void handleMenuBarEvent(const GuiEvent& event);
    /**
     * @brief Handles a tool bar event.
     * @param event The GUI event to handle.
     */
    void handleToolBarEvent(const GuiEvent& event);
    /**
     * @brief Handles a left panel event.
     * @param event The GUI event to handle.
     */
    void handleLeftPanelEvent(const GuiEvent& event);
    /**
     * @brief Handles a material list event.
     * @param event The GUI event to handle.
     */
    void handleMaterialListEvent(const GuiEvent& event);
    /**
     * @brief Handles a right panel event.
     * @param event The GUI event to handle.
     */
    void handleRightPanelEvent(const GuiEvent& event);
    /**
     * @brief Handles a model list event.
     * @param event The GUI event to handle.
     */
    void handleModelListEvent(const GuiEvent& event);
    /**
     * @brief Handles a mesh list event.
     * @param event The GUI event to handle.
     * @param hModel Handle to the model object associated with the mesh.
     */
    void handleMeshListEvent(const GuiEvent& event, const DbObjHandle& hModel);
    /**
     * @brief Handles a mesh list texture event.
     * @param event The GUI event to handle.
     * @param hMaterial Handle to the material object associated with the mesh.
     */
    void handleMeshListTextureEvent(const GuiEvent& event, const DbObjHandle& hMaterial);
    /**
     * @brief Handles a settings window event.
     * @param event The GUI event to handle.
     */
    void handleSettingsWindowEvent(const GuiEvent& event);
    /**
     * @brief Resets the application configuration to default settings.
     */
    void resetDefaultConfig();
    /**
     * @brief Loads a texture into a mesh material.
     * @tparam Fn Function type for setting the material texture path.
     * @param hMaterial Handle to the material object.
     * @param textureFlag The texture flag to load.
     * @param setMaterialFn Function to set the material texture path.
     */
    template<typename Fn>
    void loadMeshTexture(
        const DbObjHandle& hMaterial,
        PtMaterial::MaterialFlag textureFlag,
        Fn&& setMaterialFn
    ) {
        std::string texturePath = loadImage();
        if (texturePath.empty())
            return;
        auto materialFlags = PtMaterial::getFlags(hMaterial);
        materialFlags.set(textureFlag);
        DbUtils::TxnGuard txnGuard(AppDataManager::instance().getDB());
        if (PtMaterial::setFlags(hMaterial, materialFlags) != DB::Result::SUCCESS)
            return;
        if (std::forward<Fn>(setMaterialFn)(hMaterial, texturePath) != DB::Result::SUCCESS)
            return;
        txnGuard.commit();
        updateUiMeshListItem(hMaterial);
        m_previewer->updateObjects({ hMaterial });
    };
    /**
     * @brief Removes a texture from a mesh material.
     * @tparam Fn Function type for setting the material texture path.
     * @param hMaterial Handle to the material object.
     * @param textureFlag The texture flag to remove.
     * @param setMaterialFn Function to set the material texture path.
     */
    template<typename Fn>
    void removeMeshTexture(
        const DbObjHandle& hMaterial,
        PtMaterial::MaterialFlag textureFlag,
        Fn&& setMaterialFn
    ) {
        auto materialFlags = PtMaterial::getFlags(hMaterial);
        materialFlags.unset(textureFlag);
        DbUtils::TxnGuard txnGuard(AppDataManager::instance().getDB());
        if (PtMaterial::setFlags(hMaterial, materialFlags) != DB::Result::SUCCESS)
            return;
        if (std::forward<Fn>(setMaterialFn)(hMaterial, "") != DB::Result::SUCCESS)
            return;
        txnGuard.commit();
        updateUiMeshListItem(hMaterial);
        m_previewer->updateObjects({ hMaterial });
    };

    /**
     * @brief Enumeration for display modes.
     */
    enum class DisplayMode : int {
        PREVIEW_MODE = 0,
        PATH_TRACER_OUTPUT = 1,
    };
    /**
     * @brief Sets the display mode of the application.
     * @param displayMode The display mode to set.
     */
    void setDisplayMode(DisplayMode displayMode);

    /**
     * @brief Loads a model from a file.
     * @param filename The filename of the model to load.
     * @return 0 on success, non-zero on failure.
     */
    int loadModelUtil(const std::string& filename);
    /**
     * @brief Loads spectrum waves from a text file.
     * @param filename The filename of the text file containing spectrum wave data.
     * @return 0 on success, non-zero on failure.
     */
    int loadSpectrumWavesFromTXT(const std::string& filename);
    /**
     * @brief Loads spectrum materials from a text file.
     * @param filename The filename of the text file containing spectrum material data.
     * @return 0 on success, non-zero on failure.
     */
    int loadSpectrumMaterialsFromTXT(const std::string& filename);

    /**
     * @brief Loads an image from a file picked by a file dialog.
     * @return The file path of the loaded image.
     */
    std::string loadImage();

private:
    std::shared_ptr<GuiWindow> m_window = nullptr; // The main GUI window
    std::shared_ptr<GuiWindow> m_pathTracerCtx = nullptr; // The path tracer context window

    FrameTimer m_frameTimer; // Frame timer

    DisplayMode m_displayMode = DisplayMode::PREVIEW_MODE; // Current display mode

    std::unique_ptr<Previewer> m_previewer = nullptr; // The previewer instance
    std::unique_ptr<PathTracer> m_pathTracer = nullptr; // The path tracer instance
    std::unique_ptr<PostProcesser> m_postProcesser = nullptr; // The post-processor instance

    std::atomic<bool> m_renderFinished{ false }; // Flag indicating if rendering has finished

    bool m_previewerCamInControl = false; // Flag indicating if the previewer camera is in control

    std::shared_ptr<UiMenuBar> m_menuBar = nullptr; // The menu bar view
    std::shared_ptr<UiToolBar> m_toolBar = nullptr; // The tool bar view
    std::shared_ptr<UiMainViewport> m_mainViewport = nullptr; // The main viewport view
    std::shared_ptr<UiRightPanel> m_rightPanel = nullptr; // The right panel view
    std::shared_ptr<UiStatusBar> m_statusBar = nullptr; // The status bar view
    std::shared_ptr<UiSaveDialog> m_saveDialog = nullptr; // The save dialog view
    std::shared_ptr<UiSettingsWindow> m_settingsWindow = nullptr; // The settings window view
    std::shared_ptr<UiAboutWindow> m_aboutWindow = nullptr; // The about window view
    std::shared_ptr<UiLeftPanel> m_leftPanel = nullptr; // The left panel view

    // Lookup map for model list items in the UI
    std::unordered_map<DbObjHandle, UiRightPanel::ModelListItem*> m_modelUiListItemLookUp = {};
    // Lookup map for mesh list items in the UI
    std::unordered_map<DbObjHandle, UiRightPanel::MeshListItem*> m_meshUiListItemLookUp = {};
    // Lookup map for material list items in the UI
    std::unordered_map<DbObjHandle, UiLeftPanel::MaterialListItem*> m_materialListItemLookUp = {};

    std::unordered_set<DbObjHandle> m_selectedModels = {}; // List of selected model handles
    DbObjHandle m_focusedModel; // Handle of the focused model
    DbObjHandle m_hoveredModel; // Handle of the hovered model
    /**
     * @brief Enumeration for selection modes.
     */
    enum class SelectionMode {
        SINGLE = 0,
        MULTI = 1 << 0,
        RANGE = 1 << 1,
    };
    Flags<SelectionMode> m_selectionMode = SelectionMode::SINGLE; // Current selection mode
    bool m_viewportHovered = false; // Flag indicating if the viewport is hovered

    /**
     * @brief Enumeration for render states.
     */
    enum class RenderState {
        IDLE,
        RENDERING,
        PAUSED,
        PENDING_PAUSE,
        PENDING_STOP,
        PENDING_RESTART,
    };
    RenderState m_currentRenderState = RenderState::IDLE; // Current render state
    int m_targetSample = 0; // Target number of samples for rendering
    Stopwatch m_renderStopwatch; // Stopwatch for measuring render time
    int m_nTriangles = 0; // Number of triangles in the scene

    GfxImage m_appIcon = nullptr; // Application icon image
};
