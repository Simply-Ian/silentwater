#ifndef SW_BOOKMARK_WIDGET
#define SW_BOOKMARK_WIDGET
#include <TGUI/SubwidgetContainer.hpp>
#include <TGUI/Widgets/Label.hpp>
#include <TGUI/Widgets/Button.hpp>
#include <TGUI/BackendRenderTarget.hpp>
#include "bookmark.h"
#include <TGUI/String.hpp>
#include <TGUI/Signal.hpp>

class BookmarkWidget: public tgui::SubwidgetContainer{
    public:
        using Ptr = std::shared_ptr<BookmarkWidget>;
        using ConstPtr = std::shared_ptr<const BookmarkWidget>;

        BookmarkWidget(const char* typeName = "BookmarkWidget", bool initRenderer = true) : 
                tgui::SubwidgetContainer(typeName, initRenderer){};

        static BookmarkWidget::Ptr create(sw::Bookmark s, int text_size);
        int getPageNum();
        static BookmarkWidget::Ptr copy(BookmarkWidget::ConstPtr widget);
        void setSize(const tgui::Layout2d& size) override;
        void draw(tgui::BackendRenderTargetBase& target, tgui::RenderStates states) const override;
        tgui::SignalVector2f onClick = {"Clicked"}; 
        tgui::SignalVector2f onMousePress = {"MousePressed"};
        tgui::SignalVector2f onMouseRelease = {"MouseReleased"};
        //! Псевдоним для сигнала о нажатии на кнопку BookmarkWidget::deleteButton
        tgui::Signal onClose = {"CloseButtonPressed"};
        bool isMouseOnWidget(tgui::Vector2f pos) const override;
        void leftMousePressed(tgui::Vector2f pos) override;
        void leftMouseReleased(tgui::Vector2f pos) override;
    
    private:
        void setSource(sw::Bookmark s);
        sw::Bookmark source;
        tgui::Label::Ptr chapterTitleLabel;
        tgui::Label::Ptr previewLabel;
        tgui::Label::Ptr pageNumLabel;
        tgui::Button::Ptr deleteButton;
        const tgui::Color m_default_bg_color = tgui::Color(232, 232, 232);
        const tgui::Color m_pressed_color = tgui::Color(0, 110, 200);
        tgui::Color m_bg_color = m_default_bg_color;
    
    protected:
        Widget::Ptr clone() const override {
            return std::make_shared<BookmarkWidget>(*this);
        }
        tgui::Signal& getSignal(tgui::String signalName) override;
};
#endif