#include <array>

namespace ut::type {

	template <typename T>
	auto structToA(const T& s){
		//to reinterpret a struct of same size elements as an array
		return reinterpret_cast<const uint8_t*>(&s);
	}
}