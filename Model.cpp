#include <cstring> // pugixml returns all the string data as C-strings
#include "load_text.h"
#include <vector>
#include <utility> // Для std::pair
#include <map>
#include "Model.h"
#include <sstream> // Для разбиения строки по разделителю
#include <algorithm> // Для std::find
using namespace std;

bool Walker::for_each(pugi::xml_node& node){
    const string node_name = node.name(); // Конверсия C-строки в string
    if (node_name == "body"){
        body_name = node.find_attribute([](pugi::xml_attribute x){return strcmp(x.name(), "name") == 0;}).value();
    }
    else if (depth() <= 1)
        body_name = OUT_OF_BODY;
    
    if (body_name == "" || body_name == "notes"){
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
            Fragment* to_be_added = new Fragment(node.value(), format_styles(), format_attrs(), ct::ContentType::TEXT, depth());
            // Перед разбиением на отдельные слова сохраняем в атрибутах каждого фрагмента-заголовка исходное название.
            if (cur_attrs.count("title") != 0)
                to_be_added->attrs["title"] = to_be_added->text;
            frags->push_back(to_be_added);
        }
        else if (node_name == "empty-line" || node_name == "subtitle"){
            Fragment* to_be_added = new Fragment("\n", format_styles(), {}, ct::ContentType::TEXT);
            frags->push_back(to_be_added);
        }
        else if (node_name == "p" || node_name == "stanza"){
            Fragment* to_be_added = new Fragment("&&&", format_styles(), {}, ct::ContentType::TEXT);
            frags->push_back(to_be_added);
        }
        else if (node_name == "v"){
            Fragment* break_to_be_added = new Fragment("\n", format_styles(), {}, ct::ContentType::TEXT);
            frags->push_back(break_to_be_added);
            // Fragment* line_to_be_added = new Fragment("SW_POEM_NEW_LINE", format_styles(), {}, ct::ContentType::TEXT);
            // frags->push_back(line_to_be_added);
        }
        else if (node_name == "poem"){
            Fragment* to_be_added = new Fragment("SW_POEM_START", format_styles(), {}, ct::ContentType::TEXT);
            frags->push_back(to_be_added);
        }
        else if (node_name == "image"){
            Fragment* to_be_added = new Fragment("", {}, format_attrs(), ct::ContentType::IMAGE);
            frags->push_back(to_be_added);
        }
        else if (node_name == "text-author" || node_name == "epigraph"){
            Fragment* break_to_be_added = new Fragment("\n", {}, format_attrs(), ct::ContentType::TEXT);
            frags->push_back(break_to_be_added);
            Fragment* to_be_added = new Fragment("SW_ALIGN_RIGHT_LINEWISE_START", {}, {}, ct::ContentType::TEXT);
            frags->push_back(to_be_added);
        }
    }
    else if (body_name == OUT_OF_BODY){
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

void Model::load_fb2(char* FILE_NAME){
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
    doc.traverse(w);
    extract_notes();
}

void Model::split_into_words(){
    int frags_len = fragments.size();
    vector<string> spec_codes = {"&&&", "SW_POEM_START", "SW_ALIGN_RIGHT_LINEWISE_START", "\n"};
    for (int index = 0; index < frags_len; index++){
        auto i = *(fragments.begin());
        if (i->type == ct::TEXT){
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