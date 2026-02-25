/**
 * @file GuiWindowGLFW.cpp
 * @brief Implementation of the GUI window using GLFW.
 */

#include <vulkan/vulkan.h>

#include "gui/impl/GuiWindowGLFW.h"

GuiGLFWManager::GuiGLFWManager() {
    if (glfwInit() != GLFW_TRUE)
        return;
}

GuiGLFWManager::~GuiGLFWManager() {
    glfwTerminate();
}

GuiWindowGLFW::GuiWindowGLFW
(
    GuiWindow* owner,
    const std::string& title,
    int width,
    int height,
    int samples,
    GfxRenderer& renderer
) : GuiWindowImpl(owner, title, width, height, samples) {
    GuiGLFWManager::instance(); // Ensure GLFW is initialized

    GLFWwindow* mainWindow = GuiGLFWManager::instance().getMainWindow();

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    if (GuiConfig::getGraphicsBackend() == GfxBackend::OpenGL) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, samples);
        m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, mainWindow);
        if (!m_window)
            return; // Failed to create GLFW window
        m_backend = GfxBackend::OpenGL;
        glfwMakeContextCurrent(m_window);

        GfxGLRendererConfig config;
        config.proc = reinterpret_cast<void*>(glfwGetProcAddress);
        if (GfxRendererFactory::instance().initGlobal(GfxBackend::OpenGL, config))
            return; // Failed to initialize OpenGL backend
        renderer = GfxRendererFactory::instance().create(GfxBackend::OpenGL);
        if (!renderer)
            return; // Failed to create OpenGL renderer
    } else if (GuiConfig::getGraphicsBackend() == GfxBackend::Vulkan) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
        if (!m_window)
            return; // Failed to create GLFW window
        m_backend = GfxBackend::Vulkan;

        GfxVulkanRendererConfig config;
        config.appName = m_title;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        config.extensions.reserve(glfwExtensionCount);
        for (uint32_t i = 0; i < glfwExtensionCount; ++i)
            config.extensions.push_back(glfwExtensions[i]);
        if (GfxRendererFactory::instance().initGlobal(GfxBackend::Vulkan, config))
            return; // Failed to initialize Vulkan backend
        renderer = GfxRendererFactory::instance().create(GfxBackend::Vulkan);
        if (!renderer)
            return; // Failed to create Vulkan renderer

        if (width > 0 && height > 0) {
            VkInstance vulkanInstance = static_cast<VkInstance>(renderer->getVulkanInstance());
            if (!vulkanInstance)
                return; // Vulkan instance not available
            VkSurfaceKHR surface = nullptr;
            if (glfwCreateWindowSurface(vulkanInstance, m_window, nullptr, &surface) != VK_SUCCESS)
                return; // Failed to create Vulkan surface
            renderer->setVulkanSurface(surface);
            if (renderer->setSwapchainSize(m_width, m_height))
                return; // Failed to set swapchain size
        }
    } else
        return; // Unsupported graphics backend
    renderer->setSamples(samples);

    if (!mainWindow)
        GuiGLFWManager::instance().setMainWindow(m_window);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, &framebufferSizeCb);
    glfwSetWindowCloseCallback(m_window, &windowCloseCb);
    glfwSetWindowFocusCallback(m_window, &windowFocusCb);
    glfwSetWindowIconifyCallback(m_window, &windowIconifyCb);
    glfwSetKeyCallback(m_window, &keyCb);
    glfwSetMouseButtonCallback(m_window, &mouseButtonCb);
    glfwSetScrollCallback(m_window, &scrollCb);
    glfwSetCursorPosCallback(m_window, &cursorPosCb);
    glfwSetCursorEnterCallback(m_window, &cursorEnterCb);
    glfwSetDropCallback(m_window, &dropCb);
}

GuiWindowGLFW::~GuiWindowGLFW() {
    if (m_window) {
        if (m_window == GuiGLFWManager::instance().getMainWindow())
            GuiGLFWManager::instance().setMainWindow(nullptr);
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

void GuiWindowGLFW::setWindowSize(int width, int height) {
    if (m_window) {
        glfwSetWindowSize(m_window, width, height);
        m_width = width;
        m_height = height;
    }
}

void GuiWindowGLFW::show() const {
    if (m_window) {
        glfwShowWindow(m_window);
        if (m_backend == GfxBackend::OpenGL)
            glfwMakeContextCurrent(m_window);
    }
}

void GuiWindowGLFW::hide() const {
    if (m_window)
        glfwHideWindow(m_window);
}

void GuiWindowGLFW::term() const {
    if (m_window)
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

bool GuiWindowGLFW::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

float GuiWindowGLFW::getContentScale() const {
    if (m_window) {
        float xscale = 1.0f, yscale = 1.0f;
        glfwGetWindowContentScale(m_window, &xscale, &yscale);
        return (xscale + yscale) * 0.5f;
    }
    return 1.0f;
}

void GuiWindowGLFW::getMousePos(double& x, double& y) const {
    if (m_window)
        glfwGetCursorPos(m_window, &x, &y);
}

int GuiWindowGLFW::beginFrame() const {
    if (m_backend == GfxBackend::OpenGL)
        glfwMakeContextCurrent(m_window);
    glfwPollEvents();
    return 0;
}

int GuiWindowGLFW::endFrame() const {
    if (m_backend == GfxBackend::OpenGL) {
        glfwMakeContextCurrent(m_window);
        glfwSwapBuffers(m_window);
    }
    return 0;
}

void GuiWindowGLFW::setWindowIcon(const std::vector<GuiIcon>& icons) {
    if (m_window && !icons.empty()) {
        std::vector<GLFWimage> glfwIcons;
        glfwIcons.reserve(icons.size());
        for (const auto& icon : icons) {
            GLFWimage glfwIcon{};
            glfwIcon.width = icon.width;
            glfwIcon.height = icon.height;
            glfwIcon.pixels = const_cast<unsigned char*>(icon.data.data());
            glfwIcons.push_back(glfwIcon);
        }
        glfwSetWindowIcon(m_window, static_cast<int>(glfwIcons.size()), glfwIcons.data());
    }
}

void GuiWindowGLFW::setVSyncMode(GfxVSyncMode mode) {
    if (m_backend == GfxBackend::OpenGL) {
        if (mode == GfxVSyncMode::IMMEDIATE)
            glfwSwapInterval(0);
        else if (mode == GfxVSyncMode::FIFO)
            glfwSwapInterval(1);
        else if (mode == GfxVSyncMode::MAILBOX)
            glfwSwapInterval(1); // GLFW does not support MAILBOX, use FIFO instead
    }
}

void* GuiWindowGLFW::getNativeWindow() const {
    return m_window;
}

void GuiWindowGLFW::framebufferSizeCb(GLFWwindow* window, int width, int height) {
    GuiWindowGLFW* self = static_cast<GuiWindowGLFW*>(glfwGetWindowUserPointer(window));
    if (self && self->m_owner) {
        self->m_width = width;
        self->m_height = height;
        if (width == 0 || height == 0)
            return; // Ignore zero-sized framebuffer
        self->m_owner->getRenderer()->setSwapchainSize(width, height);
        self->m_owner->onResize(width, height);
    }
}

void GuiWindowGLFW::windowCloseCb(GLFWwindow* window) {
    GuiWindowGLFW* self = static_cast<GuiWindowGLFW*>(glfwGetWindowUserPointer(window));
    if (self && self->m_owner) {
        bool shouldClose = self->m_owner->onClose();
        glfwSetWindowShouldClose(window, shouldClose ? GLFW_TRUE : GLFW_FALSE);
    }
}

void GuiWindowGLFW::windowFocusCb(GLFWwindow* window, int focused) {
    GuiWindowGLFW* self = static_cast<GuiWindowGLFW*>(glfwGetWindowUserPointer(window));
    if (self && self->m_owner)
        self->m_owner->onFocuse(focused == GLFW_TRUE);
}

void GuiWindowGLFW::windowIconifyCb(GLFWwindow* window, int iconified) {
    GuiWindowGLFW* self = static_cast<GuiWindowGLFW*>(glfwGetWindowUserPointer(window));
    if (self && self->m_owner)
        self->m_owner->onResume(iconified == GLFW_FALSE);
}

void GuiWindowGLFW::keyCb(GLFWwindow* window, int key, int scancode, int action, int mods) {
    GuiWindowGLFW* self = static_cast<GuiWindowGLFW*>(glfwGetWindowUserPointer(window));
    if (self && self->m_owner) {
        GuiKey guiKey = static_cast<GuiKey>(key);
        bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
        GuiFlags<GuiModKey> modKeys{};
        if (mods & GLFW_MOD_SHIFT)
            modKeys.set(GuiModKey::SHIFT);
        if (mods & GLFW_MOD_CONTROL)
            modKeys.set(GuiModKey::CONTROL);
        if (mods & GLFW_MOD_ALT)
            modKeys.set(GuiModKey::ALT);
        if (mods & GLFW_MOD_SUPER)
            modKeys.set(GuiModKey::SUPER);
        if (mods & GLFW_MOD_CAPS_LOCK)
            modKeys.set(GuiModKey::CAPS_LOCK);
        if (mods & GLFW_MOD_NUM_LOCK)
            modKeys.set(GuiModKey::NUM_LOCK);
        self->m_owner->onKeyboardAction(guiKey, pressed, modKeys);
    }
}

void GuiWindowGLFW::mouseButtonCb(GLFWwindow* window, int button, int action, int mods) {
    GuiWindowGLFW* self = static_cast<GuiWindowGLFW*>(glfwGetWindowUserPointer(window));
    if (self && self->m_owner) {
        GuiMouseButton mouseButton = static_cast<GuiMouseButton>(button);
        bool pressed = (action == GLFW_PRESS);
        GuiFlags<GuiModKey> modKeys{};
        if (mods & GLFW_MOD_SHIFT)
            modKeys.set(GuiModKey::SHIFT);
        if (mods & GLFW_MOD_CONTROL)
            modKeys.set(GuiModKey::CONTROL);
        if (mods & GLFW_MOD_ALT)
            modKeys.set(GuiModKey::ALT);
        if (mods & GLFW_MOD_SUPER)
            modKeys.set(GuiModKey::SUPER);
        if (mods & GLFW_MOD_CAPS_LOCK)
            modKeys.set(GuiModKey::CAPS_LOCK);
        if (mods & GLFW_MOD_NUM_LOCK)
            modKeys.set(GuiModKey::NUM_LOCK);
        self->m_owner->onMouseButton(mouseButton, pressed, modKeys);
    }
}

void GuiWindowGLFW::scrollCb(GLFWwindow* window, double xoffset, double yoffset) {
    GuiWindowGLFW* self = static_cast<GuiWindowGLFW*>(glfwGetWindowUserPointer(window));
    if (self && self->m_owner)
        self->m_owner->onMouseScroll(xoffset, yoffset);
}

void GuiWindowGLFW::cursorPosCb(GLFWwindow* window, double xpos, double ypos) {
    GuiWindowGLFW* self = static_cast<GuiWindowGLFW*>(glfwGetWindowUserPointer(window));
    if (self && self->m_owner)
        self->m_owner->onMouseMove(xpos, ypos);
}

void GuiWindowGLFW::cursorEnterCb(GLFWwindow* window, int entered) {
    GuiWindowGLFW* self = static_cast<GuiWindowGLFW*>(glfwGetWindowUserPointer(window));
    if (self && self->m_owner)
        self->m_owner->onMouseEnter(entered == GLFW_TRUE);
}

void GuiWindowGLFW::dropCb(GLFWwindow* window, int count, const char** paths) {
    GuiWindowGLFW* self = static_cast<GuiWindowGLFW*>(glfwGetWindowUserPointer(window));
    if (self && self->m_owner && count > 0 && paths) {
        std::vector<std::string> filePaths;
        filePaths.reserve(count);
        for (int i = 0; i < count; ++i)
            filePaths.emplace_back(paths[i]);
        self->m_owner->onDrop(filePaths);
    }
}
