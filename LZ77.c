#include "LZ77.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Reemplazo de std::max
#define MAX(a,b) (((a)>(b))?(a):(b))

// Esta función buscará la cadena más larga en el pasado
TuplaLZ77 buscarCoincidencia(const char* texto, int posActual, int tamVentanaBusqueda, int longitudTexto) {
    // Inicializamos con (0, 0, carácter actual)
    TuplaLZ77 mejorTupla = {0, 0, texto[posActual]};
    
    int inicioBusqueda = MAX(0, posActual - tamVentanaBusqueda);

    // Buscamos desde el inicio de nuestra ventana hasta la posición actual
    for (int i = inicioBusqueda; i < posActual; ++i) {
        int longitudMatch = 0;
        
        // Mientras las letras coincidan y no nos pasemos del texto
        // Nota: en C usamos longitudTexto porque ya no existe texto.length()
        while (posActual + longitudMatch < longitudTexto && 
               texto[i + longitudMatch] == texto[posActual + longitudMatch]) {
            longitudMatch++;
        }

        // Si encontramos una coincidencia más larga (o igual), la guardamos
        if (longitudMatch >= mejorTupla.length) {
            mejorTupla.offset = (unsigned short)(posActual - i);
            mejorTupla.length = (unsigned short)longitudMatch;
            
            // Si el match llega al final del texto, nextChar podría ser nulo
            if (posActual + longitudMatch < longitudTexto) {
                mejorTupla.nextChar = texto[posActual + longitudMatch];
            } else {
                mejorTupla.nextChar = '\0';
            }
        }
    }
    return mejorTupla;
}

// Esta función reconstruye el texto original a partir de las tuplas
// En C, devolvemos un char* y necesitamos punteros para saber el tamaño final
char* descomprimirLZ77(TuplaLZ77* comprimido, int numTuplas, int* tamanoFinal) {
    // Reservamos un tamaño inicial razonable (ej. 1MB o ajustado)
    // En un sistema real, usaríamos realloc si el texto es muy grande
    int capacidad = 1024 * 1024; 
    char* resultado = (char*)malloc(capacidad * sizeof(char));
    int lenResultado = 0;

    for (int i = 0; i < numTuplas; i++) {
        TuplaLZ77 tupla = comprimido[i];

        // 1. Copiar del pasado si hay match
        if (tupla.length > 0) {
            int inicio = lenResultado - tupla.offset;
            for (int j = 0; j < tupla.length; ++j) {
                resultado[lenResultado++] = resultado[inicio + j];
            }
        }
        
        // 2. Agregar el carácter siguiente si no es nulo
        if (tupla.nextChar != '\0') {
            resultado[lenResultado++] = tupla.nextChar;
        }
    }

    resultado[lenResultado] = '\0'; // Terminador de cadena para C
    *tamanoFinal = lenResultado; // Guardamos cuánto midió realmente
    return resultado;
}