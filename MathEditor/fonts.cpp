
#include <cmath>
#include <cstring>
// 2. Trigger the implementation right here inside the source
#define STB_TRUETYPE_IMPLEMENTATION
//A quanto pare va lasciato nel cpp e qua sopra, non so NON TOCCARE
//La cosa importante è che venga eseguito SOLO UNA VOLTA, gli header invece vengono copiati
#include "fonts.h"
using namespace fonts;

#define FONT_PATH "asset/fonts/"

Font fonts::loadStandard()
{

	const int atlasW = 1024;
	const int atlasH = 1024;

	//Apre un file ttf (in binary)

	std::vector<uint8_t> ttfBuffer(0);
	const char* path = FONT_PATH "NotoSans-Regular.ttf";
	int rcode = ut::readBFile(path, ttfBuffer);
	std::cout << "Codice di lettura file:" << rcode << std::endl;
	std::vector<uint8_t> atlasBitmap(atlasW * atlasH, 0);

	//Setup per la bitmap
	// 
	// Allocate separate tracking arrays for each range
	//Usa vector per metterli nell'heap, occupano decine di kb nello stack è un po' too much
	std::vector<stbtt_packedchar> ascii_chars(95);
	std::vector<stbtt_packedchar> greek_chars(144);
	std::vector<stbtt_packedchar> math_ops(256);

	// Set up the packing ranges
	stbtt_pack_range ranges[3] = {};

	// 1. Basic ASCII
	ranges[0].font_size = 32.0f;
	ranges[0].first_unicode_codepoint_in_range = 0x0020;
	ranges[0].num_chars = 95;
	ranges[0].chardata_for_range = ascii_chars.data();

	// 2. Greek Letters
	ranges[1].font_size = 32.0f;
	ranges[1].first_unicode_codepoint_in_range = 0x0370;
	ranges[1].num_chars = 144;
	ranges[1].chardata_for_range = greek_chars.data();

	// 3. Math Operators (\int, \sum, \partial, etc)
	ranges[2].font_size = 32.0f;
	ranges[2].first_unicode_codepoint_in_range = 0x2200;
	ranges[2].num_chars = 256;
	ranges[2].chardata_for_range = math_ops.data();

	// Initialize the packer onto your bitmap workspace
	stbtt_pack_context pc;
	stbtt_PackBegin(&pc, atlasBitmap.data(), atlasW, atlasH, 0, 1, nullptr);
	// Pack everything beautifully! The library tightly packs them like Tetris blocks
	int packResult = stbtt_PackFontRanges(&pc, ttfBuffer.data(), 0, ranges, 3);
	stbtt_PackEnd(&pc);

	if (packResult == 0) {
		std::cout << "Errore, packResult = 0 che significa qualcosa boh" << std::endl;
	}

	//Bisogna assegnare una mappa geometrica ai codepoint (uint unicode) che dicano dove guardare nella texture
	std::unordered_map<uint32_t, stbtt_packedchar> glyphs;

	for (int i = 0; i < 95; ++i) {
		uint32_t codepoint = 0x0020 + i;
		glyphs[codepoint] = ascii_chars[i];
	}
	// Travaso 2: Greco
	for (int i = 0; i < 144; ++i) {
		uint32_t codepoint = 0x0370 + i;
		glyphs[codepoint] = greek_chars[i];
	}
	// Travaso 3: Matematica
	for (int i = 0; i < 256; ++i) {
		uint32_t codepoint = 0x2200 + i;
		glyphs[codepoint] = math_ops[i];
	}
	return { std::move(atlasBitmap), atlasW, atlasH, 0, std::move(glyphs)};
}

Font fonts::loadFontBitmap(FontType font)
{
	switch (font)
	{
	case fonts::FontType::STANDARD:
		return loadStandard();
		break;
	case fonts::FontType::PLAINTEXT:
		break;
	default:
		break;
	}
	return { std::vector<uint8_t>(), NULL,NULL,NULL };
}
