#pragma once
#include "../config.h"
#include <vector>
#include "render_def.h"

//Usiamo il renderer standard di SDL3
#include <SDL3/SDL.h>

namespace render {
	class Render {

		SDL_Renderer* renderer;
		std::vector<ColorCore> background_colors; //Pool per colorCore, chiamato "background" per enfatizzare che si occupa solo delle statiche
		std::vector<SDL_FRect> render_rects; //Parallelo a background_colors
	public:
		Render(SDL_Window* window);
	};
}