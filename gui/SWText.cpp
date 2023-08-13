#include "SWText.h"

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

void SWText::setSelected(bool v){
    setOutlineColor(v? selectionColor : sf::Color::Black);
    setOutlineThickness(v? 10.f : 0.f);
    is_selected = v;
}

void SWText::setString(const string& str){
    sf::Text::setString(sf::String::fromUtf8(str.begin(), str.end()));
    source_text = str;
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
