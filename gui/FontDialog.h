#ifndef SW_FONTDIALOG
#define SW_FONTDIALOG
#include <TGUI/Widgets/ChildWindow.hpp>
#include <TGUI/Widgets/Button.hpp>
#include <TGUI/Widgets/Slider.hpp>
#include "MyScrollablePanel.h"
#include <TGUI/Widgets/Label.hpp>
#include <TGUI/Widgets/EditBox.hpp>
#include <TGUI/Font.hpp>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct FontDialogResult{
    string fontPath;
    int fontSize;
    int lineInt;
    FontDialogResult(string fp, int fs, int li) : fontPath(fp), fontSize(fs), lineInt(li){}
    FontDialogResult(){}
};

class FontDialog : public tgui::ChildWindow{
    public:
        using Ptr = std::shared_ptr<FontDialog>;
        using ConstPtr = std::shared_ptr<const FontDialog>;

        FontDialog(const char* typeName = "FontDialog", bool initRenderer = true) :
            tgui::ChildWindow(typeName, initRenderer){}
        static FontDialog::Ptr create(string curPath, int curFontSize, int curLineInt, string preview);
        static FontDialog::Ptr copy(FontDialog::ConstPtr widget);

        /// @brief Дает доступ к результату пользовательского выбора
        /// @return Экземпляр структуры FontDialogResult
        FontDialogResult& getResult();

    protected:
        Widget::Ptr clone() const override {
            return std::make_shared<FontDialog>(*this);
        }
    
    private:
        vector<string> fontPaths;
        FontDialogResult result;
        const int GUI_TEXT_SIZE = 20;

        MyScrollablePanel::Ptr fontBox;
        tgui::Label::Ptr fontSizeTitle;
        tgui::Slider::Ptr fontSizeSlider;
        tgui::Label::Ptr fontSize;
        tgui::Label::Ptr lineIntTitle;
        tgui::EditBox::Ptr lineIntBox;
        tgui::Label::Ptr previewLabel;
        tgui::Button::Ptr cancelButton;
        tgui::Button::Ptr okButton;
};
#endif