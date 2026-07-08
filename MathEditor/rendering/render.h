#pragma once
#include "../config.h"

//Usiamo il renderer standard di SDL3
#include <SDL3/SDL.h>

namespace render {
	class Render {

		SDL_Renderer* renderer;

	public:
		Render(SDL_Window* window);
	};
}