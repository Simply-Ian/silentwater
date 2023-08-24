#include <cstring> // pugixml returns all the string data as C-strings
#include "decode_funs/load_text.h"
#include <vector>
#include <utility> // Для std::pair
#include <map>
#include "Model.h"
#include <sstream> // Для разбиения строки по разделителю
#include <algorithm> // Для std::find
#include <filesystem> // Проверка существования файла
#include <cmath>
#include <fstream>
#include "decode_funs/get_abs_path_to_folder.h"
using namespace std;

bool Walker::for_each(pugi::xml_node& node){
    const string node_name = node.name(); // Конверсия C-строки в string
    if (node_name == "body"){
        body_name = node.attribute("name").value();
    }
    else if (depth() <= 1)
        body_name = OUT_OF_BODY;
    
    if (body_name != OUT_OF_BODY){
        // Очищаем стили и атрибуты, установленные более низкими элементами
        while ((!cur_style.empty()) && cur_style.at(cur_style.size()-1).second >= depth()){
            cur_style.pop_back();
        }
        while((!cur_attrs.empty()) && (--cur_attrs.end())->second.second >= depth()){
            cur_attrs.erase(--cur_attrs.end());
        }

        if (names_to_styles.count(node_name) > 0)
            cur_style.push_back(style_t(names_to_styles[node_name], depth()));
        
        for (pugi::xml_attribute attr: node.attributes()){
            // Сохраняются только атрибуты *:href
            if (strcmp(attr.name(), (doc_links_name + ":href").c_str()) == 0 || strcmp(attr.name(), "type") == 0)
                cur_attrs[attr.name()] = attr_t(attr.value(), depth());
        }
        if (node_name == "title"){ 
            /* Указываем, что следующие фрагменты относятся к заголовку. Это необходимо для постройки оглавления.
            Здесь важно не значение атрибута, а просто его наличие. */
            cur_attrs["title"] = attr_t("", depth());
        }

        if (node.type() == pugi::xml_node_type::node_pcdata){
            Fragment* to_be_added = new Fragment(node.value(), format_styles(), format_attrs(), sw::ContentType::TEXT, depth());
            // Перед разбиением на отдельные слова сохраняем в атрибутах каждого фрагмента-заголовка исходное название.
            if (cur_attrs.count("title") != 0)
                to_be_added->attrs["title"] = to_be_added->text;
            frags->push_back(to_be_added);
        }
        else if (node_name == "empty-line" || node_name == "subtitle"){
            Fragment* to_be_added = new Fragment("\n", format_styles(), {}, sw::ContentType::TEXT);
            frags->push_back(to_be_added);
        }
        else if (node_name == "p" || node_name == "stanza"){
            Fragment* to_be_added = new Fragment("&&&", format_styles(), {}, sw::ContentType::TEXT);
            frags->push_back(to_be_added);
        }
        else if (node_name == "v"){
            Fragment* break_to_be_added = new Fragment("\n", format_styles(), {}, sw::ContentType::TEXT);
            frags->push_back(break_to_be_added);
            // Fragment* line_to_be_added = new Fragment("SW_POEM_NEW_LINE", format_styles(), {}, ct::ContentType::TEXT);
            // frags->push_back(line_to_be_added);
        }
        else if (node_name == "poem"){
            Fragment* to_be_added = new Fragment("SW_POEM_START", format_styles(), {}, sw::ContentType::TEXT);
            frags->push_back(to_be_added);
        }
        else if (node_name == "image"){
            Fragment* to_be_added = new Fragment("", {}, format_attrs(), sw::ContentType::IMAGE);
            frags->push_back(to_be_added);
        }
        else if (node_name == "text-author" || node_name == "epigraph"){
            Fragment* break_to_be_added = new Fragment("\n", {}, format_attrs(), sw::ContentType::TEXT);
            frags->push_back(break_to_be_added);
            Fragment* to_be_added = new Fragment("SW_ALIGN_RIGHT_LINEWISE_START", {}, {}, sw::ContentType::TEXT);
            frags->push_back(to_be_added);
        }
    }
    else{
        if (node_name == "binary"){
            string href = node.attribute("id").value();
            string encoded = node.first_child().value();
            // Вычищаем символы перевода строки, которые попадаются в base64 внутри файлов от некоторых библиотек
            size_t break_pos = encoded.find('\n');
            while (break_pos != string::npos){
                encoded.erase(break_pos, 1);
                break_pos = encoded.find('\n', break_pos + 1);
            }
            
            binaries->insert({href, encoded});
        }
    }
    return true;
}

vector<Styles> Walker::format_styles(){
    vector<Styles> style;
    for (auto i = cur_style.begin(); i != cur_style.end(); i++)
        style.push_back(i->first);
        return style;
}

map<string, string> Walker::format_attrs(){
    map<string, string> formatted_attrs;
    for(auto i = cur_attrs.begin(); i != cur_attrs.end(); i++){
        formatted_attrs[i->first] = i->second.first;
    }
    return formatted_attrs;
}

void Model::load_fb2(const char* FILE_NAME){
    book_path = FILE_NAME;

    pugi::xml_parse_result result = doc.load_file(FILE_NAME, pugi::parse_declaration);
    if (!result) throw runtime_error(result.description());
    char* encoding_name = (char*)(doc.first_child().last_attribute().value());
    string* re_opened = load_text(FILE_NAME, encoding_name);
    doc.reset();
    result = doc.load_string(re_opened->c_str());
    delete re_opened;

    auto FB = doc.first_element_by_path("/FictionBook");
    for (pugi::xml_attribute attr: FB.attributes()){
        if (strcmp(attr.value(), "http://www.w3.org/1999/xlink") == 0){
            string name = attr.name();
            doc_links_name = name.substr(6, string::npos);
            break;
        }
    }
    w.doc_links_name = this->doc_links_name;
    this->doc_uid = doc.first_element_by_path("/FictionBook/description/document-info/id").first_child().value();
    this->doc_title = doc.first_element_by_path("/FictionBook/description/title-info/book-title").first_child().value();
    doc.traverse(w);
    extract_notes();
}

void Model::split_into_words(){
    int frags_len = fragments.size();
    vector<string> spec_codes = {"&&&", "SW_POEM_START", "SW_ALIGN_RIGHT_LINEWISE_START", "\n"};
    for (int index = 0; index < frags_len; index++){
        auto i = *(fragments.begin());
        if (i->type == sw::TEXT){
            if (find(spec_codes.begin(), spec_codes.end(), i->text) == spec_codes.end()){
                stringstream s(i->text);
                string word;
                while(getline(s, word, ' ')){
                    word += " ";
                    Fragment* word_frag = new Fragment(word, i->styles, i->attrs, i->type, i->depth);
                    fragments.push_back(word_frag);
                }
            }
            else{
                fragments.push_back(i);
            }
        }
        else
            fragments.push_back(i);
        fragments.pop_front();
    }
    this->fragments.erase(this->fragments.begin()); // Обрезаем пустую строку перед первым абзацем
}

void Model::extract_notes(){
    pugi::xml_node notes_body = doc.find_node([](pugi::xml_node n){return strcmp(n.attribute("name").as_string(),
                                                                     "notes") == 0;});
    string id;
    string content;
    if (notes_body){
        pugi::xml_node first_note = notes_body.find_child([](pugi::xml_node n){return strcmp(n.name(), "section") == 0;});
        for(pugi::xml_node i = first_note; i.type() != pugi::node_null ; i = i.next_sibling()){
            id = i.attribute("id").as_string();
            content = i.find_child([](pugi::xml_node n){return strcmp(n.name(), "p") == 0;}).child_value();
            this->notes.insert({id, content});
        }
    }
}

/*!
    Функция открывает и парсит файл с закладками. Информация о собственно закладках помещается в Model::bookmarks,
    страница, на которой пользователь остановился последний раз, возвращается.
    \returns Номер страницы, на которой пользователь остановился последний раз.
*/
int Model::load_bm_file(string doc_uid){
    filesystem::path bookmark_filepath = compose_bookmark_filepath(doc_uid);
    if (filesystem::exists(bookmark_filepath)){
        pugi::xml_parse_result result = this->bookmarks_doc.load_file(bookmark_filepath.c_str());
        if (!result) throw runtime_error(result.description());

        int checkpoint_page = round(atof(bookmarks_doc.first_element_by_path("/SilentWater_bm/checkpoint")
                                    .first_attribute().value()) * pages.size());
        for (auto bm_tag = bookmarks_doc.first_child().child("bm"); bm_tag.type() != pugi::node_null; bm_tag = bm_tag.next_sibling())
            this->bookmarks.push_back(sw::Bookmark(bm_tag.first_child().value(), 
                                            bm_tag.attribute("chapter").value(), 
                                            round(atof(bm_tag.attribute("part").value()) * pages.size())));
        return checkpoint_page;
    }
    else{
        // Создаем файл закладок, если таковой не существует
        pugi::xml_node root = bookmarks_doc.append_child("SilentWater_bm");
        pugi::xml_node doc_node = root.append_child("document");
        doc_node.append_child(pugi::node_pcdata).set_value(doc_title.c_str());
        pugi::xml_node checkpoint_node = root.append_child("checkpoint");
        checkpoint_node.append_attribute("page") = 0;
        return 0;
    }
}

filesystem::path Model::compose_bookmark_filepath(string doc_uid){
    filesystem::path base_folder_path = get_abs_path_to_folder();
    return (base_folder_path / "bookmarks" / (filesystem::path(doc_uid + ".swbm"))).c_str();
}

void Model::save_bm_file(string doc_uid){
    // Все параметры, кроме кодировки, по умолчанию
    filesystem::path path = compose_bookmark_filepath(doc_uid);
    bookmarks_doc.save_file(path.c_str(), "\t", 1U, pugi::encoding_utf8); //Закомментировано, т. к. при использовании vg load_bm_file() выдает ошибку
}

void Model::update_checkpoint_data(int page_num){
    bookmarks_doc.first_element_by_path("/SilentWater_bm/checkpoint")
            .first_attribute().set_value(static_cast<float>(page_num) / pages.size());
}

void Model::add_bm_data(int page_num, string chapter, string preview){
    pugi::xml_node tag = bookmarks_doc.first_child().append_child("bm");
    tag.append_attribute("part").set_value(static_cast<float>(page_num) / pages.size());
    tag.append_attribute("chapter").set_value(chapter.c_str());
    tag.append_child(pugi::xml_node_type::node_pcdata).set_value(preview.c_str());
}

void Model::delete_bm_data(string page){
    pugi::xml_node bm = bookmarks_doc.first_element_by_path("/SilentWater_bm/bm");
    while ((round(atof(bm.attribute("part").value())) != round(static_cast<float>(atoi(page.c_str())) / pages.size())) &&
         (bm.type() != pugi::xml_node_type::node_null)) bm = bm.next_sibling();
    if (bm.type() != pugi::xml_node_type::node_null)
        bookmarks_doc.child("SilentWater_bm").remove_child(bm);
    else throw;
}

Settings Model::load_settings_file(){
    string path = (get_abs_path_to_folder() + "/config.xml").c_str();
    if (filesystem::exists({path})){
        pugi::xml_parse_result result = settings.load_file(path.c_str());
        if (!result) throw runtime_error(result.description());

        const char* theme_path = settings.first_element_by_path("/SilentWater_settings/theme").first_child().value();
        pugi::xml_node text_color = settings.first_element_by_path("/SilentWater_settings/text-color");
        pugi::xml_node bg_color = settings.first_element_by_path("/SilentWater_settings/bg-color");

        Settings for_return(theme_path, atoi(bg_color.attribute("r").value()), atoi(bg_color.attribute("g").value()), atoi(bg_color.attribute("b").value()),
                        atoi(text_color.attribute("r").value()), atoi(text_color.attribute("g").value()), atoi(text_color.attribute("g").value()));
        
        pugi::xml_node font = settings.first_element_by_path("/SilentWater_settings/font");
        for_return.font_path = font.attribute("path").value();
        for_return.font_size = atoi(font.attribute("size").value());
        for_return.line_int = atoi(font.attribute("line-spacing").value());
        for_return.last_seen = settings.first_element_by_path("/SilentWater_settings/last-seen").attribute("path").value();

        return for_return;
    }
    else{
        ofstream out;
        out.open(get_abs_path_to_folder() + "/config.xml");
        string contents = "<SilentWater_settings> \n"
            "<theme></theme> \n"
            "<text-color r='67' g='67' b='67'/>\n"
            "<bg-color r='255' g='255' b='255'/> \n"
            "<font path='' size='22' line-spacing='3'/> \n"
            "<last-seen path=''/> \n "
        "</SilentWater_settings>";
        out << contents.c_str();
        out.close();
        return load_settings_file();
    }
}

void Model::save_settings_file(Settings to_save){
    pugi::xml_node text_color = settings.first_element_by_path("/SilentWater_settings/text-color");
    text_color.attribute("r").set_value(to_save.textColor.r);
    text_color.attribute("g").set_value(to_save.textColor.g);
    text_color.attribute("b").set_value(to_save.textColor.b);

    pugi::xml_node bg_color = settings.first_element_by_path("/SilentWater_settings/bg-color");
    bg_color.attribute("r").set_value(to_save.bgColor.r);
    bg_color.attribute("g").set_value(to_save.bgColor.g);
    bg_color.attribute("b").set_value(to_save.bgColor.b);

    pugi::xml_node font = settings.first_element_by_path("/SilentWater_settings/font");
    font.attribute("path").set_value(to_save.font_path.c_str());
    font.attribute("size").set_value(to_save.font_size);
    font.attribute("line-spacing").set_value(to_save.line_int);

    settings.first_element_by_path("/SilentWater_settings/last-seen").attribute("path").set_value(to_save.last_seen.c_str());

    settings.save_file((get_abs_path_to_folder() + "/config.xml").c_str(), "\t", 1U, pugi::encoding_utf8);
}