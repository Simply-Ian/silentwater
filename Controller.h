#include "Model.h"
#include "View.h"
#include "tocElem.h"
#include "Logger.h"

using imagepair_t = pair<sf::Sprite*, sf::Texture*>;

struct Page{
    vector<SWText> words;
    vector<imagepair_t> pics;
};

class Controller{
    Model model;
    View view;
    Page cur_page;
    sf::Font bookFont;
    sf::Text pageNumberText;
    sf::Color textColor{67, 67, 67};
    int cur_page_num = 0;
    int bookFontSize = 20 * view.SCALE;
    int lineInt = 3;

    SWText* create_text_from_instance(Fragment frag);
    pair<sf::Sprite*, sf::Texture*> create_image_from_instance(Fragment frag);
    void set_page_num(int new_num);
    void set_page_num_and_update_toc(int new_num);
    void turn_page_back();
    void turn_page_fw();
    void draw_page();
    void toc_navigate(tgui::String name);

    void build_up_pages_from_frags();
    float pic_resize_logic(sf::FloatRect obj_bounds, bool fullpage_mode=false);
    void add_coverpage();
    void new_page(vector<Fragment> &cur_page, sf::Vector2f &carriage_pos);
    void add_image(Fragment frag, vector<Fragment> &cur_page, sf::Vector2f &carriage_pos);
    void add_text(Fragment frag, vector<Fragment> &cur_page, sf::Vector2f &carriage_pos);

    public:
        vector<tocElem> table_of_contents;
        Logger logger{"log.txt"};
        Controller();
        void load_book(char* path);
        void loop();
};