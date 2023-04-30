#ifndef SW_MODEL_H
#define SW_MODEL_H

#include <utility> // Для std::pair
#include <string>
#include <vector>
#include <list> //Для хранения фрагментов
#include "pugixml.hpp"
#include <map>
#include "AlignmentGroup.h"
#include "Fragment.h"
using namespace std;


using attr_t = pair<string, int>;

using matrix_t = vector<vector<Fragment*>>;

struct Walker: pugi::xml_tree_walker{
    public:
        map<string, attr_t> cur_attrs;
        list<Fragment*> *frags;
        map<string, string>* binaries;
        vector<style_t> cur_style;
        string body_name = "out_of_body"; /*<! Согласно стандарту fb2, первый безымянный раздел body документа содержит основной 
        текст; раздел body с именем "notes" -- примечания */
        string doc_links_name;

        Walker(list<Fragment*> *f, map<string, string> *b) : frags(f), binaries(b) {};
        virtual bool for_each(pugi::xml_node& node);
    
    private:
        map<string, Styles> names_to_styles = {
            {"strong", Styles::BOLD},
            {"emphasis", Styles::ITALIC},
            {"a", Styles::LINK},
            {"title", Styles::HEADER},
            {"image", Styles::IMAGE},
            {"poem", Styles::POEM},
            {"text-author", Styles::TEXT_AUTHOR},
            {"subtitle", Styles::SUBTITLE},
            {"epigraph", Styles::EPIGRAPH}
        };

        // Создает список стилей для передачи конструктору Fragment
        vector<Styles> format_styles();
        // Создает список атрибутов для передачи конструктору Fragment
        map<string, string> format_attrs();
        const string OUT_OF_BODY = "out_of_body";
};

class Model{
    public:
        list<Fragment*> fragments;
        matrix_t pages;
        string doc_links_name;
        map<string, string> binaries;
        map<string, string> notes;

        // Загружает fb2-файл, определяет кодировку, перезагружает файл в Юникод и запускает разбор xml
        void load_fb2(char* FILE_NAME);
        void split_into_words();
    private:
        Walker w{&fragments, &binaries};
        pugi::xml_document doc;
};

#endif