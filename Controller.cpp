#include "Controller.h"
#include <map>
#include <utility>
#include <iostream>

void Controller::load_book(char* path){
    model.load_fb2(path);
    model.split_into_words();
    create_word_matrix();
    build_up_pages(-1);
}

void Controller::create_word_matrix(){
    for (Fragment frag: model.fragments){
        sf::Text* word = new sf::Text;
        word->setString(sf::String::fromUtf8(frag.text.begin(), frag.text.end()));
        word->setFont(view.bookFont);
        word->setFillColor({67, 67, 67});
        word->setCharacterSize(view.bookFontSize);
        for (Styles style: frag.styles){
            if (style == Styles::BOLD) word->setStyle(sf::Text::Style::Bold);
            if (style == Styles::HEADER) {
                word->setStyle(word->getStyle() | sf::Text::Style::Bold);
                word->setCharacterSize(view.bookFontSize + 5);
            }
            if (style == Styles::ITALIC) word->setStyle(word->getStyle() | sf::Text::Style::Bold);
            if (style == Styles::LINK){
                // TODO: добавить атрибут href и регион, нажатие на который будет проверять View
                word->setStyle(word->getStyle() | sf::Text::Style::Underlined);
                word->setFillColor(sf::Color::Blue);
            }
        }
        word_matrix.push_back(word);
    }
}

void Controller::build_up_pages(int line_len_px){
    // Stub
    page_t* line0 = new page_t;
    for(int i = 1; i < 10; i++)
        line0->push_back(word_matrix.at(i));
    page_matrices.push_back(line0);
}

void Controller::draw_page(){
    view.drawPage(page_matrices[0]);
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
            cout << event.type << "\n";
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