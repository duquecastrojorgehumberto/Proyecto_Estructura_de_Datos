#ifndef HUFFMAN_H
#define HUFFMAN_H

typedef struct NodoHuffman {
    char caracter;
    int frecuencia;
    struct NodoHuffman *izq, *der;
} NodoHuffman;

void construirArbolHuffman(int frecuencias[256]);

#endif