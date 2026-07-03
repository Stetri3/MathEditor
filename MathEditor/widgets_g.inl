#include "widgets.h"
namespace widget {
	template <uint32_t blocknum>
	static void Geometry::init() {
		visualArr.resize(blocknum, GeoCore());
		orderedHandles.reserve(blocknum);
	}

}