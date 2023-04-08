#include "View.h"
#include <string>
#include <cmath> // Для расчета границ символа


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
    gui.draw();
    win.draw(pageSprite);
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

bool SWText::checkMouseOn(sf::Vector2i pos){
    sf::FloatRect self_pos = getGlobalBounds();
    int click_x = parent->SCALE * (pos.x - parent->pageSprite.getPosition().x);
    int click_y = parent->SCALE * (pos.y - parent->pageSprite.getPosition().y);
    return (click_x >= self_pos.left && click_x <= self_pos.left + self_pos.width)
            && (click_y >= self_pos.top && click_y <= self_pos.top + self_pos.height);
}

SWText::SWText(View* parent): sf::Text::Text(){
    this->parent = parent;
}

void SWText::open_URL(){
    string href_name = parent->doc_links_name + ":href";
    string command = "xdg-open " + this->attrs[href_name];
    if (this->attrs.count(href_name) != 0){
        if (this->attrs[href_name].substr(0, 4) == "http")
            system(command.c_str());
    }
}

void SWText::changeCursor(){
    parent->gui.setOverrideMouseCursor(tgui::Cursor::Type::Hand);
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