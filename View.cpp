#include "View.h"
#include <iostream>
#include <string>


View::View(){
    sf::ContextSettings settings;
    settings.antialiasingLevel = 5;
    win.create(sf::VideoMode::getDesktopMode(), "Silent Water Reader", sf::Style::Default, settings);
    win.setFramerateLimit(50);

    PAGE_WIDTH = (win.getSize().x * 0.37) * SCALE;
    PAGE_HEIGHT = (win.getSize().y - 2*TOP_BAR_HEIGHT) * SCALE;
    page.create(PAGE_WIDTH, PAGE_HEIGHT);
    page.setSmooth(true);
    float pageSpriteX = (win.getSize().x - (PAGE_WIDTH / SCALE)) / 2.0f;
    float pageSpriteY = 75;
    pageSprite.setPosition({pageSpriteX, pageSpriteY});
    pageSprite.setScale(1.0f / SCALE, 1.0f / SCALE);

    gui.setTarget(win);

    leftButton = tgui::BitmapButton::create();
    gui.add(leftButton, "leftButton");
    leftButton->setImage(tgui::Texture("Icons/chevronLeft.png"));
    leftButton->setSize(64, PAGE_HEIGHT / SCALE);
    leftButton->setOrigin(0, 0);
    leftButton->setPosition(pageSpriteX - 64, pageSpriteY);

    rightButton = tgui::BitmapButton::create();
    gui.add(rightButton, "rightButton");
    rightButton->setImage(tgui::Texture("Icons/chevronRight.png"));
    rightButton->setSize(64, PAGE_HEIGHT / SCALE);
    rightButton->setOrigin(0, 0);
    rightButton->setPosition(pageSpriteX + PAGE_WIDTH / SCALE, pageSpriteY);
}

void View::onWinResize(sf::Event::SizeEvent new_size){
    // auto mode = sf::VideoMode::getDesktopMode();
    // if (win.getSize().x != mode.width || win.getSize().y != mode.height)
    // win.setSize(sf::Vector2u(mode.width, mode.height));
}

void View::update(){
    win.clear({155, 155, 155, 255});
    gui.draw();
    win.draw(pageSprite);
    win.display();
}

bool SWText::checkMouseOn(sf::Vector2i pos){
    sf::FloatRect self_pos = getGlobalBounds();
    int click_x = parent->SCALE * (pos.x - parent->pageSprite.getPosition().x);
    int click_y = parent->SCALE * (pos.y - parent->pageSprite.getPosition().y);
    return (click_x >= self_pos.left && click_x <= self_pos.left + self_pos.width)
            && (click_y >= self_pos.top && click_y <= self_pos.top + self_pos.height);
}
SWText::SWText(View* parent): sf::Text::Text(){
    this->parent = parent;
}

void SWText::open_URL(){
    for (auto i: this->attrs){
        cout << i.first << " " << i.second << "\n";
    }
    string href_name = parent->doc_links_name + ":href";
    string command = "xdg-open " + this->attrs[href_name];
    if (this->attrs.count(href_name) != 0){
        if (this->attrs[href_name].substr(0, 4) == "http")
            system(command.c_str());
    }
}

void SWText::changeCursor(){
    parent->gui.setOverrideMouseCursor(tgui::Cursor::Type::Hand);
}