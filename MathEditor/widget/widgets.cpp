#include "config.h"
#include "widgets.h"
#include "utils.h"
#include <cstring>
#include "utils/type_utils.hpp"
#ifdef DEBUG
#include <iostream>
#endif // DEBUG


namespace widget {
	Canvas::Canvas() 
	{
		instance = this;
		std::cout << "Reserving RAM: (" << (blocknum * (sizeof(WidgetCore)  + sizeof(ID::Id)*5)) / 1024 << ") KB... ";
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
		DebugMessage("[Update] ROOT FirstChild: " << cores[0].indexing.firstChild)
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

		DebugMessage("[EndUpdate] ROOT FirstChild:" << cores[0].indexing.firstChild)
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
		DebugMessage("[Place] ROOT FirstChild: " << cores[0].indexing.firstChild)
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
	int Geometry::update(const std::vector<WidgetCore>& cores, const std::vector<ID::Id> exe_list)
	{
#ifdef DEBUG
		DebugMessage("[GeoUpdate]:" << cores[0].indexing.firstChild)
#endif // DEBUG

		if constexpr (_SAFE) {
			if (cores.size() != exe_list.size()) return -1;
		}
		//Inizializza l'arr a tutti 0
		std::memset(visualArr.data(), 0, visualArr.size() * sizeof(GeoCore));
		std::memset(idealSizes.data(), 0, idealSizes.size() * sizeof(RectSize));

		orderedHandles.clear();
		//Passaggio bottom up: qui solo resizing "ideal"
		//E E E anche salvataggi utili per il top down
		//Da migliorare quel bruttissimo ++
		ID::Id lastParent = cores[exe_list.back()].indexing.parent;
		for (uint32_t j = exe_list.size() ; j > 0; --j)
		{
#ifdef DEBUG
			std::cout << "Passaggio bottom up, iterazione" << (exe_list.size() - j) << "\n";
#endif // DEBUG

			//Indice traslato (blabla)
			const std::uint16_t i = static_cast<uint16_t>(j - 1);

			const ID::Id curr = exe_list[i];
			const WidgetCore& core = cores[curr];
			const Handle currHandle = core.handle;
			
			//Ordina la lista secondo lo stesso ordine
			orderedHandles.push_back(currHandle);
			if (i == 0) break;
			const ID::Id parent = core.indexing.parent;

			//Se parent è none, questo era il ramo più alto (root), fine resizing qui
			if (parent == ID::NONE) {
				continue;
			}
			
			++childCount[parent];


			const WidgetCore& parentCore = cores[parent];

			const bool is_horizontal = parentCore.layoutParams.layoutOptions & LAYOUT::HORIZONTAL;
			const bool stretch_fill = core.layoutParams.layoutOptions & LAYOUT::STRETCH_FILL;
			const bool no_cross_stretch = (parent == 0) || !((parentCore.layoutParams.layoutOptions & LAYOUT::CROSS_MASK) & LAYOUT::CROSS_STRETCH);

			uint16_t margin_w = core.layoutParams.margin.left + core.layoutParams.margin.right;
			uint16_t margin_h = core.layoutParams.margin.top + core.layoutParams.margin.bottom;

			//Se il widget è un container il valore è già aggiunto (si vede sotto)
			if (getTypeFlagsFromHandle(core.handle) & TYPE::CONTAINER) {

				//Check che non sia container vuoto (però devi essere davvero stupido per mettercelo cioè)
				if (core.indexing.firstChild != ID::NONE) [[likely]] {

					//Aggiungere padding (perché rimosso da iterazioni prima)
					idealSizes[curr].h += core.layoutParams.padding.topBottom * 2;
					idealSizes[curr].w += core.layoutParams.padding.leftRight * 2;
				}
				else {
					idealSizes[curr].h = core.size.h;
					idealSizes[curr].w = core.size.w;
				}
			}
			else {
				idealSizes[curr].h = core.size.h;
				idealSizes[curr].w = core.size.w;

			}//endif !is container

			uint16_t total_w = idealSizes[curr].w + margin_w;
			uint16_t total_h = idealSizes[curr].h + margin_h;

			uint16_t padding_w = parentCore.layoutParams.padding.leftRight;
			uint16_t padding_h = parentCore.layoutParams.padding.topBottom;
			uint16_t space_w; 
			uint16_t space_h;

			if (is_horizontal) {
				space_h = parentCore.size.h - 2 * padding_h;
				//Se NON c'è cross stretch (ovvero la cross size del parent è fissa)
				//Notare la contraption parent == 0 (root) dato che di default il root non si stretcha
				if (no_cross_stretch) {
					//In caso fill il widget si riadatta al parent, altrimenti è solo limitato ma cerca di restare uguale
					idealSizes[curr].h = (stretch_fill ? space_h : std::min(space_h, total_h)) - margin_h;
					
				}
				else {
					//Se c'è cross stretch, si massimizza la dimensione cross del container:
					//Non riaggiungiamo padding: intrinseco, andrà aggiunto alla sua iterazione
					idealSizes[parent].h = std::max(space_h, total_h);
					
				}
				idealSizes[parent].w += total_w;
			}
			else {
				space_w = parentCore.size.w - 2 * padding_w;
				if (no_cross_stretch) {
					idealSizes[curr].w = (stretch_fill ? space_w : std::min(space_w, total_w)) - margin_w;
					
				}
				else {
					idealSizes[parent].w = std::max(space_w, total_w);
					
				}
				idealSizes[parent].h += total_h;
			} //endif !horizontal
			DebugMessage("[Fine bottom up]: " << cores[0].indexing.firstChild)
		}//end for loop


		//passaggio top down: posizionamento, stretching

		//Dimensione BG = dimensione finestra
		visualArr[0].h = cores[0].size.h;
		visualArr[0].w = cores[0].size.w;
		visualArr[0].x = 0;
		visualArr[0].y = 0;
		for (uint32_t  i = 0; i < exe_list.size(); ++i) {
			DebugMessage("Ciclo top down, iterazione " << i)

			ID::Id curr = exe_list[i];
			const WidgetCore& core = cores[curr];

			//Iteriamo solo sui container
			if (!(getTypeFlagsFromHandle(core.handle) & TYPE::CONTAINER)) continue;


			//Variabili iter relative
			bool is_horizontal = core.layoutParams.layoutOptions & LAYOUT::HORIZONTAL;

			auto& mDim_curr = visualArr[curr].*(GeoCore::mDim[is_horizontal]);
			auto& mPos_curr = visualArr[curr].*(GeoCore::mPos[is_horizontal]);
			auto& cDim_curr = visualArr[curr].*(GeoCore::cDim[is_horizontal]);
			auto& cPos_curr = visualArr[curr].*(GeoCore::cPos[is_horizontal]);

			const uint16_t mDim_ideal = is_horizontal ? idealSizes[curr].w : idealSizes[curr].h;
			const uint16_t cDim_ideal = is_horizontal ? idealSizes[curr].h : idealSizes[curr].w;

			const uint16_t pad_m = is_horizontal ? core.layoutParams.padding.leftRight : core.layoutParams.padding.topBottom;
			const uint16_t pad_c = is_horizontal ? core.layoutParams.padding.topBottom : core.layoutParams.padding.leftRight;

			// 1. Calcola lo spazio totale dei pesi e lo spazio minimo occupato dai figli nell'asse main
			uint16_t free_m = mDim_curr - 2*pad_m;
			uint16_t free_c = cDim_curr - 2* pad_c;

			uint16_t used_m = mDim_ideal - 2 * pad_m;
			uint16_t used_c = cDim_ideal - 2 * pad_c;

			//Dovrebbero compilare in 1 sola istruzione (carry/sub flag)
			bool align_overflow = free_m < used_m;
			uint16_t extra_m = align_overflow ? used_m - free_m : free_m - used_m;
			bool cross_overflow = free_c < used_c;
			uint16_t extra_c = cross_overflow ? used_c - free_c: free_c - used_c;

			uint16_t mCursor = 0;

			ID::Id child = core.indexing.firstChild; 
			uint8_t j = 0; //Se crei 256 child attivi dello stesso container hai un problema mentale
			if (IS_DEBUG || core.logicFlags & LOGIC::CHILD_DEFAULT_WEIGHTS) {
				//Se i pesi sono default, il layout è sviluppato da start a end tagliando chi non sta dentro
				//Nel futuro possibilmente si può fare da end a start se l'alignment è impostato in quel modo
				while (child != ID::NONE) {
					DebugMessage("Inizio ciclo while")
					const WidgetCore& childCore = cores[child];
					uint16_t& mDim_child = visualArr[child].*(GeoCore::mDim[is_horizontal]);
					int16_t& mPos_child = visualArr[child].*(GeoCore::mPos[is_horizontal]);
					uint16_t& cDim_child = visualArr[child].*(GeoCore::cDim[is_horizontal]);
					int16_t& cPos_child = visualArr[child].*(GeoCore::cPos[is_horizontal]);

					const uint16_t mIdeal_child = idealSizes[child].*(RectSize::main[is_horizontal]);
					const uint16_t cIdeal_child = idealSizes[child].*(RectSize::cross[is_horizontal]);
					/* METODO PIù SICURO MA PIù LENTO
					const uint8_t mMStart_child = is_horizontal ? childCore.layoutParams.margin.left : childCore.layoutParams.margin.top;
					const uint8_t mMEnd_child = is_horizontal ? childCore.layoutParams.margin.right : childCore.layoutParams.margin.bottom;
					const uint8_t cMStart_child = is_horizontal ? childCore.layoutParams.margin.top : childCore.layoutParams.margin.left;
					const uint8_t cMEnd_child = is_horizontal ? childCore.layoutParams.margin.bottom : childCore.layoutParams.margin.right;
					*/
					const uint8_t* margin_arr = ut::type::structToA(childCore.layoutParams.margin);
					
					// Se h=1 (Horiz) -> left(2).  Se h=0 (Vert) -> top(0)
					const uint8_t mMStart_child = margin_arr[is_horizontal << 1];

					// Se h=1 (Horiz) -> right(3). Se h=0 (Vert) -> bottom(1)
					const uint8_t mMEnd_child = margin_arr[(is_horizontal << 1) | 1];

					// Se h=1 (Horiz) -> top(0).   Se h=0 (Vert) -> left(2)
					const uint8_t cMStart_child = margin_arr[(is_horizontal ^ 1) << 1];

					// Se h=1 (Horiz) -> bottom(1). Se h=0 (Vert) -> right(3)
					const uint8_t cMEnd_child = margin_arr[((is_horizontal ^ 1) << 1) | 1];
					//Per ora usiamo solo align start, in futuro sarà implementato l'alignment custom
					//l'indexing è ordinato da start a end di default
					// 
					//Logica di update posizioni e dimensioni dei child tramite spazio free e pesi

					mCursor += mMStart_child;

					if (align_overflow) {
						mDim_child = (mCursor >= free_m) ? 0u : std::min(mIdeal_child, static_cast<uint16_t>(free_m - mCursor));
					} else {
						mDim_child = mIdeal_child;
					}
					mPos_child = mPos_curr + pad_m + mCursor;
					mCursor += mDim_child + mMEnd_child;

					if (cross_overflow) {
						const bool margin_of = free_c < cMStart_child + cMEnd_child;
						const uint16_t avail = margin_of ? 0 : free_c - cMStart_child - cMEnd_child;
						const bool has_stretchfill = ut::bit::checkMasked(childCore.layoutParams.layoutOptions, LAYOUT::STRETCH_MASK, LAYOUT::STRETCH_FILL);
						cDim_child = has_stretchfill ? avail : std::min(avail, cIdeal_child);
					}
					else {
						cDim_child = cIdeal_child;
					}

					cPos_child = cPos_curr + pad_c + cMStart_child;

					DebugMessage("Valori: " << cDim_child << cPos_child)
					//Purtroppo non c'è modo di evitare lo sparse senza introdurre un overhead maggiore
					child = childCore.indexing.nextBro;
					++j;
				}
			}
			else {
				return 1; //1 -> non supportata per ora
			}
		}

		return 0;

	}

	void WidgetCore::resize(RectSize maxSize) {
		Canvas::instance->doResize(*this, maxSize);
	}

	void WidgetCore::staticResize(RectSize maxSize)
	{
		switch (M2U(logicFlags, LOGIC::RESIZE_MASK))
		{
		case 0:
			return;
		case 1:
			size = maxSize;
			break;
		case 2: 
			if (size.w > 0 && size.h > 0) [[likely]] {
				float scaleW = (float)maxSize.w / size.w;
				float scaleH = (float)maxSize.h / size.h;
				float scale = (scaleW < scaleH) ? scaleW : scaleH;
				size.w = (int)(size.w * scale + 0.5f);
				size.h = (int)(size.h * scale + 0.5f);
			} else {
				size = maxSize;
			}
			break;
		case 3:
			resize(maxSize);
			break;
		default:
			std::cout << "Se leggi questo c'è un errore (widgets.cpp, resizeStatic())\n";
			break;
		}
	}

	void Canvas::doResize(WidgetCore& core, RectSize maxSize) {
		//todo
	}

	
}
