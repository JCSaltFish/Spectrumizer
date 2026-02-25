/**
 * @file PathTracerApp.cpp
 * @brief Implementation of the PathTracerApp class.
 */

#include "app/PathTracerApp.h"

#include <tinyfiledialogs.h>

#include "app/AppTextureManager.h"
#include "app/AppClipboard.h"
#include "res/LangStrings.h"
#include "utils/Logger.hpp"
#include "utils/Mesh.h"
#include "utils/Image.h"
#include "utils/ScopeGuard.hpp"

PathTracerApp::PathTracerApp(int argc, char** argv) :
    BaseApp(argc, argv) {}

int PathTracerApp::init() {
    // Init global config
    GuiConfig::setAppName(Application::APP_NAME);
    GuiConfig::setGraphicsBackend(GfxBackend::Vulkan);
    std::string langCfgStr = AppConfig::instance().getConfig("general_lang");
    LangStrings::Lang language = LangStrings::Lang::EN_US;
    if (!langCfgStr.empty())
        language = static_cast<LangStrings::Lang>(std::stoi(langCfgStr));
    GuiText::load(LangStrings::get(language));

    if (initWindow())
        return 1;

    GfxRenderer renderer = m_window->getRenderer();
    // Init texture manager
    AppTextureManager::instance().init(renderer);

    // Init previewer
    m_previewer = std::make_unique<Previewer>(renderer);
    DbObjHandle hScene = AppDataManager::instance().getDB()->getRootObject();
    int resX = 0, resY = 0;
    PtScene::getResolution(hScene, resX, resY);
    std::string samplesStr = AppConfig::instance().getConfig("previewer_samples");
    int samples = samplesStr.empty() ? 1 : std::stoi(samplesStr);
    m_previewer->init(resX, resY, samples);
    std::string bgColorStr = AppConfig::instance().getConfig("previewer_bg_color");
    Math::Vec3 bgColor =
        bgColorStr.empty() ? Math::Vec3(0.0f) : AppConfigUitls::StringToVec3(bgColorStr);
    m_previewer->setBackgroundColor(bgColor);
    std::string hiliteColorHoveredStr =
        AppConfig::instance().getConfig("previewer_hilite_color_hovered");
    Math::Vec3 hiliteColorHovered =
        hiliteColorHoveredStr.empty() ? Math::Vec3(0.9f, 0.9f, 0.1f) :
        AppConfigUitls::StringToVec3(hiliteColorHoveredStr);
    std::string hiliteColorPickedStr =
        AppConfig::instance().getConfig("previewer_hilite_color_picked");
    Math::Vec3 hiliteColorPicked =
        hiliteColorPickedStr.empty() ? Math::Vec3(0.1f, 0.7f, 0.9f) :
        AppConfigUitls::StringToVec3(hiliteColorPickedStr);
    m_previewer->setHighlightColors(hiliteColorHovered, hiliteColorPicked);
    std::string camMoveSpeedStr = AppConfig::instance().getConfig("previewer_cam_move_speed");
    int camMoveSpeed = camMoveSpeedStr.empty() ? 3 : std::stoi(camMoveSpeedStr);
    m_previewer->setCameraMoveSpeed(static_cast<float>(camMoveSpeed) * 0.1f);
    m_previewer->loadScene(AppDataManager::instance().getDB()->getRootObject());
    GfxImage previewFrame = m_previewer->getFrameImage();
    m_mainViewport->frameTexture =
        AppUiManager::instance().getImGuiTexture(renderer, previewFrame);

    // Init path tracer
    m_pathTracerCtx = std::make_unique<GuiWindow>("PathTracerContext", 0, 0);
    m_pathTracerCtx->setOnDrawCb([this] { onPathTracerRender(); });
    m_pathTracer = std::make_unique<PathTracer>(m_pathTracerCtx->getRenderer());
    m_pathTracer->init();

    // Init post processer
    m_postProcesser = std::make_unique<PostProcesser>(renderer);
    m_postProcesser->init();

    // Init settings window with saved config
    auto langConfig = UiSettingsWindow::Language::EN_US;
    switch (language) {
    case LangStrings::Lang::EN_US:
        langConfig = UiSettingsWindow::Language::EN_US;
        break;
    case LangStrings::Lang::ZH_CN:
        langConfig = UiSettingsWindow::Language::ZH_CN;
        break;
    default:
        break;
    }
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::LANGUAGE),
        static_cast<int>(langConfig)
    );
    auto msaaConfig = UiSettingsWindow::MsaaLevel::MSAA_1X;
    switch (samples) {
    case 1:
        msaaConfig = UiSettingsWindow::MsaaLevel::MSAA_1X;
        break;
    case 2:
        msaaConfig = UiSettingsWindow::MsaaLevel::MSAA_2X;
        break;
    case 4:
        msaaConfig = UiSettingsWindow::MsaaLevel::MSAA_4X;
        break;
    case 8:
        msaaConfig = UiSettingsWindow::MsaaLevel::MSAA_8X;
        break;
    }
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::MSAA),
        static_cast<int>(msaaConfig)
    );
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::CAM_NAV_SPEED),
        static_cast<int>(camMoveSpeed)
    );
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::BACKGROUND_COLOR),
        AppUiUtils::vec3ToArray(bgColor)
    );
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::HOVERED_COLOR),
        AppUiUtils::vec3ToArray(hiliteColorHovered)
    );
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::PICKED_COLOR),
        AppUiUtils::vec3ToArray(hiliteColorPicked)
    );

    // Load scene from command line argument
    if (m_argc > 1) {
        std::string filename = m_argc > 1 ? m_argv[1] : "";
        if (filename.empty())
            return 0;
        loadNewScene(filename);
    }

    return 0;
}

int PathTracerApp::run() {
    m_window->show();

    std::thread pathTracerThread(
        [this] {
            while (!m_pathTracerCtx->shouldClose()) {
                if (m_pathTracer->isRendering()) {
                    m_pathTracerCtx->drawFrame();
                    if (m_targetSample > 0) {
                        if (m_pathTracer->getCurrentSample() >= m_targetSample)
                            stopRendering();
                    }
                    m_renderFinished.store(true, std::memory_order_release);
                    m_pathTracer->markDisplayImageReady();
                }
            }
        }
    );

    while (!m_window->shouldClose()) {
        if (m_window->drawFrame())
            return 1;
    }

    m_pathTracerCtx->term();
    pathTracerThread.join();

    return 0;
}

void PathTracerApp::term() {
    GfxRenderer renderer = m_window->getRenderer();

    renderer->waitDeviceIdle();

    AppUiManager::instance().destroyImGuiTextures(renderer);

    if (m_appIcon) {
        renderer->destroyImage(m_appIcon);
        m_appIcon = nullptr;
    }

    m_previewer->term();
    m_previewer.reset();

    m_pathTracer->term();
    m_pathTracer.reset();

    m_postProcesser->term();
    m_postProcesser.reset();

    AppUiUtils::termForImGui(renderer);

    AppTextureManager::instance().term();

    m_pathTracerCtx.reset();

    m_window.reset();
}

void PathTracerApp::onGuiEvent(const GuiEvent& event) {
    if (event.viewLabel == UiMenuBar::LABEL)
        handleMenuBarEvent(event);
    else if (event.viewLabel == UiToolBar::LABEL)
        handleToolBarEvent(event);
    else if (event.viewLabel == UiMainViewport::LABEL) {
        // Handle mouse coordinate event
        if (m_displayMode != DisplayMode::PREVIEW_MODE)
            return;
        Math::Vec3 mouseCoord =
            AppUiUtils::arrayToVec3(getEventValue<std::array<float, 3>>(event));
        m_viewportHovered = mouseCoord.z > 0.0f && !m_previewerCamInControl;
        if (!m_viewportHovered)
            return;
        int pixelCoordX = static_cast<int>(mouseCoord.x - 0.5f);
        int pixelCoordY = static_cast<int>(mouseCoord.y - 0.5f);
        DbObjHandle hMesh = m_previewer->getMeshAtPixel(pixelCoordX, pixelCoordY);
        if (hMesh.isValid()) {
            m_previewer->hightlightObject(hMesh, Previewer::HightlightState::HOVERED);
            m_hoveredModel = PtMesh::getModel(hMesh);
        }
    } else if (event.viewLabel == UiRightPanel::LABEL)
        handleRightPanelEvent(event);
    else if (event.viewLabel == UiLeftPanel::LABEL)
        handleLeftPanelEvent(event);
    else if (event.viewLabel == UiSettingsWindow::LABEL)
        handleSettingsWindowEvent(event);
}

void PathTracerApp::onResizeWindow(int width, int height) {
    AppUiManager::instance().getUiSizes().windowWidth = width;
    AppUiManager::instance().getUiSizes().windowHeight = height;
}

void PathTracerApp::onDrawWindow() {
    m_frameTimer.beginFrame();

    m_pathTracer->syncDisplayImage();

    // Render main viewport
    m_viewportHovered = false;
    m_hoveredModel = DbObjHandle();
    m_mainViewport->frameWidth = m_previewer->getFrameImage()->getWidth();
    m_mainViewport->frameHeight = m_previewer->getFrameImage()->getHeight();
    if (m_displayMode == DisplayMode::PREVIEW_MODE) {
        if (m_previewerCamInControl) {
            float frameDuration = static_cast<float>(m_frameTimer.getFrameInterval());
            m_previewer->getCameraController().processMovement(frameDuration);
            m_rightPanel->setWidgetValue(
                static_cast<int>(UiRightPanel::ID::CAM_POS),
                AppUiUtils::vec3ToArray(m_previewer->getCameraPosition())
            );
        }
        for (const auto& hModel : m_selectedModels)
            m_previewer->hightlightObject(hModel, Previewer::HightlightState::PICKED);
        m_previewer->renderFrame();
    } else {
        m_postProcesser->setInputImage(m_pathTracer->getCurrentDisplayImage());
        m_postProcesser->renderFrame();
    }

    // Update UI elements' states
    auto db = AppDataManager::instance().getDB();
    m_menuBar->enableWidget(
        static_cast<int>(UiMenuBar::ID::EDIT_UNDO),
        db->canUndo() && m_currentRenderState == RenderState::IDLE
    );
    m_menuBar->enableWidget(
        static_cast<int>(UiMenuBar::ID::EDIT_REDO),
        db->canRedo() && m_currentRenderState == RenderState::IDLE
    );
    m_menuBar->enableWidget(
        static_cast<int>(UiMenuBar::ID::EDIT_CUT),
        !m_selectedModels.empty() && m_currentRenderState == RenderState::IDLE
    );
    m_menuBar->enableWidget(
        static_cast<int>(UiMenuBar::ID::EDIT_COPY),
        !m_selectedModels.empty()
    );
    m_menuBar->enableWidget(
        static_cast<int>(UiMenuBar::ID::EDIT_PASTE),
        AppClipboard::instance().hasData() && m_currentRenderState == RenderState::IDLE
    );
    m_menuBar->enableWidget(
        static_cast<int>(UiMenuBar::ID::EDIT_DELETE),
        !m_selectedModels.empty() && m_currentRenderState == RenderState::IDLE
    );
    m_rightPanel->enableWidget(
        static_cast<int>(UiRightPanel::ID::MODEL_ITEM_PASTE),
        AppClipboard::instance().hasData() && m_currentRenderState == RenderState::IDLE
    );
    updateUiStatusBar();

    if (m_renderFinished.exchange(false, std::memory_order_acquire))
        m_pathTracer->renderFinishCallback();

    AppUiUtils::newFrameForImGui(m_window->getRenderer());
}

void PathTracerApp::onDrawWindowFinished() {
    AppUiUtils::renderForImGui(m_window->getRenderer());
    m_frameTimer.endFrame();
}

void PathTracerApp::onMouseButton(GuiMouseButton button, bool pressed, GuiFlags<GuiModKey> mod) {
    if (button == GuiMouseButton::RIGHT) {
        if (m_displayMode != DisplayMode::PREVIEW_MODE)
            return;
        if (m_currentRenderState != RenderState::IDLE)
            return;
        auto db = AppDataManager::instance().getDB();
        DbObjHandle hScene = db->getRootObject();
        if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME) {
            Logger() << "Invalid scene handle";
            return;
        }
        if (pressed && m_viewportHovered) {
            m_previewerCamInControl = true;
            double mouseX = 0.0, mouseY = 0.0;
            m_window->getMousePos(mouseX, mouseY);
            Math::Vec2 mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));
            Math::Vec3 camRot = PtScene::getCamera(hScene).rotation;
            m_previewer->getCameraController().beginRotation(camRot, mousePos);
        } else if (m_previewerCamInControl) {
            m_previewerCamInControl = false;
            m_previewer->getCameraController().clearMovement();
            auto camera = PtScene::getCamera(hScene);
            camera.position = m_previewer->getCameraPosition();
            camera.rotation = m_previewer->getCameraRotation();
            if (DbUtils::txnFn(db, PtScene::setCamera, hScene, camera) != DB::Result::SUCCESS)
                Logger() << "Failed to update scene camera";
        }
    } else if (button == GuiMouseButton::LEFT && pressed) {
        if (m_displayMode != DisplayMode::PREVIEW_MODE || !m_viewportHovered)
            return;
        if (m_hoveredModel.isValid())
            selectModel(m_hoveredModel);
        else {
            m_rightPanel->modelListView.clearSelection();
            m_selectedModels.clear();
        }
    }
}

void PathTracerApp::onMouseMove(double x, double y) {
    if (m_displayMode == DisplayMode::PREVIEW_MODE && m_previewerCamInControl) {
        Math::Vec2 mousePos(static_cast<float>(x), static_cast<float>(y));
        m_previewer->getCameraController().rotate(mousePos);
        m_rightPanel->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::CAM_ROT),
            AppUiUtils::vec3ToArray(m_previewer->getCameraRotation())
        );
    }
}

void PathTracerApp::onKeyboardAction(GuiKey key, bool pressed, GuiFlags<GuiModKey> mod) {
    if (AppUiManager::instance().isInTextEditing())
        return;

    // Previewer camera controls
    if (m_displayMode == DisplayMode::PREVIEW_MODE && m_previewerCamInControl) {
        switch (key) {
        case GuiKey::W:
            m_previewer->getCameraController().moveForward(pressed);
            break;
        case GuiKey::S:
            m_previewer->getCameraController().moveBackward(pressed);
            break;
        case GuiKey::A:
            m_previewer->getCameraController().moveLeft(pressed);
            break;
        case GuiKey::D:
            m_previewer->getCameraController().moveRight(pressed);
            break;
        default:
            break;
        }
    }

    // Selection mode controls
    else if (m_displayMode == DisplayMode::PREVIEW_MODE) {
        switch (key) {
        case GuiKey::LEFT_CONTROL:
        case GuiKey::RIGHT_CONTROL:
        {
            if (pressed)
                m_selectionMode.set(SelectionMode::MULTI);
            else
                m_selectionMode.unset(SelectionMode::MULTI);
            break;
        }
        case GuiKey::LEFT_SHIFT:
        case GuiKey::RIGHT_SHIFT:
        {
            if (pressed)
                m_selectionMode.set(SelectionMode::RANGE);
            else
                m_selectionMode.unset(SelectionMode::RANGE);
            break;
        }
        default:
            break;
        }
    }

    // Global shortcuts
    if (pressed) {
        if (mod.check(GuiModKey::CONTROL)) {
            switch (key) {
            case GuiKey::N:
                newScene();
                break;
            case GuiKey::O:
                openScene();
                break;
            case GuiKey::S:
                if (mod.check(GuiModKey::SHIFT))
                    saveAsScene();
                else
                    saveScene();
                break;
            case GuiKey::L:
                loadModel();
                break;
            case GuiKey::E:
                exportImage();
                break;

            case GuiKey::Z:
                undo();
                break;
            case GuiKey::Y:
                redo();
                break;
            case GuiKey::X:
                cutSeletedModels();
                break;
            case GuiKey::C:
                copySeletedModels();
                break;
            case GuiKey::V:
                pasteCopiedModels();
                break;
            case GuiKey::A:
                selectAllModels();
                break;
            default:
                break;
            }
        } else if (key == GuiKey::DELETE)
            deleteSeletedModels();
        else if (mod.check(GuiModKey::SHIFT)) {
            if (key == GuiKey::ONE)
                setDisplayMode(DisplayMode::PREVIEW_MODE);
            else if (key == GuiKey::TWO)
                setDisplayMode(DisplayMode::PATH_TRACER_OUTPUT);
        } else {
            switch (key) {
            case GuiKey::F5:
                startRendering();
                break;
            case GuiKey::F6:
                pauseRendering();
                break;
            case GuiKey::F7:
                stopRendering();
                break;
            case GuiKey::F8:
                restartRendering();
                break;
            default:
                break;
            }
        }
    }
}

void PathTracerApp::onPathTracerRender() {
    m_pathTracer->renderFrame();
}

void PathTracerApp::onFileDrop(const std::vector<std::string>& paths) {
    std::string scenePath{}; // path to the first .sps file
    for (const auto& path : paths) {
        std::string ext = std::filesystem::path(path).extension().string();
        if (ext == ".sps" && scenePath.empty())
            scenePath = path;
        else if (ext == ".obj")
            loadModelUtil(path);
    }
    if (!scenePath.empty())
        loadNewScene(scenePath);
}

bool PathTracerApp::onCloseWindow() {
    if (AppDataManager::instance().getDB()->isModified()) {
        saveFileDialog();
        m_saveDialog->setEventCallback(
            [this](const GuiEvent& event) {
                switch (static_cast<UiSaveDialog::ID>(event.widgetID)) {
                case UiSaveDialog::ID::YES:
                {
                    saveScene();
                    m_window->term();
                    break;
                }
                case UiSaveDialog::ID::NO:
                {
                    m_window->term();
                    break;
                }
                default:
                    break;
                }
            }
        );
        return false;
    }

    return true;
}

int PathTracerApp::initWindow() {
    int windowWidth = AppUiManager::instance().getUiSizes().windowWidth;
    int windowHeight = AppUiManager::instance().getUiSizes().windowHeight;
    m_window =
        std::make_unique<GuiWindow>(GuiText::get("app_name"), windowWidth, windowHeight);
    std::vector<GuiIcon> appIcons;
    if (AppUiManager::instance().getAppIcons(appIcons) == 0)
        m_window->setWindowIcon(appIcons);
    m_window->setOnResizeCb([this](int width, int height) { onResizeWindow(width, height); });
    m_window->setOnDrawCb([this] { onDrawWindow(); });
    m_window->setOnDrawFinishedCb([this] { onDrawWindowFinished(); });
    m_window->setOnMouseButtonCb(
        [this](GuiMouseButton button, bool pressed, GuiFlags<GuiModKey> mod) {
            onMouseButton(button, pressed, mod);
        }
    );
    m_window->setOnMouseMoveCb([this](double x, double y) { onMouseMove(x, y); });
    m_window->setOnKeyboardActionCb(
        [this](GuiKey key, bool pressed, GuiFlags<GuiModKey> mod) {
            onKeyboardAction(key, pressed, mod);
        }
    );
    m_window->setOnDropCb(
        [this](const std::vector<std::string>& paths) {
            onFileDrop(paths);
        }
    );
    m_window->setOnCloseCb([this] { return onCloseWindow(); });

    GfxRenderer renderer = m_window->getRenderer();

    // Init UI
    if (AppUiUtils::initForImGui(renderer, m_window)) {
        Logger() << "Failed to init ImGui";
        return 1;
    }
    AppUiManager::instance().initUI(m_window->getContentScale());

    // Bind views and listeners
    m_menuBar = std::make_shared<UiMenuBar>();
    m_menuBar->addListener(this);
    m_window->addView(m_menuBar);
    m_toolBar = std::make_shared<UiToolBar>();
    m_toolBar->addListener(this);
    m_window->addView(m_toolBar);
    m_mainViewport = std::make_shared<UiMainViewport>();
    m_mainViewport->addListener(this);
    m_window->addView(m_mainViewport);
    m_rightPanel = std::make_shared<UiRightPanel>();
    m_rightPanel->addListener(this);
    m_window->addView(m_rightPanel);
    m_statusBar = std::make_shared<UiStatusBar>();
    m_statusBar->addListener(this);
    m_window->addView(m_statusBar);
    m_saveDialog = std::make_shared<UiSaveDialog>();
    m_saveDialog->addListener(this);
    m_window->addView(m_saveDialog);
    m_settingsWindow = std::make_shared<UiSettingsWindow>();
    m_settingsWindow->addListener(this);
    m_window->addView(m_settingsWindow);
    m_aboutWindow = std::make_shared<UiAboutWindow>();
    m_aboutWindow->addListener(this);
    m_window->addView(m_aboutWindow);
    m_leftPanel = std::make_shared<UiLeftPanel>();
    m_leftPanel->addListener(this);
    m_window->addView(m_leftPanel);

    // Create app icon image
    GuiIcon& icon = appIcons.back();
    GfxImageInfo iconImageInfo{};
    iconImageInfo.width = icon.width;
    iconImageInfo.height = icon.height;
    iconImageInfo.format = GfxFormat::R8G8B8A8_UNORM;
    iconImageInfo.usages.set(GfxImageUsage::SAMPLED_TEXTURE);
    m_appIcon = renderer->createImage(iconImageInfo);
    if (m_appIcon == nullptr) {
        Logger() << "Failed to create app icon image";
        return 1;
    }
    renderer->setImageData(m_appIcon, icon.data.data());

    return 0;
}

void PathTracerApp::syncDirtyObjects(const std::unordered_set<DbObjHandle>& hObjects) {
    std::vector<DbObjHandle> dirtyObjects(hObjects.begin(), hObjects.end());
    // sort by type priority: Scene > Model > Mesh > Material
    std::sort(
        dirtyObjects.begin(),
        dirtyObjects.end(),
        [](const DbObjHandle& a, const DbObjHandle& b) {
            static const std::unordered_map<std::string, int> typePriority = {
                { PtScene::TYPE_NAME, 0 },
                { PtModel::TYPE_NAME, 1 },
                { PtMesh::TYPE_NAME, 2 },
                { PtMaterial::TYPE_NAME, 3 }
            };

            auto getPriority = [](const std::string& type) {
                auto it = typePriority.find(type);
                return it != typePriority.end() ? it->second : 4;
                };

            return getPriority(a.getType()) < getPriority(b.getType());
        }
    );
    bool resolutionChanged = false;
    GfxImage frameImage = m_previewer->getFrameImage();
    m_previewer->updateObjects(dirtyObjects, resolutionChanged);
    m_nTriangles = m_previewer->countTriangles();
    if (resolutionChanged)
        updatePreviewerFrameImage(frameImage);
    updateUiLeftPanel(dirtyObjects);
    updateUiRightPanel(dirtyObjects);
}

void PathTracerApp::updatePreviewerFrameImage(GfxImage image) {
    GfxRenderer renderer = m_window->getRenderer();
    AppUiManager::instance().destroyImGuiTexture(renderer, image);
    if (m_displayMode == DisplayMode::PREVIEW_MODE) {
        m_mainViewport->frameTexture = AppUiManager::instance().getImGuiTexture(
            renderer,
            m_previewer->getFrameImage()
        );
    }
}

void PathTracerApp::updateUiLeftPanel(const std::vector<DbObjHandle> &hObjects) {
    bool updateWaves = false, updateMaterials = false;
    for (const auto& hObj : hObjects) {
        const std::string type = hObj.getType();
        if (type == PtScene::TYPE_NAME) {
            m_leftPanel->setWidgetValue(
                static_cast<int>(UiLeftPanel::ID::SKY_TEMPERATURE),
                PtScene::getSkyTemperature(hObj)
            );
        } else if (type == SpWave::TYPE_NAME)
            updateWaves = true;
        else if (type == SpMaterial::TYPE_NAME) {
            updateUiMaterialListItem(hObj);
            updateMaterials = true;
        }
    }

    if (updateWaves)
        updateUiLeftPanelWaves();
    if (updateMaterials)
        updateUiSpectrumMaterialCombos();
}

void PathTracerApp::updateUiLeftPanelWaves() {
    DbObjHandle hScene = AppDataManager::instance().getDB()->getRootObject();
    std::vector<DbObjHandle> waveHandles = PtScene::getWaves(hScene);
    m_leftPanel->setWidgetValue(
        static_cast<int>(UiLeftPanel::ID::WAVE_COUNT),
        static_cast<int>(waveHandles.size())
    );

    // Display channel
    int channel = m_leftPanel->getWidgetValue<int>(
        static_cast<int>(UiLeftPanel::ID::OUTPUT_DISP_CHANNEL)
    );
    if (waveHandles.empty())
        channel = -1;
    else {
        channel = std::min(
            std::max(0, channel),
            static_cast<int>(waveHandles.size()) - 1
        );
    }
    m_leftPanel->setWidgetValue(
        static_cast<int>(UiLeftPanel::ID::OUTPUT_DISP_CHANNEL),
        channel
    );

    // Wave node
    int waveIndex = m_leftPanel->getWidgetValue<int>(
        static_cast<int>(UiLeftPanel::ID::WAVES_INDEX)
    );
    if (waveHandles.empty())
        waveIndex = -1;
    else {
        waveIndex = std::min(
            std::max(0, waveIndex),
            static_cast<int>(waveHandles.size()) - 1
        );
    }
    m_leftPanel->setWidgetValue(
        static_cast<int>(UiLeftPanel::ID::WAVES_INDEX),
        waveIndex
    );
    if (!waveHandles.empty()) {
        float waveNumber = SpWave::getWaveNumber(waveHandles[waveIndex]);
        m_leftPanel->setWidgetValue(
            static_cast<int>(UiLeftPanel::ID::WAVES_WAVE_NUMBER),
            waveNumber
        );
    }

    // Materials node
    for (int i = 0; i < m_leftPanel->materialListView.size(); i++) {
        auto& item =
            m_leftPanel->materialListView.getItem<UiLeftPanel::MaterialListItem>(i);
        if (item == nullptr)
            continue;
        waveIndex = item->getWidgetValue<int>(
            static_cast<int>(UiLeftPanel::ID::MATERIAL_WAVE_INDEX)
        );
        DbObjHandle hMaterial = item->hMaterial;
        std::vector<float> emissivities = SpMaterial::getEmissivities(hMaterial);
        if (emissivities.empty())
            waveIndex = -1;
        else {
            waveIndex = std::min(
                std::max(0, waveIndex),
                static_cast<int>(emissivities.size()) - 1
            );
        }
        item->setWidgetValue(
            static_cast<int>(UiLeftPanel::ID::MATERIAL_WAVE_INDEX),
            waveIndex
        );
        if (emissivities.empty())
            continue;
        item->setWidgetValue(
            static_cast<int>(UiLeftPanel::ID::MATERIAL_EMISSIVITY),
            emissivities[waveIndex]
        );
    }
}

void PathTracerApp::updateUiSpectrumMaterialCombos() {
    std::vector<std::string> strList = {};
    DbObjHandle hScene = AppDataManager::instance().getDB()->getRootObject();

    // Mesh list items
    std::vector<DbObjHandle> materialHandles = PtScene::getSpectrumMaterials(hScene);
    strList.reserve(materialHandles.size());
    for (const auto& hMaterial : materialHandles)
        strList.push_back(SpMaterial::getName(hMaterial));
    m_rightPanel->setWidgetComboItems(
        static_cast<int>(UiRightPanel::ID::MESH_MATERIAL),
        strList
    );
    for (const auto& [hMesh, item] : m_meshUiListItemLookUp) {
        DbObjHandle hMaterial = PtMesh::getMaterial(hMesh);
        DbObjHandle hSpMaterial = PtMaterial::getSpectrumMaterial(hMaterial);
        int index = -1;
        auto it = std::find(materialHandles.begin(), materialHandles.end(), hSpMaterial);
        if (it != materialHandles.end())
            index = it - materialHandles.begin();
        item->setWidgetValue(static_cast<int>(UiRightPanel::ID::MESH_MATERIAL), index);
    }

    // Sky material
    DbObjHandle hSkyMaterial = PtScene::getSkyMaterial(hScene);
    int index = -1;
    auto it = std::find(materialHandles.begin(), materialHandles.end(), hSkyMaterial);
    if (it != materialHandles.end())
        index = it - materialHandles.begin();
    m_leftPanel->setWidgetValue(static_cast<int>(UiLeftPanel::ID::SKY_MATERIAL), index);
}

void PathTracerApp::updateUiMaterialListItem(const DbObjHandle &hMaterial) {
    if (hMaterial.isValid()) {
        UiLeftPanel::MaterialListItem* materialListItem = nullptr;
        if (m_materialListItemLookUp.count(hMaterial) == 0) {
            // add new material list item
            materialListItem =
                m_leftPanel->materialListView.addItem<UiLeftPanel::MaterialListItem>().get();
            materialListItem->setExpanded(false);
            materialListItem->hMaterial = hMaterial;
            m_materialListItemLookUp[hMaterial] = materialListItem;
        } else
            materialListItem = m_materialListItemLookUp[hMaterial];
        if (materialListItem == nullptr)
            return;

        materialListItem->setWidgetValue(
            static_cast<int>(UiLeftPanel::ID::MATERIAL_NAME),
            SpMaterial::getName(hMaterial)
        );
        int waveIndex = materialListItem->getWidgetValue<int>(
            static_cast<int>(UiLeftPanel::ID::MATERIAL_WAVE_INDEX)
        );
        std::vector<float> emissivities = SpMaterial::getEmissivities(hMaterial);
        materialListItem->setWidgetValue(
            static_cast<int>(UiLeftPanel::ID::MATERIAL_EMISSIVITY),
            waveIndex < emissivities.size() ? emissivities[waveIndex] : 0.0f
        );
    } else if (m_materialListItemLookUp.count(hMaterial) != 0) {
        // remove material list item
        auto materialListItem = m_materialListItemLookUp[hMaterial];
        m_leftPanel->materialListView.removeItem(materialListItem->getIndex());
        m_materialListItemLookUp.erase(hMaterial);
    }
}

void PathTracerApp::updateUiRightPanel(const std::vector<DbObjHandle>& hObjects) {
    for (const auto& hObj : hObjects) {
        const std::string type = hObj.getType();
        if (type == PtScene::TYPE_NAME) {
            // Image node
            m_rightPanel->setWidgetValue(
                static_cast<int>(UiRightPanel::ID::TRACE_DEPTH),
                PtScene::getTraceDepth(hObj)
            );
            int resX = 0, resY = 0;
            PtScene::getResolution(hObj, resX, resY);
            m_rightPanel->setWidgetValue(static_cast<int>(UiRightPanel::ID::RES_X), resX);
            m_rightPanel->setWidgetValue(static_cast<int>(UiRightPanel::ID::RES_Y), resY);

            // Camera node
            PtScene::Camera cam = PtScene::getCamera(hObj);
            m_rightPanel->setWidgetValue(
                static_cast<int>(UiRightPanel::ID::FOCUS_DIST),
                cam.focusDist
            );
            m_rightPanel->setWidgetValue(static_cast<int>(UiRightPanel::ID::F_STOP), cam.fStop);
            m_rightPanel->setWidgetValue(
                static_cast<int>(UiRightPanel::ID::CAM_POS),
                AppUiUtils::vec3ToArray(cam.position)
            );
            m_rightPanel->setWidgetValue(
                static_cast<int>(UiRightPanel::ID::CAM_ROT),
                AppUiUtils::vec3ToArray(cam.rotation)
            );
        } else if (type == PtModel::TYPE_NAME) {
            // Model node
            updateUiModelListItem(hObj);
        } else {
            // Mesh node
            updateUiMeshListItem(hObj);
        }
    }
}

void PathTracerApp::updateUiModelListItem(const DbObjHandle& hModel) {
    if (hModel.isValid()) {
        UiRightPanel::ModelListItem* modelListItem = nullptr;
        if (m_modelUiListItemLookUp.count(hModel) == 0) {
            // add new model list item
            modelListItem =
                m_rightPanel->modelListView.addItem<UiRightPanel::ModelListItem>().get();
            modelListItem->setExpanded(false);
            modelListItem->hModel = hModel;
            m_modelUiListItemLookUp[hModel] = modelListItem;

            // populate mesh list items
            std::vector<DbObjHandle> meshHandles = PtModel::getMeshes(hModel);
            for (const auto& hMesh : meshHandles) {
                auto meshListItem =
                    modelListItem->meshListView.addItem<UiRightPanel::MeshListItem>().get();
                meshListItem->setExpanded(false);
                meshListItem->hMesh = hMesh;
                m_meshUiListItemLookUp[hMesh] = meshListItem;
                meshListItem->setWidgetValue
                (
                    static_cast<int>(UiRightPanel::ID::MESH_NAME),
                    PtMesh::getName(hMesh)
                );
                DbObjHandle hMaterial = PtMesh::getMaterial(hMesh);
                updateUiMeshListItem(hMaterial);
            }
        } else
            modelListItem = m_modelUiListItemLookUp[hModel];
        if (modelListItem == nullptr)
            return;

        modelListItem->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::MODEL_NAME),
            PtModel::getName(hModel)
        );
        modelListItem->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::MODEL_LOCATION),
            AppUiUtils::vec3ToArray(PtModel::getLocation(hModel))
        );
        modelListItem->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::MODEL_ROTATION),
            AppUiUtils::vec3ToArray(PtModel::getRotation(hModel))
        );
        modelListItem->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::MODEL_SCALE),
            AppUiUtils::vec3ToArray(PtModel::getScale(hModel))
        );
    } else if (m_modelUiListItemLookUp.count(hModel) != 0) {
        // remove model list item
        auto modelListItem = m_modelUiListItemLookUp[hModel];
        for (int i = 0; i < modelListItem->meshListView.size(); i++) {
            auto meshListItem =
                modelListItem->meshListView.getItem<UiRightPanel::MeshListItem>(i);
            if (meshListItem == nullptr)
                continue;
            m_meshUiListItemLookUp.erase(meshListItem->hMesh);
        }
        m_rightPanel->modelListView.removeItem(modelListItem->getIndex());
        m_modelUiListItemLookUp.erase(hModel);
    }
}

void PathTracerApp::updateUiMeshListItem(const DbObjHandle& hObj) {
    const std::string type = hObj.getType();
    if (type == PtMesh::TYPE_NAME) {
        if (!hObj.isValid())
            return;
        UiRightPanel::MeshListItem* meshListItem = nullptr;
        if (m_meshUiListItemLookUp.count(hObj) == 0)
            return;
        meshListItem = m_meshUiListItemLookUp[hObj];
        meshListItem->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::MESH_NAME),
            PtMesh::getName(hObj)
        );
    } else if (type == PtMaterial::TYPE_NAME) {
        if (!hObj.isValid())
            return;
        DbObjHandle hMesh = PtMaterial::getMesh(hObj);
        if (!hMesh.isValid())
            return;
        if (m_meshUiListItemLookUp.count(hMesh) == 0)
            return;
        auto meshListItem = m_meshUiListItemLookUp[hMesh];

        PtMaterial::MaterialType materialType = PtMaterial::getType(hObj);
        meshListItem->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::MESH_MATERIAL_TYPE),
            static_cast<int>(materialType)
        );
        meshListItem->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::MESH_ROUGHNESS),
            PtMaterial::getRoughness(hObj)
        );
        meshListItem->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::MESH_TEMPERATURE),
            PtMaterial::getTemperature(hObj)
        );
        meshListItem->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::MESH_IOR),
            PtMaterial::getIOR(hObj)
        );

        int value = 0;
        DbObjHandle hSpMaterial = PtMaterial::getSpectrumMaterial(hObj);
        DbObjHandle hScene = AppDataManager::instance().getDB()->getRootObject();
        std::vector<DbObjHandle> materialHandles = PtScene::getSpectrumMaterials(hScene);
        auto it = std::find(materialHandles.begin(), materialHandles.end(), hSpMaterial);
        if (it != materialHandles.end())
            value = it - materialHandles.begin();
        meshListItem->setWidgetValue(
            static_cast<int>(UiRightPanel::ID::MESH_MATERIAL),
            value
        );

        GfxRenderer renderer = m_window->getRenderer();
        Flags<PtMaterial::MaterialFlag> materialFlags = PtMaterial::getFlags(hObj);

        std::string normalTexPath = PtMaterial::getNormalTexPath(hObj);
        GfxImage normalTex = AppTextureManager::instance().getTexture(normalTexPath);
        meshListItem->normalTexture =
            materialFlags.check(PtMaterial::MaterialFlag::NORMAL_MAP) ?
            AppUiManager::instance().getImGuiTexture(renderer, normalTex) : 0;
        std::string roughnessTexPath = PtMaterial::getRoughnessTexPath(hObj);
        GfxImage roughnessTex = AppTextureManager::instance().getTexture(roughnessTexPath);
        meshListItem->roughnessTexture =
            materialFlags.check(PtMaterial::MaterialFlag::ROUGHNESS_MAP) ?
            AppUiManager::instance().getImGuiTexture(renderer, roughnessTex) : 0;
        std::string intensityTexPath = PtMaterial::getTemperatureTexPath(hObj);
        GfxImage intensityTex =
            AppTextureManager::instance().getIntensityPreviewTexture(intensityTexPath);
        meshListItem->temperatureTexture =
            materialFlags.check(PtMaterial::MaterialFlag::TEMPERATURE_MAP) ?
            AppUiManager::instance().getImGuiTexture(renderer, intensityTex) : 0;
    }
}

void PathTracerApp::updateUiStatusBar() {
    int currentSample = m_pathTracer->getCurrentSample();
    float progress = 0.0f;
    if (m_targetSample > 0)
        progress = static_cast<float>(currentSample) / static_cast<float>(m_targetSample);
    float timeElapsed = m_renderStopwatch.elapsed() * 0.001f; // ms to s
    float efficiency = 0.0f;
    if (currentSample > 0 && timeElapsed > 0.0f)
        efficiency = timeElapsed / static_cast<float>(currentSample);

    m_statusBar->setWidgetValue(
        static_cast<int>(UiStatusBar::ID::RENDERING_SAMPLES),
        m_currentRenderState == RenderState::PAUSED ? -currentSample : currentSample
    );
    m_statusBar->setWidgetValue(
        static_cast<int>(UiStatusBar::ID::RENDERING_PROGRESS),
        progress
    );
    m_statusBar->setWidgetValue(
        static_cast<int>(UiStatusBar::ID::EFFICIENCY),
        efficiency
    );
    m_statusBar->setWidgetValue(
        static_cast<int>(UiStatusBar::ID::TIME_ELAPSED),
        timeElapsed
    );
    m_statusBar->setWidgetValue(
        static_cast<int>(UiStatusBar::ID::TRIANGLE_COUNT),
        m_nTriangles
    );
}

void PathTracerApp::selectModel(const DbObjHandle& hModel) {
    if (m_modelUiListItemLookUp.count(hModel) == 0)
        return;

    if (m_selectionMode == SelectionMode::SINGLE) {
        m_rightPanel->modelListView.clearSelection();
        m_selectedModels.clear();
    }

    auto item = m_modelUiListItemLookUp[hModel];
    if (m_selectionMode.check(SelectionMode::RANGE) && m_focusedModel.isValid()) {
        auto itemFocused = m_modelUiListItemLookUp[m_focusedModel];
        int idxStart = std::min(itemFocused->getIndex(), item->getIndex());
        int idxEnd = std::max(itemFocused->getIndex(), item->getIndex());
        for (int i = idxStart; i <= idxEnd; i++) {
            auto rangeItem =
                m_rightPanel->modelListView.getItem<UiRightPanel::ModelListItem>(i);
            if (rangeItem == nullptr)
                continue;
            if (!rangeItem->isSelected()) {
                m_rightPanel->modelListView.selectItem(rangeItem->getIndex(), true);
                m_selectedModels.insert(rangeItem->hModel);
            }
        }
    } else {
        if (!item->isSelected()) {
            m_rightPanel->modelListView.selectItem(item->getIndex(), true);
            m_selectedModels.insert(hModel);
        } else if (m_selectionMode.check(SelectionMode::MULTI)) {
            m_rightPanel->modelListView.selectItem(item->getIndex(), false);
            m_selectedModels.erase(hModel);
        }
    }
    m_focusedModel = hModel;
}

void PathTracerApp::saveFileDialog() {
    std::string currentFilename = AppDataManager::instance().getCurrentDbPath();
    if (currentFilename.empty())
        currentFilename = GuiText::get("save_dialog.default_filename");
    else
        currentFilename = std::filesystem::path(currentFilename).filename().string();
    m_saveDialog->setWidgetValue(
        static_cast<int>(UiSaveDialog::ID::FILENAME),
        currentFilename
    );
    m_saveDialog->open();
}

int PathTracerApp::loadNewScene(const std::string& filename) {
    // clear first
    m_previewer->clearScene();
    m_pathTracer->clearScene();
    AppUiManager::instance().destroyImGuiTextures(m_window->getRenderer());
    AppTextureManager::instance().clearCache();
    AppDataManager::instance().resetDB();
    ScopeGuard displayModeGuard([this]() { setDisplayMode(DisplayMode::PREVIEW_MODE); });

    // load the scene
    if (!filename.empty() && AppDataManager::instance().loadDbFromFile(filename)) {
        Logger() << "Failed to load scene from file: " << filename;
        return 1;
    }
    auto db = AppDataManager::instance().getDB();
    DbObjHandle hScene = db->getRootObject();
    if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME) {
        Logger() << "Invalid scene handle";
        return 1;
    }

    // update previewer
    if (m_previewer->loadScene(hScene)) {
        m_previewer->clearScene();
        Logger() << "Failed to load scene into previewer";
        return 1;
    }

    // update right panel
    m_modelUiListItemLookUp.clear();
    m_meshUiListItemLookUp.clear();
    updateUiRightPanel({ hScene });
    m_rightPanel->modelListView.clear();
    if (!filename.empty()) {
        for (const auto& hModel : PtScene::getModels(hScene))
            updateUiModelListItem(hModel);
    }

    // update left panel
    m_materialListItemLookUp.clear();
    updateUiLeftPanel({ hScene });
    m_leftPanel->materialListView.clear();
    if (!filename.empty()) {
        for (const auto& hMaterial : PtScene::getSpectrumMaterials(hScene))
            updateUiMaterialListItem(hMaterial);
        updateUiLeftPanelWaves();
        updateUiSpectrumMaterialCombos();
    }

    // update status bar
    m_nTriangles = m_previewer->countTriangles();
    setDisplayMode(DisplayMode::PREVIEW_MODE);
    m_renderStopwatch.reset();

    return 0;
}

void PathTracerApp::newScene() {
    bool condition =
        m_currentRenderState == RenderState::IDLE ||
        m_currentRenderState == RenderState::PAUSED;
    if (!condition)
        return;

    if (AppDataManager::instance().getDB()->isModified()) {
        saveFileDialog();
        m_saveDialog->setEventCallback(
            [this](const GuiEvent& event) {
                switch (static_cast<UiSaveDialog::ID>(event.widgetID)) {
                case UiSaveDialog::ID::YES:
                {
                    saveScene();
                    loadNewScene();
                    break;
                }
                case UiSaveDialog::ID::NO:
                {
                    loadNewScene();
                    break;
                }
                default:
                    break;
                }
            }
        );
    } else
        loadNewScene();
}

void PathTracerApp::openScene() {
    bool condition =
        m_currentRenderState == RenderState::IDLE ||
        m_currentRenderState == RenderState::PAUSED;
    if (!condition)
        return;
    auto openSceneImpl = [this]() {
        const char* filters[1] = { "*.sps" };
        const char* filename = tinyfd_openFileDialog(
            GuiText::get("open_scene_dialog.title").c_str(),
            "",
            1,
            filters,
            GuiText::get("open_scene_dialog.filter_desc").c_str(),
            0
        );
        if (!filename)
            return;
        loadNewScene(filename);
        };

    if (AppDataManager::instance().getDB()->isModified()) {
        saveFileDialog();
        m_saveDialog->setEventCallback(
            [this, openSceneImpl](const GuiEvent& event) {
                switch (static_cast<UiSaveDialog::ID>(event.widgetID)) {
                case UiSaveDialog::ID::YES:
                {
                    saveScene();
                    openSceneImpl();
                    break;
                }
                case UiSaveDialog::ID::NO:
                {
                    openSceneImpl();
                    break;
                }
                default:
                    break;
                }
            }
        );
    } else
        openSceneImpl();
}

void PathTracerApp::saveScene() const {
    std::string currentFilename = AppDataManager::instance().getCurrentDbPath();
    if (currentFilename.empty())
        saveAsScene();
    else {
        if (AppDataManager::instance().saveDbToFile(currentFilename))
            Logger() << "Failed to save scene to file: " << currentFilename;
    }
}

void PathTracerApp::saveAsScene() const {
    std::string savePath = AppDataManager::instance().getCurrentDbPath();
    if (savePath.empty())
        savePath = GuiText::get("save_dialog.default_filename");
    const char* filterItems[1] = { "*.sps" };
    const char* filename = tinyfd_saveFileDialog(
        GuiText::get("save_scene_dialog.title").c_str(),
        savePath.c_str(),
        1,
        filterItems,
        GuiText::get("save_scene_dialog.filter_desc").c_str()
    );
    if (!filename)
        return;
    if (AppDataManager::instance().saveDbToFile(filename))
        Logger() << "Failed to save scene to file: " << filename;
}

void PathTracerApp::loadModel() {
    if (m_currentRenderState != RenderState::IDLE)
        return;

    const char* filters[1] = { "*.obj" };
    const char* filename = tinyfd_openFileDialog(
        GuiText::get("load_model_dialog.title").c_str(),
        "",
        1,
        filters,
        GuiText::get("load_model_dialog.filter_desc").c_str(),
        0
    );
    if (!filename)
        return;

    loadModelUtil(filename);
}

void PathTracerApp::exportImage() const {
    bool condition =
        m_currentRenderState == RenderState::IDLE ||
        m_currentRenderState == RenderState::PAUSED;
    if (!condition)
        return;

    // Generate default filename with timestamp
    auto now = std::chrono::system_clock::now();
    time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::chrono::milliseconds ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm* tm = std::localtime(&now_time);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y%m%d_%H%M%S");
    oss << "_" << std::setfill('0') << std::setw(3) << ms.count();
    std::string filename = oss.str();

    // Show save file dialog
    const char* filters[1] = { "*.txt" };
    const char* filePath = tinyfd_saveFileDialog(
        GuiText::get("export_txt_dialog.title").c_str(),
        filename.c_str(),
        1,
        filters,
        GuiText::get("export_txt_dialog.filter_desc").c_str()
    );
    if (!filePath)
        return;
    filename = filePath;

    // Get rendered data
    DbObjHandle hScene = AppDataManager::instance().getDB()->getRootObject();
    int width = 0, height = 0;
    PtScene::getResolution(hScene, width, height);
    int nWaves = PtScene::getWaves(hScene).size();
    std::vector<float> data = {};
    if (m_pathTracer->getImageData(data, width, height, nWaves))
        data.resize(static_cast<size_t>(width * height * nWaves), 0);

    // Write data to file
    std::ofstream file(filename);
    if (!file.is_open())
        return;
    for (int wave = 0; wave < nWaves; ++wave) {
        for (int row = height - 1; row >= 0; --row) {
            for (int col = 0; col < width; ++col) {
                size_t idx = static_cast<size_t>(wave * height * width + row * width + col);
                file << data[idx];
                if (col < width - 1)
                    file << " ";
            }
            file << "\n";
        }
    }
    file.close();
}

void PathTracerApp::undo() {
    if (m_currentRenderState != RenderState::IDLE)
        return;
    auto db = AppDataManager::instance().getDB();
    if (db->canUndo() == false)
        return;
    std::unordered_set<DbObjHandle> dirtyObjectSet{};
    if (db->undo(dirtyObjectSet) != DB::Result::SUCCESS)
        Logger() << "Failed to undo last action";
    else
        syncDirtyObjects(dirtyObjectSet);
}

void PathTracerApp::redo() {
    if (m_currentRenderState != RenderState::IDLE)
        return;
    auto db = AppDataManager::instance().getDB();
    if (db->canRedo() == false)
        return;
    std::unordered_set<DbObjHandle> dirtyObjectSet{};
    if (db->redo(dirtyObjectSet) != DB::Result::SUCCESS)
        Logger() << "Failed to redo last action";
    else
        syncDirtyObjects(dirtyObjectSet);
}

void PathTracerApp::cutSeletedModels() {
    if (m_currentRenderState != RenderState::IDLE)
        return;
    if (m_selectedModels.empty())
        return;
    std::vector<DbObjHandle> modelsToCut(m_selectedModels.begin(), m_selectedModels.end());
    AppClipboard::instance().cut(modelsToCut);
    updateUiRightPanel(modelsToCut);
    m_previewer->updateObjects(modelsToCut);
    m_nTriangles = m_previewer->countTriangles();
    // Clear selection
    m_rightPanel->modelListView.clearSelection();
    m_selectedModels.clear();
}

void PathTracerApp::copySeletedModels() {
    if (m_selectedModels.empty())
        return;
    std::vector<DbObjHandle> modelsToCopy(m_selectedModels.begin(), m_selectedModels.end());
    AppClipboard::instance().copy(modelsToCopy);
}

void PathTracerApp::pasteCopiedModels() {
    if (m_currentRenderState != RenderState::IDLE)
        return;
    if (!AppClipboard::instance().hasData())
        return;
    std::vector<DbObjHandle> pastedModels = AppClipboard::instance().paste();
    updateUiRightPanel(pastedModels);
    m_previewer->updateObjects(pastedModels);
    m_nTriangles = m_previewer->countTriangles();
    // Clear selection
    m_rightPanel->modelListView.clearSelection();
    m_selectedModels.clear();
}

void PathTracerApp::deleteSeletedModels() {
    if (m_currentRenderState != RenderState::IDLE)
        return;
    if (m_selectedModels.empty())
        return;
    std::vector<DbObjHandle> modelsToDel(m_selectedModels.begin(), m_selectedModels.end());
    auto db = AppDataManager::instance().getDB();
    DbObjHandle hScene = db->getRootObject();
    if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME) {
        Logger() << "Invalid scene handle";
        return;
    }
    DbUtils::TxnGuard txnGuard(db);
    for (const auto hModel : modelsToDel) {
        if (PtScene::delModel(hScene, hModel) != DB::Result::SUCCESS)
            continue;
    }
    txnGuard.commit();
    updateUiRightPanel(modelsToDel);
    m_previewer->updateObjects(modelsToDel);
    m_nTriangles = m_previewer->countTriangles();
    // Clear selection
    m_rightPanel->modelListView.clearSelection();
    m_selectedModels.clear();
}

void PathTracerApp::selectAllModels() {
    m_selectedModels.clear();
    auto db = AppDataManager::instance().getDB();
    for (const auto& it : m_modelUiListItemLookUp)
        m_selectedModels.insert(it.first);
    m_rightPanel->modelListView.selectAll();
}

void PathTracerApp::startRendering() {
    bool condition =
        m_currentRenderState == RenderState::IDLE ||
        m_currentRenderState == RenderState::PAUSED;
    if (!condition)
        return;
    if (m_pathTracer->getCurrentSample() == 0) {
        GfxRenderer renderer = m_window->getRenderer();
        auto outputImage = m_postProcesser->getOutputImage();
        AppUiManager::instance().destroyImGuiTexture(renderer, outputImage);
        auto db = AppDataManager::instance().getDB();
        DbObjHandle hScene = AppDataManager::instance().getDB()->getRootObject();
        int width = 0, height = 0;
        PtScene::getResolution(hScene, width, height);
        if (m_pathTracer->buildScene(hScene))
            return;
        if (m_postProcesser->initFrame(width, height, m_pathTracer->getDisplayImages()))
            return;
    }
    m_pathTracer->render();

    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_NEW_SCENE), false);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_OPEN_SCENE), false);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_LOAD_MODEL), false);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_EXPORT), false);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_START), false);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_PAUSE), true);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_STOP), true);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_RESTART), true);

    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::EXPORT), false);
    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::LOAD_MODEL), false);
    m_toolBar->setWidgetVisible(static_cast<int>(UiToolBar::ID::START), false);
    m_toolBar->setWidgetVisible(static_cast<int>(UiToolBar::ID::PAUSE), true);
    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::PAUSE), true);
    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::STOP), true);
    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::RESTART), true);
    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::TARGET_SAMPLES), false);

    m_rightPanel->enableWidget(static_cast<int>(UiRightPanel::ID::IMAGE_NODE), false);
    m_rightPanel->enableWidget(static_cast<int>(UiRightPanel::ID::CAMERA_NODE), false);
    m_rightPanel->enableWidget(static_cast<int>(UiRightPanel::ID::SCENE_NODE), false);

    m_leftPanel->enableWidget(static_cast<int>(UiLeftPanel::ID::WAVES_NODE), false);
    m_leftPanel->enableWidget(static_cast<int>(UiLeftPanel::ID::MATERIALS_NODE), false);
    m_leftPanel->enableWidget(static_cast<int>(UiLeftPanel::ID::SKY_NODE), false);

    setDisplayMode(DisplayMode::PATH_TRACER_OUTPUT);

    m_currentRenderState = RenderState::RENDERING;
    m_renderStopwatch.start();
}

void PathTracerApp::pauseRendering() {
    if (m_currentRenderState != RenderState::RENDERING)
        return;
    m_currentRenderState = RenderState::PENDING_PAUSE;
    m_pathTracer->pause();

    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_PAUSE), false);
    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::PAUSE), false);

    m_pathTracer->setRenderFinishCallback(
        [this]() {
            m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_NEW_SCENE), true);
            m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_OPEN_SCENE), true);
            m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_EXPORT), true);

            m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::EXPORT), true);
            m_toolBar->setWidgetVisible(static_cast<int>(UiToolBar::ID::START), true);
            m_toolBar->setWidgetVisible(static_cast<int>(UiToolBar::ID::PAUSE), false);

            m_currentRenderState = RenderState::PAUSED;
            m_renderStopwatch.pause();
        }
    );
}

void PathTracerApp::stopRendering() {
    bool condition =
        m_currentRenderState == RenderState::RENDERING ||
        m_currentRenderState == RenderState::PENDING_PAUSE ||
        m_currentRenderState == RenderState::PAUSED;
    if (!condition)
        return;
    m_pathTracer->stop();

    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_PAUSE), false);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_STOP), false);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_RESTART), false);

    m_toolBar->setWidgetVisible(static_cast<int>(UiToolBar::ID::PAUSE), false);
    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::STOP), false);
    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::RESTART), false);

    auto stop = [this]() {
        m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_NEW_SCENE), true);
        m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_OPEN_SCENE), true);
        m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_LOAD_MODEL), true);
        m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::FILE_EXPORT), true);
        m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_START), true);

        m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::EXPORT), true);
        m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::LOAD_MODEL), true);
        m_toolBar->setWidgetVisible(static_cast<int>(UiToolBar::ID::START), true);
        m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::TARGET_SAMPLES), true);

        m_rightPanel->enableWidget(static_cast<int>(UiRightPanel::ID::IMAGE_NODE), true);
        m_rightPanel->enableWidget(static_cast<int>(UiRightPanel::ID::CAMERA_NODE), true);
        m_rightPanel->enableWidget(static_cast<int>(UiRightPanel::ID::SCENE_NODE), true);

        m_leftPanel->enableWidget(static_cast<int>(UiLeftPanel::ID::WAVES_NODE), true);
        m_leftPanel->enableWidget(static_cast<int>(UiLeftPanel::ID::MATERIALS_NODE), true);
        m_leftPanel->enableWidget(static_cast<int>(UiLeftPanel::ID::SKY_NODE), true);

        m_currentRenderState = RenderState::IDLE;
        m_renderStopwatch.reset();
        };

    if (m_currentRenderState == RenderState::PAUSED)
        stop();
    else {
        m_currentRenderState = RenderState::PENDING_STOP;
        m_pathTracer->setRenderFinishCallback(stop);
    }
}

void PathTracerApp::restartRendering() {
    bool condition =
        m_currentRenderState == RenderState::RENDERING ||
        m_currentRenderState == RenderState::PAUSED;
    if (!condition)
        return;
    m_currentRenderState = RenderState::PENDING_RESTART;
    m_pathTracer->restart();

    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_PAUSE), false);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_STOP), false);
    m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_RESTART), false);

    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::PAUSE), false);
    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::STOP), false);
    m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::RESTART), false);

    m_pathTracer->setRenderFinishCallback(
        [this]() {
            m_toolBar->setWidgetVisible(static_cast<int>(UiToolBar::ID::START), false);
            m_toolBar->setWidgetVisible(static_cast<int>(UiToolBar::ID::PAUSE), true);

            m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_PAUSE), true);
            m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_STOP), true);
            m_menuBar->enableWidget(static_cast<int>(UiMenuBar::ID::RENDER_RESTART), true);

            m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::PAUSE), true);
            m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::STOP), true);
            m_toolBar->enableWidget(static_cast<int>(UiToolBar::ID::RESTART), true);

            m_currentRenderState = RenderState::RENDERING;
            setDisplayMode(DisplayMode::PATH_TRACER_OUTPUT);
            m_renderStopwatch.reset();
            m_renderStopwatch.start();
        }
    );
}

void PathTracerApp::handleMenuBarEvent(const GuiEvent& event) {
    switch (static_cast<UiMenuBar::ID>(event.widgetID)) {
    case UiMenuBar::ID::FILE_NEW_SCENE:
    {
        newScene();
        break;
    }
    case UiMenuBar::ID::FILE_OPEN_SCENE:
    {
        openScene();
        break;
    }
    case UiMenuBar::ID::FILE_SAVE:
    {
        saveScene();
        break;
    }
    case UiMenuBar::ID::FILE_SAVE_AS:
    {
        saveAsScene();
        break;
    }
    case UiMenuBar::ID::FILE_LOAD_MODEL:
    {
        loadModel();
        break;
    }
    case UiMenuBar::ID::FILE_EXPORT:
    {
        exportImage();
        break;
    }
    case UiMenuBar::ID::FILE_SETTINGS:
    {
        m_settingsWindow->open();
        break;
    }
    case UiMenuBar::ID::FILE_EXIT:
    {
        if (onCloseWindow())
            m_window->term();
        break;
    }
    case UiMenuBar::ID::EDIT_UNDO:
    {
        undo();
        break;
    }
    case UiMenuBar::ID::EDIT_REDO:
    {
        redo();
        break;
    }
    case UiMenuBar::ID::RENDER_START:
    {
        startRendering();
        break;
    }
    case UiMenuBar::ID::RENDER_PAUSE:
    {
        pauseRendering();
        break;
    }
    case UiMenuBar::ID::RENDER_STOP:
    {
        stopRendering();
        break;
    }
    case UiMenuBar::ID::RENDER_RESTART:
    {
        restartRendering();
        break;
    }
    case UiMenuBar::ID::VIEW_PREVIEW_MODE:
    {
        setDisplayMode(DisplayMode::PREVIEW_MODE);
        break;
    }
    case UiMenuBar::ID::VIEW_PATH_TRACER_OUTPUT:
    {
        setDisplayMode(DisplayMode::PATH_TRACER_OUTPUT);
        break;
    }
    case UiMenuBar::ID::HELP_ABOUT:
    {
        m_aboutWindow->appIconTexture = AppUiManager::instance().getImGuiTexture(
            m_window->getRenderer(),
            m_appIcon
        );
        m_aboutWindow->setWidgetValue(
            static_cast<int>(UiAboutWindow::ID::VERSION),
            AppVersion::getVersionString()
        );
        m_aboutWindow->open();
        break;
    }
    default:
        break;
    }
}

void PathTracerApp::handleToolBarEvent(const GuiEvent& event) {
    switch (static_cast<UiToolBar::ID>(event.widgetID)) {
    case UiToolBar::ID::SAVE:
    {
        saveScene();
        break;
    }
    case UiToolBar::ID::EXPORT:
    {
        exportImage();
        break;
    }
    case UiToolBar::ID::DISPLAY_MODE:
    {
        int value =
            m_toolBar->getWidgetValue<int>(static_cast<int>(UiToolBar::ID::DISPLAY_MODE));
        setDisplayMode(static_cast<DisplayMode>(value));
        break;
    }
    case UiToolBar::ID::LOAD_MODEL:
    {
        loadModel();
        break;
    }
    case UiToolBar::ID::START:
    {
        startRendering();
        break;
    }
    case UiToolBar::ID::PAUSE:
    {
        pauseRendering();
        break;
    }
    case UiToolBar::ID::STOP:
    {
        stopRendering();
        break;
    }
    case UiToolBar::ID::RESTART:
    {
        restartRendering();
        break;
    }
    case UiToolBar::ID::TARGET_SAMPLES:
    {
        m_targetSample = getEventValue<int>(event);
        break;
    }
    case UiToolBar::ID::SETTINGS:
    {
        m_settingsWindow->open();
        break;
    }
    default:
        break;
    }
}

void PathTracerApp::handleLeftPanelEvent(const GuiEvent &event) {
    auto db = AppDataManager::instance().getDB();
    DbObjHandle hScene = db->getRootObject();
    if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME) {
        Logger() << "Invalid scene handle";
        return;
    }
    switch (static_cast<UiLeftPanel::ID>(event.widgetID)) {
    case UiLeftPanel::ID::OUTPUT_DISP_CHANNEL:
    {
        m_postProcesser->setDisplayChannel(getEventValue<int>(event));
        break;
    }
    case UiLeftPanel::ID::WAVES_CLEAR:
    {
        DbUtils::txnFn(db, PtScene::clearWaves, hScene);
        updateUiLeftPanelWaves();
        break;
    }
    case UiLeftPanel::ID::WAVES_IMPORT:
    {
        const char* filters[1] = { "*.txt" };
        const char* filename = tinyfd_openFileDialog(
            GuiText::get("load_txt_dialog.title").c_str(),
            "",
            1,
            filters,
            GuiText::get("load_txt_dialog.filter_desc").c_str(),
            0
        );
        if (!filename)
            break;
        loadSpectrumWavesFromTXT(filename);
        updateUiLeftPanelWaves();
        break;
    }
    case UiLeftPanel::ID::WAVES_ADD:
    {
        DbUtils::TxnGuard txnGuard(db);
        DbObjHandle hWave = db->objCreate<SpWave>({});
        if (!hWave.isValid())
            break;
        if (PtScene::addWave(hScene, hWave) != DB::Result::SUCCESS)
            break;
        txnGuard.commit();
        updateUiLeftPanelWaves();
        break;
    }
    case UiLeftPanel::ID::WAVES_INDEX:
    {
        int index = getEventValue<int>(event);
        float waveNumber = 0.0f;
        std::vector<DbObjHandle> waveHandles = PtScene::getWaves(hScene);
        if (index < waveHandles.size())
            waveNumber = SpWave::getWaveNumber(waveHandles[index]);
        m_leftPanel->setWidgetValue(
            static_cast<int>(UiLeftPanel::ID::WAVES_WAVE_NUMBER),
            waveNumber
        );
        break;
    }
    case UiLeftPanel::ID::WAVES_DELETE:
    {
        int waveIndex = m_leftPanel->getWidgetValue<int>(
            static_cast<int>(UiLeftPanel::ID::WAVES_INDEX)
        );
        std::vector<DbObjHandle> waveHandles = PtScene::getWaves(hScene);
        if (waveIndex >= waveHandles.size())
            break;
        DB::Result result = DbUtils::txnFn(db, PtScene::delWave, hScene, waveHandles[waveIndex]);
        if (result == DB::Result::SUCCESS)
            updateUiLeftPanelWaves();
        break;
    }
    case UiLeftPanel::ID::WAVES_WAVE_NUMBER:
    {
        int waveIndex = m_leftPanel->getWidgetValue<int>(
            static_cast<int>(UiLeftPanel::ID::WAVES_INDEX)
        );
        std::vector<DbObjHandle> waveHandles = PtScene::getWaves(hScene);
        if (waveIndex >= waveHandles.size())
            break;
        float waveNumber = getEventValue<float>(event);
        DbUtils::txnFn(db, SpWave::setWaveNumber, waveHandles[waveIndex], waveNumber);
        break;
    }
    case UiLeftPanel::ID::MATERIALS_IMPORT:
    {
        const char* filters[1] = { "*.txt" };
        const char* filename = tinyfd_openFileDialog(
            GuiText::get("load_txt_dialog.title").c_str(),
            "",
            1,
            filters,
            GuiText::get("load_txt_dialog.filter_desc").c_str(),
            0
        );
        if (!filename)
            break;
        if (loadSpectrumMaterialsFromTXT(filename))
            break;
        m_leftPanel->materialListView.clear();
        for (const auto& hMaterial : PtScene::getSpectrumMaterials(hScene))
            updateUiMaterialListItem(hMaterial);
        updateUiSpectrumMaterialCombos();
        break;
    }
    case UiLeftPanel::ID::MATERIALS_ADD:
    {
        DbUtils::TxnGuard txnGuard(db);
        DbObjHandle hMaterial = db->objCreate<SpMaterial>({});
        if (!hMaterial.isValid())
            break;
        std::vector<DbObjHandle> materialHandles = PtScene::getSpectrumMaterials(hScene);
        int posCode = materialHandles.size();
        std::string name = GuiText::get("left_panel.material_node.default_name");
        name += std::to_string(posCode);
        if (SpMaterial::setName(hMaterial, name) != DB::Result::SUCCESS)
            break;
        if (PtScene::addSpectrumMaterial(hScene, hMaterial) != DB::Result::SUCCESS)
            break;
        txnGuard.commit();
        updateUiLeftPanel({ hMaterial });
        break;
    }
    case UiLeftPanel::ID::MATERIAL_LIST:
    {
        handleMaterialListEvent(event);
        break;
    }
    case UiLeftPanel::ID::SKY_MATERIAL:
    {
        int value = getEventValue<int>(event);
        std::vector<DbObjHandle> materialHandles = PtScene::getSpectrumMaterials(hScene);
        if (value < 0 || value >= materialHandles.size())
            break;
        DbObjHandle hSpMaterial = materialHandles[value];
        DbUtils::txnFn(db, PtScene::setSkyMaterial, hScene, hSpMaterial);
        break;
    }
    case UiLeftPanel::ID::SKY_TEMPERATURE:
    {
        float value = getEventValue<float>(event);
        DbUtils::txnFn(db, PtScene::setSkyTemperature, hScene, value);
        break;
    }
    default:
        break;
    }
}

void PathTracerApp::handleMaterialListEvent(const GuiEvent &event) {
    auto& materialListCtx = event.listViewCtx;
    if (materialListCtx == nullptr)
        return;

    auto db = AppDataManager::instance().getDB();
    DbObjHandle hScene = db->getRootObject();
    if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME) {
        Logger() << "Invalid scene handle";
        return;
    }
    std::vector<DbObjHandle> materialHandles = PtScene::getSpectrumMaterials(hScene);

    int materialListItemIdx = getEventValue<int>(event);
    if (materialListItemIdx >= materialHandles.size())
        return;
    DbObjHandle hMaterial = materialHandles[materialListItemIdx];

    switch (static_cast<UiLeftPanel::ID>(materialListCtx->widgetID)) {
    case UiLeftPanel::ID::MATERIAL_ITEM_DELETE:
    {
        DB::Result result = DbUtils::txnFn(db, PtScene::delSpectrumMaterial, hScene, hMaterial);
        if (result != DB::Result::SUCCESS)
            break;
        updateUiLeftPanel({ hMaterial });
        break;
    }
    case UiLeftPanel::ID::MATERIAL_NAME:
    {
        std::string name = getEventValue<std::string>(*materialListCtx);
        if (DbUtils::txnFn(db, SpMaterial::setName, hMaterial, name) != DB::Result::SUCCESS)
            break;
        updateUiLeftPanel({ hMaterial });
        break;
    }
    case UiLeftPanel::ID::MATERIAL_WAVE_INDEX:
    {
        int index = getEventValue<int>(*materialListCtx);
        float emissivity = 0.0f;
        std::vector<float> emissivities = SpMaterial::getEmissivities(hMaterial);
        if (index < emissivities.size())
            emissivity = emissivities[index];
        UiLeftPanel::MaterialListItem* listItem = m_materialListItemLookUp[hMaterial];
        listItem->setWidgetValue(
            static_cast<int>(UiLeftPanel::ID::MATERIAL_EMISSIVITY),
            emissivity
        );
        break;
    }
    case UiLeftPanel::ID::MATERIAL_EMISSIVITY:
    {
        float emissivity = getEventValue<float>(*materialListCtx);
        std::vector<float> emissivities = SpMaterial::getEmissivities(hMaterial);
        if (materialListItemIdx >= emissivities.size())
            break;
        emissivities[materialListItemIdx] = emissivity;
        DbUtils::txnFn(db, SpMaterial::setEmissivities, hMaterial, emissivities);
        break;
    }
    default:
        break;
    }
}

void PathTracerApp::handleRightPanelEvent(const GuiEvent& event) {
    GfxRenderer renderer = m_window->getRenderer();
    auto db = AppDataManager::instance().getDB();
    DbObjHandle hScene = db->getRootObject();
    if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME) {
        Logger() << "Invalid scene handle";
        return;
    }
    switch (static_cast<UiRightPanel::ID>(event.widgetID)) {
    case UiRightPanel::ID::TRACE_DEPTH:
    {
        if (event.type == GuiEventType::COMMIT) {
            int depth = getEventValue<int>(event);
            DbUtils::txnFn(db, PtScene::setTraceDepth, hScene, depth);
        }
        break;
    }
    case UiRightPanel::ID::RES_X:
    {
        int resX = 0, resY = 0;
        PtScene::getResolution(hScene, resX, resY);
        resX = getEventValue<int>(event);
        if (event.type != GuiEventType::COMMIT) {
            GfxImage frameImage = m_previewer->getFrameImage();
            m_previewer->setResolutionQuick(resX, resY);
            updatePreviewerFrameImage(frameImage);
        } else
            DbUtils::txnFn(db, PtScene::setResolution, hScene, resX, resY);
        break;
    }
    case UiRightPanel::ID::RES_Y:
    {
        int resX = 0, resY = 0;
        PtScene::getResolution(hScene, resX, resY);
        resY = getEventValue<int>(event);
        if (event.type != GuiEventType::COMMIT) {
            GfxImage frameImage = m_previewer->getFrameImage();
            m_previewer->setResolutionQuick(resX, resY);
            updatePreviewerFrameImage(frameImage);
        } else
            DbUtils::txnFn(db, PtScene::setResolution, hScene, resX, resY);
        break;
    }
    case UiRightPanel::ID::FOCUS_DIST:
    {
        if (event.type == GuiEventType::COMMIT) {
            PtScene::Camera camera = PtScene::getCamera(hScene);
            float focusDist = getEventValue<float>(event);
            camera.focusDist = focusDist;
            DbUtils::txnFn(db, PtScene::setCamera, hScene, camera);
        }
        break;
    }
    case UiRightPanel::ID::F_STOP:
    {
        if (event.type == GuiEventType::COMMIT) {
            PtScene::Camera camera = PtScene::getCamera(hScene);
            float fStop = getEventValue<float>(event);
            camera.fStop = fStop;
            DbUtils::txnFn(db, PtScene::setCamera, hScene, camera);
        }
        break;
    }
    case UiRightPanel::ID::CAM_POS:
    {
        PtScene::Camera camera = PtScene::getCamera(hScene);
        camera.position =
            AppUiUtils::arrayToVec3(getEventValue<std::array<float, 3>>(event));
        if (event.type != GuiEventType::COMMIT)
            m_previewer->setCameraQuick(camera.position, camera.rotation);
        else
            DbUtils::txnFn(db, PtScene::setCamera, hScene, camera);
        break;
    }
    case UiRightPanel::ID::CAM_ROT:
    {
        PtScene::Camera camera = PtScene::getCamera(hScene);
        camera.rotation =
            AppUiUtils::arrayToVec3(getEventValue<std::array<float, 3>>(event));
        if (event.type != GuiEventType::COMMIT)
            m_previewer->setCameraQuick(camera.position, camera.rotation);
        else
            DbUtils::txnFn(db, PtScene::setCamera, hScene, camera);
        break;
    }
    case UiRightPanel::ID::ADD_MODEL:
    {
        loadModel();
        break;
    }
    case UiRightPanel::ID::MODEL_LIST:
    {
        handleModelListEvent(event);
        break;
    }
    case UiRightPanel::ID::MODEL_ITEM_PASTE:
    {
        pasteCopiedModels();
        break;
    }
    case UiRightPanel::ID::MODEL_ITEM_SELECT_ALL:
    {
        selectAllModels();
        break;
    }
    default:
        break;
    }
}

void PathTracerApp::handleModelListEvent(const GuiEvent& event) {
    auto& modelListCtx = event.listViewCtx;
    if (modelListCtx == nullptr)
        return;

    auto db = AppDataManager::instance().getDB();
    DbObjHandle hScene = db->getRootObject();
    if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME) {
        Logger() << "Invalid scene handle";
        return;
    }
    std::vector<DbObjHandle> modelHandles = PtScene::getModels(hScene);

    int modelListItemIdx = getEventValue<int>(event);
    if (modelListItemIdx >= modelHandles.size())
        return;
    DbObjHandle hModel = modelHandles[modelListItemIdx];

    switch (static_cast<UiRightPanel::ID>(modelListCtx->widgetID)) {
    case UiRightPanel::ID::MODEL_NAME:
    {
        if (modelListCtx->type == GuiEventType::COMMIT) {
            std::string name = getEventValue<std::string>(*modelListCtx);
            DbUtils::txnFn(db, PtModel::setName, hModel, name);
        }
        break;
    }
    case UiRightPanel::ID::MODEL_LOCATION:
    {
        Math::Vec3 location =
            AppUiUtils::arrayToVec3(getEventValue<std::array<float, 3>>(*modelListCtx));
        if (modelListCtx->type != GuiEventType::COMMIT) {
            Previewer::ModelXform xform;
            xform.location = location;
            m_previewer->setModelXformQuick(hModel, xform);
        } else
            DbUtils::txnFn(db, PtModel::setLocation, hModel, location);
        break;
    }
    case UiRightPanel::ID::MODEL_ROTATION:
    {
        Math::Vec3 rotation =
            AppUiUtils::arrayToVec3(getEventValue<std::array<float, 3>>(*modelListCtx));
        if (modelListCtx->type != GuiEventType::COMMIT) {
            Previewer::ModelXform xform;
            xform.rotation = rotation;
            m_previewer->setModelXformQuick(hModel, xform);
        } else
            DbUtils::txnFn(db, PtModel::setRotation, hModel, rotation);
        break;
    }
    case UiRightPanel::ID::MODEL_SCALE:
    {
        Math::Vec3 scale =
            AppUiUtils::arrayToVec3(getEventValue<std::array<float, 3>>(*modelListCtx));
        const int scaleLockID = static_cast<int>(UiRightPanel::ID::MODEL_SCALE_LOCK);
        const int scaleID = static_cast<int>(UiRightPanel::ID::MODEL_SCALE);
        if (m_modelUiListItemLookUp.count(hModel) == 0)
            break;
        UiRightPanel::ModelListItem* listItem = m_modelUiListItemLookUp[hModel];
        Math::Vec3 scaleOld =
            AppUiUtils::arrayToVec3(listItem->getWidgetValue<std::array<float, 3>>(scaleID));
        auto clampNonZero = [](float v) {
            constexpr float EPS = std::numeric_limits<float>::epsilon();
            return (std::abs(v) < EPS) ? (v < 0 ? -EPS : EPS) : v;
            };
        if (listItem->getWidgetValue<int>(scaleLockID) == 1) {
            if (scale.x != scaleOld.x) {
                scale.x = clampNonZero(scale.x);
                float r = scale.x / clampNonZero(scaleOld.x);
                scale.y = scaleOld.y * r;
                scale.z = scaleOld.z * r;
            } else if (scale.y != scaleOld.y) {
                scale.y = clampNonZero(scale.y);
                float r = scale.y / clampNonZero(scaleOld.y);
                scale.x = scaleOld.x * r;
                scale.z = scaleOld.z * r;
            } else if (scale.z != scaleOld.z) {
                scale.z = clampNonZero(scale.z);
                float r = scale.z / clampNonZero(scaleOld.z);
                scale.x = scaleOld.x * r;
                scale.y = scaleOld.y * r;
            }
        }
        listItem->setWidgetValue(scaleID, AppUiUtils::vec3ToArray(scale));
        if (modelListCtx->type != GuiEventType::COMMIT) {
            Previewer::ModelXform xform;
            xform.scale = scale;
            m_previewer->setModelXformQuick(hModel, xform);
        } else
            DbUtils::txnFn(db, PtModel::setScale, hModel, scale);
        break;
    }
    case UiRightPanel::ID::MESH_LIST:
    {
        handleMeshListEvent(*modelListCtx, hModel);
        break;
    }
    case UiRightPanel::ID::MODEL_ITEM_CONTEXT:
    {
        if (m_selectedModels.size() == 1 && *m_selectedModels.begin() == hModel)
            break;
        if (m_modelUiListItemLookUp.count(hModel) == 0)
            break;

        if (m_selectedModels.find(hModel) == m_selectedModels.end()) {
            m_rightPanel->modelListView.clearSelection();
            m_selectedModels.clear();
        }

        UiRightPanel::ModelListItem* listItem = m_modelUiListItemLookUp[hModel];
        m_rightPanel->modelListView.selectItem(listItem->getIndex(), true);
        m_selectedModels.insert(hModel);

        m_focusedModel = hModel;

        break;
    }
    case UiRightPanel::ID::MODEL_ITEM_SELECTED:
    {
        selectModel(hModel);
        break;
    }
    case UiRightPanel::ID::MODEL_ITEM_HOVERED:
    {
        m_previewer->hightlightObject(hModel, Previewer::HightlightState::HOVERED);
        break;
    }
    case UiRightPanel::ID::MODEL_ITEM_CUT:
    {
        cutSeletedModels();
        break;
    }
    case UiRightPanel::ID::MODEL_ITEM_COPY:
    {
        copySeletedModels();
        break;
    }
    case UiRightPanel::ID::MODEL_ITEM_DELETE:
    {
        deleteSeletedModels();
        break;
    }
    case UiRightPanel::ID::MODEL_ITEM_SELECT_ALL:
    {
        selectAllModels();
        break;
    }
    default:
        break;
    }
}

void PathTracerApp::handleMeshListEvent(const GuiEvent& event, const DbObjHandle& hModel) {
    auto& meshListCtx = event.listViewCtx;
    if (meshListCtx == nullptr)
        return;

    auto db = AppDataManager::instance().getDB();
    std::vector<DbObjHandle> meshHandles = PtModel::getMeshes(hModel);
    int meshListItemIdx = getEventValue<int>(event);
    if (meshListItemIdx >= meshHandles.size())
        return;
    DbObjHandle hMesh = meshHandles[meshListItemIdx];
    DbObjHandle hMaterial = PtMesh::getMaterial(hMesh);
    Previewer::MaterialInfo materialInfo;

    switch (static_cast<UiRightPanel::ID>(meshListCtx->widgetID)) {
    case UiRightPanel::ID::MESH_ITEM_HOVERED:
    {
        m_previewer->hightlightObject(hMesh, Previewer::HightlightState::HOVERED);
        break;
    }
    case UiRightPanel::ID::MESH_NAME:
    {
        if (meshListCtx->type == GuiEventType::COMMIT) {
            std::string name = getEventValue<std::string>(*meshListCtx);
            DbUtils::txnFn(db, PtMesh::setName, hMesh, name);
        }
        break;
    }
    case UiRightPanel::ID::MESH_MATERIAL_TYPE:
    {
        PtMaterial::MaterialType type =
            static_cast<PtMaterial::MaterialType>(getEventValue<int>(*meshListCtx));
        DbUtils::txnFn(db, PtMaterial::setType, hMaterial, type);
        break;
    }
    case UiRightPanel::ID::MESH_ROUGHNESS:
    {
        float roughness = getEventValue<float>(*meshListCtx);
        if (meshListCtx->type != GuiEventType::COMMIT) {
            materialInfo.roughness = roughness;
            m_previewer->setMeshMaterialQuick(hMesh, materialInfo);
        } else
            DbUtils::txnFn(db, PtMaterial::setRoughness, hMaterial, roughness);
        break;
    }
    case UiRightPanel::ID::MESH_MATERIAL:
    {
        int value = getEventValue<int>(*meshListCtx);
        DbObjHandle hScene = db->getRootObject();
        std::vector<DbObjHandle> materialHandles = PtScene::getSpectrumMaterials(hScene);
        if (value < 0 || value >= materialHandles.size())
            break;
        DbObjHandle hSpMaterial = materialHandles[value];
        DbUtils::txnFn(db, PtMaterial::setSpectrumMaterial, hMaterial, hSpMaterial);
        break;
    }
    case UiRightPanel::ID::MESH_TEMPERATURE:
    {
        float intensity = getEventValue<float>(*meshListCtx);
        if (meshListCtx->type == GuiEventType::COMMIT)
            DbUtils::txnFn(db, PtMaterial::setTemperature, hMaterial, intensity);
        break;
    }
    case UiRightPanel::ID::MESH_IOR:
    {
        if (meshListCtx->type == GuiEventType::COMMIT) {
            float ior = getEventValue<float>(*meshListCtx);
            DbUtils::txnFn(db, PtMaterial::setIOR, hMaterial, ior);
        }
        break;
    }
    default:
        handleMeshListTextureEvent(*meshListCtx, hMaterial);
        break;
    }
}

void PathTracerApp::handleMeshListTextureEvent
(
    const GuiEvent& event,
    const DbObjHandle& hMaterial
) {
    switch (static_cast<UiRightPanel::ID>(event.widgetID)) {
    case UiRightPanel::ID::MESH_NORMAL_TEX_LOAD:
    {
        loadMeshTexture(
            hMaterial,
            PtMaterial::MaterialFlag::NORMAL_MAP,
            PtMaterial::setNormalTexPath
        );
        break;
    }
    case UiRightPanel::ID::MESH_NORMAL_TEX_REMOVE:
    {
        removeMeshTexture(
            hMaterial,
            PtMaterial::MaterialFlag::NORMAL_MAP,
            PtMaterial::setNormalTexPath
        );
        break;
    }
    case UiRightPanel::ID::MESH_ROUGHNESS_TEX_LOAD:
    {
        loadMeshTexture(
            hMaterial,
            PtMaterial::MaterialFlag::ROUGHNESS_MAP,
            PtMaterial::setRoughnessTexPath
        );
        break;
    }
    case UiRightPanel::ID::MESH_ROUGHNESS_TEX_REMOVE:
    {
        removeMeshTexture(
            hMaterial,
            PtMaterial::MaterialFlag::ROUGHNESS_MAP,
            PtMaterial::setRoughnessTexPath
        );
        break;
    }
    case UiRightPanel::ID::MESH_TEMPERATURE_TEX_LOAD:
    {
        const char* filters[5] = { "*.txt" };
        const char* filename = tinyfd_openFileDialog(
            GuiText::get("load_txt_dialog.title").c_str(),
            "",
            1,
            filters,
            GuiText::get("load_txt_dialog.filter_desc").c_str(),
            0
        );
        if (!filename)
            break;
        std::string texturePath = filename;
        auto materialFlags = PtMaterial::getFlags(hMaterial);
        materialFlags.set(PtMaterial::MaterialFlag::TEMPERATURE_MAP);
        DbUtils::TxnGuard txnGuard(AppDataManager::instance().getDB());
        if (PtMaterial::setFlags(hMaterial, materialFlags) != DB::Result::SUCCESS)
            break;
        if (PtMaterial::setTemperatureTexPath(hMaterial, texturePath) != DB::Result::SUCCESS)
            break;
        txnGuard.commit();
        updateUiMeshListItem(hMaterial);
        m_previewer->updateObjects({ hMaterial });
        break;
    }
    case UiRightPanel::ID::MESH_TEMPERATURE_TEX_REMOVE:
    {
        removeMeshTexture(
            hMaterial,
            PtMaterial::MaterialFlag::TEMPERATURE_MAP,
            PtMaterial::setTemperatureTexPath
        );
        break;
    }
    default:
        break;
    }
}

void PathTracerApp::handleSettingsWindowEvent(const GuiEvent& event) {
    switch (static_cast<UiSettingsWindow::ID>(event.widgetID)) {
    case UiSettingsWindow::ID::LANGUAGE:
    {
        UiSettingsWindow::Language langConfig =
            static_cast<UiSettingsWindow::Language>(getEventValue<int>(event));
        LangStrings::Lang language = LangStrings::Lang::EN_US;
        switch (langConfig) {
        case UiSettingsWindow::Language::EN_US:
            language = LangStrings::Lang::EN_US;
            break;
        case UiSettingsWindow::Language::ZH_CN:
            language = LangStrings::Lang::ZH_CN;
            break;
        default:
            break;
        }
        AppConfig::instance().setConfig(
            "general_lang",
            std::to_string(static_cast<int>(language))
        );
        GuiText::load(LangStrings::get(language));
        break;
    }
    case UiSettingsWindow::ID::MSAA:
    {
        UiSettingsWindow::MsaaLevel msaaConfig =
            static_cast<UiSettingsWindow::MsaaLevel>(getEventValue<int>(event));
        int level = 1;
        switch (msaaConfig) {
        case UiSettingsWindow::MsaaLevel::MSAA_1X:
            level = 1;
            break;
        case UiSettingsWindow::MsaaLevel::MSAA_2X:
            level = 2;
            break;
        case UiSettingsWindow::MsaaLevel::MSAA_4X:
            level = 4;
            break;
        case UiSettingsWindow::MsaaLevel::MSAA_8X:
            level = 8;
            break;
        default:
            break;
        }
        AppConfig::instance().setConfig(
            "previewer_samples",
            std::to_string(static_cast<int>(level))
        );
        m_previewer->setMSAASamples(level);
        setDisplayMode(m_displayMode);
        break;
    }
    case UiSettingsWindow::ID::CAM_NAV_SPEED:
    {
        int speed = getEventValue<int>(event);
        AppConfig::instance().setConfig(
            "previewer_cam_move_speed",
            std::to_string(speed)
        );
        m_previewer->setCameraMoveSpeed(static_cast<float>(speed) * 0.1f);
        break;
    }
    case UiSettingsWindow::ID::BACKGROUND_COLOR:
    {
        Math::Vec3 color =
            AppUiUtils::arrayToVec3(getEventValue<std::array<float, 3>>(event));
        AppConfig::instance().setConfig(
            "previewer_bg_color",
            AppConfigUitls::Vec3ToString(color)
        );
        m_previewer->setBackgroundColor(color);
        break;
    }
    case UiSettingsWindow::ID::HOVERED_COLOR:
    {
        Math::Vec3 color =
            AppUiUtils::arrayToVec3(getEventValue<std::array<float, 3>>(event));
        AppConfig::instance().setConfig(
            "previewer_hilite_color_hovered",
            AppConfigUitls::Vec3ToString(color)
        );
        std::string hiliteColorPickedStr =
            AppConfig::instance().getConfig("previewer_hilite_color_picked");
        Math::Vec3 hiliteColorPicked =
            hiliteColorPickedStr.empty() ? Math::Vec3(0.1f, 0.7f, 0.9f) :
            AppConfigUitls::StringToVec3(hiliteColorPickedStr);
        m_previewer->setHighlightColors(color, hiliteColorPicked);
        break;
    }
    case UiSettingsWindow::ID::PICKED_COLOR:
    {
        Math::Vec3 color =
            AppUiUtils::arrayToVec3(getEventValue<std::array<float, 3>>(event));
        AppConfig::instance().setConfig(
            "previewer_hilite_color_picked",
            AppConfigUitls::Vec3ToString(color)
        );
        std::string hiliteColorHoveredStr =
            AppConfig::instance().getConfig("previewer_hilite_color_hovered");
        Math::Vec3 hiliteColorHovered =
            hiliteColorHoveredStr.empty() ? Math::Vec3(0.9f, 0.9f, 0.1f) :
            AppConfigUitls::StringToVec3(hiliteColorHoveredStr);
        m_previewer->setHighlightColors(hiliteColorHovered, color);
        break;
    }
    case UiSettingsWindow::ID::RESET_DEFAULTS:
    {
        resetDefaultConfig();
        break;
    }
    default:
        break;
    }
}

void PathTracerApp::resetDefaultConfig() {
    AppConfig::instance().setConfig(
        "general_lang",
        std::to_string(static_cast<int>(LangStrings::Lang::EN_US))
    );
    GuiText::load(LangStrings::get(LangStrings::Lang::EN_US));
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::LANGUAGE),
        static_cast<int>(UiSettingsWindow::Language::EN_US)
    );
    AppConfig::instance().setConfig(
        "previewer_samples",
        std::to_string(1)
    );
    m_previewer->setMSAASamples(1);
    setDisplayMode(m_displayMode);
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::MSAA),
        static_cast<int>(UiSettingsWindow::MsaaLevel::MSAA_1X)
    );
    AppConfig::instance().setConfig(
        "previewer_cam_move_speed",
        std::to_string(3)
    );
    m_previewer->setCameraMoveSpeed(0.3f);
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::CAM_NAV_SPEED),
        3
    );
    Math::Vec3 bgColor(0.0f);
    AppConfig::instance().setConfig(
        "previewer_bg_color",
        AppConfigUitls::Vec3ToString(bgColor)
    );
    m_previewer->setBackgroundColor(bgColor);
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::BACKGROUND_COLOR),
        AppUiUtils::vec3ToArray(bgColor)
    );
    Math::Vec3 hoverColor(0.9f, 0.9f, 0.1f);
    AppConfig::instance().setConfig(
        "previewer_hilite_color_hovered",
        AppConfigUitls::Vec3ToString(hoverColor)
    );
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::HOVERED_COLOR),
        AppUiUtils::vec3ToArray(hoverColor)
    );
    Math::Vec3 pickColor(0.1f, 0.7f, 0.9f);
    AppConfig::instance().setConfig(
        "previewer_hilite_color_picked",
        AppConfigUitls::Vec3ToString(pickColor)
    );
    m_settingsWindow->setWidgetValue(
        static_cast<int>(UiSettingsWindow::ID::PICKED_COLOR),
        AppUiUtils::vec3ToArray(pickColor)
    );
    m_previewer->setHighlightColors(hoverColor, pickColor);
}

void PathTracerApp::setDisplayMode(DisplayMode displayMode) {
    m_displayMode = displayMode;
    m_toolBar->setWidgetValue(
        static_cast<int>(UiToolBar::ID::DISPLAY_MODE),
        static_cast<int>(m_displayMode)
    );
    if (m_displayMode == DisplayMode::PREVIEW_MODE) {
        m_mainViewport->frameTexture = AppUiManager::instance().getImGuiTexture(
            m_window->getRenderer(),
            m_previewer->getFrameImage()
        );
    } else {
        m_mainViewport->frameTexture = AppUiManager::instance().getImGuiTexture(
            m_window->getRenderer(),
            m_postProcesser->getOutputImage()
        );
    }
}

int PathTracerApp::loadModelUtil(const std::string& filename) {
    DbObjHandle hModel{};
    try {
        auto db = AppDataManager::instance().getDB();
        DbUtils::TxnGuard txnGuard(db);

        hModel = db->objCreate<PtModel>({});

        DbObjHandle hScene = db->getRootObject();
        if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME)
            throw std::exception();
        if (PtScene::addModel(hScene, hModel) != DB::Result::SUCCESS)
            throw std::exception();

        Mesh::Model modelData = {};
        if (MeshLoader::getInfoFromOBJ(filename, modelData))
            throw std::exception();
        if (PtModel::setName(hModel, modelData.name) != DB::Result::SUCCESS)
            throw std::exception();
        if (PtModel::setFilePath(hModel, filename) != DB::Result::SUCCESS)
            throw std::exception();

        std::vector<DbObjHandle> meshHandles;
        meshHandles.reserve(modelData.meshes.size());
        std::string strKey = "right_panel.mesh_node.default_name";
        std::string defaultMeshName = GuiText::get(strKey);
        int posCode = 0;
        for (const auto& meshData : modelData.meshes) {
            for (int i = 0; i < meshData.submeshes.size(); i++) {
                posCode++;
                const Mesh::SubMesh& submeshData = meshData.submeshes[i];
                std::string name = submeshData.name;
                if (name.empty()) {
                    name = meshData.name;
                    if (!name.empty() && meshData.submeshes.size() > 1)
                        name += std::to_string(i + 1);
                    if (name.empty())
                        name = defaultMeshName + std::to_string(posCode);
                }

                DbObjHandle hMesh = db->objCreate<PtMesh>({});
                if (!hMesh.isValid())
                    throw std::exception();
                if (PtMesh::setModel(hMesh, hModel) != DB::Result::SUCCESS)
                    throw std::exception();
                if (PtMesh::setName(hMesh, name) != DB::Result::SUCCESS)
                    throw std::exception();

                DbObjHandle hMaterial = db->objCreate<PtMaterial>({});
                if (!hMaterial.isValid())
                    throw std::exception();
                if (PtMaterial::setMesh(hMaterial, hMesh) != DB::Result::SUCCESS)
                    throw std::exception();

                if (PtMesh::setMaterial(hMesh, hMaterial) != DB::Result::SUCCESS)
                    throw std::exception();

                meshHandles.push_back(hMesh);
            }
        }
        if (PtModel::setMeshes(hModel, meshHandles) != DB::Result::SUCCESS)
            throw std::exception();

        txnGuard.commit();
    } catch (...) {
        Logger() << "Failed to load model from file: " << filename;
        return 1;
    }

    updateUiModelListItem(hModel);
    m_previewer->updateObjects({ hModel });
    m_nTriangles = m_previewer->countTriangles();
    return 0;
}

int PathTracerApp::loadSpectrumWavesFromTXT(const std::string &filename) {
    auto db = AppDataManager::instance().getDB();
    DbObjHandle hScene = db->getRootObject();
    if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME) {
        Logger() << "Invalid scene handle";
        return 1;
    }

    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        Logger() << "Failed to open txt file: " << filename;
        return 1;
    }
    std::vector<float> values;
    float v = 0.0f;
    while (ifs >> v)
        values.push_back(v);
    if (values.empty()) {
        Logger() << "No valid float values found in file.";
        return 1;
    }

    try {
        DbUtils::TxnGuard txnGuard(db);
        for (float waveNumber : values) {
            DbObjHandle hWave = db->objCreate<SpWave>({});
            if (!hWave.isValid())
                throw std::exception();
            if (SpWave::setWaveNumber(hWave, waveNumber) != DB::Result::SUCCESS)
                throw std::exception();
            if (PtScene::addWave(hScene, hWave) != DB::Result::SUCCESS)
                throw std::exception();
        }
        txnGuard.commit();
    } catch (...) {
        Logger() << "Failed to load wave";
        return 1;
    }

    return 0;
}

int PathTracerApp::loadSpectrumMaterialsFromTXT(const std::string &filename) {
    auto db = AppDataManager::instance().getDB();
    DbObjHandle hScene = db->getRootObject();
    if (!hScene.isValid() || hScene.getType() != PtScene::TYPE_NAME) {
        Logger() << "Invalid scene handle";
        return 1;
    }

    // Check if there are waves in the scene, as emissivity values depend on the wave count
    std::vector<DbObjHandle> waveHandles = PtScene::getWaves(hScene);
    const size_t waveCount = waveHandles.size();
    if (waveCount == 0)
        return 1;

    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        Logger() << "Failed to open txt file: " << filename;
        return 1;
    }

    try {
        DbUtils::TxnGuard txnGuard(db);
        
        std::string nameLine;
        std::string valueLine;
        while (true) {
            // Read a line for the material name
            if (!std::getline(ifs, nameLine))
                break;
            if (nameLine.empty())
                continue;

            // Read the next line for emissivity values
            if (!std::getline(ifs, valueLine))
                throw std::exception();
            std::vector<float> emissivities;
            {
                std::stringstream ss(valueLine);
                float v;
                while (ss >> v)
                    emissivities.push_back(v);
            }

            // Ensure the emissivities vector has the same number of elements as the wave count
            if (emissivities.size() > waveCount)
                emissivities.resize(waveCount);
            else if (emissivities.size() < waveCount)
                emissivities.resize(waveCount, 0.0f);

            // Create the SpMaterial object and set its properties
            DbObjHandle hMaterial = db->objCreate<SpMaterial>({});
            if (!hMaterial.isValid())
                throw std::exception();
            if (SpMaterial::setName(hMaterial, nameLine) != DB::Result::SUCCESS)
                throw std::exception();
            if (SpMaterial::setEmissivities(hMaterial, emissivities) != DB::Result::SUCCESS)
                throw std::exception();

            // Add the material to the scene
            if (PtScene::addSpectrumMaterial(hScene, hMaterial) != DB::Result::SUCCESS)
                throw std::exception();
        }

        txnGuard.commit();
    } catch (...) {
        Logger() << "Failed to load material";
        return 1;
    }

    return 0;
}

std::string PathTracerApp::loadImage() {
    const char* filters[5] = { "*.jpg", "*.jpeg", "*.png", "*.bmp", "*.tga" };
    const char* filename = tinyfd_openFileDialog(
        GuiText::get("load_image_dialog.title").c_str(),
        "",
        1,
        filters,
        GuiText::get("load_image_dialog.filter_desc").c_str(),
        0
    );
    if (!filename)
        return {};
    return filename;
}
