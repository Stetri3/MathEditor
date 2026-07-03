#pragma once
#include "../widgets.h"
#include <fstream>
#include <vector>
#include <string_view>
#include <cstdint>
#include <type_traits>

namespace ut::IO {

    //Scrive un file PER UN SINGOLO VETTORE
    template <typename T>
    bool save_single_vector(const std::string& filepath, const std::vector<T>& vec) {
        static_assert(std::is_trivially_copyable_v<T>, "Deve essere POD");
        // Apriamo in modalità binaria (std::ios::binary)
        std::ofstream file(filepath, std::ios::out | std::ios::binary);
        if (!file) return false;

        // 1. (Opzionale ma consigliato) Salva il numero di elementi per sapere quanti rileggerne
        uint32_t num_elements = static_cast<uint32_t>(vec.size());
        file.write(reinterpret_cast<const char*>(&num_elements), sizeof(num_elements));

        // 2. Dump atomico di tutta la memoria del vector su disco
        if (num_elements > 0) {
            file.write(reinterpret_cast<const char*>(vec.data()), num_elements * sizeof(T));
        }

        return file.good();
    }

    //Carica un file vettore singolo
    template <typename T>
    bool load_single_vector(const std::string& filepath, std::vector<T>& vec) {
        std::ifstream file(filepath, std::ios::in | std::ios::binary);
        if (!file) return false;

        // 1. Leggi il numero di elementi salvati
        uint32_t num_elements = 0;
        file.read(reinterpret_cast<char*>(&num_elements), sizeof(num_elements));

        // 2. Ridimensiona il vector: la memoria viene allocata linearmente nello stack/heap
        vec.resize(num_elements);

        // 3. Leggi i byte dal disco direttamente dentro il buffer del vector
        if (num_elements > 0) {
            file.read(reinterpret_cast<char*>(vec.data()), num_elements * sizeof(T));
        }

        return file.good();
    }


    // Hash FNV-1a a 32 bit eseguito a compile-time
    constexpr uint32_t fnv1a_hash(std::string_view str) {
        uint32_t hash = 2166136261u;
        for (char c : str) {
            hash ^= static_cast<uint32_t>(c);
            hash *= 16777619u;
        }
        return hash;
    }

    // Helper per generare un ID univoco per il tipo T basato sul suo nome testuale
    template <typename T>
    constexpr uint32_t get_type_id(std::string_view custom_name) {
        return fnv1a_hash(custom_name);
    }

    template <typename T>
    bool append_pod_vector(const std::string& filepath, std::string_view type_name, const std::vector<T>& vec) {
        static_assert(std::is_trivially_copyable_v<T>, "Il tipo deve essere un POD puro.");

        std::ofstream file(filepath, std::ios::out | std::ios::binary | std::ios::app);
        if (!file) return false;

        // Genera l'ID del tipo a compile-time/inline dalla stringa
        uint32_t type_id = get_type_id<T>(type_name);
        uint32_t size = static_cast<uint32_t>(vec.size());

        // Scrittura dell'header del blocco
        file.write(reinterpret_cast<const char*>(&type_id), sizeof(type_id));
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));

        // Scrittura dei dati
        if (size > 0) {
            file.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
        }

        return file.good();
    }

    template <typename T>
    bool try_read_pod_vector(std::ifstream& file, std::string_view type_name, std::vector<T>& vec) {
        static_assert(std::is_trivially_copyable_v<T>, "Il tipo deve essere un POD puro.");

        // Salva la posizione iniziale del cursore nel caso non sia il tipo corretto
        auto entry_pos = file.tellg();

        uint32_t file_type_id = 0;
        if (!file.read(reinterpret_cast<char*>(&file_type_id), sizeof(file_type_id))) {
            return false;
        }

        uint32_t size = 0;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));

        // Verifica se l'ID nel file corrisponde all'hash del tipo atteso
        if (file_type_id == get_type_id<T>(type_name)) {
            vec.resize(size);
            if (size > 0) {
                file.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
            }
            return true;
        }

        // Se non è il tipo cercato, ripristina il cursore all'inizio dell'header del blocco
        // in modo che un'altra funzione o un loop possano tentare di leggerlo
        file.seekg(entry_pos);
        return false;
    }

}
//ESEMPIO DI UTILIZZO:
/*
// 1. Salvataggio (Append continuo di cose diverse)
io::append_pod_vector("canvas.bin", "WidgetCore", my_cores_vector);
io::append_pod_vector("canvas.bin", "CustomFlags", my_flags_vector);

// 2. Caricamento selettivo
std::ifstream file("canvas.bin", std::ios::in | std::ios::binary);

while (file.peek() != EOF) {
    // Prova a caricarlo nel vector dei core
    if (io::try_read_pod_vector(file, "WidgetCore", my_cores_vector)) {
        continue;
    }
    // Se non era un WidgetCore, vedi se è il vector delle Flag
    if (io::try_read_pod_vector(file, "CustomFlags", my_flags_vector)) {
        continue;
    }

    // Se arrivi qui, il blocco è di un tipo che non ti interessa in questo contesto:
    // Leggi l'header per sapere quanti byte saltare e vai avanti
    uint32_t dummy_id, size;
    file.read(reinterpret_cast<char*>(&dummy_id), sizeof(dummy_id));
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    // Nota: in questo scenario generico estremo dovresti salvare anche sizeof(T) nell'header
    // se vuoi saltare blocchi completamente sconosciuti senza una tabella fissa.
}
*/