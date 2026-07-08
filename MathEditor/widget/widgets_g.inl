#include "widgets.h"
namespace widget {
	template <uint32_t blocknum>
	static void Geometry::init() {
		visualArr.resize(blocknum, GeoCore());
		orderedHandles.reserve(blocknum);
		idealSizes.resize(blocknum, RectSize());
		childCount.resize(blocknum, 0);
		weights.resize(blocknum, (1<<4u) -1);
	}

}