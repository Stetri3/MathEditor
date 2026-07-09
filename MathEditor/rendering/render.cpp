#include "render.h"

namespace render {
	Render::Render(SDL_Window* window)
	{
		uint32_t MACRO_PACKNUM = 100; //placeholder, da determinare a constexpr dal widgeting
		this->background_colors.reserve(max_colors);
		this->packets.reserve(MACRO_PACKNUM);

		this->renderer = SDL_CreateRenderer(window, "detailRend");
	}
	void Render::Texturize(std::vector<RenderPacket>& packets)
	{}
}