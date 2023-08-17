#include "Controller.h"
#include <map>
#include <utility>
#include "decode_funs/decode_base64.h"
#include "datastructs/content_types.h"
#include <functional>
#include <algorithm>

using namespace std::placeholders;

Controller::Controller(){
    pageNumberText.setFont(bookFont);
    pageNumberText.setFillColor(textColor);
    pageNumberText.setCharacterSize(bookFontSize + 2);

    view.leftButton->onPress(&Controller::turn_page_back, this);
    view.leftButton->setMouseCursor(tgui::Cursor::Type::Hand);
    view.rightButton->onPress(&Controller::turn_page_fw, this);
    view.add_bm_button->onClick(&Controller::add_bookmark, this);
    view.go_to_page->onClick([](Controller* it){
        int new_pos = it->view.go_to_box->getText().toInt() - 1;
        if (0 < new_pos && new_pos < it->model.pages.size()){
            it->set_page_num_and_update_toc(new_pos);
            it->view.go_to_box->setText("");
        }
    }, this);

    view.go_to_percent->onClick([](Controller* it){
        int new_perc = it->view.go_to_box->getText().toInt();
        if (0 <= new_perc && new_perc <= 100){
            int new_pos = (new_perc * it->model.pages.size()) / 100;
            it->set_page_num_and_update_toc(new_pos);
            it->view.go_to_box->setText("");
        }
    }, this);
    view.rightButton->setMouseCursor(tgui::Cursor::Type::Hand);
    bookFont.loadFromFile("Fonts/Georgia.ttf");
    comd->c_to_v.defaultFontName = bookFont.getInfo().family;
    view.chooseFontButton->setText(comd->c_to_v.defaultFontName + " " + to_string(bookFontSize / view.SCALE));

}

void Controller::load_book(char* path){
    model.load_fb2(path);
    model.split_into_words();
    build_up_pages_from_frags();

    view.doc_links_name = model.doc_links_name;
    view.build_up_toc(this->table_of_contents);
    view.tocList->onItemSelect(&Controller::toc_navigate, this);
    set_page_num_and_update_toc(model.load_bm_file(model.doc_uid));
    populate_bm_list(model.bookmarks);
    view.win.setTitle("Silent Water -- " + sf::String::fromUtf8(model.doc_title.begin(), model.doc_title.end())); //!< \todo Заголовок включает в себя имя (-ена) и фамилию (-и) автора (-ов)
}

SWText* Controller::create_text_from_instance(Fragment* frag){
    SWText* word = new SWText(&view);
    word->setString(frag->text);
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
            if (word->is_a_note_link())
                word->onHover = std::bind(&Controller::show_wordnote, this, _1);
                // word->onHover = &SWText::show_word_note;
            else
                word->onHover = &SWText::changeCursor;
            word->is_clickable = true;
        }
        if (style == Styles::TEXT_AUTHOR){
            word->setStyle(word->getStyle() | sf::Text::Bold | sf::Text::Italic);
        }
        if (style == Styles::EPIGRAPH){
            word->setStyle(word->getStyle() | sf::Text::Italic);
            word->setCharacterSize(bookFontSize - 2);
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
            if(table_of_contents.size() == 0 || table_of_contents.back() != to_be_added)
                table_of_contents.push_back(to_be_added);
        }
        if (frag_ptr->type == sw::ContentType::TEXT){
            add_text(frag_ptr, page, carriage_pos);
        }
        else if (frag_ptr->type == sw::ContentType::IMAGE){
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
    if (frag->has_a_style(Styles::EPIGRAPH)) line_len_px /= 2;
    
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
        // if(){} разнесен на две части, т. .к если объединить 2 условия через "И", на странице будет 
        // отрисовываться спецкод SW_POEM_START
        if (!frag->has_a_style(Styles::EPIGRAPH))
            // Встречая специальный код, контроллер создает группу выравнивания
            align_groups.push_back(AlignmentGroup(alignType::CENTER_BLOCKWISE, line_len_px - 2*view.RF_WIDTH));
    }
    else if (frag->text == "SW_ALIGN_RIGHT_LINEWISE_START"){
        // Встречая специальный код, контроллер создает группу выравнивания
        align_groups.push_back(AlignmentGroup(alignType::RIGHT_LINEWISE, line_len_px));
    }
    else {
        // Если текущее слово -- часть стихотворной строфы / эпиграфа / указания автора, 
        // то фрагменту назначается группа выравнивания
        if (!align_groups.empty()){
            if(frag->has_a_style(Styles::POEM) || frag->has_a_style(Styles::TEXT_AUTHOR) || frag->has_a_style(Styles::EPIGRAPH))
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
            sw::ContentType::IMAGE);
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
    // view.word_note.render(view.win);
}

void Controller::loop(){
    sf::Event event;
    sf::Time sleep_dur = sf::milliseconds(20);
    bool updateFlag;
    bool eventHandledByTGUI = false;
    draw_page();
    view.update();
    SWText first_clicked_word(&view);
    SWText hovered_word(&view);
    sf::Vector2i latest_click_pos;
    while (view.win.isOpen()){
        updateFlag = false;
        while (view.win.pollEvent(event)){
            if (event.type == sf::Event::Closed){
                // Сохраняем в файл номер страницы, на которой остановился пользователь, и данные о закладках
                model.save_bm_file(model.doc_uid);
                view.win.close();
            }
            else if (event.type == sf::Event::Resized){
                view.onWinResize(event.size);
            }
            else if (event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Left) turn_page_back();
                else if(event.key.code == sf::Keyboard::Right) turn_page_fw();
                else if (event.key.control && event.key.code == sf::Keyboard::C){
                    string text = get_selected_text();
                    sf::Clipboard::setString(sf::String::fromUtf8(text.begin(), text.end()));
                    view.showTemporalNotification("Текст скопирован", 1500);
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed){
                for (SWText& word: cur_page.words){
                    if (word.checkMouseOn({event.mouseButton.x, 
                                        event.mouseButton.y})){
                        word.onClick(&word);
                        first_clicked_word = word;
                    }
                }
                latest_click_pos = {event.mouseButton.x, event.mouseButton.y};
            }
            else if (event.type == sf::Event::MouseMoved){
                if(!eventHandledByTGUI){
                    bool wordHoveredFlag = false;
                    for (SWText& word: cur_page.words){
                        if (word.checkMouseOn({event.mouseMove.x, 
                                            event.mouseMove.y})){
                            word.onHover(&word);
                            wordHoveredFlag = true;
                            hovered_word = word;
                        }
                    }
                    if (!wordHoveredFlag){
                        if (view.word_note.isVisible() && 
                            !view.word_note.isMouseOn({static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y - 5)}))
                            view.word_note.setVisible(false);
                        view.gui.restoreOverrideMouseCursor();
                    }
                    else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
                        if (view.cur_cursor_type != tgui::Cursor::Type::Text){
                            view.gui.setOverrideMouseCursor(tgui::Cursor::Type::Text);
                            view.cur_cursor_type = tgui::Cursor::Type::Text;
                        }
                        // wordHoveredFlag = true гарантирует, что hovered_word -- не пустышка
                        auto fw = find(cur_page.words.begin(), cur_page.words.end(), first_clicked_word);
                        auto sw = find(cur_page.words.begin(), cur_page.words.end(), hovered_word);
                        if (max(fw, sw) != cur_page.words.end()){
                            for (auto w_i = min(fw, sw); w_i <= max(fw, sw); w_i++){
                                w_i->setSelected(true);
                            }
                            // Снимаем выделение, если пользователь сдвинул границу к началу выд-я
                            vector<SWText>::iterator border;
                            bool reverse_flag = false;
                            if (sw > fw)
                                border = ++sw;
                            else{
                                border = --sw;
                                reverse_flag = true;
                            }
                            while (border->getSelected()){
                                border->setSelected(false);
                                if (reverse_flag) border--; else border++;
                            }
                        }
                        else first_clicked_word = hovered_word;
                    }
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased){
                view.gui.restoreOverrideMouseCursor();
                hovered_word = SWText(&view);
                first_clicked_word = SWText(&view);
                if (latest_click_pos == sf::Vector2i{event.mouseButton.x, event.mouseButton.y}){
                    for (SWText& word: cur_page.words)
                        word.setSelected(false);
                }
            }
            updateFlag = true;
            eventHandledByTGUI = view.gui.handleEvent(event);
        }
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
            if (frag->type == sw::ContentType::TEXT){
                SWText* cur_word = create_text_from_instance(frag);
                cur_page.words.push_back(*cur_word);
                delete cur_word;
            }
            else if (frag->type == sw::ContentType::IMAGE){
                pair<sf::Sprite*, sf::Texture*> cur_pic = create_image_from_instance(frag);
                cur_pic.first->setScale(view.SCALE, view.SCALE);
                cur_page.pics.push_back(cur_pic);
            }
        }
        int preview_len = 15;
        string preview;
        for (int i = 0; i < preview_len; i++){
            Fragment* frag = model.pages[cur_page_num].at(i);
            if (frag->type == sw::ContentType::TEXT)
                preview += frag->text;
            else i--;
        }
        comd->c_to_v.preview = preview;
        model.update_checkpoint_data(new_num);
    }
}

void Controller::show_wordnote(SWText* t){
    t->changeCursor();
    string text = model.notes[t->attrs[model.doc_links_name + ":href"].substr(1)];
    sf::Vector2f pos{(t->getPosition().x  + t->getLocalBounds().width / 2) / view.SCALE + view.pageSprite.getGlobalBounds().left, 
        t->getPosition().y / view.SCALE + view.pageSprite.getGlobalBounds().top};
    view.showFloatingNote(text, 
                        pos);
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

void Controller::populate_bm_list(vector<sw::Bookmark> bms){
    int counter = 0;
    for (sw::Bookmark bm: bms){
        BookmarkWidget::Ptr item = BookmarkWidget::create(bm, view.GUI_TEXT_SIZE);
        item->setWidgetName(tgui::String::fromNumber(bm.page));
        item->onClick(&Controller::set_page_num_and_update_toc, this, bm.page);
        item->onClose(&Controller::delete_bookmark, this, item->getWidgetName());
        view.bmPan->add(item);
        counter++;
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

void Controller::add_bookmark(){
    const int header_len = 35;
    string preview;

    // Получаем название главы и обрезаем ведущие пробелы
    string chapter = table_of_contents[view.tocList->getSelectedItemIndex()].headline;
    if (chapter.size()){
        int first_letter_pos = 0;
        while (chapter.at(first_letter_pos) == ' ') first_letter_pos++;
        chapter = chapter.substr(first_letter_pos, header_len + first_letter_pos);
    }
    chapter += "...";
    chapter = chapter.substr(0, header_len - 10);

    int lines_count = model.pages.size();
    for (auto frag_ptr = model.pages[cur_page_num].begin(); 
            preview.size() < header_len && frag_ptr != model.pages[cur_page_num].end(); frag_ptr++)
        if ((*frag_ptr)->type == sw::ContentType::TEXT)
            preview += (*frag_ptr)->text;
    preview = preview.substr(0, header_len - 5) + "...";
    model.add_bm_data(cur_page_num, chapter, preview);
    populate_bm_list({sw::Bookmark(preview, chapter, cur_page_num)});
}

void Controller::delete_bookmark(tgui::String name){
    logger.log(name.toStdString(), Logger::levels::INFO);
    model.delete_bm_data(name.toStdString());
    logger.log("Model data deleted", Logger::levels::INFO);
    tgui::Widget::Ptr to_be_removed;
    for (auto i : view.bmPan->getWidgets()){
        if (i->getWidgetName() == name){
            to_be_removed = i;
            break;
        }
    }
    view.bmPan->remove(to_be_removed);
    logger.log("GUI data deleted", Logger::levels::INFO);
}

string Controller::get_selected_text(){
    string text;
    for (SWText& word: cur_page.words)
        if (word.getSelected()) text += word.source_text;
    return text;
}

int main(int argc, char** argv){
    Controller cont;
    char FILE_NAME[] = "Books/Дары волхвов.fb2";
    cont.load_book(argc == 1? FILE_NAME : argv[1]);
    cont.loop();
    return 0;
}