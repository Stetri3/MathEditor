#pragma once

#include "utils.h"

#include "third_party/stb_truetype.h"
#include <unordered_map>
#include <iostream>

namespace fonts {
	enum class FontType {
		STANDARD = 0,
		PLAINTEXT = 1
	};
	struct Font {
		std::vector<uint8_t> bitmap;
		int bWidth; //Larghezza bitmap
		int bHeight; //Altezza bitmap
		int flags;
		std::unordered_map<uint32_t, stbtt_packedchar> glyphs;
	};
	Font loadStandard();


	Font loadFontBitmap(FontType font);

}