/**
 * @file GuiWindow.cpp
 * @brief Implementation of the GUI window class.
 */

#include "gui/GuiPr.h"
#include "gui/impl/GuiWindowGLFW.h"

std::string GuiConfig::s_appName = "";
GfxBackend GuiConfig::s_backend = GfxBackend::OpenGL;

/**
 * @brief Implementation details for the GuiWindow class.
 */
struct GuiWindow::Impl {
    std::unique_ptr<GuiWindowImpl> impl; // Pointer to the implementation of the GUI window
};

GuiWindow::GuiWindow(const std::string& title, int width, int height, int samples) :
    m_title(title),
    m_impl(std::make_unique<Impl>()) {
    m_onResizeCb = [](int, int) {};
    m_onDrawCb = []() {};
    m_onDrawFinishedCb = []() {};
    m_onCloseCb = []() { return true; };
    m_onFocuseCb = [](bool) {};
    m_onResumeCb = [](bool) {};
    m_onKeyboardActionCb = [](GuiKey, bool, GuiFlags<GuiModKey>) {};
    m_onMouseButtonCb = [](GuiMouseButton, bool, GuiFlags<GuiModKey>) {};
    m_onMouseScrollCb = [](double, double) {};
    m_onMouseMoveCb = [](double, double) {};
    m_onMouseEnterCb = [](bool) {};
    m_onDropCb = [](const std::vector<std::string>&) {};

    m_impl->impl =
        std::make_unique<GuiWindowGLFW>(this, title, width, height, samples, m_renderer);

    if (width > 0 && height > 0) {
        m_renderer->getPipelineStateMachine()->setViewport({ 0, 0, width, height });
        m_renderer->getPipelineStateMachine()->setScissor({ 0, 0, width, height });
    }

    m_views = {};
}

GuiWindow::~GuiWindow() = default; // Destructor must be defined after the Impl definition

void GuiWindow::setWindowSize(int width, int height) {
    if (!m_impl && !m_impl->impl)
        return;
    m_impl->impl->setWindowSize(width, height);
}

void GuiWindow::show() const {
    if (!m_impl && !m_impl->impl)
        return;
    m_impl->impl->show();
}

void GuiWindow::hide() const {
    if (!m_impl && !m_impl->impl)
        return;
    m_impl->impl->hide();
}

void GuiWindow::term() const {
    if (!m_impl && !m_impl->impl)
        return;
    m_impl->impl->term();
}

GfxRenderer GuiWindow::getRenderer() const {
    return m_renderer;
}

bool GuiWindow::shouldClose() const {
    if (!m_impl && !m_impl->impl)
        return true;
    return m_impl->impl->shouldClose();
}

float GuiWindow::getContentScale() const {
    if (!m_impl && !m_impl->impl)
        return 1.0f;
    return m_impl->impl->getContentScale();
}

void GuiWindow::getMousePos(double& x, double& y) const {
    if (!m_impl && !m_impl->impl)
        return;
    m_impl->impl->getMousePos(x, y);
}

int GuiWindow::drawFrame() {
    if (!m_impl && !m_impl->impl)
        return 1;

    if (m_impl->impl->beginFrame())
        return 1;

    if (m_impl->impl->m_width <= 0 || m_impl->impl->m_height <= 0)
        return 0;

    if (m_renderer->beginFrame())
        return 1;

    m_onDrawCb();

    for (const auto& view : m_views) {
        if (view)
            view->draw();
    }

    m_onDrawFinishedCb();

    if (m_renderer->endFrame())
        return 1;

    for (const auto& view : m_views) {
        if (view)
            view->notifyListeners();
    }

    return m_impl->impl->endFrame();
}

void* GuiWindow::getNativeWindow() const {
    if (!m_impl && !m_impl->impl)
        return nullptr;
    return m_impl->impl->getNativeWindow();
}

void GuiWindow::setOnResizeCb(const std::function<void(int, int)>& callback) {
    m_onResizeCb = callback;
}

void GuiWindow::setOnDrawCb(const std::function<void()>& callback) {
    m_onDrawCb = callback;
}

void GuiWindow::setOnDrawFinishedCb(const std::function<void()>& callback) {
    m_onDrawFinishedCb = callback;
}

void GuiWindow::setOnCloseCb(const std::function<bool()>& callback) {
    m_onCloseCb = callback;
}

void GuiWindow::setOnFocuseCb(const std::function<void(bool)>& callback) {
    m_onFocuseCb = callback;
}

void GuiWindow::setOnResumeCb(const std::function<void(bool)>& callback) {
    m_onResumeCb = callback;
}

void GuiWindow::setOnKeyboardActionCb(
    const std::function<void(GuiKey, bool, GuiFlags<GuiModKey>)>& callback
) {
    m_onKeyboardActionCb = callback;
}

void GuiWindow::setOnMouseButtonCb(
    const std::function<void(GuiMouseButton, bool, GuiFlags<GuiModKey>)>& callback
) {
    m_onMouseButtonCb = callback;
}

void GuiWindow::setOnMouseScrollCb(const std::function<void(double, double)>& callback) {
    m_onMouseScrollCb = callback;
}

void GuiWindow::setOnMouseMoveCb(const std::function<void(double, double)>& callback) {
    m_onMouseMoveCb = callback;
}

void GuiWindow::setOnMouseEnterCb(const std::function<void(bool)>& callback) {
    m_onMouseEnterCb = callback;
}

void GuiWindow::setOnDropCb(
    const std::function<void(const std::vector<std::string>&)>& callback
) {
    m_onDropCb = callback;
}

void GuiWindow::setWindowIcon(const std::vector<GuiIcon>& icons) {
    if (!m_impl && !m_impl->impl)
        return;
    m_impl->impl->setWindowIcon(icons);
}

void GuiWindow::setVSyncMode(GfxVSyncMode mode) {
    if (!m_impl && !m_impl->impl)
        return;
    m_impl->impl->setVSyncMode(mode);
    m_renderer->setVSyncMode(mode);
}

void GuiWindow::addView(std::shared_ptr<GuiView> view) {
    if (view)
        m_views.push_back(view);
}

void GuiWindow::onResize(int width, int height) {
    if (!m_impl && !m_impl->impl)
        return;
    m_onResizeCb(width, height);
    if (m_renderer->beginFrame())
        return;
    m_onDrawCb();
    for (const auto& view : m_views) {
        if (view)
            view->draw();
    }
    m_onDrawFinishedCb();
    if (m_renderer->endFrame())
        return;
    m_impl->impl->endFrame();
}

bool GuiWindow::onClose() {
    return m_onCloseCb();
}

void GuiWindow::onFocuse(bool focused) {
    m_onFocuseCb(focused);
}

void GuiWindow::onResume(bool resume) {
    m_onResumeCb(resume);
}

void GuiWindow::onKeyboardAction(GuiKey key, bool pressed, GuiFlags<GuiModKey> mod) {
    m_onKeyboardActionCb(key, pressed, mod);
}

void GuiWindow::onMouseButton(GuiMouseButton button, bool pressed, GuiFlags<GuiModKey> mod) {
    m_onMouseButtonCb(button, pressed, mod);
}

void GuiWindow::onMouseScroll(double x, double y) {
    m_onMouseScrollCb(x, y);
}

void GuiWindow::onMouseMove(double x, double y) {
    m_onMouseMoveCb(x, y);
}

void GuiWindow::onMouseEnter(bool entered) {
    m_onMouseEnterCb(entered);
}

void GuiWindow::onDrop(const std::vector<std::string>& paths) {
    m_onDropCb(paths);
}
