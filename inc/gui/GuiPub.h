/*
 * Copyright 2025 Jed Wang
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file GuiPub.h
 * @brief Public interface for the GUI module.
 */

#pragma once

#include "GuiCommon.h"

#include "gfx/gfxPub.h"

/**
 * @brief Class for managing GUI configuration settings.
 */
class GuiConfig {
private:
    GuiConfig() = default;
    ~GuiConfig() = default;

public:
    /**
     * @brief Sets the application name.
     * @param appName The name of the application.
     */
    static void setAppName(const std::string& appName) {
        s_appName = appName;
    };
    /**
     * @brief Gets the application name.
     * @return The name of the application.
     */
    static const std::string& getAppName() {
        return s_appName;
    };
    /**
     * @brief Sets the graphics backend.
     * @param backend The graphics backend to use.
     */
    static void setGraphicsBackend(GfxBackend backend) {
        s_backend = backend;
    }
    /**
     * @brief Gets the current graphics backend.
     * @return The current graphics backend.
     */
    static GfxBackend getGraphicsBackend() {
        return s_backend;
    };

private:
    static std::string s_appName; // Application name
    static GfxBackend s_backend; // Graphics backend
};

/**
 * @brief Class for managing GUI text resources.
 */
class GuiText {
private:
    GuiText() = default;
    GuiText(const GuiText&) = delete;
    GuiText& operator=(const GuiText&) = delete;
    GuiText(GuiText&&) = delete;
    GuiText& operator=(GuiText&&) = delete;
public:
    ~GuiText();

    /**
     * @brief Initializes the GUI text resource with the provided language JSON.
     * @param lang JSON string containing language resources.
     */
    static void load(const std::string& lang);
    /**
     * @brief Retrieves a string resource by key.
     * @param key The key for the string resource.
     * @return The string resource associated with the key, or an empty string if not found.
     */
    static std::string get(const std::string& key);
    /**
     * @brief Formats a string with the provided arguments.
     * @param format The format string.
     * @param args The arguments to format into the string.
     * @return The formatted string.
     */
    static std::string formatString(
        const std::string& format,
        const std::vector<std::string>& args
    );

private:
    class Impl; // Forward declaration of implementation details
    static std::unique_ptr<Impl> s_impl; // Pointer to implementation details
};

using GuiWidgetValue = std::variant<int, float, std::string, std::array<float, 3>>;
/**
 * @brief Enumeration of GUI event types.
 */
enum class GuiEventType {
    MODIFY,
    COMMIT,
};
/**
 * @brief Structure representing a GUI event.
 */
struct GuiEvent {
    std::string viewLabel = {}; // Label of the view generating the event
    int widgetID = 0; // Widget ID in the view
    GuiWidgetValue value; // Value associated with the event
    GuiEventType type = GuiEventType::COMMIT; // Type of the event
    std::shared_ptr<GuiEvent> listViewCtx = nullptr; // Context for list view events
};
/**
 * @brief Interface for GUI event listeners.
 */
class GuiEventListener {
public:
    virtual ~GuiEventListener() = default;

    /**
     * @brief Handles a GUI event.
     * @param event The GUI event to handle.
     */
    virtual void onGuiEvent(const GuiEvent& event) = 0;

protected:
    /**
     * @brief Retrieves the event value cast to type T.
     * @tparam T The type to cast the event value to.
     * @param event The GUI event.
     * @return The event value cast to type T, or a default-constructed T if the cast fails.
     */
    template<typename T>
    static T getEventValue(const GuiEvent& event) {
        GuiWidgetValue value = event.value;
        if (std::holds_alternative<T>(value))
            return std::get<T>(value);
        return {};
    }
};

/**
 * @brief Abstract base class for drawable GUI elements.
 */
class GuiDrawable {
public:
    /**
     * @brief Draws the GUI element.
     */
    virtual void draw() = 0;

    /**
     * @brief Sets the visibility of a widget.
     * @param id The widget ID.
     * @param visible True to make the widget visible, false to hide it.
     */
    void enableWidget(int id, bool enable);
    /**
     * @brief Sets the value of a widget.
     * @param id The widget ID.
     * @param value The value to set for the widget.
     */
    void setWidgetValue(int id, const GuiWidgetValue& value);
    /**
     * @brief Sets the visibility of a widget.
     * @param id The widget ID.
     * @param visible True to make the widget visible, false to hide it.
     */
    void setWidgetVisible(int id, bool visible);
    /**
     * @brief Retrieves the value of a widget cast to type T.
     * @tparam T The type to cast the widget value to.
     * @param id The widget ID.
     * @return The value of the widget cast to type T, or a default-constructed
               T if the cast fails.
     */
    template<typename T>
    T getWidgetValue(int id) {
        GuiWidgetValue widgetValue = m_widgetStates[id].value;
        if (std::holds_alternative<T>(widgetValue))
            return std::get<T>(widgetValue);
        return {};
    }
    void setWidgetComboItems(int id, const std::vector<std::string>& items);
    std::vector<std::string> getWidgetComboItems(int id) const;

protected:
    /**
     * @brief Structure representing the state of a widget.
     */
    struct WidgetState {
        bool enabled = true; // Whether the widget is enabled
        bool visible = true; // Whether the widget is visible
        GuiWidgetValue value; // Current value of the widget
        std::optional<std::vector<std::string>> comboItems;
    };
    std::map<int, WidgetState> m_widgetStates; // Map of widget states by ID
};

/**
 * @brief Abstract base class for GUI views.
 */
class GuiView : public GuiDrawable {
    friend class GuiListItem;

public:
    /**
     * @brief Adds a listener to the GUI view.
     * @param listener The listener to add.
     */
    void addListener(GuiEventListener* listener);
    /**
     * @brief Removes a listener from the GUI view.
     * @param listener The listener to remove.
     */
    void removeListener(GuiEventListener* listener);
    /**
     * @brief Notifies all registered listeners of the queued events.
     */
    virtual void notifyListeners();

protected:
    /**
     * @brief Pushes a GUI event to the event queue.
     * @param event The GUI event to push.
     */
    void pushEvent(const GuiEvent& event);

protected:
    std::vector<GuiEvent> m_eventQueue; // Queue of GUI events

private:
    std::vector<GuiEventListener*> m_listeners; // List of event listeners
};

/**
 * @brief Class representing a GUI dialog view.
 */
class GuiDialogView : public GuiView {
public:
    /**
     * @brief Opens the dialog.
     */
    void open();
    /**
     * @brief Closes the dialog.
     */
    void close();

    /**
     * @brief Sets the event callback function for the dialog.
     * @param cb The callback function to set.
     */
    void setEventCallback(const std::function<void(const GuiEvent&)>& cb);

    void notifyListeners() override;

protected:
    /**
     * @brief Checks if the dialog is currently shown.
     * @return True if the dialog is shown, false otherwise.
     */
    bool isShown() const;

private:
    bool m_show = false; // Whether the dialog is currently shown
    std::function<void(const GuiEvent&)> m_eventCb = nullptr; // Event callback function
};

class GuiListView;
/**
 * @brief Abstract base class for items in a GUI list view.
 */
class GuiListItem : public GuiDrawable {
    friend class GuiListView;

public:
    /**
     * @brief Checks if the list item is selected.
     * @return True if the item is selected, false otherwise.
     */
    bool isSelected() const;
    /**
     * @brief Sets the selected state of the list item.
     * @param selected True to select the item, false to deselect it.
     */
    void setSelected(bool selected);
    /**
     * @brief Checks if the list item is expanded.
     * @return True if the item is expanded, false otherwise.
     */
    bool isExpanded() const;
    /**
     * @brief Sets the expanded state of the list item.
     * @param expanded True to expand the item, false to collapse it.
     */
    void setExpanded(bool expanded);

    /**
     * @brief Gets the index of the item in the list view.
     * @return The index of the item.
     */
    int getIndex() const;

    /**
     * @brief Gets a pointer to the parent list view of the item.
     * @return Pointer to the parent GuiListView.
     */
    const GuiListView* getListView() const { return m_listView; };

protected:
    /**
     * @brief Generates a unique identifier string for the list item.
     * @param prefix Optional prefix to prepend to the UID string.
     * @return The unique identifier string for the item.
     */
    std::string getUidString(const std::string& prefix = "") const;

    /**
     * @brief Pushes a GUI event associated with this list item.
     * @param event The GUI event to push.
     */
    void pushEvent(const GuiEvent& event);

private:
    GuiListView* m_listView = nullptr; // Pointer to the parent list view
};
/**
 * @brief Class representing a GUI list view.
 */
class GuiListView {
public:
    GuiListView() = delete;
    GuiListView(GuiView* view, int id) :
        m_view(view), m_parent(nullptr), m_ID(id) {};
    GuiListView(GuiListItem* parent, int id) :
        m_view(nullptr), m_parent(parent), m_ID(id) {};

    /**
     * @brief Gets the parent GUI view of the list view.
     * @return Pointer to the parent GuiView.
     */
    GuiView* getView() const;
    /**
     * @brief Gets the parent GUI list item of the list view.
     * @return Pointer to the parent GuiListItem.
     */
    GuiListItem* getParent() const;
    /**
     * @brief Gets the ID of the list view.
     * @return The ID of the list view.
     */
    int getID() const;

    /**
     * @brief Adds a new item of type T to the list view.
     * @tparam T The type of the item to add (must derive from GuiListItem).
     * @return Shared pointer to the newly added item of type T, or nullptr if T
               does not derive from GuiListItem.
     */
    template<typename T>
    std::shared_ptr<T> addItem() {
        if (!std::is_base_of<GuiListItem, T>::value)
            return nullptr;
        std::shared_ptr<T> item = std::make_shared<T>();
        item->m_listView = this;
        m_items.push_back(item);
        m_itemIndices[item.get()] = static_cast<int>(m_items.size() - 1);
        m_itemTypes.push_back(std::type_index(typeid(T)));
        m_itemSelectedStates.push_back(false);
        m_itemExpandedStates.push_back(true);
        return item;
    }
    /**
     * @brief Retrieves the item at the specified index and casts it to type T.
     * @tparam T The type to cast the item to (must derive from GuiListItem).
     * @param index The index of the item to retrieve.
     * @return Shared pointer to the item of type T, or nullptr if the index is
               invalid or the type does not match.
     */
    template<typename T>
    std::shared_ptr<T> getItem(int index) {
        if (index < 0 || index >= static_cast<int>(m_items.size()))
            return nullptr;
        if (m_itemTypes[index] != std::type_index(typeid(T)))
            return nullptr;
        return std::static_pointer_cast<T>(m_items[index]);
    }
    /**
     * @brief Removes the item at the specified index from the list view.
     * @param index The index of the item to remove.
     */
    void removeItem(int index);
    /**
     * @brief Gets the index of the specified item in the list view.
     * @param item Pointer to the GuiListItem.
     * @return The index of the item, or -1 if the item is not found.
     */
    int getItemIndex(GuiListItem* item) const;
    /**
     * @brief Clears all items from the list view.
     */
    void clear();
    /**
     * @brief Gets the number of items in the list view.
     * @return The number of items.
     */
    int size() const;

    /**
     * @brief Selects or deselects the item at the specified index.
     * @param index The index of the item to select or deselect.
     * @param select True to select the item, false to deselect it.
     */
    void selectItem(int index, bool select);
    /**
     * @brief Clears the selection of all items in the list view.
     */
    void clearSelection();
    /**
     * @brief Selects all items in the list view.
     */
    void selectAll();
    /**
     * @brief Checks if the item at the specified index is selected.
     * @param index The index of the item to check.
     * @return True if the item is selected, false otherwise.
     */
    bool isItemSelected(int index) const;
    /**
     * @brief Expands or collapses the item at the specified index.
     * @param index The index of the item to expand or collapse.
     * @param expand True to expand the item, false to collapse it.
     */
    void expandItem(int index, bool expand);
    /**
     * @brief Expands all items in the list view.
     */
    void expandAll();
    /**
     * @brief Collapses all items in the list view.
     */
    void collapseAll();
    /**
     * @brief Checks if the item at the specified index is expanded.
     * @param index The index of the item to check.
     * @return True if the item is expanded, false otherwise.
     */
    bool isItemExpanded(int index) const;

    /**
     * @brief Draws the list view and its items.
     */
    void draw();

private:
    GuiView* m_view = nullptr; // Pointer to the parent GUI view
    GuiListItem* m_parent = nullptr; // Pointer to the parent GUI list item
    int m_ID = 0; // ID of the list view

    std::vector<std::shared_ptr<GuiListItem>> m_items; // List of items
    std::vector<std::type_index> m_itemTypes; // List of item types
    std::unordered_map<GuiListItem*, int> m_itemIndices; // Map of item indices

    std::vector<char> m_itemSelectedStates; // Selection states of items
    std::vector<char> m_itemExpandedStates; // Expansion states of items
};

/**
 * @brief Structure representing a GUI icon.
 */
struct GuiIcon {
    int width = 0; // Width of the icon
    int height = 0; // Height of the icon
    std::vector<unsigned char> data = {}; // Pixel data of the icon (RGBA format)
};
/**
 * @brief Class representing a GUI window.
 */
class GuiWindow {
public:
    GuiWindow
    (
        const std::string& title,
        int width,
        int height,
        int samples = 1
    );
    virtual ~GuiWindow();

    /**
     * @brief Sets the size of the GUI window.
     * @param width The width of the window.
     * @param height The height of the window.
     */
    void setWindowSize(int width, int height);
    /**
     * @brief Shows the GUI window.
     */
    void show() const;
    /**
     * @brief Hides the GUI window.
     */
    void hide() const;
    /**
     * @brief Terminates the GUI window.
     */
    void term() const;
    /**
     * @brief Gets the graphics renderer associated with the window.
     * @return The GfxRenderer instance.
     */
    GfxRenderer getRenderer() const;
    /**
     * @brief Checks if the window should close.
     * @return True if the window should close, false otherwise.
     */
    bool shouldClose() const;
    /**
     * @brief Gets the content scale factor of the window.
     * @return The content scale factor.
     */
    float getContentScale() const;
    /**
     * @brief Gets the current mouse position within the window.
     * @param x Reference to store the x-coordinate of the mouse.
     * @param y Reference to store the y-coordinate of the mouse.
     */
    void getMousePos(double& x, double& y) const;

    /**
     * @brief Draws a single frame of the GUI window.
     * @return 0 on success, non-zero on failure.
     */
    int drawFrame();

    /**
     * @brief Gets the native window handle.
     * @return Pointer to the native window handle.
     */
    void* getNativeWindow() const;

    /**
     * @brief Sets the callback function for window resize events.
     *        See onResize() for the callback function signature.
     * @param callback The callback function to be called on resize.
     */
    void setOnResizeCb(const std::function<void(int, int)>& callback);
    /**
     * @brief Sets the callback function for drawing events.
     *        The callback function has the following signature:
     *        void onDraw();
     * @param callback The callback function to be called for drawing.
     */
    void setOnDrawCb(const std::function<void()>& callback);
    /**
     * @brief Sets the callback function to be called after drawing is finished.
     *        The callback function has the following signature:
     *        void onDrawFinished();
     * @param callback The callback function to be called after drawing is finished.
     */
    void setOnDrawFinishedCb(const std::function<void()>& callback);
    /**
     * @brief Sets the callback function for window close events.
     *        See onClose() for the callback function signature.
     * @param callback The callback function to be called on close.
     */
    void setOnCloseCb(const std::function<bool()>& callback);
    /**
     * @brief Sets the callback function for window focus events.
     *        See onFocuse() for the callback function signature.
     * @param callback The callback function to be called on focus change.
     */
    void setOnFocuseCb(const std::function<void(bool)>& callback);
    /**
     * @brief Sets the callback function for window resume events.
     *        See onResume() for the callback function signature.
     * @param callback The callback function to be called on resume.
     */
    void setOnResumeCb(const std::function<void(bool)>& callback);

    /**
     * @brief Sets the callback function for keyboard actions.
     *        See onKeyboardAction() for the callback function signature.
     * @param callback The callback function to be called on keyboard action.
     */
    void setOnKeyboardActionCb(
        const std::function<void(GuiKey, bool, GuiFlags<GuiModKey>)>& callback
    );
    /**
     * @brief Sets the callback function for mouse button actions.
     *        See onMouseButton() for the callback function signature.
     * @param callback The callback function to be called on mouse button action.
     */
    void setOnMouseButtonCb(
        const std::function<void(GuiMouseButton, bool, GuiFlags<GuiModKey>)>& callback
    );
    /**
     * @brief Sets the callback function for mouse scroll events.
     *        See onMouseScroll() for the callback function signature.
     * @param callback The callback function to be called on mouse scroll.
     */
    void setOnMouseScrollCb(const std::function<void(double, double)>& callback);
    /**
     * @brief Sets the callback function for mouse movement events.
     *        See onMouseMove() for the callback function signature.
     * @param callback The callback function to be called on mouse move.
     */
    void setOnMouseMoveCb(const std::function<void(double, double)>& callback);
    /**
     * @brief Sets the callback function for mouse enter/leave events.
     *        See onMouseEnter() for the callback function signature.
     * @param callback The callback function to be called on mouse enter/leave.
     */
    void setOnMouseEnterCb(const std::function<void(bool)>& callback);
    /**
     * @brief Sets the callback function for file drop events.
     *        See onDrop() for the callback function signature.
     * @param callback The callback function to be called on file drop.
     */
    void setOnDropCb(const std::function<void(const std::vector<std::string>&)>& callback);

    /**
     * @brief Sets the window icon using the provided icons.
     * @param icons A vector of GuiIcon structures representing the icons in various sizes.
     */
    void setWindowIcon(const std::vector<GuiIcon>& icons);

    /**
     * @brief Sets the VSync mode for the window.
     * @param mode The VSync mode to set (IMMEDIATE, FIFO, MAILBOX).
     */
    void setVSyncMode(GfxVSyncMode mode);

    /**
     * @brief Adds a GUI view to the window.
     * @param view The GUI view to add.
     */
    void addView(std::shared_ptr<GuiView> view);

    /**
     * @brief Callback for window resize events.
     * @param width The new width of the window.
     * @param height The new height of the window.
     */
    void onResize(int width, int height);
    /**
     * @brief Callback for window close events.
     * @return True to allow the window to close, false to prevent it.
     */
    bool onClose();
    /**
     * @brief Callback for window focus events.
     * @param focused True if the window is focused, false otherwise.
     */
    void onFocuse(bool focused);
    /**
     * @brief Callback for window resume events.
     * @param resume True if the window is resuming, false if paused.
     */
    void onResume(bool resume);

    /**
     * @brief Callback for keyboard actions.
     * @param key The key that was pressed or released.
     * @param pressed True if the key was pressed, false if released.
     * @param mod The modifier keys that were active during the action.
     */
    void onKeyboardAction(GuiKey key, bool pressed, GuiFlags<GuiModKey> mod);
    /**
     * @brief Callback for mouse button actions.
     * @param button The mouse button that was pressed or released.
     * @param pressed True if the button was pressed, false if released.
     * @param mod The modifier keys that were active during the action.
     */
    void onMouseButton(GuiMouseButton button, bool pressed, GuiFlags<GuiModKey> mod);
    /**
     * @brief Callback for mouse scroll events.
     * @param x The horizontal scroll amount.
     * @param y The vertical scroll amount.
     */
    void onMouseScroll(double x, double y);
    /**
     * @brief Callback for mouse movement events.
     * @param x The new x-coordinate of the mouse cursor.
     * @param y The new y-coordinate of the mouse cursor.
     */
    void onMouseMove(double x, double y);
    /**
     * @brief Callback for mouse enter/leave events.
     * @param entered True if the mouse entered the window, false if it left.
     */
    void onMouseEnter(bool entered);
    /**
     * @brief Callback for file drop events.
     * @param paths A vector of file paths that were dropped onto the window.
     */
    void onDrop(const std::vector<std::string>& paths);

private:
    std::string m_title; // Window title
    GfxRenderer m_renderer = nullptr; // Graphics renderer instance

    std::function<void(int, int)> m_onResizeCb; // Resize callback function
    std::function<void()> m_onDrawCb; // Draw callback function
    std::function<void()> m_onDrawFinishedCb; // Draw finished callback function
    std::function<bool()> m_onCloseCb; // Close callback function
    std::function<void(bool)> m_onFocuseCb; // Focus callback function
    std::function<void(bool)> m_onResumeCb; // Resume callback function
    // Keyboard action callback
    std::function<void(GuiKey, bool, GuiFlags<GuiModKey>)> m_onKeyboardActionCb;
    // Mouse button callback
    std::function<void(GuiMouseButton, bool, GuiFlags<GuiModKey>)> m_onMouseButtonCb;
    std::function<void(double, double)> m_onMouseScrollCb; // Mouse scroll callback
    std::function<void(double, double)> m_onMouseMoveCb; // Mouse move callback
    std::function<void(bool)> m_onMouseEnterCb; // Mouse enter callback
    std::function<void(const std::vector<std::string>&)> m_onDropCb; // File drop callback

    std::vector<std::shared_ptr<GuiView>> m_views; // List of GUI views

    struct Impl; // Forward declaration of implementation details
    std::unique_ptr<Impl> m_impl; // Pointer to implementation details
};
