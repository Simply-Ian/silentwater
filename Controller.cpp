#include "Controller.h"
#include <map>
#include <utility>
#include <algorithm> // Для std::find()

Controller::Controller(){
    pageNumberText.setFont(bookFont);
    pageNumberText.setFillColor(textColor);
    pageNumberText.setCharacterSize(bookFontSize + 2);
    view.leftButton->onPress(&Controller::turn_page_back, this);
    view.leftButton->setMouseCursor(tgui::Cursor::Type::Hand);
    view.rightButton->onPress(&Controller::turn_page_fw, this);
    view.rightButton->setMouseCursor(tgui::Cursor::Type::Hand);
    bookFont.loadFromFile("Fonts/Georgia.ttf");
}

void Controller::load_book(char* path){
    model.load_fb2(path);
    model.split_into_words();
    build_up_pages_from_frags();

    view.doc_links_name = model.doc_links_name;
    view.build_up_toc(this->table_of_contents);
    view.tocList->onItemSelect(&Controller::toc_navigate, this);
    set_page_num(0);
}

SWText* Controller::create_text_from_instance(Fragment frag){
    SWText* word = new SWText(&view);
    word->setString(sf::String::fromUtf8(frag.text.begin(), frag.text.end()));
    word->setFont(bookFont);
    word->setFillColor(textColor);
    word->setCharacterSize(bookFontSize);
    word->attrs = frag.attrs;
    for (Styles style: frag.styles){
        if (style == Styles::BOLD) word->setStyle(word->getStyle() | sf::Text::Style::Bold);
        if (style == Styles::HEADER) {
            word->setStyle(word->getStyle() | sf::Text::Style::Bold);
            word->setCharacterSize(bookFontSize + 5);
        }
        if (style == Styles::ITALIC) word->setStyle(word->getStyle() | sf::Text::Style::Bold);
        if (style == Styles::LINK){
            word->setStyle(word->getStyle() | sf::Text::Style::Underlined);
            word->setFillColor(sf::Color::Blue);
            word->onClick = &SWText::open_URL;
            word->onHover = &SWText::changeCursor;
            word->is_clickable = true;
        }
    }
    word->setPosition({static_cast<float>(frag.x), static_cast<float>(frag.y)});
    return word;
}

void Controller::build_up_pages_from_frags(){
    vector<Fragment> page;
    int line_len_px = view.PAGE_WIDTH - view.LF_WIDTH - view.RF_WIDTH;
    sf::Vector2f carriage_pos = {view.LF_WIDTH, view.H_HEIGHT}; // Position relative to the lt corner of view.pageSprite
    
    int frags_len = model.fragments.size();
    float w_width;
    float w_height;
    for (int index = 0; index < frags_len; index++){
        Fragment frag = *(model.fragments.begin());

        // Добавляем элемент оглавления
        if (std::find(frag.styles.begin(), frag.styles.end(), Styles::HEADER) != frag.styles.end()){
            // Walker::for_each() гарантирует, что в атрибутах каждого фрагмента со стилем HEADLINE есть ключ "title"
            tocElem to_be_added(model.pages.size(), frag.attrs["title"], frag.depth);

            // Добавляем только уникальные элементы (из-за разбития на слова заголовки дублируются)
            if(table_of_contents.size() == 0 || *(--table_of_contents.end()) != to_be_added)
                table_of_contents.push_back(to_be_added);
        }

        SWText* word = create_text_from_instance(frag);
        w_width = word->getBounds().width;
        w_height = word->getBounds().height;
        if (carriage_pos.x + w_width - 1 > line_len_px){
            if (carriage_pos.y + w_height >= view.PAGE_HEIGHT - view.F_HEIGHT){ // Страница заполнена
                model.pages.push_back(page);
                page.clear();
                carriage_pos = {view.LF_WIDTH, view.H_HEIGHT};
            }
            else{
                carriage_pos = {view.LF_WIDTH, carriage_pos.y + bookFontSize + lineInt};
            }
        }

        if (word->getString().toAnsiString() == string("&&&")){
            float new_y = carriage_pos.y + 1.5f*(bookFontSize + lineInt);
            if (new_y >= view.PAGE_HEIGHT - view.F_HEIGHT){
                carriage_pos = {view.LF_WIDTH, view.H_HEIGHT};
                model.pages.push_back(page);
                page.clear();
            }
            else{
                carriage_pos = {view.LF_WIDTH, new_y};
            }
        }
        else if (word->getString().toAnsiString() == string("\n")){
            float new_y = carriage_pos.y + (bookFontSize + lineInt) * 2;
            if (new_y >= view.PAGE_HEIGHT - view.F_HEIGHT){
                carriage_pos = {view.LF_WIDTH, view.H_HEIGHT};
                model.pages.push_back(page);
                page.clear();
            }
            else{
                carriage_pos = {view.LF_WIDTH, new_y};
            }
        }
        else {
            frag.x = carriage_pos.x;
            frag.y = carriage_pos.y;
            carriage_pos.x += w_width;
            page.push_back(frag);
        }
        
        model.fragments.pop_front();
        delete word;
    }
    if (page.size() > 0) model.pages.push_back(page); // Adding the last page of the book.
    // Добавляем обложку, если документ содержит соответствующую картинку
    if (model.binaries.count("#cover.jpg") != 0)
        model.pages.insert(model.pages.begin(), {Fragment("", {}, {{"id", "#cover.jpg"}}, ct::IMAGE)});
}

void Controller::draw_page(){
    view.page.clear(sf::Color::White);
    for (SWText word: this->cur_page){
        view.page.draw(word);
    }
    pageNumberText.setString(sf::String(to_string(cur_page_num + 1)));
    pageNumberText.setPosition(view.PAGE_WIDTH - view.RF_WIDTH - pageNumberText.getLocalBounds().width, \
                view.PAGE_HEIGHT - view.F_HEIGHT + 50);
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
            if (event.type == sf::Event::Closed){
                view.win.close();
            }
            else if (event.type == sf::Event::Resized){
                view.onWinResize(event.size);
            }
            else if (event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Left) turn_page_back();
                else if(event.key.code == sf::Keyboard::Right) turn_page_fw();
            }
            else if (event.type == sf::Event::MouseButtonPressed){
                for (SWText word: cur_page){
                    if (word.checkMouseOn({event.mouseButton.x, 
                                        event.mouseButton.y}))
                        word.onClick(&word);
                }
            }
            else if (event.type == sf::Event::MouseMoved){
                if(!eventHandledByTGUI){
                    bool wordHoveredFlag = false;
                    for (SWText word: cur_page){
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
        eventHandledByTGUI = view.gui.handleEvent(event);
        if (updateFlag){
            draw_page();
        }
        view.update();
        sf::sleep(sleep_dur);
    }
}

void Controller::set_page_num(int new_num){
    if (new_num >= 0 && new_num < this->model.pages.size()){
        cur_page.clear();
        cur_page_num = new_num;
        for (Fragment frag: model.pages[cur_page_num]){
            SWText* cur_word = create_text_from_instance(frag);
            cur_page.push_back(*cur_word);
            delete cur_word;
        }
    }
}

void Controller::turn_page_back(){
    set_page_num_and_update_toc(cur_page_num - 1);
}

void Controller::turn_page_fw(){
    set_page_num_and_update_toc(cur_page_num + 1);
}

void Controller::toc_navigate(tgui::String name){
    tgui::String selectedId = view.tocList->getSelectedItemId();
    if (selectedId != ""){
        int page_num = selectedId.toInt();
        set_page_num(page_num);
    }
}

void Controller::set_page_num_and_update_toc(int new_num){
    // Обновляем оглавление
    if(new_num > cur_page_num){
        int toc_size = table_of_contents.size();
        for (int index = view.tocList->getSelectedItemIndex() + 1; index < toc_size - 1; index++){
            if (table_of_contents[index].page <= new_num && table_of_contents[index + 1].page > new_num){
                view.tocList->setSelectedItemByIndex(index);
            }
        }
        // Проверка последнего элемента. Если переход осуществляется с последнего элемента, есть риск выхода за границы вектора
        if (table_of_contents[table_of_contents.size() - 1].page <= new_num){
            view.tocList->setSelectedItemByIndex(table_of_contents.size() - 1);
        }
    }
    else if(new_num < cur_page_num){
        int cur_index = view.tocList->getSelectedItemIndex();
        for (int index = 0; index < cur_index; index++){
            if (table_of_contents[index].page <= new_num && table_of_contents[index + 1].page >= new_num){
                view.tocList->setSelectedItemByIndex(index);
                break;
            }
        }
    }
    set_page_num(new_num);
}

int main(int argc, char** argv){
    Controller cont;
    char FILE_NAME[] = "Books/Дары волхвов.fb2";
    cont.load_book(argc == 1? FILE_NAME : argv[1]);
    cont.loop();
    return 0;
}