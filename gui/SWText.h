#ifndef SW_SWTEXT
#define SW_SWTEXT
#include <string>
#include <SFML/Graphics/Text.hpp>
#include <functional>
#include "../View.h"
using namespace std;

class SWText : public sf::Text{
    View* parent;
    bool is_selected = false;
    sf::Color selectionColor{0, 149, 255, 128};
    public:
        string source_text;
        std::function<void(SWText*)> onClick = [](SWText* t){};
        std::function<void (SWText*)> onHover = [](SWText* t){};
        map<string, string> attrs;
        bool is_clickable;
        bool getSelected(){return is_selected;}

        SWText(View* parent);
        bool checkMouseOn(sf::Vector2i pos);
        void open_URL();
        void changeCursor();
        bool is_a_note_link();
        /* Функция для расчетов габаритов слова. Сделана из фрагментов кода sf::Text::ensureGeometryUpdate(),
        однако, в отличие от нее, не обновляет вектор с вершинами sf::Vertex => расходует ощутимо меньше оперативной памяти
        */
        sf::FloatRect getBounds();
        void setSelected(bool v);
        void setString(const string& str);
        bool operator ==(const SWText& obj) const{
            return (obj.source_text == source_text) && (obj.getPosition() == getPosition());
        }
};
#endif