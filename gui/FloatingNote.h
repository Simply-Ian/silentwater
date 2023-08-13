#ifndef SW_FLOATINGNOTE
#define SW_FLOATINGNOTE

#include <string>
#include <SFML/System/Vector2.hpp>
#include <TGUI/Vector2.hpp>
#include <TGUI/Backends/SFML.hpp>
#include <TGUI/Widgets/Label.hpp>
using namespace std;

class FloatingNote{
    string text;
    tgui::Label::Ptr label;
    const int MAX_LABEL_HEIGHT = 250;
    const int MAX_LABEL_WIDTH = 210;

    public:
        void updateLabel(string text, sf::Vector2f pos);
        void setVisible(bool v);
        bool isVisible();
        bool isMouseOn(tgui::Vector2f pos);
        // FloatingNote(string t, int f, sf::Vector2f pos, tgui::GuiSFML& g);
        FloatingNote(tgui::GuiSFML& g, int fs);
        ~FloatingNote();
        // void render(sf::RenderWindow& win);
        tgui::Vector2f getPosition() const;
};
#endif