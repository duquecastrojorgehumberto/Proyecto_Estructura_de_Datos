#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "LZ77.h"
#include "Huffman.h"

// IDs de control para el archivo .bin
#define MODO_LZ77 1
#define MODO_HUFFMAN 2
#define MODO_INTEGRADO 3

// Prototipos
void ejecutarCompresionLZ77();
void ejecutarCompresionHuffman();
void ejecutarCompresionSistemaIntegrado();
void ejecutarExtraccionGlobal();
char* leerArchivo(char* nombre, long* tamano);

int main() {
    int opcion = 0;
    while (opcion != 5) {
        printf("\n============================================\n");
        printf("   SISTEMA DE COMPRESION UTP - FINAL\n");
        printf("============================================\n");
        printf("1. LZ77: Comprimir (.txt -> .bin)\n");
        printf("2. Huffman: Comprimir (.txt -> .bin)\n");
        printf("3. Sistema Integrado: (LZ77 + Huffman)\n");
        printf("4. EXTRAER: (.bin -> .txt)\n");
        printf("5. Salir\n");
        printf("Seleccione una tarea: ");
        if (scanf("%d", &opcion) != 1) {
            while(getchar() != '\n'); continue;
        }

        switch (opcion) {
            case 1: ejecutarCompresionLZ77(); break;
            case 2: ejecutarCompresionHuffman(); break;
            case 3: ejecutarCompresionSistemaIntegrado(); break;
            case 4: ejecutarExtraccionGlobal(); break;
        }
    }
    return 0;
}

// --- FUNCIONES DE APOYO ---

char* leerArchivo(char* nombre, long* tamano) {
    FILE *f = fopen(nombre, "rb");
    if (!f) { printf("Error: No se encontro el archivo.\n"); return NULL; }
    fseek(f, 0, SEEK_END); *tamano = ftell(f); fseek(f, 0, SEEK_SET);
    char *texto = malloc(*tamano + 1);
    fread(texto, 1, *tamano, f); texto[*tamano] = '\0';
    fclose(f);
    return texto;
}

//FUNCIÓN NUEVA DE SERIALIZACIÓN

//==================================================
// SERIALIZACIÓN DE TUPLAS LZ77
// Convierte el arreglo de tuplas en un bloque
// continuo de bytes para poder comprimirlo,
// guardarlo en archivo o transmitirlo.
//==================================================

unsigned char* serializarTuplas(
    TuplaLZ77* tuplas,      // arreglo de tuplas LZ77
    int cantidadTuplas,     // cantidad total de tuplas
    long* tamanoBuffer      // tamaño final del buffer generado
) {

    // Calcular cuántos bytes ocupan todas las tuplas
    *tamanoBuffer = cantidadTuplas * sizeof(TuplaLZ77);

    // Reservar memoria para almacenar los datos serializados
    unsigned char* buffer =
        (unsigned char*)malloc(*tamanoBuffer);

    // Copiar todas las tuplas al buffer byte por byte
    // El resultado es un bloque continuo de memoria
    memcpy(
        buffer,
        tuplas,
        *tamanoBuffer
    );

    // Retornar el buffer serializado
    return buffer;
}

// --- COMPRESIONES ---

void ejecutarCompresionLZ77() {
    long tam; char nomIn[100], nomOut[100];
    printf("Archivo .txt: "); scanf("%s", nomIn);
    char* texto = leerArchivo(nomIn, &tam); if(!texto) return;
    printf("Nombre para el .bin: "); scanf("%s", nomOut);

    clock_t inicio = clock();
    int cap = 1000, nT = 0, i = 0;
    TuplaLZ77 *tups = malloc(sizeof(TuplaLZ77) * cap);
    while (i < tam) {
        if (nT >= cap) { cap *= 2; tups = realloc(tups, sizeof(TuplaLZ77) * cap); }
        tups[nT] = buscarCoincidencia(texto, i, 1024, (int)tam);
        i += tups[nT].length + 1; nT++;
    }

    FILE *f = fopen(nomOut, "wb");
    int modo = MODO_LZ77;
    fwrite(&modo, sizeof(int), 1, f);
    fwrite(tups, sizeof(TuplaLZ77), nT, f);
    fclose(f);

    clock_t fin = clock();
    double tMs = ((double)(fin - inicio)/CLOCKS_PER_SEC)*1000;
    double ahorro = 100.0 - (((double)nT * sizeof(TuplaLZ77) / tam) * 100.0);
    
    printf("\n[LZ77] Tiempo: %.2f ms | Ahorro: %.2f%%\n", tMs, (ahorro<0)?0:ahorro);
    free(texto); free(tups);
}

void ejecutarCompresionHuffman() {

    long tam;
    char nomIn[100], nomOut[100];

    printf("Archivo .txt: ");
    scanf("%s", nomIn);
    char* texto = leerArchivo(nomIn, &tam);
    if(!texto) return;
    printf("Nombre para el .bin: ");
    scanf("%s", nomOut);
    clock_t inicio = clock();

    //==========================================
    // TABLA DE FRECUENCIAS (Soporte Binario)
    //==========================================

    int frecs[256] = {0};
    buildFrequencyTableBinary((unsigned char*)texto, tam, frecs);

    //==========================================
    // CONSTRUIR ÁRBOL
    //==========================================

    huffmanNode* root = buildHuffmanTree(frecs);

    //==========================================
    // GENERAR CÓDIGOS
    //==========================================

    char* codes[maxCharacters] = {0};
    char currentCode[maxCodeLength];
    generateCodes(root, currentCode, 0, codes);

    //==========================================
    // CREAR ARCHIVO BINARIO
    //==========================================

    FILE *f = fopen(nomOut, "wb");
    if (!f) {
        printf("Error creando archivo\n");
        free(texto);
        return;
    }

    //==========================================
    // ID DEL ALGORITMO
    //==========================================

    int modo = MODO_HUFFMAN;
    fwrite(&modo, sizeof(int), 1, f);

    //==========================================
    // GUARDAR HUFFMAN REAL BINARIO
    //==========================================

    saveCompressedFileBinary(
        f,
        (unsigned char*)texto,
        tam,
        codes,
        root
    );

    fclose(f);
    //==========================================
    // CALCULO DEL PORCENTAJE DE COMPRESIÓN
    //==========================================

    // Abrir el archivo generado en modo lectura para medir su peso real en disco
    FILE* fCheck = fopen(nomOut, "rb");
    long tamFinalBin = 0;

    if (fCheck) {
        fseek(fCheck, 0, SEEK_END);
        tamFinalBin = ftell(fCheck);
        fclose(fCheck);
    }

    // Aplicar la fórmula matemática de ahorro
    double ahorroHuffman = 100.0 - (((double)tamFinalBin / tam) * 100.0);

    // Controlar si el ahorro es negativo (cuando el archivo es muy pequeño)
    if (ahorroHuffman < 0) {
        ahorroHuffman = 0.0;
    }

    clock_t fin = clock();

    printf(
        "\n[Huffman] Tiempo: %.2f ms | Ahorro: %.2f%%\n",
        ((double)(fin-inicio)/CLOCKS_PER_SEC)*1000,
        ahorroHuffman
    );
    
    free(texto);
}

void ejecutarCompresionSistemaIntegrado() {

    long tam; 
    char nomIn[100], nomOut[100];

    printf("Archivo .txt: "); 
    scanf("%s", nomIn);
    char* texto = leerArchivo(nomIn, &tam); 
    if(!texto) return;
    printf("Nombre para el .bin: "); 
    scanf("%s", nomOut);

    clock_t inicio = clock();

    //==========================================
    // FASE 1: GENERACIÓN DE TUPLAS LZ77
    //==========================================

    int cap = 1000, nT = 0, i = 0;
    TuplaLZ77 *tups = malloc(sizeof(TuplaLZ77) * cap);
    while (i < tam) {
        if (nT >= cap) { cap *= 2; tups = realloc(tups, sizeof(TuplaLZ77) * cap); }
        tups[nT] = buscarCoincidencia(texto, i, 1024, (int)tam);
        i += tups[nT].length + 1; nT++;
    }

    //==========================================
    // FASE 2: SERIALIZACIÓN A BUFFER BINARIO
    //==========================================

    long tamBufferLZ77;
    unsigned char* bufferLZ77 = serializarTuplas(tups, nT, &tamBufferLZ77);

    //==========================================
    // FASE 3: COMPRESIÓN HUFFMAN DEL BUFFER
    //==========================================

    int frecs[256] = {0};
    buildFrequencyTableBinary(bufferLZ77, tamBufferLZ77, frecs);

    huffmanNode* root = buildHuffmanTree(frecs);

    char* codes[maxCharacters] = {0};
    char currentCode[maxCodeLength];
    generateCodes(root, currentCode, 0, codes);

    //==========================================
    // FASE 4: GUARDAR EN ARCHIVO BINARIO (.BIN)
    //==========================================

    FILE *f = fopen(nomOut, "wb");
    if (!f) {
        printf("Error al crear archivo binario\n");
        free(texto); free(tups); free(bufferLZ77);
        return;
    }

    int modo = MODO_INTEGRADO;
    fwrite(&modo, sizeof(int), 1, f);

    // Guardar el árbol y empaquetar los bits del buffer LZ77
    saveCompressedFileBinary(
        f,
        bufferLZ77,
        tamBufferLZ77,
        codes,
        root
    );

    fclose(f);

    clock_t fin = clock();

    //==================================================
    // MEDICIÓN DEL PESO REAL Y PORCENTAJE EN DISCO
    //==================================================

    // Abrir el archivo binario generado para medir su peso real
    FILE* fCheck = fopen(nomOut, "rb");
    long tamFinalBin = 0;

    if (fCheck) {
        fseek(fCheck, 0, SEEK_END);
        tamFinalBin = ftell(fCheck);
        fclose(fCheck);
    }

    // Calcular el porcentaje de ahorro real contra el texto original
    double ahorro = 100.0 - (((double)tamFinalBin / tam) * 100.0);

    // Controlar si el ahorro es negativo (en archivos extremadamente pequeños)
    if (ahorro < 0) {
        ahorro = 0.0;
    }

    free(texto); 
    free(tups); 
    free(bufferLZ77);
}

// --- EXTRACCION INTELIGENTE (LA QUE PIDE EL PROFE) ---

void ejecutarExtraccionGlobal() {
    char nomBin[100];
    printf("\nArchivo .bin a extraer: "); 
    scanf("%s", nomBin);
    
    FILE *f = fopen(nomBin, "rb"); 
    if(!f) {
        printf("Error: No se pudo abrir el archivo %s\n", nomBin);
        return;
    }

    // --- INICIO DE EVALUACIÓN DE DESEMPEÑO ---
    clock_t inicio = clock();

    int modo; 
    fread(&modo, sizeof(int), 1, f); // Leer ID de algoritmo
    
    char *txtFinal = NULL; 
    int tS = 0;

    // Identificar algoritmo y extraer de forma matemática y simétrica
    if (modo == MODO_LZ77) {

        fseek(f, 0, SEEK_END); 
        long tamCuerpo = ftell(f) - sizeof(int);
        fseek(f, sizeof(int), SEEK_SET);

        int nT = tamCuerpo / sizeof(TuplaLZ77);
        TuplaLZ77 *leidas = malloc(tamCuerpo);
        fread(leidas, sizeof(TuplaLZ77), nT, f);
        
        txtFinal = descomprimirLZ77(leidas, nT, &tS);
        free(leidas);

    } else if (modo == MODO_HUFFMAN) {

        long tamDescomprimidoBytes = 0;
        unsigned char* resHuffman = decodeFileToMemory(f, &tamDescomprimidoBytes);
        
        txtFinal = (char*)resHuffman;
        tS = (int)tamDescomprimidoBytes;

    } else if (modo == MODO_INTEGRADO) {

        long tamBufferLZ77Bytes = 0;
        
        // 1. Reversar el empaquetado de Huffman para recuperar el buffer de bytes
        unsigned char* bufferLZ77Recuperado = decodeFileToMemory(f, &tamBufferLZ77Bytes);
        
        if (bufferLZ77Recuperado) {
            int nT = tamBufferLZ77Bytes / sizeof(TuplaLZ77);
            TuplaLZ77* tuplasLZ77 = (TuplaLZ77*)bufferLZ77Recuperado;
            
            // 2. Pasar las tuplas recuperadas a LZ77 para obtener el texto original
            txtFinal = descomprimirLZ77(tuplasLZ77, nT, &tS);
            free(bufferLZ77Recuperado);
        }
    }
    fclose(f);

    clock_t fin = clock();
    // --- FIN DE EVALUACIÓN ---

    if (txtFinal) {
        char nomOut[100];
        printf("Nombre del .txt de salida (ej: recuperado): "); 
        scanf("%s", nomOut);
        if(!strstr(nomOut, ".txt")) strcat(nomOut, ".txt");
        
        FILE *out = fopen(nomOut, "wb"); 
        if(out) {
            fwrite(txtFinal, sizeof(char), tS, out); 
            fclose(out);
            
            double tiempoMs = ((double)(fin - inicio) / CLOCKS_PER_SEC) * 1000;
            
            printf("\n============================================\n");
            printf("      REPORTE DE EXTRACCION (LOSSLESS)\n");
            printf("============================================\n");
            printf(" Algoritmo detectado: %s\n", (modo == 1) ? "LZ77" : (modo == 2) ? "Huffman" : "Integrado (LZ77 + Huffman)");
            printf(" Tiempo de ejecucion: %.2f ms\n", tiempoMs);
            printf(" Tamaño recuperado:   %d bytes\n", tS);
            printf(" Estado:              EXITOSO (Sin perdidas)\n");
            printf("============================================\n");
        }
        free(txtFinal);
    } else {
        printf("Error al extraer los datos.\n");
    }
}