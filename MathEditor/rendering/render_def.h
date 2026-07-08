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
}