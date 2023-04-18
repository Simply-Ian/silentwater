#include <utility>
#include <string>
#include <vector>
#include <list> //Для хранения фрагментов
#include "pugixml.hpp"
#include <map>
#include "content_types.h"
#include "AlignmentGroup.h"
using namespace std;

enum Styles {BOLD, /*!< Полужирное начертание*/
            ITALIC, /*!< Курсивное начертание*/
            LINK, /*!< Гиперссылка. Рисуется как подчеркнутый текст синего цвета*/
            HEADER, /*!< Заголовок. Рисуется как полужирный текст размером на 5 пт больше*/
            IMAGE,
            POEM,
            SUBTITLE
};

using style_t = pair<Styles, int>; // Элемент списка стилей
using attr_t = pair<string, int>;

struct Fragment{
    Fragment(string t, vector<Styles> s, map<string, string> a, ct::ContentType ct, int d=-1):
        text(t), 
        styles(s), 
        attrs(a),
        depth(d),
        type(ct)
        {

        }

    Fragment(string t, vector<Styles> s, map<string, string> a, ct::ContentType ct, int _x, int _y, int d=-1):
    text(t), 
    styles(s), 
    attrs(a),
    depth(d),
    type(ct),
    x(_x),
    y(_y)
    {

    }
    
    Fragment(): text(""), styles{}, attrs{}, depth(-1), type(ct::ContentType::TEXT)
    {

    }

    string text;
    ct::ContentType type;
    vector<Styles> styles;
    map<string, string> attrs;
    int align_group_num = -1; // Индекс группы выравнивания в Controller::align_groups
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
        map<string, string>* binaries;
        vector<style_t> cur_style;
        string body_name = "out_of_body"; /*<! Согласно стандарту fb2, первый безымянный раздел body документа содержит основной 
        текст; раздел body с именем "notes" -- примечания */
        string doc_links_name;

        Walker(list<Fragment> *f, map<string, string> *b) : frags(f), binaries(b) {};
        virtual bool for_each(pugi::xml_node& node);
    
    private:
        map<string, Styles> names_to_styles = {
            pair<string, Styles>("strong", Styles::BOLD),
            pair<string, Styles>("emphasis", Styles::ITALIC),
            pair<string, Styles>("a", Styles::LINK),
            pair<string, Styles>("title", Styles::HEADER),
            pair<string, Styles>("image", Styles::IMAGE),
            pair<string, Styles>("poem", Styles::POEM),
            pair<string, Styles>("subtitle", Styles::SUBTITLE)
        };

        // Создает список стилей для передачи конструктору Fragment
        vector<Styles> format_styles();
        // Создает список атрибутов для передачи конструктору Fragment
        map<string, string> format_attrs();
        const string OUT_OF_BODY = "out_of_body";
};

class Model{
    public:
        list<Fragment> fragments;
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