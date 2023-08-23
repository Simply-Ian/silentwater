#ifndef SW_COMMONDATA
#define SW_COMMONDATA
#include <string>
#include <SFML/Graphics/Color.hpp>
using namespace std;

/// Предназначена для передачи данных от контроллера представлению и обратно
struct commonData{
    struct controller_to_view{
        int bookFontSize;
        int lineInterval;
        string defaultFontName;
        string preview;
        string theme_path;
        sf::Color textColor;
        sf::Color bgColor;

        controller_to_view(string name, string tp, int fs, int li, string pre, sf::Color bg, sf::Color tc) : 
                defaultFontName(name), theme_path(tp), bookFontSize(fs), lineInterval(li), preview(pre), bgColor(bg), textColor(tc) {}
    };

    commonData(string name, string tp, int fs, int li, string pre, sf::Color bg, sf::Color tc) : 
        c_to_v(name, tp, fs, li, pre, bg, tc){}

    controller_to_view c_to_v;
};

#endif