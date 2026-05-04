#include "LZ77.h"
#include <vector>
#include <string>
#include <iostream>

// Esta función buscará la cadena más larga en el pasado
TuplaLZ77 buscarCoincidencia(const std::string& texto, int posActual, int tamVentanaBusqueda) {
    TuplaLZ77 mejorTupla = {0, 0, texto[posActual]};
    int inicioBusqueda = std::max(0, posActual - tamVentanaBusqueda);

    // Buscamos desde el inicio de nuestra ventana hasta la posición actual
    for (int i = inicioBusqueda; i < posActual; ++i) {
        int longitudMatch = 0;
        
        // Mientras las letras coincidan y no nos pasemos del texto
        while (posActual + longitudMatch < texto.length() && 
               texto[i + longitudMatch] == texto[posActual + longitudMatch]) {
            longitudMatch++;
        }

        // Si encontramos una coincidencia más larga que la anterior, la guardamos
        if (longitudMatch >= mejorTupla.length) {
            mejorTupla.offset = posActual - i; // Distancia hacia atrás
            mejorTupla.length = longitudMatch;
            mejorTupla.nextChar = texto[posActual + longitudMatch];
        }
    }
    return mejorTupla;
}
// Esta función reconstruye el texto original a partir de las tuplas
std::string descomprimirLZ77(const std::vector<TuplaLZ77>& comprimido) {
    std::string resultado = "";
    for (const auto& tupla : comprimido) {
        if (tupla.length > 0) {
            int inicio = resultado.length() - tupla.offset;
            for (int i = 0; i < tupla.length; ++i) {
                resultado += resultado[inicio + i];
            }
        }
        if (tupla.nextChar != '\0') { // Si hay un carácter siguiente, lo agregamos
            resultado += tupla.nextChar;
        }
    }
    return resultado;
}