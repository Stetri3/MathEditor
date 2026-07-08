#pragma once
#include "config.h"
#include <cstddef>
//File di definizioni per tipi usati
//Include variabili globali e constexpr generiche
//Da includere come prima cosa in ogni altro header widget

//!Ricordarsi di tenere un alignment congruo, evitare padding il più possibile
class DebugHelper;
void testing();
#ifndef _SAFE
#define _SAFE false
#endif // !SAFE



namespace Mem { //Spazio per gestione memoria stack

	constexpr uint8_t ALIGNMENT = 32;
	//Size = Numero di byte di uno slot della grid
	//In futuro si può cambiare in modo che la grid sia solo un sottomultiplo della dim delle classi, per risparmiare spazio
	constexpr size_t SIZE = []() {
		constexpr size_t RAW_SIZE = 32; //Taglia raw (da cambiare con un constexpr max())
		return (((RAW_SIZE + ALIGNMENT - 1) / ALIGNMENT) * ALIGNMENT); //Allineamento al multiplo maggiore o uguale
		}();

	//Deprecated
	constexpr uint32_t MAX_BLOCKS = (1024 * 512) / SIZE; //Impostiamo un max per lasciare breathing room, per ora 512kb
	constexpr uint32_t CAP_BLOCKS = 200; //Impostiamo un cap del numero di widget (modificabile a compile)

	constexpr size_t EFF_BLOCKS = std::min(MAX_BLOCKS, CAP_BLOCKS); //Allocazione effettiva

	

}
#ifndef MAX_BLOCKS
#define MAX_BLOCKS 200u
#define IS_MAX_BLOCKS_SET false
#else
#define IS_MAX_BLOCKS_SET true
#endif
static_assert((+(MAX_BLOCKS) >= 0), "Errore, macro MAX_BLOCKS non convertibile in uint");

namespace widget { //Gestione oggetti widget

	//Tipi di widget
	//TODO: spostare WType in un altro file, aggiungere look up tables per la customizzazione di widget "standard" compile time
	//(Così sarà anche possibile aggiungere widget "salvati" personalizzati tra più progetti)

	namespace LAYOUT {
		using Option = uint8_t;
		constexpr Option NONE = 0;
		constexpr Option HORIZONTAL = 1 << 0; //Altrimenti vertical
		constexpr Option ALIGN_MASK = 3 << 1;
		constexpr Option ALIGN_START = 0 << 1; //Significa top in vertical
		constexpr Option ALIGN_CENTER = 1 << 1;
		constexpr Option ALIGN_END = 2 << 1; //Bottom in vertical
		constexpr Option ALIGN_SPACED = 3 << 1; //Spazio diviso equamente
		constexpr Option CROSS_MASK = 3 << 3; //Mask per allineamento in direzione perpendicolare
		constexpr Option CROSS_START = 0 << 3;
		constexpr Option CROSS_CENTER = 1 << 3;
		constexpr Option CROSS_END = 2 << 3;
		constexpr Option CROSS_STRETCH = 3 << 3;
		//Fine solo container
		//Per leaf (widget non container)
		// ???
		//Fine solo leaf
		constexpr Option STRETCH_MASK = 3 << 5; //Definisce dove posizionarsi se il parent ha CROSS_STRETCH e allarga più della size della leaf
		constexpr Option STRETCH_START = 0 << 5; //Se non ha cross dimension massima, o NO_STRETCH, sta nel cross start
		constexpr Option STRETCH_CENTER = 1 << 5; //Come sopra ... center
		constexpr Option STRETCH_END = 2 << 5; //Come sopra ... end
		constexpr Option STRETCH_FILL = 3 << 5; //Per ora vale come start, non voglio implementare dimensione leaf variabile
		constexpr Option NO_STRETCH = 1 << 7; //Se non stretcha il parent quando quello ha cross stretch
		
	}
	namespace LOGIC {
		//Flag per la logica, utili per canvas
		using Flag = uint16_t;
		constexpr Flag NONE = 0;
		constexpr Flag DEAD = 1 << 0; //Se morto, ovvero spazio free nell'array
		constexpr Flag DRAWABLE = 1 << 1; //Se è da passare al renderer per il drawing
		constexpr Flag CLICKABLE = 1 << 2;
		constexpr Flag RESIZE_CHILD = 1 << 3; //Se container
		constexpr Flag RESIZE_IMAGE = 1 << 3; //Se non container, comunque non lo userei ma giusto in caso
		constexpr Flag CUSTOM_BEHAVIOR = 1 << 4; //Se ci sono event e callback personalizzati. In realtà non è indipendente dalla successiva, ma utile tenerselo
		constexpr Flag ONCLICK = 1 << 5; //Se esiste onClick per questo widget. OnClick è speciale, per cui utile usarlo dedicato
		constexpr Flag IMMOVABLE = 1 << 6; //Se il widget è in uno stato in cui non può essere spostato (es. linking async)
		//Nota: in generale, anche se è una possibilità, è bad practice usarlo per passare raw pointer async, meglio usare sempre canvas come interfaccia
		constexpr Flag RESIZE_MASK = 3 << 7;
		constexpr Flag RESIZE_NONE = 0 << 7;
		constexpr Flag RESIZE_FREE = 1 << 7;
		constexpr Flag RESIZE_PROP = 2 << 7;
		constexpr Flag RESIZE_CUSTOM = 3 << 7;
		constexpr Flag RESIZE_GEOMETRY = 1 << 9; //Se chiama resize quando è deformato nel layouting
		constexpr Flag CHILD_DEFAULT_WEIGHTS = 1 << 10; //ATTENZIONE: SOLO SUGGERIMENTO (per geometry),
		//usarlo se i child variano in weight non rovina il programma ma peggiora le performance
	}

	namespace TYPE {
		//Flag immutabili che dipendono dal tipo, utili a catalog
		using Flag = uint16_t;
		constexpr Flag NONE = 0;
		constexpr Flag CONTAINER = 1 << 0;
		constexpr Flag DRAWABLE = 1 << 1; //Se è disegnabile in generale (altrimenti hidden) (deprecated, usare flag dinamica)
		constexpr Flag CLICKABLE = 1 << 2; //Ogni widget con clickable static deve avere un metodo onclick inizializzato tramite canvas, o si usa il default
		constexpr Flag VIRTUAL = 1 << 3; //Ogni widget con virtual deve avere un getVisible(Id virtualID) default
		constexpr Flag TEXTABLE = 1 << 4; //A i textable di default è assegnata una classe per i metodi di import/export testo
		constexpr Flag DRAW_POST = 1 << 5; //Se l'immagine è modificabile post costruzione, utile per ottimizzazione.
		constexpr Flag EDITABLE = 1 << 6; //Se editable
	}
	namespace STATIC {
		//Flag da assegnare a costruzione, queste non dipendono dal tipo (generalmente) ma vanno comunque assegnate a compile time
		//In realtà non tutte sono assegnabili a ogni tipo, ma va bene piuttosto che scrivere 200 righe per dichiararlo, comunque il compiler in caso le ignora non succede niente
		
		using Flag = uint8_t;
		constexpr Flag DRAWABLE = 1 << 0; //Se il widget è da passare al rendering (consigliato qui al posto di in TYPE)
		constexpr Flag RAW_REND = 1 << 1; //Se il widget può essere impostato con rendering async
		constexpr Flag CUSTOM_LOGIC = 1 << 2; //Se la logica (event, coordinazione,...) può essere modificata dalla default.
		//Attenzione: widget dinamici come button e textField includono già richieste di callback nel default behavior
		//CUSTOM_LOGIC è da usare solo se si vuole personalizzare la codifica e logica degli event specifici (onHover, bitmap relation per il testo, ecc.)
		constexpr Flag ROUNDED = 1 << 7; //Se gli angoli sono smussati (ancora in fase di sviluppo)
		enum class LinkType: uint8_t {
			CHILD = 0, //Se si connette a un container parent
			FREE = 1, //Se è spostabile fuori dalla struttura ad albero
			FREEUNIQUE = 2, //Come free ma non può sovrapporsi ad altri free/freeunique, utile per ottimizzare logica
			//Di default i free/freeunique hanno z massima (-100), per due free sovrapposti la z è maggiore nell'ultimo spostato
		};
	}
	//Si noti come molte flag si sovrappongano in TYPE, LOGIC e STATIC.
	//Questo è per permettere al manager di ottimizzare la gestione in più punti diversi del programma.


	struct Padding {
		uint8_t topBottom = 0;
		uint8_t leftRight = 0;
	};
	struct Margin {
		uint8_t top = 0;
		uint8_t bottom = 0;
		uint8_t left = 0;
		uint8_t right = 0;
	};
	
	struct RectSize{//Semplice struct per size
		uint16_t w = 0;
		uint16_t h = 0;

		//Per l'accesso tramite bool is_horizontal
		using Ptr = uint16_t RectSize::*;
		static inline Ptr main[2] = { &RectSize::h, &RectSize::w };
		static inline Ptr cross[2] = { &RectSize::w, &RectSize::h };
	};

	struct CoreLayout { //Pure layout 
		Margin margin; //4 byte
		Padding padding; //2 byte
		uint8_t layoutOptions = 0;
		uint8_t w = 0; //1 byte. Peso, ex. chi vince in arroganza per il layout. A pareggio si divide a metà, ma NON CERCARE di pareggiare
	};

	struct AnimStatic {
		void* ptr;
		size_t size;
	};

}
namespace ID { //Gestione indicizzazione memoria

	using Id = uint16_t;
	constexpr Id NONE = 0xFFFF; //Id "non c'è quest'elemento"
	constexpr Id ROOT = 0; //Id del background/nodo iniziale
	struct Indexing {
		Id parent = NONE; //Se il tipo è free (quindi segue che è root) Id parent significa il z index
		Id nextBro = NONE;
		Id prevBro = NONE;
		Id firstChild = NONE;
		Id lastChild = NONE;
	};

	using RenderId = uint16_t;
}

namespace widget {
	using Handle = uint32_t;

	constexpr Handle BG_HANDLE = 0;

	struct WidgetCore {//La vera struct (da inizializzare sempre staticamente) del widget
		Handle handle = 0; //Handle unico del widget (codifica info sul widget stesso)
		ID::Indexing indexing; //Struct per l'indexing, va sempre tenuta vicino all'inizio
		LOGIC::Flag logicFlags = 1; //Flag della logica, variano a runtime, specificano lo stato logico del widget

		RectSize size; //4 byte

		CoreLayout layoutParams;
		//Renderla inistanziabile dall'esterno (per istanziarla correttamente solo da canvas)
		uint8_t pad[4] = {0, 0, 0, 0};
		void resize(RectSize maxSize);
		void staticResize(RectSize maxSize); //resize implicito, no logica associata
	private:
		WidgetCore() = default;
		friend class Canvas;
		//per il debug
		friend class DebugHelper;

	};
	
	struct VirtualCore {
		//è un reinterpret di WidgetCore, da usare dato che molti dati sono "inutili"
		Handle handle = 0;
		ID::Indexing indexing;
		LOGIC::Flag logicFlags = 1;
		RectSize sizeExt; //Fino a qui uguale a WidgetCore

		RectSize sizeInt;
		int16_t posX = 0; //La posizione virtuale del clipping in questo momento (x,y)
		LAYOUT::Option layoutFlags = 0; //Tenuto allo stesso offset che in WidgetCore
		int16_t posY = 0;
	private:
		VirtualCore() = default;
		friend class Canvas;
	};

	struct BackgroundCore {
		const Handle handle = BG_HANDLE;
		uint8_t pad[6];
		ID::Id firstChild = ID::NONE;
		ID::Id lastChild = ID::NONE;
		uint8_t pad2[2];
		RectSize size;
		uint8_t pad3[4];
		Padding padding;
		LAYOUT::Option layoutOptions = 0;
		uint8_t pad4[5];
	};

	//Check compile time per il reinterpret
	namespace {
		static_assert(offsetof(widget::WidgetCore, handle) == offsetof(widget::VirtualCore, handle), "Mismatch offset: handle");
		static_assert(offsetof(widget::WidgetCore, indexing) == offsetof(widget::VirtualCore, indexing), "Mismatch offset: indexing");
		static_assert(offsetof(widget::WidgetCore, logicFlags) == offsetof(widget::VirtualCore, logicFlags), "Mismatch offset: logicFlags");
		static_assert(offsetof(widget::WidgetCore, size) == offsetof(widget::VirtualCore, sizeExt), "Mismatch offset: size / sizeExt");
		static_assert(offsetof(widget::WidgetCore, layoutParams.layoutOptions) == offsetof(widget::VirtualCore, layoutFlags),
			"CRITICAL: layoutOptions e layoutFlags non sono allineati in memoria!");
		static_assert(sizeof(widget::WidgetCore) == sizeof(widget::VirtualCore), "Le due struct hanno dimensioni diverse!");
		static_assert(sizeof(widget::WidgetCore) <= 32, "Il widget ha superato i 32 byte strategici!");

	}
	namespace geometry {
		struct GeoRequest {
			uint16_t stretch_x;
			uint16_t stretch_y;
			int16_t mov_x;
			int16_t mov_y;
		};
	}

	struct GeoRel {
		int16_t mPos;
		uint16_t mDim;
		int16_t cPos;
		uint16_t cDim;
	};
	struct GeoCore {
		int16_t x;
		int16_t y;
		uint16_t w;
		uint16_t h;


		//0 memoria, per evitare branching esterno
		using PosPtr = int16_t GeoCore::*;
		using DimPtr = uint16_t GeoCore::*;

		static inline PosPtr mPos[2] = { &GeoCore::y, &GeoCore::x };
		static inline PosPtr cPos[2] = { &GeoCore::x, &GeoCore::y };
		static inline DimPtr mDim[2] = { &GeoCore::h, &GeoCore::w };
		static inline DimPtr cDim[2] = { &GeoCore::w, &GeoCore::h };

		constexpr auto getRelative(bool is_hor);
	};

	struct ColorCore {
		union {
			void* texturePtr; //Cambiare tipo in qualche texture type
			struct {
				uint32_t RGBA;
				uint32_t method; //addizionali, da flag
			}
		};
		uint16_t colorFlags;
	};

	struct RenderPacket {
		Handle handle;
		GeoCore geo; //Size/pos del layout
		uint8_t pad[2];
		LOGIC::Flag logicFlags;
		RectSize size; //Size del drawing
		uint8_t pad2[4];
		Padding padding;
	};
}
