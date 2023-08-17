#include "MyScrollablePanel.h"
#include <TGUI/Vector2.hpp>
#include <TGUI/Layout.hpp>

int MyScrollablePanel::getNewSlot(){
    const std::vector<tgui::Widget::Ptr>& widgets = getWidgets();
    int last_pos = widgets.empty() ? -spacer_height : widgets.back()->getPosition().y + widgets.back()->getSize().y;
    return last_pos + spacer_height;
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
    widgetPtr->setPosition({0, getNewSlot()});
    tgui::ScrollablePanel::add(widgetPtr, widgetName);
}