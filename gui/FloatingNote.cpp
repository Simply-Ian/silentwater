#include "FloatingNote.h"

FloatingNote::FloatingNote(tgui::GuiSFML& g, int fs){
    label = tgui::Label::create();
    label->setTextSize(fs);
    label->getRenderer()->setBackgroundColor(tgui::Color::Black);
    label->getRenderer()->setTextColor(tgui::Color::White);
    label->setMaximumTextWidth(MAX_LABEL_WIDTH);
    label->setVisible(false);
    label->setScrollbarPolicy(tgui::Scrollbar::Policy::Automatic);
    g.add(label);
}

tgui::Vector2f FloatingNote::getPosition() const{
    return this->label->getAbsolutePosition();
}

void FloatingNote::setVisible(bool v){
    this->label->setVisible(v);
}

bool FloatingNote::isVisible(){
    return this->label->isVisible();
}

void FloatingNote::updateLabel(string text, sf::Vector2f pos){
    if (label->getText() != text)
        label->setHeight(0); // Обнуляем высоту, чтобы установка размера сработала корректно
    label->setText(text);
    int debug_size = label->getSize().y;
    if (debug_size >= MAX_LABEL_HEIGHT){
        label->setAutoSize(false);
        label->setHeight(MAX_LABEL_HEIGHT);
    }
    else label->setAutoSize(true);

    int y_pos = pos.y - label->getSize().y;
    label->setPosition(pos.x - (label->getSize().x / 2), y_pos >= 0? y_pos : pos.y + 15);
    label->moveToFront();
}

FloatingNote::~FloatingNote(){
    this->label->getParentGui()->remove(this->label);
}

bool FloatingNote::isMouseOn(tgui::Vector2f pos){
    return this->label->isMouseOnWidget(pos);
}