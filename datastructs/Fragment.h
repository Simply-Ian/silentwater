#ifndef SW_FRAGMENT_H
#define SW_FRAGMENT_H
#include "content_types.h"
#include <string>
#include <map>
#include <vector>

using namespace std;
enum Styles {BOLD, /*!< Полужирное начертание*/
            ITALIC, /*!< Курсивное начертание*/
            LINK, /*!< Гиперссылка. Рисуется как подчеркнутый текст синего цвета*/
            HEADER, /*!< Заголовок. Рисуется как полужирный текст размером на 5 пт больше*/
            IMAGE,
            POEM,
            TEXT_AUTHOR,
            SUBTITLE,
            EPIGRAPH
};
using style_t = pair<Styles, int>; // Элемент списка стилей

struct Fragment{
    Fragment(string t, vector<Styles> s, map<string, string> a, sw::ContentType ct, int d=-1):
        text(t), 
        styles(s), 
        attrs(a),
        depth(d),
        type(ct)
        {

        }

    Fragment(string t, vector<Styles> s, map<string, string> a, sw::ContentType ct, int _x, int _y, int d=-1):
    text(t), 
    styles(s), 
    attrs(a),
    depth(d),
    type(ct),
    x(_x),
    y(_y)
    {

    }
    
    Fragment(): text(""), styles{}, attrs{}, depth(-1), type(sw::ContentType::TEXT)
    {

    }

    string text;
    sw::ContentType type;
    vector<Styles> styles;
    map<string, string> attrs;
    // int align_group_num = -1; // Индекс группы выравнивания в Controller::align_groups
    int x = 0;
    int y = 0;
    int width = 0;
    int depth = 0;

    operator string() const;
    bool has_a_style(Styles style);
};
#endif