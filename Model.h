#include <utility>
#include <string>
#include <vector>
#include "pugixml.hpp"
#include <map>
using namespace std;

enum Styles {BOLD, ITALIC, LINK, HEADER};
using style_t = pair<Styles, int>; // Элемент списка стилей

struct Fragment{
    Fragment(string t, vector<Styles> s, map<string, string> a){text = t; styles = s; attrs = a;}
    string text;
    vector<Styles> styles;
    map<string, string> attrs;
    int len();

    operator string() const;
};

struct Walker: pugi::xml_tree_walker{
    public:
        map<string, string> cur_attrs;
        vector<Fragment> *frags;
        vector<style_t> cur_style;
        bool is_in_body = false;

        Walker(vector<Fragment> *f);
        virtual bool for_each(pugi::xml_node& node);
    
    private:
        map<string, Styles> names_to_styles = {
            pair<string, Styles>("strong", Styles::BOLD),
            pair<string, Styles>("emphasis", Styles::ITALIC),
            pair<string, Styles>("a", Styles::LINK),
            pair<string, Styles>("title", Styles::HEADER)
        };

        // Создает список стилей для передачи конструктору Fragment
        vector<Styles> format_styles();
};

class Model{
    public:
        vector<Fragment> fragments;

        // Загружает fb2-файл, определяет кодировку, перезагружает файл в Юникод и запускает разбор xml
        void load_fb2(char* FILE_NAME);
        void simple_out();
        void split_into_words();
        string doc_links_name;
    
    private:
        Walker w{&fragments};
        pugi::xml_document doc;
};