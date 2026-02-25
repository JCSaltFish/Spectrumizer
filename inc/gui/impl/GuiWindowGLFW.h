/**
 * @file GuiWindowGLFW.h
 * @brief Header file for the GUI window implementation using GLFW.
 */

#pragma once

#include <GLFW/glfw3.h>

#include "gui/GuiPr.h"

class GuiWindowGLFW;
/**
 * @brief Singleton manager for GLFW-related operations.
 */
class GuiGLFWManager {
private:
    GuiGLFWManager();
    ~GuiGLFWManager();
    GuiGLFWManager(const GuiGLFWManager&) = delete;
    GuiGLFWManager& operator=(const GuiGLFWManager&) = delete;
    GuiGLFWManager(GuiGLFWManager&&) = delete;
    GuiGLFWManager& operator=(GuiGLFWManager&&) = delete;

public:
    static GuiGLFWManager& instance() {
        static GuiGLFWManager instance;
        return instance;
    };

    /**
     * @brief Sets the main GLFW window.
     * @param window Pointer to the main GLFW window.
     */
    void setMainWindow(GLFWwindow* window) {
        m_mainWindow = window;
    };
    /**
     * @brief Gets the main GLFW window.
     * @return Pointer to the main GLFW window.
     */
    GLFWwindow* getMainWindow() const {
        return m_mainWindow;
    };

private:
    GLFWwindow* m_mainWindow = nullptr; // Pointer to the main GLFW window
};

/**
 * @brief Implementation of the GUI window using GLFW.
 */
class GuiWindowGLFW : public GuiWindowImpl {
public:
    GuiWindowGLFW
    (
        GuiWindow* owner,
        const std::string& title,
        int width,
        int height,
        int samples,
        GfxRenderer& renderer
    );
    ~GuiWindowGLFW() override;

    void setWindowSize(int width, int height) override;
    void show() const override;
    void hide() const override;
    void term() const override;
    bool shouldClose() const override;
    float getContentScale() const override;
    void getMousePos(double& x, double& y) const override;

    int beginFrame() const override;
    int endFrame() const override;

    void setWindowIcon(const std::vector<GuiIcon>& icons) override;

    void setVSyncMode(GfxVSyncMode mode) override;

    void* getNativeWindow() const override;

    /**
     * @brief Callback function for framebuffer size changes.
     * @param window Pointer to the GLFW window.
     * @param width New width of the framebuffer.
     * @param height New height of the framebuffer.
     */
    static void framebufferSizeCb(GLFWwindow* window, int width, int height);
    /**
     * @brief Callback function for window close events.
     * @param window Pointer to the GLFW window.
     */
    static void windowCloseCb(GLFWwindow* window);
    /**
     * @brief Callback function for window focus events.
     * @param window Pointer to the GLFW window.
     * @param focused True if the window is focused, false otherwise.
     */
    static void windowFocusCb(GLFWwindow* window, int focused);
    /**
     * @brief Callback function for window iconify (minimize) events.
     * @param window Pointer to the GLFW window.
     * @param iconified True if the window is iconified, false otherwise.
     */
    static void windowIconifyCb(GLFWwindow* window, int iconified);
    /**
     * @brief Callback function for keyboard input events.
     * @param window Pointer to the GLFW window.
     * @param key The keyboard key that was pressed or released.
     * @param scancode The system-specific scancode of the key.
     * @param action The action (press, release, repeat).
     * @param mods Bitfield describing which modifier keys were held down.
     */
    static void keyCb(GLFWwindow* window, int key, int scancode, int action, int mods);
    /**
     * @brief Callback function for mouse button input events.
     * @param window Pointer to the GLFW window.
     * @param button The mouse button that was pressed or released.
     * @param action The action (press or release).
     * @param mods Bitfield describing which modifier keys were held down.
     */
    static void mouseButtonCb(GLFWwindow* window, int button, int action, int mods);
    /**
     * @brief Callback function for mouse scroll events.
     * @param window Pointer to the GLFW window.
     * @param xoffset The scroll offset along the x-axis.
     * @param yoffset The scroll offset along the y-axis.
     */
    static void scrollCb(GLFWwindow* window, double xoffset, double yoffset);
    /**
     * @brief Callback function for mouse movement events.
     * @param window Pointer to the GLFW window.
     * @param xpos The new x-coordinate of the mouse cursor.
     * @param ypos The new y-coordinate of the mouse cursor.
     */
    static void cursorPosCb(GLFWwindow* window, double xpos, double ypos);
    /**
     * @brief Callback function for mouse enter/leave events.
     * @param window Pointer to the GLFW window.
     * @param entered True if the cursor entered the window, false if it left.
     */
    static void cursorEnterCb(GLFWwindow* window, int entered);
    /**
     * @brief Callback function for file drop events.
     * @param window Pointer to the GLFW window.
     * @param count The number of files dropped.
     * @param paths Array of file paths that were dropped.
     */
    static void dropCb(GLFWwindow* window, int count, const char** paths);

public:
    GLFWwindow* m_window = nullptr; // Pointer to the GLFW window
};
