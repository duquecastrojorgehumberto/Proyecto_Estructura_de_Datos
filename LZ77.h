#ifndef LZ77_H
#define LZ77_H

typedef struct {
    unsigned short offset;
    unsigned short length;
    char nextChar;
} TuplaLZ77;

TuplaLZ77 buscarCoincidencia(const char* texto, int posActual, int tamVentanaBusqueda, int longitudTexto);
char* descomprimirLZ77(TuplaLZ77* comprimido, int numTuplas, int* tamanoFinal);

#endif