#include <iostream>
#include <cstring> // pugixml returns all the string data as C strings
#include "load_text.h"
#include <vector>
#include <utility> // Для std::pair
#include <map>
#include "Model.h"
#include <sstream>
using namespace std;

Fragment::operator string() const{
    if(text != "&&&"){
        string for_return = "Style: <";
        for (auto i = styles.begin(); i != styles.end(); i++)
            for_return += to_string((int)(*i)) + ", ";
        for_return += "> ";
        for_return += text;
        return for_return;
    }
    else return "\n";
}

void Fragment::erase(){
    text.clear();
    text.shrink_to_fit();
    styles.clear();
    styles.shrink_to_fit();
}

Walker::Walker(vector<Fragment> *f){
    frags = f; // Сохраняет ссылку на вектор, куда будут добавляться фрагменты текста
}

bool Walker::for_each(pugi::xml_node& node){
    if (is_in_body){
        // Очищаем стили, установленные более низкими элементами
        while ((!cur_style.empty()) && cur_style.at(cur_style.size()-1).second >= depth()){
            cur_style.pop_back();
        }
        const string node_name = node.name(); // Конверсия C-строки в string
        if (names_to_styles.count(node_name) > 0)
            cur_style.push_back(style_t(names_to_styles[node_name], depth()));
        
        for (pugi::xml_attribute attr: node.attributes()){
            cur_attrs[attr.name()] = attr.value();
        }

        if (node.type() == pugi::xml_node_type::node_pcdata){
            frags->push_back(Fragment(node.value(), format_styles(), cur_attrs));
        }
        else if (strcmp(node.name(), "empty-line") == 0){
            frags->push_back(Fragment("\n", format_styles(), {}));
        }
        else if (strcmp(node.name(), "p") == 0){
            frags->push_back(Fragment("&&&", format_styles(), {}));
        }
    }
    else{
        is_in_body = strcmp(node.name(), "body") == 0;
    }
    return true;
}

vector<Styles> Walker::format_styles(){
    vector<Styles> style;
    for (auto i = cur_style.begin(); i != cur_style.end(); i++)
        style.push_back(i->first);
        return style;
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
        }
    }
    doc.traverse(w);
}

void Model::simple_out(){
    for(Fragment i: fragments)
        cout << (string)i << "\n";
}

void Model::split_into_words(){
    vector<Fragment> words;
    char* word;
    for (Fragment i: fragments){
        if (i.text != "&&&"){
            stringstream s(i.text);
            string word;
            while(getline(s, word, ' ')){
                word += " ";
                Fragment word_frag(word, i.styles, i.attrs);
                words.push_back(word_frag);
            }
        }
        else{
            Fragment word_frag("&&&", {}, {});
            words.push_back(word_frag);
        }
    }
    this->fragments = words;
    this->fragments.erase(this->fragments.begin()); // Обрезаем пустую строку перед первым абзацем
}

void Model::clear_fragments(){
    for (auto frag_it = this->fragments.begin(); frag_it != this->fragments.end(); frag_it++){
        frag_it->erase();
    }
    vector<Fragment>().swap(this->fragments);
}