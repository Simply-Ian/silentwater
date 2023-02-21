#include <iostream>
#include "pugixml.hpp"
#include <cstring> // pugixml returns all the string data as C strings
#include "load_text.h"
#include <vector>
#include <utility> // Для std::pair
#include <map>
using namespace std;

const char* node_types[] =
{
    "null", "document", "element", "pcdata", "cdata", "comment", "pi", "declaration"
};

enum class Styles {BOLD, ITALIC, LINK, HEADER};
using style_t = pair<Styles, int>; // Элемент списка стилей

struct Fragment{
    Fragment(const char* t, vector<Styles> s){text = t; styles = s;}
    const char* text;
    vector<Styles> styles;

    operator string() const{
        string for_return = "Style: ";
        for (auto i = styles.begin(); i != styles.end(); i++)
            for_return += to_string((int)(*i)) + ", ";
        for_return += "\n";
        for_return += text;
        return for_return;
    }
};

struct Walker: pugi::xml_tree_walker{
    public:
        vector<Fragment> *frags;
        vector<style_t> cur_style = {};
        bool is_in_body = false;

        Walker(vector<Fragment> *f, vector<style_t> a){
            frags = f; cur_style = a;
        }

        virtual bool for_each(pugi::xml_node& node){
            
            if (is_in_body){
                // Очищаем стили, установленные более низкими элементами
                while ((!cur_style.empty()) && cur_style.at(cur_style.size()-1).second >= depth()){
                    cur_style.pop_back();
                }

                const string node_name = node.name(); // Конверсия C-строки в string
                if (names_to_styles.count(node_name) > 0)
                    cur_style.push_back(style_t(names_to_styles[node_name], depth()));
                
                if (node.type() == pugi::xml_node_type::node_pcdata){
                    frags->push_back(Fragment(node.value(), format_styles()));
                }
                else if (strcmp(node.name(), "empty-line") == 0){
                    frags->push_back(Fragment("\n", format_styles()));
                }
            }
            else{
                is_in_body = strcmp(node.name(), "body") == 0;
            }
            return true;
        }
    private:
        map<string, Styles> names_to_styles = {
            pair<string, Styles>("strong", Styles::BOLD),
            pair<string, Styles>("emphasis", Styles::ITALIC),
            pair<string, Styles>("a", Styles::LINK),
            pair<string, Styles>("title", Styles::HEADER)
        };
        // Создает список стилей для передачи конструктору Fragment
        vector<Styles> format_styles(){
            vector<Styles> style;
            for (auto i = cur_style.begin(); i != cur_style.end(); i++)
                style.push_back(i->first);
            return style;
        }
};

int main(){
    char FILE_NAME[] = "RedRing.fb2";
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(FILE_NAME, pugi::parse_declaration);
    if (!result) throw runtime_error(result.description());
    char* encoding_name = (char*)(doc.first_child().last_attribute().value());
    // cout << encoding_name << "\n";

    string* re_opened = load_text(FILE_NAME, encoding_name);
    doc.reset();
    result = doc.load_string(re_opened->c_str());
    delete re_opened;

    vector<Fragment> fragments;
    Walker w{&fragments, {}};
    doc.traverse(w);
    for(Fragment i: fragments){
        cout << (string)i << "\n";
    }
    return 0;
}