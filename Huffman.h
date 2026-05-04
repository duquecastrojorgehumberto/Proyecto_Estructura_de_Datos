#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>
#include <map>
#include <vector>
#include <queue>

// Estructura de cada nodo del árbol de Huffman
struct NodoHuffman {
    char caracter;      // El carácter que estamos guardando
    int frecuencia;    // Cuántas veces aparece
    NodoHuffman *izq, *der;

    // Constructor para crear nodos nuevos fácilmente
    NodoHuffman(char c, int f) {
        izq = der = nullptr;
        this->caracter = c;
        this->frecuencia = f;
    }
};

// Objeto de comparación para que la cola de prioridad 
// ponga siempre el de menor frecuencia arriba
struct Comparar {
    bool operator()(NodoHuffman* l, NodoHuffman* r) {
        return l->frecuencia > r->frecuencia;
    }
};
std::map<char, std::string> construirArbolHuffman(std::map<char, int>& frecuencias);

#endif