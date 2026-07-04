#include <type_traits>
#include <iostream>
#include "widgets.h"
#include <bit>


namespace widget {
	template<typename Func, bool topdown>
	inline void Canvas::execute_branch(ID::Id node_id, Func& action) {
		//Esegue action su tutta la branch sotto a node (non incluso), in top-down (prima i più alti), o bottom-up(il contrario) se <false>
		//Safety measure a compile time per evitare di passare i blocchi a value
		static_assert(std::is_invocable_v<Func, ID::Id>,
			"Errore: La lambda deve avere firma void(ID::Id)");
		if constexpr (topdown) {
			for (size_t i = 0; i < flat_exe_list.size(); ++i) {
				ID::Id id = flat_exe_list[i];
				action(id);
			}
		}
		else {
			for (int i = flat_exe_list.size() - 1; i >= 0; --i) {
				ID::Id id = flat_exe_list[i];
				action(id);
			}
		}
	}

	template <uint8_t oldCase>
	inline void Canvas::makeBackground()
	{
		//OldCase: 0 per overwrite totale, 1 per "modifica esistente", 2 per "modifica se esiste"

		BackgroundCore bgCore;
		//TODO: aggiungere parametri a makeBackground() per inizializzare bgCore
		if constexpr (oldCase == 1) {
			BackgroundCore oldCore = std::bit_cast<BackgroundCore, WidgetCore>(cores[0]);
			bgCore.firstChild = oldCore.firstChild;
			bgCore.lastChild = oldCore.lastChild;
		}
		else if constexpr (oldCase == 2) {
			BackgroundCore oldCore = std::bit_cast<BackgroundCore, WidgetCore>(cores[0]);
			if (oldCore.firstChild != ID::NONE) {
				bgCore.firstChild = oldCore.firstChild;
				bgCore.lastChild = oldCore.lastChild;
			}
		}
		if (this->next_free == 0) {
			next_free = cores[0].indexing.nextBro;
		}
		cores[0] = std::bit_cast<WidgetCore, BackgroundCore>(bgCore);

	}
	inline constexpr Handle Canvas::makeHandle(WType wType, STATIC::Flag stFlags, uint32_t count)
	{
		//I primi 6 bit sono wType, gli 8 successivi le flag, e i 18 finali uniqueId dal count
		const uint32_t cutCount = count & 0x3FFFF; //Taglia fino al 18
		return (static_cast<Handle>(wType) << 26) |
			(static_cast<Handle>(stFlags) << 18) |
			cutCount;
	}
}
