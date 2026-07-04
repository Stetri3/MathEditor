#include "debug_load.h"

void DebugHelper::doCanvas()
{

}

std::vector<widget::WidgetCoreInfo> DebugHelper::generateDebugWidgetInfo() {
    using namespace widget;
    std::vector<WidgetCoreInfo> test_vector;

    // 0: Background
    {
        WidgetCoreInfo w{};
        w.type = widget::WType::Background;
        w.flags = widget::STATIC::DRAWABLE;
        w.logicFlags = widget::LOGIC::DRAWABLE;
        w.size = { 1920, 1080 };
        w.margin = { 0, 0, 0, 0 };
        w.padding = { 10, 10 };
        w.weight = 1;
        w.layoutOptions = widget::LAYOUT::HORIZONTAL | widget::LAYOUT::ALIGN_CENTER;
        test_vector.push_back(w);
    }

    // 1: Container di Layout (es. barra laterale)
    {
        WidgetCoreInfo w{};
        w.type = widget::WType::Layout;
        w.flags = widget::STATIC::CUSTOM_LOGIC;
        w.logicFlags = widget::LOGIC::DRAWABLE | widget::LOGIC::CLICKABLE;
        w.size = { 400, 1060 };
        w.margin = { 5, 5, 5, 5 };
        w.padding = { 0, 0 };
        w.weight = 0;
        w.layoutOptions =  widget::LAYOUT::CROSS_STRETCH;
        test_vector.push_back(w);
    }

    // 2: TextField per input testo
    {
        WidgetCoreInfo w{};
        w.type = widget::WType::TextField;
        w.flags = widget::STATIC::DRAWABLE | widget::STATIC::ROUNDED;
        w.logicFlags = widget::LOGIC::DRAWABLE | widget::LOGIC::CLICKABLE | widget::LOGIC::ONCLICK;
        w.size = { 1480, 1060 };
        w.margin = { 5, 5, 5, 5 };
        w.padding = { 20, 20 };
        w.weight = 2;
        w.layoutOptions = widget::LAYOUT::NONE;
        test_vector.push_back(w);
    }

    // 3: Semplice Button congelato (LOGIC::DEAD)
    {
        WidgetCoreInfo w{};
        w.type = widget::WType::Button;
        w.flags = widget::STATIC::DRAWABLE;
        w.logicFlags = widget::LOGIC::DEAD;
        w.size = { 150, 50 };
        w.margin = { 2, 2, 2, 2 };
        w.padding = { 5, 5 };
        w.weight = 0;
        w.layoutOptions = widget::LAYOUT::NONE;
        test_vector.push_back(w);
    }

    return test_vector;
}