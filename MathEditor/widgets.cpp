#include "widgets.h"

namespace widget {
	Canvas::Canvas()
	{
		std::cout << "Reserving RAM (" << (blocknum * sizeof(WidgetCore) * 5) / 1024 << ") KB... ";
		this->cores.resize(blocknum, WidgetCore{});
		this->flat_exe_list.reserve(blocknum);
		this->stack.reserve(blocknum);
		this->exe_list_buffer.reserve(blocknum);
		this->chronological.reserve(blocknum);
		this->chronological.resize(blocknum);
		std::cout << "Done.\n";
		std::cout << "Indexing init...";
		for (uint32_t i = 0; i < blocknum - 1; ++i)
		{
			cores[i].indexing.nextBro = i + 1;
		}
		cores[blocknum - 1].indexing.nextBro = ID::NONE;
		next_free = 0;
		std::cout << "Done.\n";
	}

	
	void Canvas::updateExeList() {
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
	uint16_t Canvas::warmChain(uint16_t num, bool forceLoad)
	{
		//Check se la chain di widget ha almeno num spazi liberi, altrimenti return di (num - spazi liberi) (ovvero overflow extra)
		//Chiamata warm perché utile se fatta subito prima di operazioni sugli elementi, dati restano in cache
		//ForceLoad dice se caricare in cache anche quando non necessario (forzare il warming)
		if (this->next_free == ID::NONE) return num; //Zero widget liberi

		if (dataOrdered && dataCompact) {
			uint16_t freesize = static_cast<uint16_t>(num - (this->cores.size() - next_free - 1));
			uint16_t loadedSize = num > freesize ? freesize : num;
			if (!forceLoad) {
				return num - loadedSize;
			}
			for (uint16_t i = 0; i < loadedSize; i += 2)
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
	ID::Id Canvas::fromHandle(Handle handle)
	{
		return this->chronological[handle & 0x3FFFF];
	}
	bool Canvas::destroyWidget(ID::Id index) {
		ID::Indexing& indexing = cores[index].indexing;
		indexing.nextBro = this->next_free; //Imposta la catena a mo' di pop/push
		next_free = index; //Salva il last destroyed
		//Esegui distruzione, ecc.
		return true;
	}

	

	WidgetCore Canvas::newWidget(WidgetCoreInfo widgetInfo)
	{
		//if (widgetInfo.type == WType::VLayout) return &WidgetCore();

		WidgetCore wCore;
		wCore.layoutParams.layoutOptions = widgetInfo.layoutOptions;
		wCore.layoutParams.margin = widgetInfo.margin;
		wCore.layoutParams.padding = widgetInfo.padding;
		wCore.layoutParams.w = widgetInfo.weight;
		wCore.logicFlags = widgetInfo.logicFlags;
		wCore.size = widgetInfo.size;
		Handle handle = this->makeHandle(widgetInfo.type, widgetInfo.flags, this->count);
		wCore.handle = handle;
		++count;
		return wCore;
	}

	ID::Indexing Canvas::placeWidget(const WidgetCore& core, ID::Id parent)
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
		if (parent != ID::NONE) {
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
		}
		WidgetCore& retCore = cores[widgetID];
		retCore = core;

		//Todo: aggiungere modifica all'exe list, aggiungere widget in cores
		retCore.logicFlags |= LOGIC::DEAD; //Imposta il widget dead (come se non esistesse, ma aggiunto)
		//All'update finale del frame sarà rimossa la flag, così l'aggiunta reale avviene solo al frame successivo
		//E non disturba la logica effettiva con stati volatili
		retCore.indexing = indexing;
		this->exe_list_buffer.push_back(widgetID);
		this->chronological[retCore.handle & 0x3FFFF] = widgetID;
		//Ricordarsi prima dello sleep, per ultima cosa di fare l'update (updateExeList)
		return indexing;
	}

	VirtualCore Canvas::newWidget(VirtualCoreInfo widgetInfo)
	{
		if (!(widget::getTypeFlags(widgetInfo.type) & TYPE::VIRTUAL)) return VirtualCore();
		VirtualCore core;
		core.logicFlags = widgetInfo.logicFlags;
		core.layoutFlags = widgetInfo.layoutOptions;
		core.sizeExt = widgetInfo.extSize;
		core.sizeInt = widgetInfo.intSize;
		core.posX = widgetInfo.xInit;
		core.posY = widgetInfo.yInit;
		Handle handle = this->makeHandle(widgetInfo.type, widgetInfo.flags, this->count);
		core.handle = handle;
		++count;
		return core;
	}

	

	WidgetCore& Canvas::getBlock(ID::Id id)
	{
		return cores[id];
	}
	void Geometry::update(const std::vector<WidgetCore>& cores, const std::vector<ID::Id> exe_list)
	{
		visualArr.clear();
		orderedHandles.clear();
		//Passaggio bottom up
		ut::static_vector<WidgetCore, 64u> broBuffer; //64 fratelli max, 64*32 = 2 KB
		ut::static_vector<Handle, 64u> handleBuffer;
		GeoArr<64u> evaluatedBuffer;
		ID::Id lastParent = 65534; //2nd to last id, riservato
		ID::Id currParent;
		for (uint32_t i = exe_list.size(); i < 65536u; --i)
		{
			const WidgetCore& core = cores[exe_list[i]];
			broBuffer.push_back(core);
			handleBuffer.push_back(core.handle);
			currParent = cores[exe_list[i]].indexing.parent;
			if (currParent != lastParent) {
				if (lastParent == 0) {
					//Allora erano di primo livello
				}
				else {
					//cambiato parent, quindi dato che exe_list è ordinata top down l'ultimo era il parent degli altri
					GeoArr<64u> parsedElements = parseContainer(broBuffer, evaluatedBuffer);
				}
			}
			else {
				//Siamo ancora fratelli, continua con l'iter
				lastParent = currParent;
			}

		}
		//passaggio top down
	}
}
