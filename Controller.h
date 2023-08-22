#ifndef SW_CONTROLLER
#define SW_CONTROLLER
#include "Model.h"
#include "View.h"
#include "gui/SWText.h"
#include "datastructs/tocElem.h"
#include "Logger.h"
#include "AlignmentGroup.h"
#include <TGUI/String.hpp>
#include "datastructs/commonData.h"

using imagepair_t = pair<sf::Sprite*, sf::Texture*>;

struct Page{
    vector<SWText> words;
    vector<imagepair_t> pics;
};

class Controller{
    Model model;
    Page cur_page;
    sf::Font bookFont;
    sf::Text pageNumberText;
    sf::Color textColor{67, 67, 67};
    sf::Color bgColor{sf::Color::White};
    int cur_page_num = 0;
    int bookFontSize = 22 * view.SCALE;
    int lineInt = 3;
    vector<AlignmentGroup> align_groups;

    commonData* comd{new commonData{"", bookFontSize / view.SCALE, lineInt, "", bgColor, textColor}};
    View view{comd};

    SWText* create_text_from_instance(Fragment* frag);
    pair<sf::Sprite*, sf::Texture*> create_image_from_instance(Fragment* frag);
    void set_page_num(int new_num);
    void turn_page_back();
    void turn_page_fw();
    void draw_page();
    void toc_navigate(tgui::String name);
    void show_wordnote(SWText* t);
    void apply_font_change();
    void apply_color_change(bool is_bg);
    /// @brief Стирает информацию о страницах, фрагментах, закладках, оглавлении и группах выравнивания
    void clean_up();

    void build_up_pages_from_frags();
    float pic_resize_logic(sf::FloatRect obj_bounds, bool fullpage_mode=false);
    void add_coverpage();
    void new_page(vector<Fragment*> &page, sf::Vector2f &carriage_pos);
    void add_image(Fragment* frag, vector<Fragment*> &page, sf::Vector2f &carriage_pos);
    void add_text(Fragment* frag, vector<Fragment*> &page, sf::Vector2f &carriage_pos);
    /* Когда после стихотворных строк появляется фрагмент, не имеющий стиля Styles::POEM (это проверяется внутри add_text()),
    это значит, что стихотворение закончилось и его можно выровнять по центру. Этим и занимается apply_alignments()
    */
    void apply_alignments();
    //! Создает виджеты закладок и назнаачает callback-и
    void populate_bm_list(vector<sw::Bookmark> bms);
    string get_selected_text();

    public:
        vector<tocElem> table_of_contents;
        Logger logger{"log.txt"};
        Controller();
        void load_book(char* path);
        void loop();
        void set_page_num_and_update_toc(int new_num);
        void add_bookmark();
        void delete_bookmark(tgui::String name);
        string getFontName(){return bookFont.getInfo().family;}
        int getFontSize(){return bookFontSize / view.SCALE;}
        void setFontSize(int new_size){
            if (0 < new_size && new_size <= 72)
                bookFontSize = new_size * view.SCALE;
        }
        void openNewFile();
};

#endif