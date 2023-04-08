#include <utility>
#include <string>
#include <vector>
#include <list> //Для хранения фрагментов
#include "pugixml.hpp"
#include <map>
using namespace std;

enum Styles {BOLD, /*!< Полужирное начертание*/
            ITALIC, /*!< Курсивное начертание*/
            LINK, /*!< Гиперссылка. Рисуется как подчеркнутый текст синего цвета*/
            HEADER /*!< Заголовок. Рисуется как полужирный текст размером на 5 пт больше*/
};
using style_t = pair<Styles, int>; // Элемент списка стилей
using attr_t = pair<string, int>;

struct Fragment{
    Fragment(string t, vector<Styles> s, map<string, string> a, int d=-1):
        text(t), 
        styles(s), 
        attrs(a),
        depth(d)
        {

        }
    string text;
    vector<Styles> styles;
    map<string, string> attrs;
    int x = 0;
    int y = 0;
    int depth = 0;

    operator string() const;
};

using matrix_t = vector<vector<Fragment>>;

struct Walker: pugi::xml_tree_walker{
    public:
        map<string, attr_t> cur_attrs;
        list<Fragment> *frags;
        vector<style_t> cur_style;
        bool is_in_body = false;
        string doc_links_name;

        Walker(list<Fragment> *f);
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
        // Создает список атрибутов для передачи конструктору Fragment
        map<string, string> format_attrs();
};

class Model{
    public:
        list<Fragment> fragments;
        matrix_t pages;

        // Загружает fb2-файл, определяет кодировку, перезагружает файл в Юникод и запускает разбор xml
        void load_fb2(char* FILE_NAME);
        void split_into_words();
        string doc_links_name;
    
    private:
        Walker w{&fragments};
        pugi::xml_document doc;
};