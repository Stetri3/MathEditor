#pragma once

#include <vector>
#include <cstdint>
#include "utils.h"
#include "widget_def.h"
#include "widget_catalog.h"

//Vedere widget_def per definizioni


namespace widget {
	
	
	template <uint32_t blocknum> class Canvas;

	

	using BlockAction = void(ID::Id, WidgetCore&);

	//Struct per la creazione di widget da canvas
	struct WidgetCoreInfo {
		WType type;
		STATIC::Flag flags;
		LOGIC::Flag logicFlags;
		RectSize size;
		Margin margin;
		Padding padding;
		uint8_t weight;
		LAYOUT::Option layoutOptions;
	};
	struct VirtualCoreInfo {
		WType type;
		STATIC::Flag flags;
		LOGIC::Flag logicFlags;
		RectSize extSize;
		RectSize intSize;
		LAYOUT::Option layoutOptions;
		int16_t xInit;
		int16_t yInit;
	};

	//Geometry class. Questa NON può effettuare richieste, è l'engine che fa da bridge tra spazio visivo e spazio logico
	//Non dovrebbe avere ownership se non di variabili interne , e in passati solo in const ref
	class Geometry {

	};

	//Manager class
	template <uint32_t blocknum = Mem::EFF_BLOCKS>
	class Canvas {
	private:
		//Riservare lo spazio per widgeting
		std::vector<ID::Id> flat_exe_list; //Ordine di esecuzione
		std::vector<ID::Id> stack; //Lista per operazioni temp
		std::vector<WidgetCore> cores; //Array dei core (parti principali) dei widget
		std::vector<ID::Id> exe_list_buffer; //Array dei pending changes per exe list

		std::vector<ID::Id> chronological; //Array in ordine di count, per trovare index da handle

		//Se i widget sono ordinati top down
		bool dataOrdered = false;
		//Se i widget NON hanno buchi
		bool dataCompact = false;

		ID::Id next_free = ID::NONE; //Ultimo widget liberato

		//Numero di variabili create dall'inizio del programma
		uint32_t count = 0;

	public:

		Canvas();
		void updateExeList();

		template <typename Func, bool topdown = true>
		void execute_branch(ID::Id node_id, Func& action);

		uint16_t warmChain(uint16_t num, bool forceLoad = false);

		ID::Id fromHandle(Handle handle);

		bool destroyWidget(ID::Id index);

		static constexpr Handle makeHandle(WType wType, STATIC::Flag stFlags, uint32_t count);
		
		WidgetCore& newWidget(WidgetCoreInfo widgetInfo);

		ID::Indexing placeWidget(const WidgetCore& core, ID::Id parent);

		std::pair<Handle, ID::Indexing> newWidget(VirtualCoreInfo widgetInfo);


		WidgetCore& getBlock(ID::Id id);
	};

	//Da lasciare alla fine, def per la creazione dei widget da parte dell'user
	//constexpr WidgetInfo makeWidget(); (ancora da fare)
}
#include "widgets.inl"