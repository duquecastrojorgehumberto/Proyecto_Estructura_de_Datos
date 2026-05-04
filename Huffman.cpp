#include "Huffman.h"
#include <iostream>

// Función para generar los códigos (recorriendo el árbol)
void generarCodigos(NodoHuffman* raiz, std::string codigo, std::map<char, std::string>& tabla) {
    if (!raiz) return;

    // Si es una hoja, guardamos el código acumulado
    if (!raiz->izq && !raiz->der) {
        tabla[raiz->caracter] = codigo;
    }

    generarCodigos(raiz->izq, codigo + "0", tabla);
    generarCodigos(raiz->der, codigo + "1", tabla);
}

// Función principal para construir el árbol
std::map<char, std::string> construirArbolHuffman(std::map<char, int>& frecuencias) {
    std::priority_queue<NodoHuffman*, std::vector<NodoHuffman*>, Comparar> minHeap;

    // 1. Crear un nodo hoja para cada carácter y añadirlo a la cola
    for (auto const& [caracter, frec] : frecuencias) {
        minHeap.push(new NodoHuffman(caracter, frec));
    }

    // 2. Mientras haya más de un nodo en la cola
    while (minHeap.size() != 1) {
        // Extraer los dos nodos con menor frecuencia
        NodoHuffman *izq = minHeap.top(); minHeap.pop();
        NodoHuffman *der = minHeap.top(); minHeap.pop();

        // Crear un nuevo nodo interno con la suma de las frecuencias
        // Usamos un carácter especial (como '\0') para nodos internos
        NodoHuffman *unido = new NodoHuffman('\0', izq->frecuencia + der->frecuencia);
        unido->izq = izq;
        unido->der = der;

        minHeap.push(unido);
    }

    // 3. El nodo restante es la raíz del árbol
    std::map<char, std::string> tablaCodigos;
    generarCodigos(minHeap.top(), "", tablaCodigos);
    
    return tablaCodigos;
}