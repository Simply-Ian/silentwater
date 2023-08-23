#ifndef SW_SETTINGS
#define SW_SETTINGS
#include <string>
#include <SFML/Graphics/Color.hpp>

struct Settings{
    std::string theme_path;
    std::string font_path;
    int font_size;
    int line_int;
    sf::Color textColor;
    sf::Color bgColor;
    std::string last_seen;
    
    Settings(const char* p, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) : 
        theme_path(p), bgColor(r1, g1, b1), textColor(r2, g2, b2){}
    Settings(){};
};
#endif