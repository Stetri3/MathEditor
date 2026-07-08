#include "render.h"

namespace render {
	Render::Render(SDL_Window* window)
	{
		this->background_colors.reserve(max_colors);

		this->renderer = SDL_CreateRenderer(window, "detailRend");
	}
}