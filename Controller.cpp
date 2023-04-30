#include "Controller.h"
#include <map>
#include <utility>
#include "decode_base64.h"
#include "content_types.h"

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

SWText* Controller::create_text_from_instance(Fragment* frag){
    SWText* word = new SWText(&view);
    word->setString(sf::String::fromUtf8(frag->text.begin(), frag->text.end()));
    word->setFont(bookFont);
    word->setFillColor(textColor);
    word->setCharacterSize(bookFontSize);
    word->attrs = frag->attrs;
    for (Styles style: frag->styles){
        if (style == Styles::BOLD) word->setStyle(word->getStyle() | sf::Text::Style::Bold);
        if (style == Styles::HEADER) {
            word->setStyle(word->getStyle() | sf::Text::Style::Bold);
            word->setCharacterSize(bookFontSize + 5);
        }
        if (style == Styles::SUBTITLE) {
            word->setStyle(word->getStyle() | sf::Text::Style::Bold);
            word->setCharacterSize(bookFontSize + 2);
        }
        if (style == Styles::ITALIC) word->setStyle(word->getStyle() | sf::Text::Style::Bold);
        if (style == Styles::LINK){
            word->setStyle(word->getStyle() | sf::Text::Style::Underlined);
            word->setFillColor(sf::Color::Blue);
            word->onClick = &SWText::open_URL;
            word->onHover = &SWText::changeCursor;
            word->is_clickable = true;
        }
        if (style == Styles::TEXT_AUTHOR){
            word->setStyle(word->getStyle() | sf::Text::Bold | sf::Text::Italic);
        }
    }
    word->setPosition({static_cast<float>(frag->x), static_cast<float>(frag->y)});
    return word;
}

pair<sf::Sprite*, sf::Texture*> Controller::create_image_from_instance(Fragment* frag){
    string attr_name = model.doc_links_name + ":href";
    if (frag->attrs.count(attr_name) != 0){
        string pic_name = frag->attrs[model.doc_links_name + ":href"];
        pic_name.erase(0, 1);
        long decoded_len = model.binaries[pic_name].size()/4 * 3;
        char* decoded = decode_base64(model.binaries[pic_name].c_str());
        sf::Texture* picture = new sf::Texture;
        picture->loadFromMemory(decoded, decoded_len);
        sf::Sprite* sprite = new sf::Sprite;
        sprite->setTexture(*picture);
        sprite->setPosition(frag->x, frag->y);
        delete[] decoded;
        return {sprite, picture};
    }
    throw;
}

void Controller::build_up_pages_from_frags(){
    vector<Fragment*> page;
    sf::Vector2f carriage_pos = {view.LF_WIDTH, view.H_HEIGHT}; // Position relative to the lt corner of view.pageSprite
    int frags_len = model.fragments.size();

    for (int index = 0; index < frags_len; index++){
        Fragment* frag_ptr = *(model.fragments.begin());

        // Добавляем элемент оглавления
        if (frag_ptr->has_a_style(Styles::HEADER)
                        && frag_ptr->text !="&&&" && frag_ptr->text !="\n"){
            // Walker::for_each() гарантирует, что в атрибутах каждого фрагмента со стилем HEADLINE есть ключ "title"
            tocElem to_be_added(model.pages.size() + 1, frag_ptr->attrs["title"], frag_ptr->depth); // +1 из-за обложки

            // Добавляем только уникальные элементы (из-за разбития на слова заголовки дублируются)
            if(table_of_contents.size() == 0 || *(--table_of_contents.end()) != to_be_added)
                table_of_contents.push_back(to_be_added);
        }
        if (frag_ptr->type == ct::ContentType::TEXT){
            add_text(frag_ptr, page, carriage_pos);
        }
        else if (frag_ptr->type == ct::ContentType::IMAGE){
            add_image(frag_ptr, page, carriage_pos);
        }
        model.fragments.pop_front();
    }
    if (page.size() > 0) model.pages.push_back(page); // Adding the last page of the book.

    // Добавляем обложку, если документ содержит соответствующую картинку
    add_coverpage();
    apply_alignments();
}

void Controller::add_text(Fragment* frag, vector<Fragment*> &page, sf::Vector2f &carriage_pos){
    
    SWText* obj = create_text_from_instance(frag);
    float w_width = obj->getBounds().width;
    float w_height = obj->getBounds().height;
    int line_len_px = view.PAGE_WIDTH - view.LF_WIDTH - view.RF_WIDTH;
    
    if (carriage_pos.x + w_width - 1 > line_len_px){
        if (carriage_pos.y + w_height >= view.PAGE_HEIGHT - view.F_HEIGHT) // Страница заполнена
            new_page(page, carriage_pos);
        else
            carriage_pos = {view.LF_WIDTH, carriage_pos.y + bookFontSize + lineInt};
    }

    if (frag->text == "&&&"){
        float new_y = carriage_pos.y + 2.f*(bookFontSize + lineInt);
        if (new_y >= view.PAGE_HEIGHT - view.F_HEIGHT)
            new_page(page, carriage_pos);
        else{
            carriage_pos = {view.LF_WIDTH, new_y};
        }
    }
    else if (frag->text == "\n"){
        float new_y = carriage_pos.y + (bookFontSize + lineInt);
        if (new_y >= view.PAGE_HEIGHT - view.F_HEIGHT)
            new_page(page, carriage_pos);
        else{
            carriage_pos = {view.LF_WIDTH, new_y};
        }
    }
    else if (frag->text == "SW_POEM_START"){
        // Встречая специальный код, контроллер создает группу выравнивания
        align_groups.push_back(AlignmentGroup(alignType::CENTER_BLOCKWISE, line_len_px - 2*view.RF_WIDTH));
    }
    else if (frag->text == "SW_ALIGN_RIGHT_START"){
        // Встречая специальный код, контроллер создает группу выравнивания
        align_groups.push_back(AlignmentGroup(alignType::RIGHT_LINEWISE, line_len_px));
    }
    else {
        // Если текущее слово -- часть стихотворной строфы / эпиграфа / указания автора, 
        // то фрагменту назначается группа выравнивания
        if (!align_groups.empty()){
            if(frag->has_a_style(Styles::POEM) || frag->has_a_style(Styles::TEXT_AUTHOR))
                align_groups.back().addFragment(frag);
        }
        frag->x = carriage_pos.x;
        frag->y = carriage_pos.y;
        carriage_pos.x += w_width;
        frag->width = w_width;
        page.push_back(frag);
    }
    
    delete obj;
}

void Controller::apply_alignments(){
    for (auto group = this->align_groups.begin(); group != align_groups.end(); group++){
        group->align();
    }
}

void Controller::add_coverpage(){
    if (model.binaries.count("cover.jpg") != 0){
        Fragment* cover = new Fragment("", {}, {{model.doc_links_name + ":href", "#cover.jpg"}}, 
            ct::ContentType::IMAGE);
        imagepair_t IP = create_image_from_instance(cover);
        sf::Sprite* obj = IP.first;
        sf::FloatRect obj_bounds = obj->getLocalBounds();
        float resize_factor = pic_resize_logic(obj_bounds, true);
        int width = obj_bounds.width * resize_factor;
        int height = obj_bounds.height * resize_factor;

        cover->x = (view.PAGE_WIDTH  - width) / 2;
        cover->y = (view.PAGE_HEIGHT  - height) / 2;

        model.pages.insert(model.pages.begin(), {cover});
    }
}

void Controller::new_page(vector<Fragment*> &page, sf::Vector2f &carriage_pos){
    model.pages.push_back(page);
    page.clear();
    carriage_pos = {view.LF_WIDTH, view.H_HEIGHT};
}

void Controller::add_image(Fragment* frag, vector<Fragment*> &page, sf::Vector2f &carriage_pos){
    imagepair_t IP = create_image_from_instance(frag);
    sf::Sprite* obj = IP.first;
    sf::FloatRect obj_bounds = obj->getLocalBounds();
    float resize_factor = pic_resize_logic(obj_bounds);
    obj_bounds.width *= resize_factor;
    obj_bounds.height *= resize_factor;

    if (carriage_pos.x != view.LF_WIDTH)
        carriage_pos = {view.LF_WIDTH, carriage_pos.y +  + 1.5f*(bookFontSize + lineInt)};
    
    if (carriage_pos.y + obj_bounds.height >= view.PAGE_HEIGHT - view.F_HEIGHT)
        new_page(page, carriage_pos);
    // Коэффициент 3, так как правое поле слишком узкое
    frag->x = view.LF_WIDTH + ((view.PAGE_WIDTH - view.LF_WIDTH - view.RF_WIDTH*3) - obj_bounds.width) / 2;
    frag->y = carriage_pos.y;
    page.push_back(frag);
    carriage_pos.y += obj_bounds.height;
    delete IP.first, IP.second;
}

float Controller::pic_resize_logic(sf::FloatRect obj_bounds, bool fullpage_mode){
    // Коэффициент 3, так как правое поле слишком узкое
    int accessable_width = (view.PAGE_WIDTH - view.LF_WIDTH - view.RF_WIDTH * 3); 
    int accessable_height = (view.PAGE_HEIGHT - view.H_HEIGHT - view.F_HEIGHT);
    obj_bounds.width *= view.SCALE;
    obj_bounds.height *= view.SCALE;
    float h_resize_factor;
    float v_resize_factor;
    float resize_factor;
    if (fullpage_mode){
        h_resize_factor = accessable_width / obj_bounds.width;
        v_resize_factor = accessable_height / obj_bounds.height;
    }
    else{
        h_resize_factor = accessable_width < obj_bounds.width ? accessable_width / obj_bounds.width : 1;
        v_resize_factor = accessable_height < obj_bounds.height ? accessable_height / obj_bounds.height : 1;
    }
    resize_factor = std::min(h_resize_factor, v_resize_factor);

    return view.SCALE * resize_factor;
}

void Controller::draw_page(){
    view.page.clear(sf::Color::White);
    for (SWText word: this->cur_page.words){
        view.page.draw(word);
    }
    for (imagepair_t pic: this->cur_page.pics){
        float scale_factor = pic_resize_logic(pic.first->getLocalBounds(), 
                cur_page_num == 0 && model.binaries.count("cover.jpg") > 0);
        pic.first->setScale(scale_factor, scale_factor);
        view.page.draw(*pic.first);
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
                for (SWText word: cur_page.words){
                    if (word.checkMouseOn({event.mouseButton.x, 
                                        event.mouseButton.y}))
                        word.onClick(&word);
                }
            }
            else if (event.type == sf::Event::MouseMoved){
                if(!eventHandledByTGUI){
                    bool wordHoveredFlag = false;
                    for (SWText word: cur_page.words){
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
        cur_page.words.clear();
        for (imagepair_t IP: cur_page.pics){
            delete IP.first;
            delete IP.second;
        }
        cur_page.pics.clear();
        cur_page_num = new_num;
        for (Fragment* frag: model.pages[cur_page_num]){
            if (frag->type == ct::ContentType::TEXT){
                SWText* cur_word = create_text_from_instance(frag);
                cur_page.words.push_back(*cur_word);
                delete cur_word;
            }
            else if (frag->type == ct::ContentType::IMAGE){
                pair<sf::Sprite*, sf::Texture*> cur_pic = create_image_from_instance(frag);
                cur_pic.first->setScale(view.SCALE, view.SCALE);
                cur_page.pics.push_back(cur_pic);
            }
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
    if (table_of_contents.size() > 0){
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