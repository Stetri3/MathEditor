#include <type_traits>
#include <iostream>
#include "widgets.h"


namespace widget {
	template <std::uint32_t blocknum>
	widget::Canvas<blocknum>::Canvas()
	{
		std::cout << "Reserving RAM (" << (blocknum * sizeof(WidgetCore) * 5) / 1024 << ") KB... ";
		this->cores.resize(blocknum, WidgetCore{});
		this->flat_exe_list.reserve(blocknum);
		this->stack.reserve(blocknum);
		this->exe_list_buffer.reserve(blocknum);
		this->chronological.reserve(blocknum);
		std::cout << "Done.\n";
		std::cout << "Indexing init...";
			for (uint32_t i = 0; i < blocknum-1; ++i)
			{
				cores[i].indexing.nextBro = i + 1;
			}
			cores[blocknum - 1].indexing.nextBro = ID::NONE;
			next_free = 0;
			std::cout << "Done.\n";
	}

	template <std::uint32_t blocknum>
	template<typename Func, bool topdown>
	void Canvas<blocknum>::execute_branch(ID::Id node_id, Func& action) {
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
	template <std::uint32_t blocknum>
	inline void Canvas<blocknum>::updateExeList() {
		// 1. Rimuovi la flag DEAD dai widget che erano in attesa di inserimento
		for (ID::Id id : exe_list_buffer) {
			cores[id].logicFlags &= ~LOGIC::DEAD;
		}
		exe_list_buffer.clear();

		// 2. Svuota la vecchia lista di esecuzione
		flat_exe_list.clear();
		stack.clear();

		// 3. Ricostruisci la lista in modo perfettamente contiguo (O(N))
		stack.push_back(ID::ROOT);

		while (!stack.empty()) {
			ID::Id curr = stack.back();
			stack.pop_back();


			flat_exe_list.push_back(curr);

			ID::Id child = cores[curr].indexing.lastChild;
			while (child != ID::NONE) {
				stack.push_back(child);
				child = cores[child].indexing.prevBro;
			}
		}
	}
	template<uint32_t blocknum>
	inline uint16_t Canvas<blocknum>::warmChain(uint16_t num, bool forceLoad)
	{
		//Check se la chain di widget ha almeno num spazi liberi, altrimenti return di (num - spazi liberi) (ovvero overflow extra)
		//Chiamata warm perché utile se fatta subito prima di operazioni sugli elementi, dati restano in cache
		//ForceLoad dice se caricare in cache anche quando non necessario (forzare il warming)
		if (this->next_free == ID::NONE) return num; //Zero widget liberi

		if (dataOrdered && dataCompact){
			uint16_t freesize = num - (this->cores.size() - next_free - 1);
			uint16_t loadedSize = num > freesize ? freesize : num;
			if (!forceLoad) {
				return num - loadedSize;
			}
			for (uint16_t i = 0; i < loadedSize; i+2)
			{
				//Sostituire con un dummy load di cores[i]
			}
			return num - loadedSize;
		}
		//Iter through sparse memory
		ID::Id nextBlock = this->next_free;
		uint16_t i = 0;
		for (i = 0; i < num; ++i) {
			if (nextBlock == ID::NONE) break;
			nextBlock = cores[nextBlock].indexing.nextBro;
		}
		return num - i;
	}
	template<uint32_t blocknum>
	inline ID::Id Canvas<blocknum>::fromHandle(Handle handle)
	{
		return this->chronological[handle & 0x3FFFF];
	}
	template <std::uint32_t blocknum>
	inline bool Canvas<blocknum>::destroyWidget(ID::Id index) {
		ID::Indexing& indexing = cores[index].indexing;
		indexing.nextBro = this->next_free; //Imposta la catena a mo' di pop/push
		next_free = index; //Salva il last destroyed
		//Esegui distruzione, ecc.
	}

	template<uint32_t blocknum>
	inline constexpr Handle Canvas<blocknum>::makeHandle(WType wType, STATIC::Flag stFlags, uint32_t count)
	{
		//I primi 6 bit sono wType, gli 8 successivi le flag, e i 18 finali uniqueId dal count
		const uint32_t cutCount = count & 0x3FFFF; //Taglia fino al 18
		return (static_cast<Handle>(wType) << 26) |
			(static_cast<Handle>(stFlags) << 18) |
			cutCount;
	}

	template<uint32_t blocknum>
	inline WidgetCore& Canvas<blocknum>::newWidget(WidgetCoreInfo widgetInfo)
	{
		if constexpr (widgetInfo.type == WType::VLayout) return WidgetCore();

		Handle handle = this->makeHandle(widgetInfo.type, widgetInfo.flags, this->count);
		WidgetCore wCore;
		wCore.handle = handle;
		wCore.layoutParams = widgetInfo.layoutOptions;
		wCore.logicFlags = widgetInfo.logicFlags;
		wCore.size = widgetInfo.size;
		++count;
		return wCore;
	}

	template<uint32_t blocknum>
	inline ID::Indexing Canvas<blocknum>::placeWidget(const WidgetCore& core, ID::Id parent)
	{
		if (this->warmChain(1))
		{
			std::cout << "Errore, non abbastanza spazio libero!\n";
			return ID::Indexing();
		}
		ID::Indexing indexing = core.indexing;
		indexing.parent = parent;
		indexing.nextBro = ID::NONE;
		if (getTypeFlagsFromHandle(core.handle) & TYPE::CONTAINER) {//Se non container potrebbero essere recastate meglio non toccare
			indexing.firstChild = ID::NONE;
			indexing.lastChild = ID::NONE;
		}
		ID::Id widgetID = next_free;
		next_free = cores[next_free].indexing.nextBro;

		WidgetCore& parentCore = cores[parent];
		indexing.prevBro = parentCore.indexing.lastChild;
		if (parentCore.indexing.firstChild == ID::NONE) {
			parentCore.indexing.firstChild = widgetID;
		}
		else {
			//Imposta l'ultimo lastChild a nextBro nuovo widget
			cores[parentCore.indexing.lastChild].indexing.nextBro = widgetID;
		}
		parentCore.indexing.lastChild = widgetID;

		WidgetCore& retCore = cores[widgetID];
		retCore = core;

		//Todo: aggiungere modifica all'exe list, aggiungere widget in cores
		retCore.logicFlags |= LOGIC::DEAD; //Imposta il widget dead (come se non esistesse, ma aggiunto)
		//All'update finale del frame sarà rimossa la flag, così l'aggiunta reale avviene solo al frame successivo
		//E non disturba la logica effettiva con stati volatili
		retCore.indexing = indexing;
		this->exe_list_buffer.push_back(widgetID);
		//Ricordarsi prima dello sleep, per ultima cosa di fare l'update (updateExeList)
		return indexing;
	}

	template<uint32_t blocknum>
	inline std::pair<Handle, ID::Indexing> Canvas<blocknum>::newWidget(VirtualCoreInfo widgetInfo)
	{
		return std::pair<Handle, ID::Indexing>();
	}

	template<uint32_t blocknum>
	inline WidgetCore& Canvas<blocknum>::getBlock(ID::Id id)
	{
		return cores[id];
	}
}
