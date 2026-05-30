#include "Huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void construirArbolHuffman(int frecuencias[256]) {
    printf("\n========================================\n");
    printf("    FASE HUFFMAN: ANALISIS DE PESOS      \n");
    printf("========================================\n");
    printf("%-10s | %-10s\n", "CARACTER", "FRECUENCIA");
    printf("-----------|-----------\n");

    int caracteresDistintos = 0;
    for (int i = 0; i < 256; i++) {
        if (frecuencias[i] > 0) {
            caracteresDistintos++;
            if (i == '\n') printf("%-10s | %-10d\n", "'\\n'", frecuencias[i]);
            else if (i == ' ') printf("%-10s | %-10d\n", "'ESP'", frecuencias[i]);
            else if (i == '\t') printf("%-10s | %-10d\n", "'\\t'", frecuencias[i]);
            else printf("'%c'        | %-10d\n", (char)i, frecuencias[i]);
        }
    }
    printf("----------------------------------------\n");
    printf("Total de simbolos unicos: %d\n", caracteresDistintos);
    printf("----------------------------------------\n");
}

// 1. CONSTRUCCIÓN DEL ÁRBOL EN RAM
NodoHuffman* generarArbolReal(int frecuencias[256]) {
    NodoHuffman* lista[256];
    int n = 0;

    // Crear un nodo hoja por cada carácter que exista en el archivo
    for (int i = 0; i < 256; i++) {
        if (frecuencias[i] > 0) {
            lista[n++] = crearNuevoNodo((char)i, frecuencias[i]);
        }
    }

    if (n == 0) return NULL;
    if (n == 1) {
        NodoHuffman* padre = crearNuevoNodo('\0', lista[0]->frecuencia);
        padre->izq = lista[0];
        return padre;
    }

    // Bucle de fusión: Combinar los dos nodos con menores frecuencias
    while (n > 1) {
        // Ordenar la lista por frecuencia de mayor a menor (los menores quedan al final)
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                if (lista[i]->frecuencia < lista[j]->frecuencia) {
                    NodoHuffman* temp = lista[i];
                    lista[i] = lista[j];
                    lista[j] = temp;
                }
            }
        }

        // Extraer los dos más pequeños
        NodoHuffman* primero = lista[n - 1];
        NodoHuffman* segundo = lista[n - 2];

        // Crear el nodo padre
        NodoHuffman* padre = crearNuevoNodo('\0', primero->frecuencia + segundo->frecuencia);
        padre->izq = primero;
        padre->der = segundo;

        // Reemplazar en la lista de trabajo
        lista[n - 2] = padre;
        n--;
    }
    return lista[0]; // La raíz del árbol de Huffman
}

// 2. RECORRIDO DEL ÁRBOL PARA ASIGNAR CÓDIGOS BINARIOS ('0' y '1')
void generarCodigos(NodoHuffman* raiz, char codigos[256][100], char* codigoActual, int top) {
    if (raiz->izq) {
        codigoActual[top] = '0';
        generarCodigos(raiz->izq, codigos, codigoActual, top + 1);
    }
    if (raiz->der) {
        codigoActual[top] = '1';
        generarCodigos(raiz->der, codigos, codigoActual, top + 1);
    }
    // Si es un nodo hoja, guardamos el código obtenido
    if (!raiz->izq && !raiz->der) {
        codigoActual[top] = '\0';
        strcpy(codigos[(unsigned char)raiz->caracter], codigoActual);
    }
}

// 3. BIT-PACKING: COMPACTAR CARACTERES EN BITS REALES PARA EL DISCO DURO
void escribirBitsFisicos(char* texto, long tam, char codigos[256][100], FILE* f) {
    unsigned char bitBuffer = 0;
    int bitsCargados = 0;

    for (long i = 0; i < tam; i++) {
        char* codigo = codigos[(unsigned char)texto[i]];
        for (int j = 0; codigo[j] != '\0'; j++) {
            bitBuffer <<= 1; // Desplazar bits a la izquierda
            if (codigo[j] == '1') {
                bitBuffer |= 1; // Encender el bit con una operación OR
            }
            bitsCargados++;

            // Si completamos 8 bits, tenemos 1 byte real -> Escribir al archivo
            if (bitsCargados == 8) {
                fwrite(&bitBuffer, 1, 1, f);
                bitBuffer = 0;
                bitsCargados = 0;
            }
        }
    }

    // Rellenar con ceros si el último byte no se completó completamente
    if (bitsCargados > 0) {
        bitBuffer <<= (8 - bitsCargados);
        fwrite(&bitBuffer, 1, 1, f);
    }
}

// 4. DECODIFICACIÓN Y LECTURA BIT A BIT DESDE EL DISCO
char* decodificarBitsFisicos(FILE* f, NodoHuffman* raiz, long tamCuerpo, int* tamSalida) {
    long capDest = 1000;
    int lenDest = 0;
    char* destino = malloc(capDest);
    
    NodoHuffman* actual = raiz;
    unsigned char byteLeido;
    long bytesProcesados = 0;

    // Leer el archivo byte por byte
    while (bytesProcesados < tamCuerpo && fread(&byteLeido, 1, 1, f) == 1) {
        bytesProcesados++;
        // Extraer los 8 bits individuales de este byte
        for (int i = 7; i >= 0; i--) {
            int bit = (byteLeido >> i) & 1;

            if (bit == 0) actual = actual->izq;
            else actual = actual->der;

            // Si es un nodo hoja, encontramos un carácter original
            if (!actual->izq && !actual->der) {
                if (lenDest >= capDest - 1) {
                    capDest *= 2;
                    destino = realloc(destino, capDest);
                }
                destino[lenDest++] = actual->caracter;
                actual = raiz; // Regresar a la raíz para el siguiente carácter
            }
        }
    }
    destino[lenDest] = '\0';
    *tamSalida = lenDest;
    return destino;
}

void liberarArbol(NodoHuffman* raiz) {
    if (!raiz) return;
    liberarArbol(raiz->izq);
    liberarArbol(raiz->der);
    free(raiz);
}