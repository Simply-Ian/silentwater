#include "View.h"
#include <string>
#include <cmath> // Для расчета границ символа
#include <iostream>
#include <algorithm>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>


View::View(){
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


    this->min_width = getPageScreenWidth() + leftPan->getSize().x*2 + leftButton->getSize().x*2;
    this->min_height = getPageScreenHeight() + 50;
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

bool SWText::checkMouseOn(sf::Vector2i pos, bool mouse_pressed){
    sf::FloatRect self_pos = getGlobalBounds();
    int click_x = parent->SCALE * (pos.x - parent->pageSprite.getPosition().x);
    int click_y = parent->SCALE * (pos.y - parent->pageSprite.getPosition().y);
    if ((click_x >= self_pos.left && click_x <= self_pos.left + self_pos.width)
            && (click_y >= self_pos.top && click_y <= self_pos.top + self_pos.height)){
        if (mouse_pressed) onClick(this);
        else onHover(this);
        return true;
    }
    return false;
}

SWText::SWText(View* parent): sf::Text::Text(){
    // this->onClick = std::bind(&SWText::, this);
    this->parent = parent;
    this->onSelected = &SWText::showSelection;
}

void SWText::showSelection(){
    this->setOutlineColor(selectionColor);
    this->setOutlineThickness(10.f);
    this->setSelected(true);
}

void SWText::open_URL(){
    string href_name = parent->doc_links_name + ":href";
    if (this->attrs.count(href_name) != 0){
        if (this->attrs[href_name].substr(0, 4) == "http"){
            string command = "xdg-open " + this->attrs[href_name];
            system(command.c_str());
        }
    }
}

void SWText::changeCursor(){
    parent->gui.setOverrideMouseCursor(tgui::Cursor::Type::Hand);
}

bool SWText::is_a_note_link(){
    // Проверяем, что текст -- ссылка на примечание
    if (this->attrs.count("type") != 0)
        return this->attrs["type"] == "note";
    return false;
}

bool SWText::checkIsSelected(sf::Vector2i pos1, sf::Vector2i pos2){
    sf::Vector2i leftTop(parent->SCALE*std::min(pos1.x, pos2.x), 
                        parent->SCALE*std::min(pos1.y, pos2.y));
    sf::Vector2i rightBottom(parent->SCALE*std::max(pos1.x, pos2.x),
                                parent->SCALE*std::max(pos1.y, pos2.y));
    sf::Vector2f pos = getPosition(); // Учесть поля!
    sf::FloatRect bounds = getBounds();
    if (pos.y >= leftTop.y && rightBottom.y - (pos.y + bounds.height) >= getCharacterSize() * 1.15){
        onSelected(this);
        return true;
    }
    else if (sf::FloatRect(pos.x, pos.y, bounds.width, bounds.height)
                        .intersects(sf::FloatRect(leftTop.x, leftTop.y, rightBottom.x - leftTop.x, rightBottom.y - leftTop.y))){
        onSelected(this);
        return true;
    }
    else if (pos.y <= leftTop.y && pos.y + bounds.height >= leftTop.y){
        if (rightBottom.y > pos.y + bounds.height && leftTop.x <= pos.x){
            onSelected(this);
            return true;
        }
    }
    else if (pos.y <= rightBottom.y && pos.y + bounds.height >= rightBottom.y){
        if(rightBottom.x >= pos.x && leftTop.y < pos.y){
            onSelected(this);
            return true;
        }
    }
    return false;
}

void SWText::hideSelection(){
    // setOutlineColor(getFillColor());
    // setOutlineThickness(0);
}

void SWText::setString(const sf::String& string){
    sf::Text::setString(string);
    source_text = string.toAnsiString();
}

sf::FloatRect SWText::getBounds(){
     // Precompute the variables needed by the algorithm
    bool isBold = getStyle() & Bold;
    float italicShear = (getStyle() & Italic) ? 0.209f : 0.f; // 12 degrees in radians
    unsigned int m_characterSize = getCharacterSize();
    sf::String m_string = getString();
    const sf::Font* m_font = getFont();
    float whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, isBold).advance;
    float letterSpacing = ( whitespaceWidth / 3.f ) * ( getLetterSpacing() - 1.f );
    whitespaceWidth += letterSpacing;
    float lineSpacing = getFont()->getLineSpacing(m_characterSize) * getLineSpacing();
    float x = 0.f;
    float y = static_cast<float>(m_characterSize);

    // Create one quad for each character
    float minX = static_cast<float>(m_characterSize);
    float minY = static_cast<float>(m_characterSize);
    float maxX = 0.f;
    float maxY = 0.f;
    sf::Uint32 prevChar = 0;
    for (std::size_t i = 0; i < m_string.getSize(); ++i)
    {
        sf::Uint32 curChar = m_string[i];

        // Skip the \r char to avoid weird graphical issues
        if (curChar == '\r')
            continue;

        // Apply the kerning offset
        x += m_font->getKerning(prevChar, curChar, m_characterSize);
        
        prevChar = curChar;

        // Handle special characters
        if ((curChar == L' ') || (curChar == L'\n') || (curChar == L'\t'))
        {
            // Update the current bounds (min coordinates)
            minX = std::min(minX, x);
            minY = std::min(minY, y);

            switch (curChar)
            {
                case L' ':  x += whitespaceWidth;     break;
                case L'\t': x += whitespaceWidth * 4; break;
                case L'\n': y += lineSpacing; x = 0;  break;
            }

            // Update the current bounds (max coordinates)
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);

            // Next glyph, no need to create a quad for whitespace
            continue;
        }

        // Apply the outline
        float m_outlineThickness = getOutlineThickness();
        if (m_outlineThickness != 0)
        {
            const sf::Glyph& glyph = m_font->getGlyph(curChar, m_characterSize, isBold, m_outlineThickness);

            float left   = glyph.bounds.left;
            float top    = glyph.bounds.top;
            float right  = glyph.bounds.left + glyph.bounds.width;
            float bottom = glyph.bounds.top  + glyph.bounds.height;

            // Update the current bounds with the outlined glyph bounds
            minX = std::min(minX, x + left   - italicShear * bottom - m_outlineThickness);
            maxX = std::max(maxX, x + right  - italicShear * top    - m_outlineThickness);
            minY = std::min(minY, y + top    - m_outlineThickness);
            maxY = std::max(maxY, y + bottom - m_outlineThickness);
        }

        // Extract the current glyph's description
        const sf::Glyph& glyph = m_font->getGlyph(curChar, m_characterSize, isBold);

        // Update the current bounds with the non outlined glyph bounds
        if (m_outlineThickness == 0)
        {
            float left   = glyph.bounds.left;
            float top    = glyph.bounds.top;
            float right  = glyph.bounds.left + glyph.bounds.width;
            float bottom = glyph.bounds.top  + glyph.bounds.height;

            minX = std::min(minX, x + left  - italicShear * bottom);
            maxX = std::max(maxX, x + right - italicShear * top);
            minY = std::min(minY, y + top);
            maxY = std::max(maxY, y + bottom);
        }

        // Advance to the next character
        x += glyph.advance + letterSpacing;
    }
    return sf::FloatRect{minX, minY, maxX - minX, maxY - minY};
}

FloatingNote::FloatingNote(tgui::GuiSFML& g, int fs){
    label = tgui::Label::create();
    label->setTextSize(fs);
    label->getRenderer()->setBackgroundColor(tgui::Color::Black);
    label->getRenderer()->setTextColor(tgui::Color::White);
    label->setMaximumTextWidth(MAX_LABEL_WIDTH);
    label->setVisible(false);
    label->setScrollbarPolicy(tgui::Scrollbar::Policy::Automatic);
    g.add(label);
}

tgui::Vector2f FloatingNote::getPosition() const{
    return this->label->getAbsolutePosition();
}

void FloatingNote::setVisible(bool v){
    this->label->setVisible(v);
}

bool FloatingNote::isVisible(){
    return this->label->isVisible();
}

void FloatingNote::updateLabel(string text, sf::Vector2f pos){
    if (label->getText() != text)
        label->setHeight(0); // Обнуляем высоту, чтобы установка размера сработала корректно
    label->setText(text);
    int debug_size = label->getSize().y;
    if (debug_size >= MAX_LABEL_HEIGHT){
        label->setAutoSize(false);
        label->setHeight(MAX_LABEL_HEIGHT);
    }
    else label->setAutoSize(true);

    int y_pos = pos.y - label->getSize().y;
    label->setPosition(pos.x - (label->getSize().x / 2), y_pos >= 0? y_pos : pos.y + 15);
    label->moveToFront();
}

FloatingNote::~FloatingNote(){
    this->label->getParentGui()->remove(this->label);
}

bool FloatingNote::isMouseOn(tgui::Vector2f pos){
    return this->label->isMouseOnWidget(pos);
}