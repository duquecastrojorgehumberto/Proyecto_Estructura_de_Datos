#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>

typedef struct NodoHuffman {
    char caracter;
    int frecuencia;
    struct NodoHuffman *izq, *der;
} NodoHuffman;

// Crea un nodo individual en RAM
NodoHuffman* crearNuevoNodo(char c, int f);

// Muestra la tabla visual requerida por el proyecto
void construirArbolHuffman(int frecuencias[256]);

// --- FUNCIONES REALES DEL ALGORITMO ---

// Construye el árbol real combinando los nodos y devuelve la raíz
NodoHuffman* generarArbolReal(int frecuencias[256]);

// Recorre el árbol para asignar las cadenas de '0' y '1' a cada carácter
void generarCodigos(NodoHuffman* raiz, char codigos[256][100], char* codigoActual, int top);

// Toma las cadenas de '0' y '1' y las empaqueta en bytes reales (Bit-Packing) en el archivo
void escribirBitsFisicos(char* texto, long tam, char codigos[256][100], FILE* f);

// Lee los bytes del disco, extrae sus bits y camina por el árbol para recuperar el texto
char* decodificarBitsFisicos(FILE* f, NodoHuffman* raiz, long tamCuerpo, int* tamSalida);

// Libera la memoria RAM del árbol binario al terminar
void liberarArbol(NodoHuffman* raiz);

#endif