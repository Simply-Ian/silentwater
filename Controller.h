#include "Model.h"
#include "View.h"
using page_t = vector<sf::Text*>;

struct Controller{
    Model model;
    View view;
    page_t word_matrix;
    vector <page_t*> page_matrices;
    int cur_page = 0;

    Controller(){};
    void load_book(char* path);
    void build_up_pages(int line_len_px);
    void create_word_matrix();
    void loop();
    void draw_page();
};