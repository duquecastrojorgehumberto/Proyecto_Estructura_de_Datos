#include "Huffman.h"

//==================================================
// 3. FUNCIONES BÁSICAS (NODOS)
//Las hojas contienen caracteres
//Los nodos internos solo suman frecuencias
//crear Nodo con carácter y frecuencia
//==================================================

huffmanNode* createNode(char character, int frequency) {
    huffmanNode* newNode = (huffmanNode*)malloc(sizeof(huffmanNode));

    newNode->character = character;
    newNode->frequency = frequency;
    newNode->leftChild = NULL;
    newNode->rightChild = NULL;

    return newNode;
}

//==================================================
// 4. FUNCIONES DEL HEAP
//==================================================

// Intercambia dos nodos dentro del heap
void swapNodes(huffmanNode** a, huffmanNode** b) {
    huffmanNode* temp = *a;
    *a = *b;
    *b = temp;
}

//Heapify (mantener orden del heap)
// Mantiene la propiedad de minHeap desde un índice dado
void heapify(minHeap* heap, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    // Verificar si hijo izquierdo es menor
    if (left < heap->size &&
        heap->nodes[left]->frequency < heap->nodes[smallest]->frequency) {
        smallest = left;
    }

    // Verificar si hijo derecho es menor
    if (right < heap->size &&
        heap->nodes[right]->frequency < heap->nodes[smallest]->frequency) {
        smallest = right;
    }

    // Si encontramos un nodo más pequeño, intercambiamos
    if (smallest != index) {
        swapNodes(&heap->nodes[smallest], &heap->nodes[index]);
        heapify(heap, smallest);
    }
}

// Extrae el nodo con menor frecuencia
huffmanNode* extractMin(minHeap* heap) {
    huffmanNode* minNode = heap->nodes[0];

    // Reemplazamos raíz con el último nodo
    heap->nodes[0] = heap->nodes[heap->size - 1];
    heap->size--;

    // Restauramos el heap
    heapify(heap, 0);

    return minNode;
}

// Inserta un nodo en el heap manteniendo el orden
void insertHeap(minHeap* heap, huffmanNode* node) {
    int i = heap->size;
    heap->size++;

    // Subimos el nodo mientras sea menor que su padre
    while (i > 0 &&
           node->frequency < heap->nodes[(i - 1) / 2]->frequency) {
        heap->nodes[i] = heap->nodes[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    heap->nodes[i] = node;
}

// Construye el heap inicial (desde frecuencias) con todos los caracteres
minHeap* createMinHeap(int frequencies[]) {
    minHeap* heap = (minHeap*)malloc(sizeof(minHeap));
    heap->size = 0;

    // Crear nodos para cada carácter con frecuencia > 0
    for (int i = 0; i < maxCharacters; i++) {
        if (frequencies[i] > 0) {
            heap->nodes[heap->size++] = createNode((char)i, frequencies[i]);
        }
    }

    // Convertir arreglo en heap válido
    for (int i = (heap->size - 1) / 2; i >= 0; i--) {
        heapify(heap, i);
    }

    return heap;
}


//==================================================
// 5. ÁRBOL DE HUFFMAN
// Construye el árbol de Huffman combinando nodos de menor frecuencia
//==================================================

huffmanNode* buildHuffmanTree(int frequencies[]) {
    minHeap* heap = createMinHeap(frequencies);

    // Mientras haya más de un nodo
    while (heap->size > 1) {
        // Sacamos los dos más pequeños
        huffmanNode* leftNode = extractMin(heap);
        huffmanNode* rightNode = extractMin(heap);

        // Creamos nodo padre (sin carácter)
        huffmanNode* parentNode = createNode('\0',
            leftNode->frequency + rightNode->frequency);

        parentNode->leftChild = leftNode;
        parentNode->rightChild = rightNode;

        // Insertamos de nuevo en el heap
        insertHeap(heap, parentNode);
    }

    // El último nodo es la raíz
    return extractMin(heap);
}

//==================================================
// 6. GENERACIÓN DE CÓDIGOS
// Genera los códigos binarios recorriendo el árbol
//==================================================

void generateCodes(huffmanNode* root, char* currentCode, int depth, char* codes[]) {
    if (root == NULL) return;

    // Si es hoja → guardar código
    if (root->leftChild == NULL && root->rightChild == NULL) {
        currentCode[depth] = '\0';
        codes[(unsigned char)root->character] = strdup(currentCode);
        return;
    }

    // Ir a la izquierda (agregar '0')
    currentCode[depth] = '0';
    generateCodes(root->leftChild, currentCode, depth + 1, codes);

    // Ir a la derecha (agregar '1')
    currentCode[depth] = '1';
    generateCodes(root->rightChild, currentCode, depth + 1, codes);
}

//==================================================
// 7. ENTRADA (FRECUENCIAS)
//Tabla de frecuencias
// Cuenta cuántas veces aparece cada carácter
//==================================================

void buildFrequencyTable(const char* text, int frequencies[]) {
    for (int i = 0; i < maxCharacters; i++) {
        frequencies[i] = 0;
    }

    for (int i = 0; text[i] != '\0'; i++) {
        frequencies[(unsigned char)text[i]]++;
    }
}


//==============NUEVO Refactorización====================================
// TABLA DE FRECUENCIAS BINARIA
// Cuenta cuántas veces aparece cada byte (0-255)
// en un bloque de datos binarios.
//==================================================

void buildFrequencyTableBinary(
    const unsigned char* data, // arreglo de bytes del archivo
    long dataSize,             // cantidad total de bytes
    int frequencies[]          // tabla de frecuencias
) {
    
    // Inicializar todas las frecuencias en 0
    for (int i = 0; i < maxCharacters; i++) {
        frequencies[i] = 0;
    }

    // Recorrer todos los bytes del archivo
    for (long i = 0; i < dataSize; i++) {

        // Aumentar frecuencia del byte actual
        // data[i] puede tener valores entre 0 y 255
        frequencies[data[i]]++;
    }
}

//==================================================
// 8. COMPRESIÓN
// Escribe un archivo binario con los códigos Huffman
//==================================================

void encodeToBinaryFile(FILE* outputFile, const char* inputText, char* codes[]) {

    unsigned char buffer = 0;
    int bitCount = 0;

    for (int i = 0; inputText[i] != '\0'; i++) {
        char* code = codes[(unsigned char)inputText[i]];

        for (int j = 0; code[j] != '\0'; j++) {
            buffer <<= 1;

            if (code[j] == '1') {
                buffer |= 1;
            }

            bitCount++;

            if (bitCount == 8) {
                fwrite(&buffer, sizeof(unsigned char), 1, outputFile);
                buffer = 0;
                bitCount = 0;
            }
        }
    }

    if (bitCount > 0) {
        buffer <<= (8 - bitCount);
        fwrite(&buffer, sizeof(unsigned char), 1, outputFile);
    }

}

//SEGMENTO NUEVO//
//OJO OJO OJO 
// Implementación de encodeToBinaryFileRaw (Usa dataSize en vez de buscar '\0')
void encodeToBinaryFileRaw(FILE* outputFile, const unsigned char* inputData, long dataSize, char* codes[]) {
    unsigned char buffer = 0;
    int bitCount = 0;

    for (long i = 0; i < dataSize; i++) {
        char* code = codes[inputData[i]];

        for (int j = 0; code[j] != '\0'; j++) {
            buffer <<= 1;
            if (code[j] == '1') {
                buffer |= 1;
            }
            bitCount++;

            if (bitCount == 8) {
                fwrite(&buffer, sizeof(unsigned char), 1, outputFile);
                buffer = 0;
                bitCount = 0;
            }
        }
    }

    if (bitCount > 0) {
        buffer <<= (8 - bitCount);
        fwrite(&buffer, sizeof(unsigned char), 1, outputFile);
    }
}

void saveTree(huffmanNode* root, FILE* file) {
    if (root == NULL) return;

    if (root->leftChild == NULL && root->rightChild == NULL) {
        fputc('1', file);
        fputc(root->character, file);
    } else {
        fputc('0', file);
        saveTree(root->leftChild, file);
        saveTree(root->rightChild, file);
    }
}

void saveCompressedFile(FILE* file, const char* inputText, char* codes[], huffmanNode* root) {
    if (!file) {
        printf("Error al crear archivo\n");
        return;
    }
    int textLength = strlen(inputText);
    long dataSizeCast = (long)textLength;
    fwrite(&dataSizeCast, sizeof(long), 1, file);
    saveTree(root, file);
    encodeToBinaryFile(file, inputText, codes);
}

// Implementación de saveCompressedFileBinary
void saveCompressedFileBinary(FILE* file, const unsigned char* inputData, long dataSize, char* codes[], huffmanNode* root) {
    if (!file) {
        printf("Error al crear archivo\n");
        return;
    }
    // Guardamos la longitud real (long) del bloque entrante
    fwrite(&dataSize, sizeof(long), 1, file);
    saveTree(root, file);
    encodeToBinaryFileRaw(file, inputData, dataSize, codes);
}

//==================================================
// 9. DESCOMPRESIÓN
//==================================================

// Función: loadTree
huffmanNode* loadTree(FILE* file) {

    char flag = fgetc(file);

    if (flag == '1') {
        char character = fgetc(file);
        return createNode(character, 0);
    }

    huffmanNode* node = createNode('\0', 0);
    node->leftChild = loadTree(file);
    node->rightChild = loadTree(file);
    return node;
}

// Función: decodeFile
void decodeFile(const char* inputFileName) {
    FILE* file = fopen(inputFileName, "rb");
    if (!file) {
        printf("Error al abrir archivo\n");
        return;
    }

    long originalLength;
    fread(&originalLength, sizeof(long), 1, file);

    huffmanNode* root = loadTree(file);
    huffmanNode* currentNode = root;
    unsigned char currentByte;
    int decodedCharacters = 0;

    while (fread(&currentByte, sizeof(unsigned char), 1, file)) {
        for (int bitIndex = 7; bitIndex >= 0; bitIndex--) {
            int currentBit = (currentByte >> bitIndex) & 1;

            if (currentBit == 0) {
                currentNode = currentNode->leftChild;
            } else {
                currentNode = currentNode->rightChild;
            }

            if (currentNode->leftChild == NULL &&
                currentNode->rightChild == NULL) {

                printf("%c", currentNode->character);
                decodedCharacters++;

                if (decodedCharacters == originalLength) {
                    fclose(file);
                    return;
                }
                currentNode = root;
            }
        }
    }
    fclose(file);
}

//==================================================
// EXTRACTOR EN MEMORIA BINARIO
//==================================================

unsigned char* decodeFileToMemory(
    FILE* file, 
    long* outputLength
) {
    long originalLength;

    // Leer la longitud original del bloque
    if (fread(&originalLength, sizeof(long), 1, file) != 1) {
        return NULL;
    }

    *outputLength = originalLength;

    // Cargar el árbol de Huffman reconstruido
    huffmanNode* root = loadTree(file);
    huffmanNode* currentNode = root;

    // Reservar memoria exacta para el buffer de salida
    unsigned char* decodedData = (unsigned char*)malloc(originalLength);
    long decodedBytes = 0;
    unsigned char currentByte;

    // Procesar los bits del archivo binario
    while (decodedBytes < originalLength && fread(&currentByte, sizeof(unsigned char), 1, file)) {

        for (int bitIndex = 7; bitIndex >= 0; bitIndex--) {

            int currentBit = (currentByte >> bitIndex) & 1;

            if (currentBit == 0) {
                currentNode = currentNode->leftChild;
            } else {
                currentNode = currentNode->rightChild;
            }

            // Si llegamos a una hoja, recuperamos el byte
            if (currentNode->leftChild == NULL && currentNode->rightChild == NULL) {

                decodedData[decodedBytes++] = (unsigned char)currentNode->character;

                if (decodedBytes == originalLength) {
                    break;
                }

                currentNode = root;
            }
        }
    }

    return decodedData;
}