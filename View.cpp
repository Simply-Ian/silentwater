#include "View.h"
#include <iostream>
#include <string>


View::View(){
    win.create(sf::VideoMode::getDesktopMode(), "FB2 Reader");
    win.setFramerateLimit(50);
    gui.setTarget(win);
    PAGE_WIDTH = win.getSize().x * 0.37;
    PAGE_HEIGHT = win.getSize().y - 2*TOP_BAR_HEIGHT;
    page.create(PAGE_WIDTH, PAGE_HEIGHT);
    page.setSmooth(true);
    pageSprite.setPosition({(win.getSize().x - PAGE_WIDTH) / 2.0f,
                            75 + (win.getSize().y - 75 - PAGE_HEIGHT) / 2.0f});
    bookFont.loadFromFile("Fonts/Georgia.ttf");
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

void View::drawPage(vector<sf::Text*>* page_matrix){
    page.clear(sf::Color::White);
    sf::Vector2f word_pos{LF_WIDTH, H_HEIGHT};
    for (auto word = page_matrix->begin(); word != page_matrix->end(); ++word){
        (*word)->setPosition(word_pos);
        sf::Text word_copy;
        word_copy.setFont(bookFont);
        word_copy.setString((*word)->getString());
        word_copy.setCharacterSize((*word)->getCharacterSize());
        word_copy.setStyle((*word)->getStyle());
        word_copy.setFillColor((*word)->getFillColor());
        word_copy.setPosition(word_pos);
        page.draw(word_copy);
        word_pos.x += word_copy.getGlobalBounds().width;
    }
    page.display();
    pageSprite.setTexture(page.getTexture());
}