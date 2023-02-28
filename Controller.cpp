#include "Controller.h"
#include <map>
#include <utility>
#include <iostream>

void Controller::load_book(char* path){
    bookFont.loadFromFile("Fonts/Georgia.ttf");
    model.load_fb2(path);
    model.split_into_words();
    create_word_matrix();
    build_up_pages();

    pageNumberText.setFont(bookFont);
    pageNumberText.setFillColor(textColor);
    pageNumberText.setCharacterSize(bookFontSize + 2);
    pageNumberText.setPosition(sf::Vector2f{view.PAGE_WIDTH - view.RF_WIDTH - 30, view.PAGE_HEIGHT - view.F_HEIGHT + 30});
}

void Controller::create_word_matrix(){
    for (Fragment frag: model.fragments){
        sf::Text word;
        word.setString(sf::String::fromUtf8(frag.text.begin(), frag.text.end()));
        word.setFont(bookFont);
        word.setFillColor(textColor);
        word.setCharacterSize(bookFontSize);
        for (Styles style: frag.styles){
            if (style == Styles::BOLD) word.setStyle(word.getStyle() | sf::Text::Style::Bold);
            if (style == Styles::HEADER) {
                word.setStyle(word.getStyle() | sf::Text::Style::Bold);
                word.setCharacterSize(bookFontSize + 5);
            }
            if (style == Styles::ITALIC) word.setStyle(word.getStyle() | sf::Text::Style::Bold);
            if (style == Styles::LINK){
                // TODO: добавить атрибут href и регион, нажатие на который будет проверять View
                word.setStyle(word.getStyle() | sf::Text::Style::Underlined);
                word.setFillColor(sf::Color::Blue);
            }
        }
        word_matrix.push_back(word);
    }
}

void Controller::build_up_pages(){
    page_t page;
    int line_len_px = view.PAGE_WIDTH - view.LF_WIDTH - view.RF_WIDTH;
    sf::Vector2f carriage_pos = {view.LF_WIDTH, view.H_HEIGHT};
    for (sf::Text word: word_matrix){
        if (carriage_pos.x + word.getGlobalBounds().width - 1 > line_len_px){
            if (carriage_pos.y + word.getGlobalBounds().height > view.PAGE_HEIGHT - view.F_HEIGHT){ // Страница заполнена
                pages.push_back(page);
                page = {};
                carriage_pos = {view.LF_WIDTH, view.H_HEIGHT};
            }
            else{
                carriage_pos = {view.LF_WIDTH, carriage_pos.y + bookFontSize + lineInt};
            }
        }

        if (word.getString().toAnsiString() == string("&&&")){
            carriage_pos = {view.LF_WIDTH, carriage_pos.y + 1.5f*(bookFontSize + lineInt)};
        }
        else if (word.getString().toAnsiString() == string("\n")){
            carriage_pos = {view.LF_WIDTH, carriage_pos.y + (bookFontSize + lineInt) * 2};
        }
        else {
            word.setPosition(carriage_pos);
            carriage_pos.x += word.getGlobalBounds().width;
            page.push_back(word);
        }
        // cout << word.getString().toAnsiString();
    }
}

void Controller::draw_page(){
    view.page.clear(sf::Color::White);
    for (sf::Text word: pages[cur_page_num]){
        view.page.draw(word);
    }
    pageNumberText.setString(sf::String(to_string(cur_page_num + 1)));
    view.page.draw(pageNumberText);
    view.page.display();
    view.pageSprite.setTexture(view.page.getTexture());
}

void Controller::loop(){
    sf::Event event;
    sf::Time sleep_dur = sf::milliseconds(20);
    bool updateFlag;
    draw_page();
    view.update();
    while (view.win.isOpen()){
        updateFlag = false;
        while (view.win.pollEvent(event)){
            view.gui.handleEvent(event);
            if (event.type == sf::Event::Closed){
                view.win.close();
            }
            else if (event.type == sf::Event::Resized){
                view.onWinResize(event.size);
            }
            // cout << event.type << "\n";
            updateFlag = true;
        }
        if (updateFlag){
            draw_page();
            view.update();
        }
        sf::sleep(sleep_dur);
    }
}

int main(){
    Controller cont;
    char FILE_NAME[] = "RedRing.fb2";
    cont.load_book(FILE_NAME);
    cont.loop();
    // for (Fragment frag: cont.model.fragments){
    //     cout << (string)frag;
    // }
    return 0;
}