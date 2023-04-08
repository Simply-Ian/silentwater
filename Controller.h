#include "Model.h"
#include "View.h"
#include "tocElem.h"
#include "Logger.h"

using page_t = vector<SWText>;

class Controller{
    Model model;
    View view;
    page_t cur_page;
    sf::Font bookFont;
    sf::Text pageNumberText;
    sf::Color textColor{67, 67, 67};
    int cur_page_num = 0;
    int bookFontSize = 20 * view.SCALE;
    int lineInt = 3;

    SWText* create_text_from_instance(Fragment frag);
    void build_up_pages_from_frags();
    void set_page_num(int new_num);
    void set_page_num_and_update_toc(int new_num);
    void turn_page_back();
    void turn_page_fw();
    void draw_page();
    void toc_navigate(tgui::String name);

    public:
        vector<tocElem> table_of_contents;
        Logger logger{"log.txt"};
        Controller();
        void load_book(char* path);
        void loop();
};