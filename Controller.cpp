#include "Controller.h"
#include <map>
#include <utility>
#include <iostream>

Controller::Controller(){
    pageNumberText.setFont(bookFont);
    pageNumberText.setFillColor(textColor);
    pageNumberText.setCharacterSize(bookFontSize + 2);
    pageNumberText.setPosition(sf::Vector2f{view.PAGE_WIDTH - view.RF_WIDTH - 35, view.PAGE_HEIGHT - view.F_HEIGHT + 50});
    view.leftButton->onPress(&Controller::turn_page_back, this);
    view.leftButton->setMouseCursor(tgui::Cursor::Type::Hand);
    view.rightButton->onPress(&Controller::turn_page_fw, this);
    view.rightButton->setMouseCursor(tgui::Cursor::Type::Hand);
    bookFont.loadFromFile("Fonts/Georgia.ttf");
}

void Controller::load_book(char* path){
    model.load_fb2(path);
    model.split_into_words();
    create_word_matrix();
    build_up_pages();

    view.doc_links_name = model.doc_links_name;
}

void Controller::create_word_matrix(){
    for (Fragment frag: model.fragments){
        SWText word(&view);
        word.setString(sf::String::fromUtf8(frag.text.begin(), frag.text.end()));
        word.setFont(bookFont);
        word.setFillColor(textColor);
        word.setCharacterSize(bookFontSize);
        word.attrs = frag.attrs;
        for (Styles style: frag.styles){
            if (style == Styles::BOLD) word.setStyle(word.getStyle() | sf::Text::Style::Bold);
            if (style == Styles::HEADER) {
                word.setStyle(word.getStyle() | sf::Text::Style::Bold);
                word.setCharacterSize(bookFontSize + 5);
            }
            if (style == Styles::ITALIC) word.setStyle(word.getStyle() | sf::Text::Style::Bold);
            if (style == Styles::LINK){
                word.setStyle(word.getStyle() | sf::Text::Style::Underlined);
                word.setFillColor(sf::Color::Blue);
                word.onClick = &SWText::open_URL;
                word.onHover = &SWText::changeCursor;
                word.is_clickable = true;
            }
        }
        word_matrix.push_back(word);
    }
}

void Controller::build_up_pages(){
    page_t page;
    int line_len_px = view.PAGE_WIDTH - view.LF_WIDTH - view.RF_WIDTH;
    sf::Vector2f carriage_pos = {view.LF_WIDTH, view.H_HEIGHT}; // Position relative to the lt corner of view.pageSprite
    for (SWText word: word_matrix){
        if (carriage_pos.x + word.getGlobalBounds().width - 1 > line_len_px){
            if (carriage_pos.y + word.getGlobalBounds().height >= view.PAGE_HEIGHT - view.F_HEIGHT){ // Страница заполнена
                pages.push_back(page);
                page = {};
                carriage_pos = {view.LF_WIDTH, view.H_HEIGHT};
            }
            else{
                carriage_pos = {view.LF_WIDTH, carriage_pos.y + bookFontSize + lineInt};
            }
        }

        if (word.getString().toAnsiString() == string("&&&")){
            float new_y = carriage_pos.y + 1.5f*(bookFontSize + lineInt);
            if (new_y >= view.PAGE_HEIGHT - view.F_HEIGHT){
                carriage_pos = {view.LF_WIDTH, view.H_HEIGHT};
                pages.push_back(page);
                page = {};
            }
            else{
                carriage_pos = {view.LF_WIDTH, new_y};
            }
        }
        else if (word.getString().toAnsiString() == string("\n")){
            float new_y = carriage_pos.y + (bookFontSize + lineInt) * 2;
            if (new_y >= view.PAGE_HEIGHT - view.F_HEIGHT){
                carriage_pos = {view.LF_WIDTH, view.H_HEIGHT};
                pages.push_back(page);
                page = {};
            }
            else{
                carriage_pos = {view.LF_WIDTH, new_y};
            }
        }
        else {
            word.setPosition(carriage_pos);
            carriage_pos.x += word.getGlobalBounds().width;
            page.push_back(word);
        }
    }
    if (page.size() > 0) pages.push_back(page);
}

void Controller::draw_page(){
    view.page.clear(sf::Color::White);
    for (SWText word: pages[cur_page_num]){
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
    bool eventHandledByTGUI = false;
    draw_page();
    view.update();
    while (view.win.isOpen()){
        updateFlag = false;
        while (view.win.pollEvent(event)){
            eventHandledByTGUI = view.gui.handleEvent(event);
            if (event.type == sf::Event::Closed){
                view.win.close();
            }
            else if (event.type == sf::Event::Resized){
                view.onWinResize(event.size);
            }
            else if (event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Left) set_page_num(cur_page_num - 1);
                else if(event.key.code == sf::Keyboard::Right) set_page_num(cur_page_num + 1);
            }
            else if (event.type == sf::Event::MouseButtonPressed){
                for (SWText word: pages[cur_page_num]){
                    if (word.checkMouseOn({event.mouseButton.x, 
                                        event.mouseButton.y}))
                        word.onClick(&word);
                }
            }
            else if (event.type == sf::Event::MouseMoved){
                if(!eventHandledByTGUI){
                    bool wordHoveredFlag = false;
                    for (SWText word: pages[cur_page_num]){
                        if (word.checkMouseOn({event.mouseMove.x, 
                                            event.mouseMove.y})){
                            word.onHover(&word);
                            wordHoveredFlag = true;
                        }
                    }
                    if (!wordHoveredFlag) 
                        view.gui.restoreOverrideMouseCursor();
                }
            }
            updateFlag = true;
        }
        if (updateFlag){
            draw_page();
            view.update();
        }
        sf::sleep(sleep_dur);
    }
}

void Controller::set_page_num(int new_num){
    if (new_num >= 0 && new_num < this->pages.size()){
        this->cur_page_num = new_num;
    }
}

void Controller::turn_page_back(){
    if (cur_page_num - 1 >= 0) {
        this->cur_page_num -= 1;
    }
}

void Controller::turn_page_fw(){
    if (cur_page_num + 1 < this->pages.size()){
        this->cur_page_num += 1;
    }
}

int main(int argc, char** argv){
    Controller cont;
    char FILE_NAME[] = "RedRing.fb2";
    cont.load_book(argc == 1? FILE_NAME : argv[1]);
    cont.loop();
    return 0;
}