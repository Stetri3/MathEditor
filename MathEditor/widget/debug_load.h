#pragma once
#include <cstdint>
#include <vector>
#include "../widget_def.h"
#include "../widgets.h"

class DebugHelper {
public:
    static constexpr std::vector<widget::WidgetCore> generateDebugWidgets() {
        std::vector<widget::WidgetCore> test_vector;

        // Supponiamo una macro o lambda locale per simulare makehandle(type/metadata, index)
        // 14 bit metadati (es. 0x0A20) spostati a sinistra di 18 bit + index/ID
        auto mock_handle = [](uint32_t meta, uint16_t id) -> widget::Handle {
            return (meta << 18) | id;
            };

        // Slot 0: Background / Root Widget
        // Handle = BG_HANDLE (0) obbligatorio da widget_def.h
        {
            widget::WidgetCore w;
            w.handle = widget::BG_HANDLE;
            w.indexing.parent = ID::NONE;
            w.indexing.nextBro = ID::NONE;
            w.indexing.prevBro = ID::NONE;
            w.indexing.firstChild = 1; // Punta all'indice 1 del vettore
            w.indexing.lastChild = 2;  // Punta all'indice 2 del vettore
            w.logicFlags = widget::LOGIC::DRAWABLE;
            w.size.w = 1920;
            w.size.h = 1080;
            w.layoutParams.layoutOptions = widget::LAYOUT::HORIZONTAL | widget::LAYOUT::ALIGN_CENTER;
            w.layoutParams.w = 1;
            test_vector.push_back(w);
        }

        // Slot 1: Primo figlio (es. Pannello laterale, indice = 1)
        {
            widget::WidgetCore w;
            w.handle = mock_handle(0x01FF, 1); // Metadati + ID reale 1
            w.indexing.parent = 0;              // Parent è root (indice 0)
            w.indexing.nextBro = 2;             // Il fratello è all'indice 2
            w.indexing.prevBro = ID::NONE;
            w.indexing.firstChild = ID::NONE;
            w.indexing.lastChild = ID::NONE;
            w.logicFlags = widget::LOGIC::DRAWABLE | widget::LOGIC::CLICKABLE;
            w.size.w = 400;
            w.size.h = 1060;
            w.layoutParams.layoutOptions = widget::LAYOUT::CROSS_STRETCH;
            test_vector.push_back(w);
        }

        // Slot 2: Secondo figlio (es. Area di testo/contenuto, indice = 2)
        {
            widget::WidgetCore w;
            w.handle = mock_handle(0x02AA, 2); // Metadati + ID reale 2
            w.indexing.parent = 0;              // Parent è root (indice 0)
            w.indexing.nextBro = ID::NONE;
            w.indexing.prevBro = 1;             // Fratello precedente all'indice 1
            w.indexing.firstChild = ID::NONE;
            w.indexing.lastChild = ID::NONE;
            w.logicFlags = widget::LOGIC::DRAWABLE | widget::LOGIC::CUSTOM_BEHAVIOR;
            w.size.w = 1480;
            w.size.h = 1060;
            w.layoutParams.w = 2;
            test_vector.push_back(w);
        }

        // Slot 3: Widget VALIDO ma logicamente DEAD (indice = 3)
        // Fa ancora parte della gerarchia (o è isolato), ma il motore lo salta se legge il flag
        {
            widget::WidgetCore w;
            w.handle = mock_handle(0x00FF, 3);
            w.indexing.parent = 0;
            w.indexing.nextBro = ID::NONE;
            w.indexing.prevBro = 2;
            w.indexing.firstChild = ID::NONE;
            w.indexing.lastChild = ID::NONE;
            w.logicFlags = widget::LOGIC::DEAD;
            w.size.w = 100;
            w.size.h = 100;
            test_vector.push_back(w);
        }

        // Slot 4: Spazio vuoto / Slot nella Free List (Puro Garbage Data, indice = 4)
        // Può avere QUALSIASI flag sporco rimasto in canna (incluso LOGIC::DEAD o roba a caso),
        // l'unica cosa che conta è che l'indexing è rotto o usato internamente per la free list.
        {
            widget::WidgetCore w;
            w.handle = 0xFFFFFFFF; // Handle totalmente invalido / spazzatura
            w.indexing.parent = ID::NONE;
            w.indexing.nextBro = ID::NONE; // Se la free list usa nextBro, qui ci sarebbe l'indice del prossimo slot libero
            w.indexing.prevBro = ID::NONE;
            w.indexing.firstChild = ID::NONE;
            w.indexing.lastChild = ID::NONE;
            w.logicFlags = widget::LOGIC::DEAD | widget::LOGIC::CLICKABLE; // Spazzatura rimasta sporca in memoria
            w.size.w = 999;  // Dati sporchi vecchio widget
            w.size.h = 999;
            test_vector.push_back(w);
        }

        return test_vector;
    }
    static void doCanvas();

    static const std::vector<widget::WidgetCore>& getCanvasCores(widget::Canvas& canvas);

    static std::vector<widget::WidgetCoreInfo> generateDebugWidgetInfo();
};