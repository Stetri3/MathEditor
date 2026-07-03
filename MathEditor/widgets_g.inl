#include "widgets.h"
namespace widget {
	template <uint32_t blocknum>
	static void Geometry::init() {
		visualArr.reserve(blocknum);
		orderedHandles.reserve(blocknum);
	}

	inline GeoArr<64> Geometry::parseContainer(const WidgetArr<64u>& members, std::span<GeoCore> evaluated)
	{
		//DA FARE QUIIIIIII

		
	}
}