#include "widgets.h"
namespace widget {
	template <uint32_t blocknum>
	static void Geometry::init() {
		visualArr.reserve(blocknum);
		orderedHandles.reserve(blocknum);
	}
	template<uint8_t out_size>
	static inline ut::static_vector<GeoCore, out_size> widget::Geometry::parseCLayout(const WidgetArr<64u>& members)
	{
		//DA FARE QUIIIIIII

		return std::array<GeoCore, 2 ^ size_bin>();
	}
}