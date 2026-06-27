#include "utils.h"

using namespace ut;

int ut::readBFile(const char* path, std::vector<uint8_t>& buffer)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open()) return -1; //Errore -1, file non trovato (o non accessibile)
	std::streamsize size = file.tellg();
	if (size < 0) return -2; //Errore -2, file corrotto o altro problema meta
	file.seekg(0, std::ios::beg);
	buffer.resize(static_cast<size_t>(size));
	file.read(reinterpret_cast<char*>(buffer.data()), size);
	if (!file) {
		if (file.bad()) return -3; //Errore logico di sistema durante/dopo lettura
		else if (file.eof()) return 1; //Letteralmente o questa funzione è errata o la lettura del file è stata cambiata in contemporanea
									   //Può essere utile tenere i dati
		else return -4; //Altro errore, fail
	}
	return 0;
}

constexpr uint64_t ut::join(uint32_t low, uint32_t high)
{
	return (static_cast<uint64_t>(high) << 32) + low;
}

/*constexpr bool ut::isInside(SDL_FRect rect, float x, float y, bool closed)
{
	if (closed)
	{
		return rect.x <= x && rect.x + rect.w >= x && rect.y <= y && rect.y + rect.h >= y;
	}
	else {
		return rect.x < x && rect.x + rect.w > x && rect.y < y && rect.y + rect.h > y;
	}
}
*/
