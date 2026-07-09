#pragma once
#include <SDL3/SDL.h>
namespace render {

#ifndef MAX_COLORS
#define MAX_COLORS 200
#endif // !MAX_COLORS
	constexpr uint32_t max_colors = MAX_COLORS;


	namespace FLAGS {
		using Flag = uint16_t;
	}

	
	union ColorCore {
			SDL_Texture* texturePtr; //Se texture custom
			struct {
				uint32_t RGBA;
				uint32_t method; //addizionali, da flag
			};
			struct {
				uint16_t textureId; //Per texture default, presenti in una pool
				uint16_t textureOptions;
				uint32_t dataId; //Alcune texture (es. text, image) supportano il retrieval di data da un'altra pool
			};
		};

	struct RenderPacket {
		uint32_t handle;
		int16_t x;
		int16_t y;
		uint16_t w;
		uint16_t h;
		union {
			uint16_t colorId; //Caso static/semistatic, per animazioni a frame loop o immagini statiche/low pattern
			uint16_t asyincId; //Caso dynamic, l'id va a una struttura user-defined che parla direttamente con il renderer
		};
		uint16_t memFlags; //Es. se fare batching, se custom writing
	};
}