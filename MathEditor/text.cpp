#include "text.h"

SDLGlyphQuad getSDLGlyphQuad(const fonts::Font& font, uint32_t codepoint, float currentX, float currentY)
{
	SDLGlyphQuad q{};
    auto it = font.glyphs.find(codepoint);
    if (it == font.glyphs.end()) {
        q.nextX = currentX + 10.0f; // Avanzamento di fallback per carattere mancante
        return q;
    }
    const stbtt_packedchar& b = it->second;
    // 1. Sorgente dall'atlas (coordinate intere in pixel)
    q.srcRect.x = static_cast<float>(b.x0);
    q.srcRect.y = static_cast<float>(b.y0);
    q.srcRect.w = static_cast<float>(b.x1 - b.x0);
    q.srcRect.h = static_cast<float>(b.y1 - b.y0);

    // 2. Destinazione sullo schermo (tenendo conto della baseline)
    q.drawRect.x = currentX + b.xoff;
    q.drawRect.y = currentY + b.yoff;
    q.drawRect.w = q.srcRect.w;
    q.drawRect.h = q.srcRect.h;

    // 3. Avanzamento cursore
    q.nextX = currentX + b.xadvance;

    return q;
}

void drawTextSDL(SDL_Renderer* renderer, SDL_Texture* fontTexture, const fonts::Font& font, const std::vector<uint32_t>& text, float startX, float startY)
{
    float cursorX = startX;
    float cursorY = startY;

    for (uint32_t codepoint : text) {
        SDLGlyphQuad glyph = getSDLGlyphQuad(font, codepoint, cursorX, cursorY);

        // Se il carattere ha dimensioni (non è uno spazio vuoto), lo mandi alla pipeline di SDL
        if (glyph.srcRect.w > 0 && glyph.srcRect.h > 0) {
            // SDL prende la texture, ritaglia srcRect, e la spara in dstRect gestendo internamente la pipeline grafica
            SDL_RenderTexture(renderer, fontTexture, &glyph.srcRect, &glyph.drawRect);
        }

        // Avanzi il cursore orizzontale
        cursorX = glyph.nextX;
    }
}