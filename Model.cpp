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

bool Walker::for_each(pugi::xml_node& node){
    if (strcmp(node.name(), "body") == 0){
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

        const string node_name = node.name(); // Конверсия C-строки в string
        if (names_to_styles.count(node_name) > 0)
            cur_style.push_back(style_t(names_to_styles[node_name], depth()));
        
        for (pugi::xml_attribute attr: node.attributes()){
            // Сохраняются только атрибуты *:href
            if (strcmp(attr.name(), (doc_links_name + ":href").c_str()) == 0)
                cur_attrs[attr.name()] = attr_t(attr.value(), depth());
        }
        if (node_name == "title"){ //??
            cur_attrs["title"] = attr_t("", depth());
        }

        if (node.type() == pugi::xml_node_type::node_pcdata){
            Fragment to_be_added(node.value(), format_styles(), format_attrs(), ct::ContentType::TEXT, depth());
            // Перед разбиением на отдельные слова сохраняем в атрибутах каждого фрагмента-заголовка исходное название.
            if (cur_attrs.count("title") != 0)
                to_be_added.attrs["title"] = to_be_added.text;
            frags->push_back(to_be_added);
        }
        else if (strcmp(node.name(), "empty-line") == 0){
            frags->push_back(Fragment("\n", format_styles(), {}, ct::ContentType::TEXT));
        }
        else if (strcmp(node.name(), "p") == 0){
            frags->push_back(Fragment("&&&", format_styles(), {}, ct::ContentType::TEXT));
        }
        else if (strcmp(node.name(), "image") == 0){
            frags->push_back(Fragment("", {}, format_attrs(), ct::ContentType::IMAGE));
        }
    }
    else if (body_name == OUT_OF_BODY){
        if (strcmp(node.name(), "binary") == 0){
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
}

void Model::split_into_words(){
    int frags_len = fragments.size();
    for (int index = 0; index < frags_len; index++){
        auto i = fragments.begin();
        if (i->type == ct::TEXT){
            if (i->text != "&&&"){
                stringstream s(i->text);
                string word;
                while(getline(s, word, ' ')){
                    word += " ";
                    Fragment word_frag(word, i->styles, i->attrs, i->type, i->depth);
                    fragments.push_back(word_frag);
                }
            }
            else{
                Fragment word_frag("&&&", {}, {}, ct::TEXT);
                fragments.push_back(word_frag);
            }
        }
        else
            fragments.push_back(*i);
        fragments.pop_front();
    }
    this->fragments.erase(this->fragments.begin()); // Обрезаем пустую строку перед первым абзацем
}
