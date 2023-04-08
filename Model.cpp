#include <cstring> // pugixml returns all the string data as C-strings
#include "load_text.h"
#include <vector>
#include <utility> // Для std::pair
#include <map>
#include "Model.h"
#include <sstream> // Для разбиения строки по разделителю
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

Walker::Walker(list<Fragment> *f){
    frags = f; // Сохраняет ссылку на вектор, куда будут добавляться фрагменты текста
}

bool Walker::for_each(pugi::xml_node& node){
    if (is_in_body){
        // Очищаем стили и атрибуты, установленные более низкими элементами
        while ((!cur_style.empty()) && cur_style.at(cur_style.size()-1).second >= depth()){
            cur_style.pop_back();
        }
        while((!cur_attrs.empty()) && (--cur_attrs.end())->second.second >= depth()){
            cur_attrs.erase(--cur_attrs.end());
        }

        const string node_name = node.name(); // Конверсия C-строки в string
        if (names_to_styles.count(node_name) > 0)
            cur_style.push_back(style_t(names_to_styles[node_name], depth()));
        
        for (pugi::xml_attribute attr: node.attributes()){
            // Сохраняются только атрибуты *:href
            if (strcmp(attr.name(), (doc_links_name + ":href").c_str()) == 0)
                cur_attrs[attr.name()] = attr_t(attr.value(), depth());
        }
        if (strcmp(node.name(), "title") == 0){
            cur_attrs["title"] = attr_t("", depth());
        }

        if (node.type() == pugi::xml_node_type::node_pcdata){
            Fragment to_be_added(node.value(), format_styles(), format_attrs(), depth());
            /* Перед разбиением на отдельные слова сохраняем в атрибутах каждого фрагмента-заголовка исходное название.
            */
            if (cur_attrs.count("title") != 0)
                to_be_added.attrs["title"] = to_be_added.text;
            frags->push_back(to_be_added);
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
        }
    }
    w.doc_links_name = this->doc_links_name;
    doc.traverse(w);
}

void Model::split_into_words(){
    char* word;
    int frags_len = fragments.size();
    for (int index = 0; index < frags_len; index++){
        auto i = fragments.begin();
        if (i->text != "&&&"){
            stringstream s(i->text);
            string word;
            while(getline(s, word, ' ')){
                word += " ";
                Fragment word_frag(word, i->styles, i->attrs, i->depth);
                fragments.push_back(word_frag);
            }
        }
        else{
            Fragment word_frag("&&&", {}, {});
            fragments.push_back(word_frag);
        }
        fragments.pop_front();
    }
    this->fragments.erase(this->fragments.begin()); // Обрезаем пустую строку перед первым абзацем
}