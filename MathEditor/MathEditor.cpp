// MathEditor.cpp: definisce il punto di ingresso dell'applicazione.
//

#include "MathEditor.h"
#include "utils.h"
#include <SDL3/SDL.h>

#include <vector>
#include "fonts.h"
#include <thread>
#include <chrono>
#include "text.h"
#include "widgets.h"

#include "widget/debug_load.h"

#define DEBUG_HEX(x) "0x" << std::hex << (x) << std::dec
#define DEBUG
using namespace std;
int main()
{
	cout << "Hello CMake." << endl;
	//Init delle variabili SDL
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		std::cerr << "SDL Init Failed: " << SDL_GetError() << "\n";
		return -1;
	}

	SDL_Window* window;
	SDL_Renderer* renderer;
	if (!SDL_CreateWindowAndRenderer("Test Font", 1280, 720, 0, &window, &renderer)) {
		SDL_Quit();
		cout << "Creazione window/renderer fallita\n";
		return -1;
	}
	SDL_SetWindowResizable(window, true);

	fonts::Font font = fonts::loadStandard();
	cout << "Larghezza bmap:" << font.bitmap.size() << endl;
	//Crea la texture dalla bitmap
	SDL_Texture* fontTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, font.bWidth, font.bHeight);

	// Trasforma i pixel da 8-bit a 32-bit RGBA
	std::vector<uint32_t> rgbaBitmap(font.bWidth * font.bHeight);
	for (size_t i = 0; i < font.bitmap.size(); ++i) {
		uint8_t alpha = font.bitmap[i];
		// Colore bianco (255,255,255) + Alpha dinamico
		rgbaBitmap[i] = (255 << 24) | (255 << 16) | (255 << 8) | alpha;
	}
	SDL_UpdateTexture(fontTexture, nullptr, rgbaBitmap.data(), font.bHeight * sizeof(uint32_t));
	SDL_SetTextureBlendMode(fontTexture, SDL_BLENDMODE_BLEND);
	// Spesso con R8 in SDL3 è necessario usare SDL_SetTextureScaleMode a lineare per evitare artefatti
	SDL_SetTextureScaleMode(fontTexture, SDL_SCALEMODE_LINEAR);

	// Forza il disegno su tutti i canali
	SDL_SetTextureColorMod(fontTexture, 255, 255, 255);
	cout << "Texture creata!!!" << endl;
	SDL_StartTextInput(window);

	//Spazio per cose init di debug (chiamate a funzioni sperimentali, inclusioni parziali ecc
	testing();
	
	bool running = true;
	SDL_Event event{};
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
		}

		//Render pass (seleziona il colore, pulisce il canvas
		SDL_SetRenderDrawColor(renderer, 30, 30, 35, 255);
		SDL_RenderClear(renderer);

		//QUI CI METTI IL DRAWING E ROBE COSì

		vector<uint32_t> testo = { 'H', 'e', 'l', 'l', 'o', ' ', 0x03B1, 0x03B2, ' ', 0x222B };
		drawTextSDL(renderer, fontTexture, font, testo, 100.0f, 35.0f);


		//Alla fine
		SDL_RenderPresent(renderer);
		//Fps
		this_thread::sleep_for(chrono::milliseconds(10));
	}
	//Pulizia finale
	SDL_DestroyTexture(fontTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	return 0;
}

void testing() {
	constexpr uint32_t mask = ut::bit::getMask(1u << 2, 1u << 1);
	std::cout << "Mask tra 1u << 2 e 1u << 1: " << endl;
	std::cout << std::hex << mask << endl;
	std::cout << "Valore di 3U secondo la mask sopra" << ut::bit::maskedToU(3u, mask) << std::endl;
	widget::Canvas canvas;
	namespace w = widget;
	canvas.makeBackground();
	w::WidgetCore bg = canvas.getBlock(0);
	std::cout << "Cose di background" << bg.handle;
	std::vector<w::WidgetCoreInfo> infos = DebugHelper::generateDebugWidgetInfo();
	infos.erase(infos.begin());
	std::vector<w::WidgetCore> cores;
	cores.reserve(4);
	cores.push_back(canvas.getBlock(0));
	for (const auto& it : infos)
	{
		w::WidgetCore core = canvas.newWidget(it);
		core.indexing = canvas.placeWidget(core, 0);
		cores.push_back(core);
	}
	canvas.updateExeList();
	w::Geometry::init<canvas.blocknum>();
	int ret = w::Geometry::update(DebugHelper::getCanvasCores(canvas), canvas.flat_exe_list);
	DebugMessage("return di update: " << ret)
	std::vector<w::GeoCore> visual = w::Geometry::getVisual();

}