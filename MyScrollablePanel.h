#ifndef SW_MY_SCROLLABLE_PANEL
#define SW_MY_SCROLLABLE_PANEL
#include <TGUI/Widgets/ScrollablePanel.hpp>

class MyScrollablePanel: public tgui::ScrollablePanel{
    const int spacer_height = 2;

    public:
        typedef std::shared_ptr<MyScrollablePanel> Ptr;
        typedef std::shared_ptr<const MyScrollablePanel> ConstPtr;

        MyScrollablePanel(const char* typeName = "MyScrollablePanel", bool initRenderer = true) :
            tgui::ScrollablePanel(typeName, initRenderer){}
        static MyScrollablePanel::Ptr create(tgui::Layout2d size, tgui::Vector2f contentSize={0, 0});
        static MyScrollablePanel::Ptr copy(MyScrollablePanel::ConstPtr widget){
            if (widget) return std::static_pointer_cast<MyScrollablePanel>(widget->clone());
            else return nullptr;
        }
        //! Рассчитывает смещение по Y для нового виджета закладки
        int getNewSlot(int bm_height);
        bool remove(const tgui::Widget::Ptr &widget) override;
        void add(const tgui::Widget::Ptr &widgetPtr, const tgui::String &widgetName = "") override;

    protected:
        Widget::Ptr clone() const override {
            return std::make_shared<MyScrollablePanel>(*this);
        }
};

#endif