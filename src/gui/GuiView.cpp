/**
 * @file GuiView.cpp
 * @brief Implementation of the GuiView class.
 */

#include "gui/GuiPr.h"

void GuiDrawable::enableWidget(int id, bool enable) {
    m_widgetStates[id].enabled = enable;
}

void GuiDrawable::setWidgetValue(int id, const GuiWidgetValue& value) {
    m_widgetStates[id].value = value;
}

void GuiDrawable::setWidgetVisible(int id, bool visible) {
    m_widgetStates[id].visible = visible;
}

void GuiDrawable::setWidgetComboItems(int id, const std::vector<std::string> &items) {
    m_widgetStates[id].comboItems = items;
}

std::vector<std::string> GuiDrawable::getWidgetComboItems(int id) const {
    if (m_widgetStates.find(id) == m_widgetStates.end())
        return {};
    if (m_widgetStates.at(id).comboItems.has_value())
        return m_widgetStates.at(id).comboItems.value();
    return {};
}

void GuiView::addListener(GuiEventListener* listener) {
    if (listener)
        m_listeners.push_back(listener);
}

void GuiView::removeListener(GuiEventListener* listener) {
    m_listeners.erase
    (
        std::remove(m_listeners.begin(), m_listeners.end(), listener),
        m_listeners.end()
    );
}

void GuiView::notifyListeners() {
    for (GuiEventListener* listener : m_listeners) {
        if (listener) {
            for (const GuiEvent& event : m_eventQueue)
                listener->onGuiEvent(event);
        }
    }
    m_eventQueue.clear();
}

void GuiView::pushEvent(const GuiEvent& event) {
    m_eventQueue.emplace_back(event);
}

int GuiListItem::getIndex() const {
    if (m_listView)
        return m_listView->getItemIndex(const_cast<GuiListItem*>(this));
    return -1;
}

std::string GuiListItem::getUidString(const std::string& prefix) const {
    std::string result = prefix;
    if (m_listView == nullptr)
        return result;

    std::vector<const GuiListItem*> itemStack = { this };
    GuiListItem* parent = m_listView->getParent();
    while (parent != nullptr) {
        itemStack.push_back(parent);
        parent = parent->m_listView->getParent();
    }

    do {
        const GuiListItem* item = itemStack.back();
        itemStack.pop_back();
        result += "_" + std::to_string(item->m_listView->getID());
        result += "-" + std::to_string(item->getIndex());
    } while (!itemStack.empty());

    return result;
}

void GuiListItem::pushEvent(const GuiEvent& event) {
    if (m_listView == nullptr)
        return;

    GuiEvent fullEvent{};
    std::vector<GuiListItem*> itemStack = { this };
    GuiEvent* ctx = &fullEvent;

    // Build the stack of parent items
    GuiListItem* parent = m_listView->getParent();
    while (parent != nullptr) {
        itemStack.push_back(parent);
        parent = parent->m_listView->getParent();
        ctx->listViewCtx = std::make_shared<GuiEvent>();
        ctx = ctx->listViewCtx.get();
    }
    ctx->listViewCtx = std::make_shared<GuiEvent>();

    // Get the root view
    GuiView* view = itemStack.back()->m_listView->getView();
    if (view == nullptr)
        return;

    // Fill in the context
    ctx = &fullEvent;
    do {
        GuiListItem* item = itemStack.back();
        itemStack.pop_back();
        ctx->viewLabel = event.viewLabel;
        ctx->widgetID = item->m_listView->getID();
        ctx->value = item->getIndex();
        ctx = ctx->listViewCtx.get();
    } while (!itemStack.empty() && ctx != nullptr);
    // Finally, set the event data
    if (ctx != nullptr) {
        ctx->viewLabel = event.viewLabel;
        ctx->widgetID = event.widgetID;
        ctx->value = event.value;
        ctx->type = event.type;
    }

    // Push the full event to the view
    view->pushEvent(fullEvent);
}

void GuiDialogView::open() {
    m_show = true;
}

void GuiDialogView::close() {
    m_show = false;
}

void GuiDialogView::setEventCallback(const std::function<void(const GuiEvent&)>& cb) {
    m_eventCb = cb;
}

void GuiDialogView::notifyListeners() {
    if (m_eventCb != nullptr)
    {
        for (const GuiEvent& event : m_eventQueue)
            m_eventCb(event);
    }
    GuiView::notifyListeners();
}

bool GuiDialogView::isShown() const {
    return m_show;
}

bool GuiListItem::isSelected() const {
    if (m_listView == nullptr)
        return false;
    return m_listView->isItemSelected(getIndex());
}

void GuiListItem::setSelected(bool selected) {
    if (m_listView != nullptr)
        m_listView->selectItem(getIndex(), selected);
}

bool GuiListItem::isExpanded() const {
    if (m_listView == nullptr)
        return false;
    return m_listView->isItemExpanded(getIndex());
}

void GuiListItem::setExpanded(bool expanded) {
    if (m_listView != nullptr)
        m_listView->expandItem(getIndex(), expanded);
}

GuiView* GuiListView::getView() const {
    return m_view;
}

GuiListItem* GuiListView::getParent() const {
    return m_parent;
}

int GuiListView::getID() const {
    return m_ID;
}

void GuiListView::removeItem(int index) {
    if (index < 0 || index >= static_cast<int>(m_items.size()))
        return;
    m_items.erase(m_items.begin() + index);
    m_itemTypes.erase(m_itemTypes.begin() + index);
    // Update item indices and states
    for (auto& pair : m_itemIndices) {
        if (pair.second > index)
            pair.second--;
    }
    m_itemSelectedStates.erase(m_itemSelectedStates.begin() + index);
    m_itemExpandedStates.erase(m_itemExpandedStates.begin() + index);
}

int GuiListView::getItemIndex(GuiListItem* item) const {
    if (m_itemIndices.find(item) != m_itemIndices.end())
        return m_itemIndices.at(item);
    return -1;
}

void GuiListView::clear() {
    m_items.clear();
    m_itemTypes.clear();
    m_itemIndices.clear();
    m_itemSelectedStates.clear();
    m_itemExpandedStates.clear();
}

int GuiListView::size() const {
    return static_cast<int>(m_items.size());
}

void GuiListView::selectItem(int index, bool select) {
    m_itemSelectedStates[index] = select;
}

void GuiListView::clearSelection() {
    m_itemSelectedStates = std::vector<char>(m_items.size(), false);
}

void GuiListView::selectAll() {
    m_itemSelectedStates = std::vector<char>(m_items.size(), true);
}

bool GuiListView::isItemSelected(int index) const {
    return m_itemSelectedStates[index];
}

void GuiListView::expandItem(int index, bool expand) {
    m_itemExpandedStates[index] = expand;
}

void GuiListView::expandAll() {
    m_itemExpandedStates = std::vector<char>(m_items.size(), true);
}

void GuiListView::collapseAll() {
    m_itemExpandedStates = std::vector<char>(m_items.size(), false);
}

bool GuiListView::isItemExpanded(int index) const {
    return m_itemExpandedStates[index];
}

void GuiListView::draw() {
    for (const auto& item : m_items) {
        if (item)
            item->draw();
    }
}
