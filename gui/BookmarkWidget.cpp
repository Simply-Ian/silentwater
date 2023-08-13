#include "BookmarkWidget.h"
#include <string>

int BookmarkWidget::getPageNum(){
    return source.page;
}

BookmarkWidget::Ptr BookmarkWidget::create(sw::Bookmark s, int text_size){
    BookmarkWidget::Ptr ptr = std::make_shared<BookmarkWidget>();
    ptr->deleteButton = tgui::Button::create("X");
    ptr->deleteButton->setSize({20, 20});
    ptr->deleteButton->setPosition({"parent.width - width", 0});
    // ptr->deleteButton->onClick(ptr->callback);
    ptr->m_container->add(ptr->deleteButton);

    ptr->setSource(s);
    ptr->chapterTitleLabel->getRenderer()->setTextStyle(tgui::TextStyle::Bold);
    ptr->chapterTitleLabel->setAutoSize(true);
    ptr->chapterTitleLabel->setTextSize(text_size + 2);
    ptr->chapterTitleLabel->setPosition({5, 5});
    ptr->m_container->add(ptr->chapterTitleLabel);

    ptr->previewLabel->getRenderer()->setTextStyle(tgui::TextStyle::Italic);
    ptr->previewLabel->setPosition({5, text_size + 7});
    ptr->previewLabel->setTextSize(text_size);
    ptr->pageNumLabel->setAutoSize(true);
    ptr->pageNumLabel->setPosition({"parent.width - width - 5", 20});
    ptr->pageNumLabel->setTextSize(text_size);

    ptr->setSize({"parent.width - 20", text_size * 2 + 14});
    
    ptr->m_container->add(ptr->previewLabel);
    ptr->m_container->add(ptr->pageNumLabel);

    return ptr;
}

void BookmarkWidget::setSource(sw::Bookmark s){
    source = s;
    chapterTitleLabel = tgui::Label::create(s.chapter);
    previewLabel = tgui::Label::create(s.preview);
    pageNumLabel = tgui::Label::create(tgui::String::fromNumber(s.page + 1));
}

BookmarkWidget::Ptr BookmarkWidget::copy(BookmarkWidget::ConstPtr widget){
    if (widget) return std::static_pointer_cast<BookmarkWidget>(widget->clone());
    else return nullptr;
}
void BookmarkWidget::setSize(const tgui::Layout2d& size){
    tgui::SubwidgetContainer::setSize(size);
}

void BookmarkWidget::draw(tgui::BackendRenderTargetBase& target, tgui::RenderStates states) const{
    target.drawFilledRect(states, {getSize().x, getSize().y},
            tgui::Color::applyOpacity(m_bg_color, m_opacityCached));
    m_container->draw(target, states);
}

// Скопировано из tgui::ClickableWidget.cpp
tgui::Signal& BookmarkWidget::getSignal(tgui::String signalName){
    if (signalName == onMousePress.getName())
        return onMousePress;
    else if (signalName == onMouseRelease.getName())
        return onMouseRelease;
    else if (signalName == onClick.getName())
        return onClick;
    else if (signalName == onClose.getName())
        return onClose;
    else
        return Widget::getSignal(std::move(signalName));
}

// Скопировано из tgui::ClickableWidget.cpp
bool BookmarkWidget::isMouseOnWidget(tgui::Vector2f pos) const{
    return tgui::FloatRect{getPosition().x, getPosition().y, getSize().x, getSize().y}.contains(pos);
}

// Скопировано из tgui::ClickableWidget.cpp
void BookmarkWidget::leftMousePressed(tgui::Vector2f pos){
    m_bg_color = m_pressed_color;
    Widget::leftMousePressed(pos);
    onMousePress.emit(this, pos - getPosition());
}

// Скопировано из tgui::ClickableWidget.cpp
void BookmarkWidget::leftMouseReleased(tgui::Vector2f pos){
    m_bg_color = m_default_bg_color;
    onMouseRelease.emit(this, pos - getPosition());
    if (m_mouseDown){
        if (deleteButton->isMouseOnWidget(pos - getPosition())) onClose.emit(this);
        else
            onClick.emit(this, pos - getPosition());
    }
    Widget::leftMouseReleased(pos);
}