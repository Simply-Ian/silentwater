#include "View.h"
#include <string>
#include <cmath> // Для расчета границ символа
#include <iostream>


View::View(commonData* c) : comd(c){
    sf::ContextSettings settings;
    settings.antialiasingLevel = 5;
    win.create(sf::VideoMode::getDesktopMode(), "Silent Water Reader", sf::Style::Default, settings);
    win.setFramerateLimit(50);

    PAGE_WIDTH = (win.getSize().x * 0.37) * SCALE;
    PAGE_HEIGHT = (win.getSize().y - 2*TOP_BAR_HEIGHT) * SCALE;
    page.create(PAGE_WIDTH, PAGE_HEIGHT);
    page.setSmooth(true);
    float pageSpriteX = (win.getSize().x - getPageScreenWidth()) / 2.0f;
    float pageSpriteY = 75;
    pageSprite.setPosition({pageSpriteX, pageSpriteY});
    pageSprite.setScale(1.0f / SCALE, 1.0f / SCALE);

    gui.setTarget(win);

    leftButton = tgui::BitmapButton::create();
    gui.add(leftButton, "leftButton");
    leftButton->setImage(tgui::Texture("Icons/chevronLeft.png"));
    leftButton->setSize(64, PAGE_HEIGHT / SCALE);
    leftButton->setOrigin(0, 0);
    leftButton->setPosition(pageSpriteX - 64, pageSpriteY);
    leftButton->getRenderer()->setOpacity(0.4f);

    rightButton = tgui::BitmapButton::create();
    gui.add(rightButton, "rightButton");
    rightButton->setImage(tgui::Texture("Icons/chevronRight.png"));
    rightButton->setSize(64, PAGE_HEIGHT / SCALE);
    rightButton->setOrigin(0, 0);
    rightButton->setPosition(pageSpriteX + getPageScreenWidth(), pageSpriteY);
    rightButton->getRenderer()->setOpacity(0.4f);
    
    leftPan = tgui::Panel::create({300, "parent.height - 50"});
    leftPan->setPosition({0, 50});
    toc_header = tgui::Label::create("Оглавление");
    toc_header->setTextSize(GUI_TEXT_SIZE + 3);
    toc_header->setPosition({5, 0});
    toc_header->setWidgetName("toc_header");
    leftPan->add(toc_header);

    tocList = tgui::ListBox::create();
    tocList->setSize("parent.width - 10", "parent.height - 8 - toc_header.height");
    tocList->setTextSize(GUI_TEXT_SIZE);
    tocList->getRenderer()->setSelectedTextStyle(tgui::TextStyle::Bold);
    tocList->getRenderer()->setBorders(1);
    tocList->getRenderer()->setBorderColor({65, 65, 65});
    tocList->setSelectedItemByIndex(0);
    tocList->setScrollbarValue(0);
    tocList->setPosition({5, "toc_header.top + toc_header.height + 3"});
    leftPan->add(tocList);
    gui.add(leftPan);

    rightPan = tgui::Panel::create({300, "parent.height - 50"});
    rightPan->setPosition({"parent.width - 300", 50});

    bm_header = tgui::Label::create("Закладки");
    bm_header->setAutoSize(true);
    bm_header->setTextSize(GUI_TEXT_SIZE + 2);
    bm_header->setPosition({5, (48 - GUI_TEXT_SIZE) / 2});
    bm_header->setWidgetName("bm_header");
    rightPan->add(bm_header);

    add_bm_button = tgui::Button::create("+");
    add_bm_button->setSize({50, 50});
    add_bm_button->setPosition({"parent.width - width - 5", 2});
    add_bm_button->setWidgetName("add_bm_button");
    rightPan->add(add_bm_button);

    bmPan = MyScrollablePanel::create({"parent.width - 10", "parent.height / 2 - 2 - add_bm_button.height"});
    bmPan->getRenderer()->setBorderColor(tgui::Color::Black);
    bmPan->getRenderer()->setBorders(1);
    bmPan->setHorizontalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
    bmPan->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Always);
    bmPan->setPosition({5, "add_bm_button.height + 5"});

    rightPan->add(bmPan);

    go_to_header = tgui::Label::create("Быстрый переход");
    go_to_header->setAutoSize(true);
    go_to_header->setTextSize(GUI_TEXT_SIZE + 2);
    go_to_header->setPosition({5, "parent.height / 2 + 18"});
    go_to_header->setWidgetName("go_to_header");
    rightPan->add(go_to_header);

    go_to_box = tgui::EditBox::create();
    go_to_box->setSize({"parent.width - 10", GUI_TEXT_SIZE + 8});
    go_to_box->setTextSize(GUI_TEXT_SIZE);
    go_to_box->setPosition({5, "go_to_header.bottom + 20"});
    go_to_box->setWidgetName("go_to_box");
    go_to_box->setInputValidator(tgui::EditBox::Validator::Float);
    go_to_box->setMaximumCharacters(5);
    go_to_box->setDefaultText("Номер стр. или % от объема");
    go_to_box->setMouseCursor(tgui::Cursor::Type::Text);
    rightPan->add(go_to_box);

    go_to_page = tgui::Button::create("Перейти на страницу");
    go_to_page->setSize({"parent.width - 10", GUI_TEXT_SIZE + 8});
    go_to_page->setPosition({5, "go_to_box.bottom + 3"});
    go_to_page->setTextSize(GUI_TEXT_SIZE);
    go_to_page->setWidgetName("go_to_page");
    rightPan->add(go_to_page);

    go_to_percent = tgui::Button::create("Перейти на % от объема");
    go_to_percent->setSize({"parent.width - 10", GUI_TEXT_SIZE + 8});
    go_to_percent->setPosition({5, "go_to_page.bottom + 3"});
    go_to_percent->setWidgetName("go_to_percent");
    go_to_percent->setTextSize(GUI_TEXT_SIZE);
    rightPan->add(go_to_percent);

    gui.add(rightPan);

    msgBox = tgui::MessageBox::create();
    msgBox->getRenderer()->setOpacity(0.7);
    msgBox->setSize({"parent.width / 15", GUI_TEXT_SIZE});
    msgBox->getRenderer()->setBackgroundColor(tgui::Color::Black);
    msgBox->setTextSize(GUI_TEXT_SIZE);
    msgBox->getRenderer()->setTextColor(tgui::Color::White);
    msgBox->getRenderer()->setTitleBarHeight(0);
    msgBox->setPosition({"(parent.width - width) / 2", "parent.height * 0.8"});
    msgBox->setWidgetName("msgBox");
    msgBox->setVisible(false);
    gui.add(msgBox);

    topPan = tgui::Panel::create({"parent.width / 3", 50});
    topPan->setPosition("(parent.width - width) / 2", 0);
    gui.add(topPan);
    chooseFontButton = tgui::Button::create();
    chooseFontButton->setPosition({10, "(parent.height - height) / 2"});
    chooseFontButton->onClick(&View::createFontDialog, this);
    topPan->add(chooseFontButton);

    this->min_width = getPageScreenWidth() + leftPan->getSize().x*2 + leftButton->getSize().x*2;
    this->min_height = getPageScreenHeight() + 50;
}

void View::createFontDialog(){
    gui.remove(gui.get("fontDial"));
    fontDial = FontDialog::create(comd->c_to_v.defaultFontName, comd->c_to_v.bookFontSize, comd->c_to_v.lineInterval, comd->c_to_v.preview);
    fontDial->setPosition("(parent.width - width) / 2", "(parent.height - height) / 2");
    fontDial->setWidgetName("fontDial");
    fontDial->okButton->onClick(onFontChange);
    gui.add(fontDial);
}

void View::onWinResize(sf::Event::SizeEvent new_size){
    sf::Vector2u win_size = win.getSize();
    float pageSpriteX = (win_size.x - getPageScreenWidth()) / 2.0f;
    float leftPanWidth = leftPan->getSize().x;
    float leftButtonWidth = leftButton->getSize().x;
    if (pageSpriteX < leftPanWidth + leftButtonWidth)
        pageSpriteX = leftPanWidth + leftButtonWidth;
    pageSprite.setPosition({pageSpriteX, 75});
    rightButton->setPosition(pageSpriteX + getPageScreenWidth(), 75);
    leftButton->setPosition(pageSpriteX - 64, 75);

    win.setView(sf::View(sf::FloatRect{0.f, 0.f, static_cast<float>(win_size.x),
                                         static_cast<float>(win_size.y)}));
    gui.setAbsoluteView(tgui::FloatRect{0.f, 0.f, static_cast<float>(win_size.x),
                                         static_cast<float>(win_size.y)});
}

void View::update(){
    win.clear({155, 155, 155, 255});
    win.draw(pageSprite);
    gui.draw();
    win.display();
}

void View::build_up_toc(vector<tocElem> items){
    int init_depth = -1;
    for (tocElem i: items){
        if (init_depth == -1) init_depth = i.depth;
        tgui::String header = i.headline;

        int cur_depth = i.depth - init_depth >= 0? i.depth - init_depth : 1;
        header.insert(0, cur_depth * 2 + 1, ' ');
        tocList->addItem(header, tgui::String::fromNumber(i.page));
    }
}

int View::getPageScreenWidth(){
    return PAGE_WIDTH / SCALE;
}

int View::getPageScreenHeight(){
    return PAGE_HEIGHT / SCALE;
}

void View::showFloatingNote(string text, sf::Vector2f pos){
    this->word_note.updateLabel(text, pos);
    word_note.setVisible(true);
}

void View::showTemporalNotification(string text, int msDur){
    msgBox->setText(text);
    msgBox->setVisible(true);
    tgui::Timer::Ptr timer;
    const auto cb = std::bind([](tgui::Timer::Ptr t, View* v){
                                t->setEnabled(false);
                                v->msgBox->setVisible(false);
                                }, placeholders::_1, this);
    timer = tgui::Timer::create(cb, tgui::Duration(msDur));
}
