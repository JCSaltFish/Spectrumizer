/**
 * @file GuiPr.h
 * @brief Private header file for the GUI module.
 */

#pragma once

#include "GuiPub.h"

 /**
  * @brief Abstract base class for GUI window implementations.
  */
class GuiWindowImpl {
public:
    GuiWindowImpl(
        GuiWindow* owner,
        const std::string& title,
        int width,
        int height,
        int samples
    ) :
        m_owner(owner),
        m_title(title) {
        m_width = std::max(width, 1);
        m_height = std::max(height, 1);
    };
    virtual ~GuiWindowImpl() = default;

    /**
     * @brief Sets the size of the GUI window.
     * @param width The new width of the window.
     * @param height The new height of the window.
     */
    virtual void setWindowSize(int width, int height) = 0;
    /**
     * @brief Shows the GUI window.
     */
    virtual void show() const = 0;
    /**
     * @brief Hides the GUI window.
     */
    virtual void hide() const = 0;
    /**
     * @brief Terminates the GUI window.
     */
    virtual void term() const = 0;
    /**
     * @brief Checks if the window should close.
     * @return True if the window should close, false otherwise.
     */
    virtual bool shouldClose() const = 0;
    /**
     * @brief Gets the content scale factor of the window.
     * @return The content scale factor.
     */
    virtual float getContentScale() const = 0;
    /**
     * @brief Gets the current mouse position within the window.
     * @param x Reference to store the x-coordinate of the mouse.
     * @param y Reference to store the y-coordinate of the mouse.
     */
    virtual void getMousePos(double& x, double& y) const = 0;

    /**
     * @brief Begins a new frame for rendering.
     * @return 0 on success, non-zero on failure.
     */
    virtual int beginFrame() const = 0;
    /**
     * @brief Ends the current frame and swaps buffers.
     * @return 0 on success, non-zero on failure.
     */
    virtual int endFrame() const = 0;

    /**
     * @brief Sets the window icon using the provided icons.
     * @param icons A vector of GuiIcon structures representing the icons.
     */
    virtual void setWindowIcon(const std::vector<GuiIcon>& icons) = 0;

    /**
     * @brief Sets the VSync mode for the GUI window.
     * @param mode The VSync mode to set (IMMEDIATE, FIFO, MAILBOX).
     */
    virtual void setVSyncMode(GfxVSyncMode mode) = 0;

    /**
     * @brief Gets the native window handle for the GUI window.
     * @return Pointer to the native window handle.
     */
    virtual void* getNativeWindow() const = 0;

public:
    GuiWindow* m_owner = nullptr; // Pointer to the owner window
    std::string m_title = {}; // Title of the window
    int m_width = 0; // Width of the window
    int m_height = 0; // Height of the window
    GfxBackend m_backend = GfxBackend::OpenGL; // Graphics backend used by the window
};
