#include "render.h"

namespace render {
	Render::Render(SDL_Window* window)
	{
		this->renderer = SDL_CreateRenderer(window, "detailRend");
	}
}