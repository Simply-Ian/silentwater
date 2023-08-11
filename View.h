#include <TGUI/Core.hpp>
#include <TGUI/Backends/SFML.hpp>
#include <TGUI/AllWidgets.hpp> //<! \todo Оставить только нужные заголовки
#include <functional>
#include <string>
#include <map>
#include <functional>
#include "tocElem.h"
#include "BookmarkWidget.h"
#include "MyScrollablePanel.h"
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

struct View{
    const int SIDEBAR_WIDTH = 350;
    int PAGE_WIDTH;
    int PAGE_HEIGHT;
    int SCALE = 2;
    int TOP_BAR_HEIGHT = 75;
    int GUI_TEXT_SIZE = 18;
    float LF_WIDTH = 55.0 * SCALE;
    float RF_WIDTH = 15.0 * SCALE;
    float H_HEIGHT = 45.0 * SCALE;
    float F_HEIGHT = 85.0 * SCALE;
    
    string doc_links_name;

    sf::RenderWindow win;
    sf::ContextSettings settings;
    tgui::GuiSFML gui;
    tgui::BitmapButton::Ptr leftButton;
    tgui::BitmapButton::Ptr rightButton;
    tgui::Panel::Ptr leftPan;
    tgui::Panel::Ptr rightPan;
    MyScrollablePanel::Ptr bmPan;
    tgui::ListBox::Ptr tocList;
    tgui::Label::Ptr toc_header;
    tgui::Label::Ptr bm_header;
    tgui::Button::Ptr add_bm_button;
    tgui::EditBox::Ptr go_to_box;
    tgui::Button::Ptr go_to_page;
    tgui::Button::Ptr go_to_percent;
    tgui::Label::Ptr go_to_header;
    sf::RenderTexture page;
    sf::Sprite pageSprite;
    FloatingNote word_note{gui, GUI_TEXT_SIZE-2};

    View();
    void onWinResize(sf::Event::SizeEvent new_size);
    void update();
    void build_up_toc(vector<tocElem> items);
    int getPageScreenWidth(); //!< Рассчитывает ширину области на экране, которую займет страница.
    void showFloatingNote(string text, sf::Vector2f pos);

    private:
        
        int getPageScreenHeight();
        unsigned int min_width; //!< Минимальные ширина и высота окна
        unsigned int min_height;
};

class SWText : public sf::Text{
    View* parent;
    bool is_selected;
    void showSelection();
    void hideSelection();
    sf::Color selectionColor{0, 149, 255, 128};

    public:
        void setSelected(bool v){is_selected = v; hideSelection();};
        string source_text;
        std::function<void(SWText*)> onClick = [](SWText* t){};
        std::function<void (SWText*)> onHover = [](SWText* t){};
        std::function<void (SWText*)> onSelected = [](SWText* t){};
        map<string, string> attrs;
        bool is_clickable;
        bool getSelected(){return is_selected;};

        SWText(View* parent);
        bool checkMouseOn(sf::Vector2i pos, bool mouse_pressed); //!< Проверяет, наведена ли мышь на слово и вызывает соответствующий коллбэк
        bool checkIsSelected(sf::Vector2i pos1, sf::Vector2i pos2);
        void open_URL();
        void changeCursor();
        bool is_a_note_link();
        /* Функция для расчетов габаритов слова. Сделана из фрагментов кода sf::Text::ensureGeometryUpdate(),
        однако, в отличие от нее, не обновляет вектор с вершинами sf::Vertex => расходует ощутимо меньше оперативной памяти
        */
        sf::FloatRect getBounds();
        void setString(const sf::String& string);
};