#pragma once

#include <fstream>
#include <vector>
#include <SDL3/SDL_RECT.h>
#include <bit>
#include <array>

constexpr bool AUTO = true; //Se implementare automaticamente la struttura default piuttosto che lasciare la logica all'user
constexpr bool SAFE = true;
namespace ut {
	struct any {
		void* data = nullptr;
		size_t size = 0;
		any() = default;
		operator void* () const {
			return data;
		};
		any(std::nullptr_t) : data(nullptr), size(0) {};
	};

	int readBFile(const char* path, std::vector<uint8_t>& buffer);
	constexpr uint64_t join(uint32_t low, uint32_t high);
	constexpr bool isInside(SDL_FRect rect, float x, float y, bool closed = false)
	{
		if (closed)
		{
			return rect.x <= x && rect.x + rect.w >= x && rect.y <= y && rect.y + rect.h >= y;
		}
		else {
			return rect.x < x && rect.x + rect.w > x && rect.y < y && rect.y + rect.h > y;
		}
	}
	namespace bit {
		/**
		* @brief Crea una maschera contigua da onehot uint 'start' a 'stop', inclusi.
		* @tparam T Deve essere un tipo intero senza segno (uint8_t, uint32_t, ecc.)
		* @param start Il valore più a sinistra (most significant)
		* @param stop Il valore più a destra
		* - dev'essere minore di start
		*/
		template <typename T>
		constexpr T getMask(T start, T stop) noexcept {
			static_assert(std::is_unsigned_v<T>, "Richiesto tipo unsigned");
			return (start | (start - 1)) & ~(stop - 1);
		}

		/**
		* @brief Estrae il valore dalla maschera contigua.
		* @tparam T Deve essere un tipo intero senza segno (uint8_t, uint32_t, ecc.)
		* @param value Il valore originario da filtrare
		* @param mask La maschera bit a bit (Default: tutti i bit a 1)
		* - La maschera dev'essere contigua
		*/
		template <typename T>
		constexpr T maskedToU(T value, T mask) noexcept {
			static_assert(std::is_unsigned_v<T>, "Richiesto tipo unsigned");
			return (value & mask) >> std::countr_zero(mask);
		}

		/**
		* @brief Impacchetta il numero in una maschera contigua
		* @tparam T Deve essere un tipo intero senza segno (uint8_t, uint32_t, ecc.)
		* @param value Il valore originario da filtrare
		* - Se il valore in bit è più largo di mask verrà tagliato a sinistra
		* @param mask La maschera bit a bit (Default: tutti i bit a 1)
		* - La maschera dev'essere contigua
		*/
		template <typename T>
		constexpr T uToMasked(T value, T mask) noexcept {
			static_assert(std::is_unsigned_v<T>, "Richiesto tipo unsigned");
			return (value << std::countr_zero(mask)) & mask;
		}
	}
	constexpr std::array<char, 19> printHex(uint64_t num) {
		std::array<char, 19> buffer{};
		const char* digits = "0123456789ABCDEF";

		buffer[0] = '0';
		buffer[1] = 'x';
		buffer[18] = '\0';

		// Riempie l'array partendo dal fondo
		for (int i = 17; i >= 2; --i) {
			buffer[i] = digits[num & 0xF];
			num >>= 4;
		}

		return buffer;
	}
}