#pragma once

#include <fstream>
#include <vector>
#include <SDL3/SDL_RECT.h>
#include <bit>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <new>
#include <utility>

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

    template <typename T, std::size_t MaxSize>
    class static_vector {
    private:
        // Allineamento corretto per il tipo T senza invocare costruttori di default
        alignas(T) std::byte m_data[MaxSize * sizeof(T)];
        std::size_t m_size = 0;

        // Helper per l'accesso ai puntatori
        T* data_ptr() noexcept { return reinterpret_cast<T*>(m_data); }
        const T* data_ptr() const noexcept { return reinterpret_cast<const T*>(m_data); }

    public:
        // Costruttori e Distruttore
        static_vector() = default;

        ~static_vector() {
            clear();
        }

        // Copy e Move Semantics
        static_vector(const static_vector& other) {
            for (std::size_t i = 0; i < other.m_size; ++i) {
                push_back(other[i]);
            }
        }

        static_vector(static_vector&& other) noexcept(std::is_move_constructible_v<T>) {
            for (std::size_t i = 0; i < other.m_size; ++i) {
                push_back(std::move(other[i]));
            }
            other.clear();
        }

        static_vector& operator=(const static_vector& other) {
            if (this != &other) {
                clear();
                for (std::size_t i = 0; i < other.m_size; ++i) {
                    push_back(other[i]);
                }
            }
            return *this;
        }

        static_vector& operator=(static_vector&& other) noexcept(std::is_move_assignable_v<T>) {
            if (this != &other) {
                clear();
                for (std::size_t i = 0; i < other.m_size; ++i) {
                    push_back(std::move(other[i]));
                }
                other.clear();
            }
            return *this;
        }

        // Modificatori
        template <typename... Args>
        T& emplace_back(Args&&... args) {
            if (m_size >= MaxSize) {
                throw std::out_of_range("static_vector overflow");
            }
            T* space = data_ptr() + m_size;
            ::new (static_cast<void*>(space)) T(std::forward<Args>(args)...);
            return data_ptr()[m_size++];
        }

        void push_back(const T& value) { emplace_back(value); }
        void push_back(T&& value) { emplace_back(std::move(value)); }

        void pop_back() {
            if (m_size > 0) {
                --m_size;
                data_ptr()[m_size].~T();
            }
        }

        void clear() noexcept {
            while (m_size > 0) {
                pop_back();
            }
        }

        // Accesso agli elementi
        T& operator[](std::size_t index) noexcept { return data_ptr()[index]; }
        const T& operator[](std::size_t index) const noexcept { return data_ptr()[index]; }

        T& at(std::size_t index) {
            if (index >= m_size) throw std::out_of_range("Index out of bounds");
            return data_ptr()[index];
        }
        const T& at(std::size_t index) const {
            if (index >= m_size) throw std::out_of_range("Index out of bounds");
            return data_ptr()[index];
        }

        T* data() noexcept { return data_ptr(); }
        const T* data() const noexcept { return data_ptr(); }

        // Capacità
        [[nodiscard]] std::size_t size() const noexcept { return m_size; }
        [[nodiscard]] std::size_t capacity() const noexcept { return MaxSize; }
        [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
        [[nodiscard]] bool full() const noexcept { return m_size == MaxSize; }

        // Iteratori (compatibilità con i cicli for-range)
        T* begin() noexcept { return data_ptr(); }
        const T* begin() const noexcept { return data_ptr(); }
        T* end() noexcept { return data_ptr() + m_size; }
        const T* end() const noexcept { return data_ptr() + m_size; }
    };

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

#define M2U(v, m) (::ut::bit::maskedToU((v), (m)))
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


        template <typename T>
        constexpr bool checkMasked(T value, T mask, T toCheck) noexcept {
            static_assert(std::is_unsigned_v<T>, "Richiesto tipo unsigned");
            return (maskedToU(value, mask) == toCheck);
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