#pragma once

#include "third_party/stb_truetype.h"
#include <SDL3/SDL.h>
#include "fonts.h"
struct SDLGlyphQuad {
	SDL_FRect srcRect; //Rettangolo della bitmap da cui prendere i pixel
	SDL_FRect drawRect; //Rettangolo su cui disegnare a schermo
	float nextX; //Cursore avanzamento
};
SDLGlyphQuad getSDLGlyphQuad(const fonts::Font& font, uint32_t codepoint, float currentX, float currentY);

void drawTextSDL(SDL_Renderer* renderer, SDL_Texture* fontTexture, const fonts::Font& font, const std::vector<uint32_t>& text, float startX, float startY);