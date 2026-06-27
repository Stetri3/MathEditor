#pragma once

#include "widget_def.h"
#include <array>
#include "utils.h"

//File di catalogazione tipi di widget

namespace widget {
	enum class WType : std::uint8_t {
		Widget = 0, //Has to default to inconstructible
		Layout = 1, //Widget container, usato per la manipolazione geometrica dei figli
		DrawLayout = 2, //Layout disegnabile post construction
		ClickDrawLayout = 3, //DrawLayout + opzione di click. Per ora, ClickLayout senza draw non esiste (quasi totalmente inutile)
		Box = 4, //Widget vuoto disegnabile a costruzione
		DrawBox = 5, //Widget per il disegno post. Può essere linkato direttamente al rendering quando soddisfa alcune condizioni speciali variabili
		TextBox = 6, //ATTENZIONE: TEXTBOX è "COMPLICATO", deve comunicare con text per spacing e parsing/inverse parsing del testo.
		//Inoltre, per essere in grado di rappresentare e selezionare il testo, dev'essere drawable con un handle specifico
		//Notare che Textbox include drawbox (anche se enorme overhead se non serve poter scrivere testo), drawbox NON include textbox
		Button = 7, //Box con supporto mouse
		DrawButton = 8, //DrawBox con supporto mouse.
		TextButton = 9, //TextBox con supporto mouse
		StateButton = 10, //Button con più stati (checkbox, option chooser, ecc.). Facoltativo, per l'ottimizzazione, animazione da definire a costruzione
		VLayout = 11, //IMPORTANTISSIMO è il container che ha uno spazio virtuale diverso da quello visibile (piccolo, non superare troppe migliaia di pixel)
		//Va sempre inserito come unico figlio all'interno di un container.
		VirtualView = 12, //Virtual layout che NON permette widget all'interno, permette logica raw (disegnare cose dinamiche, lunghe infinitamente, ecc)
		TextField = 13, //Textbox con metodi per l'editing inline. Ovviamente include anche metodi mouse

		Count
	};

	struct StaticProp {
		TYPE::Flag typeFlags = 0;
	};
	constexpr std::array<StaticProp, static_cast<size_t>(WType::Count)> TYPES{ {
		{0},//Widget
		{TYPE::CONTAINER}, //Layout
		{TYPE::CONTAINER | TYPE::DRAWABLE | TYPE::DRAW_POST}, //DrawLayout
		{TYPE::CONTAINER | TYPE::DRAWABLE | TYPE::DRAW_POST | TYPE::CLICKABLE},
		{TYPE::NONE}, //Box
		{TYPE::DRAWABLE | TYPE::DRAW_POST}, //Drawbox
		{TYPE::DRAWABLE | TYPE::DRAW_POST | TYPE::TEXTABLE}, //Textbox
		{TYPE::CLICKABLE}, //Button
		{TYPE::CLICKABLE | TYPE::DRAWABLE | TYPE::DRAW_POST},//DrawButton
		{TYPE::CLICKABLE | TYPE::DRAWABLE | TYPE::DRAW_POST | TYPE::TEXTABLE}, //TextButton
		{TYPE::CLICKABLE | TYPE::DRAWABLE | TYPE::DRAW_POST | TYPE::TEXTABLE}, //StateButton
		{TYPE::CONTAINER | TYPE::VIRTUAL}, //VLayout
		{TYPE::CONTAINER | TYPE::VIRTUAL}, //VirtualView
		{TYPE::CLICKABLE | TYPE::DRAWABLE | TYPE::DRAW_POST | TYPE::TEXTABLE | TYPE::EDITABLE}, //TextField
	}};
	
		constexpr TYPE::Flag getTypeFlags(WType type) {
			return widget::TYPES[static_cast<uint8_t>(type)].typeFlags;
		}

		constexpr widget::WType getTypeFromHandle(Handle handle) {
			constexpr uint32_t type_mask = 63u << 26;
			return static_cast<widget::WType>(handle & type_mask);
		}
		
		constexpr widget::STATIC::Flag getStaticFromHandle(Handle handle) {
			constexpr uint32_t static_mask = 255u << 18;
			return static_cast<widget::STATIC::Flag>(handle & static_mask);
		}
		constexpr TYPE::Flag getTypeFlagsFromHandle(Handle handle) {
			return getTypeFlags(getTypeFromHandle(handle));
		}
}