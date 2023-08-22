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
        sf::Color textColor;
        sf::Color bgColor;

        controller_to_view(string name, int fs, int li, string pre, sf::Color bg, sf::Color tc) : 
                defaultFontName(name), bookFontSize(fs), lineInterval(li), preview(pre), bgColor(bg), textColor(tc) {}
    };

    commonData(string name, int fs, int li, string pre, sf::Color bg, sf::Color tc) : c_to_v(name, fs, li, pre, bg, tc){}

    controller_to_view c_to_v;
};

#endif