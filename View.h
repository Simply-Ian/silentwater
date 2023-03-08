#include <TGUI/Core.hpp>
#include <TGUI/Backends/SFML.hpp>
#include <TGUI/AllWidgets.hpp>
#include <functional>
#include <string>
#include <map>
#include <functional>
using namespace std;

struct View{
    const int SIDEBAR_WIDTH = 350;
    int PAGE_WIDTH;
    int PAGE_HEIGHT;
    int SCALE = 2;
    int TOP_BAR_HEIGHT = 75;
    float LF_WIDTH = 55.0 * SCALE;
    float RF_WIDTH = 15.0 * SCALE;
    float H_HEIGHT = 45.0 * SCALE;
    float F_HEIGHT = 85.0 * SCALE;

    string doc_links_name;

    sf::RenderWindow win;
    tgui::GuiSFML gui;
    tgui::BitmapButton::Ptr leftButton;
    tgui::BitmapButton::Ptr rightButton;
    sf::RenderTexture page;
    sf::Sprite pageSprite;

    View();
    void onWinResize(sf::Event::SizeEvent new_size);
    void update();
};

class SWText : public sf::Text{
    View* parent;
    public:
        std::function<void(SWText*)> onClick = [](SWText* t){};
        std::function<void(SWText*)> onHover = [](SWText* t){};
        map<string, string> attrs;
        bool is_clickable;

        SWText(View* parent);
        bool checkMouseOn(sf::Vector2i pos);
        void open_URL();
        void changeCursor();
};