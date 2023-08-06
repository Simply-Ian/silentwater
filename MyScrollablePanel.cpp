#include "MyScrollablePanel.h"
#include <TGUI/Vector2.hpp>
#include <TGUI/Layout.hpp>

int MyScrollablePanel::getNewSlot(int bm_height){
    return getWidgets().size() * (bm_height + spacer_height);
}

MyScrollablePanel::Ptr MyScrollablePanel::create(tgui::Layout2d size, tgui::Vector2f contentSize){
    auto panel = std::make_shared<MyScrollablePanel>();
    panel->setSize(size);
    panel->setContentSize(contentSize);
    return panel;
}

bool MyScrollablePanel::remove(const tgui::Widget::Ptr &widget){
    int offset = 0;
    for (tgui::Widget::Ptr child: getWidgets()){
        if (child == widget)
            offset = widget->getSize().y + spacer_height;
        else child->setPosition(child->getPosition() - tgui::Vector2f{0, static_cast<float>(offset)});
    }
    return tgui::ScrollablePanel::remove(widget);;
}

void MyScrollablePanel::add(const tgui::Widget::Ptr &widgetPtr, const tgui::String &widgetName){
    widgetPtr->setPosition({0, getNewSlot(widgetPtr->getSize().y)});
    tgui::ScrollablePanel::add(widgetPtr, widgetName);
}