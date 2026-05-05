#include "Huffman.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Función para crear un nuevo nodo en memoria (reemplaza al constructor de C++)
 */
NodoHuffman* crearNuevoNodo(char c, int f) {
    NodoHuffman* nuevo = (NodoHuffman*)malloc(sizeof(NodoHuffman));
    if (nuevo != NULL) {
        nuevo->caracter = c;
        nuevo->frecuencia = f;
        nuevo->izq = NULL;
        nuevo->der = NULL;
    }
    return nuevo;
}

/**
 * Función que construye la tabla de frecuencias.
 * En C, esta función es vital para el reporte de resultados del proyecto.
 */
void construirArbolHuffman(int frecuencias[256]) {
    printf("\n========================================\n");
    printf("   FASE HUFFMAN: ANALISIS DE PESOS      \n");
    printf("========================================\n");
    printf("%-10s | %-10s\n", "CARACTER", "FRECUENCIA");
    printf("-----------|-----------\n");

    int caracteresDistintos = 0;

    for (int i = 0; i < 256; i++) {
        if (frecuencias[i] > 0) {
            caracteresDistintos++;
            
            // Manejo de caracteres especiales para que la tabla se vea limpia
            if (i == '\n') printf("%-10s | %-10d\n", "'\\n'", frecuencias[i]);
            else if (i == ' ') printf("%-10s | %-10d\n", "'ESP'", frecuencias[i]);
            else if (i == '\t') printf("%-10s | %-10d\n", "'\\t'", frecuencias[i]);
            else printf("'%c'        | %-10d\n", (char)i, frecuencias[i]);
        }
    }

    if (caracteresDistintos == 0) {
        printf("No se detectaron caracteres para Huffman.\n");
    } else {
        printf("----------------------------------------\n");
        printf("Total de simbolos unicos: %d\n", caracteresDistintos);
        printf("----------------------------------------\n");
    }
}