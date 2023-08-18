#include "FontDialog.h"
#include <filesystem>
#include <iostream>
#include "../decode_funs/exec.h"
#include <sstream>

FontDialog::Ptr FontDialog::copy(FontDialog::ConstPtr widget){
    if (widget)
        return std::static_pointer_cast<FontDialog>(widget->clone());
    else
        return nullptr;
}

FontDialog::Ptr FontDialog::create(string curPath, int curFontSize, int curLineInt, string preview){
    FontDialog::Ptr ptr = std::make_shared<FontDialog>();
    ptr->setTitleButtons(tgui::ChildWindow::TitleButton::Close);
    ptr->setSize({"parent.width / 2.5", "parent.height / 3"});
    ptr->setResizable(false);
    ptr->getRenderer()->setBackgroundColor(tgui::Color(170, 170, 170));
    ptr->getRenderer()->setShowTextOnTitleButtons(true);
    ptr->getRenderer()->setTitleBarColor(tgui::Color(48, 48, 48));
    ptr->setTitle("Выберите шрифт и настройте его");
    ptr->getRenderer()->setTitleBarHeight(30);
    ptr->setPositionLocked(true);

    ptr->fontBox = MyScrollablePanel::create({"parent.width / 2.7", "parent.height - 30"});
    ptr->fontBox->setPosition(0, 0);
    ptr->fontBox->setWidgetName("fontBox");
    ptr->fontBox->getRenderer()->setBackgroundColor(tgui::Color::White);
    ptr->fontBox->setHorizontalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
    ptr->add(ptr->fontBox);

    ptr->fontSizeTitle = tgui::Label::create("Размер шрифта: ");
    ptr->fontSizeTitle->setTextSize(ptr->GUI_TEXT_SIZE);
    ptr->fontSizeTitle->setAutoSize(true);
    ptr->fontSizeTitle->setPosition("fontBox.width + 10", 10);
    ptr->fontSizeTitle->setWidgetName("fsT");
    ptr->add(ptr->fontSizeTitle);

    ptr->fontSizeSlider = tgui::Slider::create(5.f, 72.f);
    ptr->fontSizeSlider->setSize("parent.width - fsT.width - fontBox.width - 70", 14);
    ptr->fontSizeSlider->setPosition("fsT.right + 10", 17);
    ptr->fontSizeSlider->setWidgetName("fsS");
    ptr->fontSizeSlider->setValue(static_cast<float>(curFontSize));
    ptr->add(ptr->fontSizeSlider);

    ptr->fontSize = tgui::Label::create(tgui::String::fromNumber(curFontSize));
    ptr->fontSize->setTextSize(ptr->GUI_TEXT_SIZE);
    ptr->fontSize->setPosition("fsS.right + 10", 14);
    ptr->fontSize->setAutoSize(true);
    ptr->add(ptr->fontSize);

    ptr->fontSizeSlider->onValueChange([](tgui::Slider::Ptr it, tgui::Label::Ptr fsL){
                int val = it->getValue();
                fsL->setText(tgui::String::fromNumber(val));
            }, ptr->fontSizeSlider, ptr->fontSize);

    ptr->lineIntTitle = tgui::Label::create("Межстрочный интервал: ");
    ptr->lineIntTitle->setAutoSize(true);
    ptr->lineIntTitle->setPosition("fontBox.width + 10", "fst.bottom + 10");
    ptr->lineIntTitle->setTextSize(ptr->GUI_TEXT_SIZE);
    ptr->lineIntTitle->setWidgetName("liT");
    ptr->add(ptr->lineIntTitle);

    ptr->lineIntBox = tgui::EditBox::create();
    ptr->lineIntBox->setPosition("liT.right + 10", "liT.top");
    ptr->lineIntBox->setSize("36", ptr->GUI_TEXT_SIZE);
    ptr->lineIntBox->setInputValidator(tgui::EditBox::Validator::UInt);
    ptr->lineIntBox->setText(tgui::String::fromNumber(curLineInt));
    ptr->add(ptr->lineIntBox);

    ptr->previewLabel = tgui::Label::create(preview);
    ptr->previewLabel->setPosition("fontBox.width + 10", "liT.bottom + 15");
    ptr->previewLabel->setTextSize(curFontSize);
    ptr->previewLabel->getRenderer()->setBackgroundColor(tgui::Color::White);
    ptr->previewLabel->setSize("parent.width - fontBox.width - 20", "parent.height - liT.bottom - 55 - 30");
    /// \todo Межстрочный интервал
    ptr->add(ptr->previewLabel);

    ptr->fontSizeSlider->onValueChange([](tgui::Slider::Ptr it, tgui::Label::Ptr preview){
        preview->setTextSize(it->getValue());
    }, ptr->fontSizeSlider, ptr->previewLabel);

    ptr->okButton = tgui::Button::create("OK");
    ptr->okButton->setSize({80, 24});
    ptr->okButton->setPosition({"parent.width - width - 3", "parent.height - 57"});
    ptr->okButton->onClick(&FontDialog::retrieveResultFromWidgets, ptr);
    ptr->add(ptr->okButton);

    ptr->cancelButton = tgui::Button::create("Отмена");
    ptr->cancelButton->setSize({80, 24});
    ptr->cancelButton->setPosition({"parent.width - width - 86", "parent.height - 57"});
    ptr->cancelButton->onClick([](FontDialog::Ptr it){it->close();}, ptr);
    ptr->add(ptr->cancelButton);

    ptr->setUpFontBox(curPath);

    return ptr;
}

void FontDialog::retrieveResultFromWidgets(){
    result.fontSize = static_cast<int>(fontSizeSlider->getValue());
    result.lineInt = lineIntBox->getText().toInt();
    result.fontPath = getFontPairBySelectedItem(cur_selected_item).first;
    setEnabled(false);
    setVisible(false);
}

void FontDialog::setUpFontBox(string curPath){
    fontLoader.load_fonts();
    int counter = 0;
    for (font_pair fp: fontLoader.fonts){
        filesystem::path fontname(fp.second.getId().c_str());
        fontname.replace_extension("");
        tgui::String promt = fontname.filename().c_str();
        tgui::Label::Ptr item = tgui::Label::create(promt);
        item->setTextSize(18);
        item->getRenderer()->setFont(fp.second);
        item->getRenderer()->setBackgroundColor(tgui::Color::White);
        item->setWidgetName(to_string(counter));
        item->onClick(&FontDialog::selectItem, this, item);
        if (curPath == fp.first) selectItem(item);
        fontBox->add(item);
        counter ++;
    }
}

void FontLoader::load_fonts(){
    auto in_str = [](string base, string piece) -> bool {
        return base.find(piece) != string::npos;
    };
    auto in_vect = [](vector<string> base, string piece) -> bool {
        return find(base.begin(), base.end(), piece) != base.end();
    };
    vector<string> valid_exts{".otf", ".ttf", ".t1"};
    /// \todo Локаль?
    stringstream font_paths { exec("fc-list :style=Regular:lang=ru") + exec("fc-list :style=Book:lang=ru") };
    string raw_info;
    filesystem::path path;
    while (getline(font_paths, raw_info, '\n')){
        path = raw_info.substr(0, raw_info.find(":"));
        if (in_vect(valid_exts, path.extension().c_str())){
            // Обработка ошибок нужна, т.к. некоторые шрифты (напр., MS Webdings) не поддерживают Юникод.
            // tgui::Font при попытке загрузить такой шрифт выбрасывает ошибку.
            try{
                fonts.push_back({path, tgui::Font(path.c_str())});
            }
            catch (tgui::Exception exc){
                cout << exc.what() << endl;
            }
        }
    }
}

void FontDialog::selectItem(tgui::Label::Ptr item){
    item->getRenderer()->setBackgroundColor(tgui::Color::Blue);
    item->getRenderer()->setTextColor(tgui::Color::White);
    if (cur_selected_item){
        cur_selected_item->getRenderer()->setBackgroundColor(tgui::Color::White);
        cur_selected_item->getRenderer()->setTextColor(tgui::Color::Black);
    }
    cur_selected_item = item;

    previewLabel->getRenderer()->setFont(getFontPairBySelectedItem(item).second);
}

font_pair FontDialog::getFontPairBySelectedItem(tgui::Label::Ptr item){
    int font_id = item->getWidgetName().toInt();
    return fontLoader.fonts.at(font_id);
}