#include "Model.h"
#include "View.h"
using page_t = vector<sf::Text>;

class Controller{
    Model model;
    View view;
    page_t word_matrix;
    vector <page_t> pages;
    sf::Font bookFont;
    sf::Text pageNumberText;
    sf::Color textColor{67, 67, 67};
    int cur_page_num = 0;
    int bookFontSize = 20 * view.SCALE;
    int lineInt = 3;

    void build_up_pages();
    void create_word_matrix();
    void draw_page();

    public:
        Controller(){};
        void load_book(char* path);
        void loop();
};