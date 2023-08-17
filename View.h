#ifndef SW_VIEW
#define SW_VIEW
#include <TGUI/Core.hpp>
#include <TGUI/Backends/SFML.hpp>
#include <TGUI/AllWidgets.hpp> //<! \todo Оставить только нужные заголовки
#include <functional>
#include <string>
#include <map>
#include <functional>
#include "datastructs/tocElem.h"
#include "gui/BookmarkWidget.h"
#include "gui/MyScrollablePanel.h"
#include "gui/FloatingNote.h"
#include "gui/FontDialog.h"
#include "datastructs/commonData.h"
using namespace std;

struct View{
    commonData* comd;
    const int SIDEBAR_WIDTH = 350;
    int PAGE_WIDTH;
    int PAGE_HEIGHT;
    static inline int SCALE {2};
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
    tgui::MessageBox::Ptr msgBox;
    tgui::Panel::Ptr topPan;
    tgui::Button::Ptr chooseFontButton;
    FontDialog::Ptr fontDial;

    sf::RenderTexture page;
    sf::Sprite pageSprite;
    FloatingNote word_note{gui, GUI_TEXT_SIZE-2};
    tgui::Cursor::Type cur_cursor_type = tgui::Cursor::Type::Arrow;

    View(commonData* c);
    void onWinResize(sf::Event::SizeEvent new_size);
    void update();
    void build_up_toc(vector<tocElem> items);
    int getPageScreenWidth(); //!< Рассчитывает ширину области на экране, которую займет страница.
    void showFloatingNote(string text, sf::Vector2f pos);
    void showTemporalNotification(string text, int msDur);
    void createFontDialog();

    private:
        
        int getPageScreenHeight();
        unsigned int min_width; //!< Минимальные ширина и высота окна
        unsigned int min_height;
};
#endif