#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//==================================================
// CONSTANTES
//==================================================

#define maxCharacters 256
#define maxCodeLength 100

//==================================================
// ESTRUCTURA NODO HUFFMAN
//==================================================

typedef struct huffmanNode {
    char character;
    int frequency;
    struct huffmanNode* leftChild;
    struct huffmanNode* rightChild;
} huffmanNode;

//==================================================
// ESTRUCTURA MIN HEAP
//==================================================

typedef struct minHeap {
    int size;
    huffmanNode* nodes[maxCharacters];
} minHeap;

//==================================================
// FUNCIONES BÁSICAS
//==================================================

huffmanNode* createNode(char character, int frequency);

//==================================================
// FUNCIONES DEL HEAP
//==================================================

void swapNodes(huffmanNode** a, huffmanNode** b);
void heapify(minHeap* heap, int index);
huffmanNode* extractMin(minHeap* heap);
void insertHeap(minHeap* heap, huffmanNode* node);
minHeap* createMinHeap(int frequencies[]);

//==================================================
// ÁRBOL DE HUFFMAN
//==================================================

huffmanNode* buildHuffmanTree(int frequencies[]);

//==================================================
// GENERACIÓN DE CÓDIGOS
//==================================================

void generateCodes(
    huffmanNode* root,
    char* currentCode,
    int depth,
    char* codes[]
);

//==================================================
// TABLA DE FRECUENCIAS
//==================================================

void buildFrequencyTable(
    const char* text,
    int frequencies[]
);

void buildFrequencyTableBinary(
    const unsigned char* data,
    long dataSize,
    int frequencies[]
);

//==================================================
// COMPRESIÓN
//==================================================

void encodeToBinaryFile(
    FILE* outputFile,
    const char* inputText,
    char* codes[]
);

// NUEVA: Para codificar buffers binarios crudos (LZ77) sin depender de '\0'
void encodeToBinaryFileRaw(
    FILE* outputFile,
    const unsigned char* inputData,
    long dataSize,
    char* codes[]
);

void saveTree(
    huffmanNode* root,
    FILE* file
);

void saveCompressedFile(
    FILE* file,
    const char* inputText,
    char* codes[],
    huffmanNode* root
);

// NUEVA: Para guardar archivos comprimidos usando el tamaño binario exacto
void saveCompressedFileBinary(
    FILE* file,
    const unsigned char* inputData,
    long dataSize,
    char* codes[],
    huffmanNode* root
);

//==================================================
// DESCOMPRESIÓN
//==================================================

huffmanNode* loadTree(FILE* file);
void decodeFile(const char* inputFileName);

// NUEVA: Extrae los bytes directamente a un arreglo en memoria (para simetría total)
unsigned char* decodeFileToMemory(FILE* file, long* outputLength);

#endif