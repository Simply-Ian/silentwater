#include "FontDialog.h"

FontDialog::Ptr FontDialog::copy(FontDialog::ConstPtr widget){
    if (widget)
        return std::static_pointer_cast<FontDialog>(widget->clone());
    else
        return nullptr;
}

FontDialog::Ptr FontDialog::create(string curPath, int curFontSize, int curLineInt, string preview){
    FontDialog::Ptr ptr = std::make_shared<FontDialog>();
    ptr->setTitleButtons(tgui::ChildWindow::TitleButton::Close);
    ptr->setSize({"parent.width / 3", "parent.height / 3"});
    ptr->setResizable(false);
    ptr->fontBox = MyScrollablePanel::create({"parent.width / 3", "parent.height * 14 / 15"});
    ptr->fontBox->setPosition(0, 0);
    ptr->fontBox->setWidgetName("fontBox");
    ptr->add(ptr->fontBox);

    ptr->fontSizeTitle = tgui::Label::create("Размер шрифта: ");
    ptr->fontSizeTitle->setTextSize(ptr->GUI_TEXT_SIZE);
    ptr->fontSizeTitle->setAutoSize(true);
    ptr->fontSizeTitle->setPosition("fontBox.width + 10", 10);
    ptr->fontSizeTitle->setWidgetName("fsT");
    ptr->add(ptr->fontSizeTitle);

    ptr->fontSizeSlider = tgui::Slider::create(5.f, 72.f);
    ptr->fontSizeSlider->setSize("234 * (parent.width / 640)", 14);
    ptr->fontSizeSlider->setPosition("fsT.right + 10", 17);
    ptr->fontSizeSlider->setWidgetName("fsS");
    ptr->fontSizeSlider->setValue(static_cast<float>(curFontSize));
    ptr->fontSizeSlider->onValueChange([](tgui::Slider::Ptr it, tgui::Label::Ptr fsL){
                fsL->setText(tgui::String::fromNumber(it->getValue()));
            }, ptr->fontSizeSlider, ptr->fontSize);
    ptr->add(ptr->fontSizeSlider);

    ptr->fontSize = tgui::Label::create(tgui::String::fromNumber(curFontSize));
    ptr->fontSize->setTextSize(ptr->GUI_TEXT_SIZE);
    ptr->fontSize->setPosition("fsS.right + 10", 14);
    ptr->fontSize->setAutoSize(true);
    ptr->add(ptr->fontSize);

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
    ptr->add(ptr->lineIntBox);

    ptr->previewLabel = tgui::Label::create(preview);
    ptr->previewLabel->setPosition("fontBox.width", "liT.bottom + 15");
    ptr->previewLabel->setTextSize(curFontSize);
    /// \todo Межстрочный интервал
    ptr->add(ptr->previewLabel);

    return ptr;
}