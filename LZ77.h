#ifndef LZ77_H
#define LZ77_H

#include <string>
#include <vector>

struct TuplaLZ77 {
    unsigned short offset; // Cambia int por unsigned short (2 bytes)
    unsigned short length; // Cambia int por unsigned short (2 bytes)
    char nextChar;         // (1 byte)
};

// Declaraciones de funciones
TuplaLZ77 buscarCoincidencia(const std::string& texto, int posActual, int tamVentanaBusqueda);
std::string descomprimirLZ77(const std::vector<TuplaLZ77>& comprimido);

#endif