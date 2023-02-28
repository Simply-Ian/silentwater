#include "View.h"
#include <iostream>
#include <string>


View::View(){
    sf::ContextSettings settings;
    settings.antialiasingLevel = 5;
    win.create(sf::VideoMode::getDesktopMode(), "FB2 Reader", sf::Style::Default, settings);
    win.setFramerateLimit(50);
    gui.setTarget(win);
    PAGE_WIDTH = (win.getSize().x * 0.37) * SCALE;
    PAGE_HEIGHT = (win.getSize().y - 2*TOP_BAR_HEIGHT) * SCALE;
    page.create(PAGE_WIDTH, PAGE_HEIGHT);
    page.setSmooth(true);
    pageSprite.setPosition({(win.getSize().x - (PAGE_WIDTH / SCALE)) / 2.0f,
                            75 + (win.getSize().y - 75 - (PAGE_HEIGHT / SCALE)) / 2.0f});
    pageSprite.setScale(1.0f / SCALE, 1.0f / SCALE);
    // bookFont.loadFromFile("Fonts/Georgia.ttf");
}

void View::onWinResize(sf::Event::SizeEvent new_size){
    auto mode = sf::VideoMode::getDesktopMode();
    if (win.getSize().x != mode.width || win.getSize().y != mode.height)
    win.setSize(sf::Vector2u(mode.width, mode.height));
}

void View::update(){
    win.clear({155, 155, 155, 255});
    gui.draw();
    win.draw(pageSprite);
    win.display();
}