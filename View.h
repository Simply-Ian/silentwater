#include <TGUI/Core.hpp>
#include <TGUI/Backends/SFML.hpp>
#include <TGUI/AllWidgets.hpp>
using namespace std;


class View{
    const int SIDEBAR_WIDTH = 350;
    int PAGE_WIDTH;
    int PAGE_HEIGHT;
    int TOP_BAR_HEIGHT = 75;
    float LF_WIDTH = 55.0;
    float RF_WIDTH = 45.0;
    float H_HEIGHT = 45.0;
    float F_HEIGHT = 70.0;

    public:
        sf::RenderWindow win;
        tgui::GuiSFML gui;
        sf::RenderTexture page;
        sf::Sprite pageSprite;
        sf::Font bookFont;
        int bookFontSize = 20;
        int lineInt = 3;

        View();
        void onWinResize(sf::Event::SizeEvent new_size);
        void update();
        void drawPage(vector<sf::Text*>* page_matrix);

};