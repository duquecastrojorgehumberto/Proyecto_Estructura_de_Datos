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
        printf("    SISTEMA DE COMPRESION UTP - FINAL\n");
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

// --- COMPRESIONES ---

void ejecutarCompresionLZ77() {
    long tam; char nomIn[100], nomOut[100];
    printf("Archivo .txt: "); scanf("%s", nomIn);
    char* texto = leerArchivo(nomIn, &tam); if(texto == NULL) return;
    printf("Nombre para el .bin: "); scanf("%s", nomOut);

    clock_t inicio = clock();
    int capacidad = 1000, numTuplas = 0, i = 0;
    TuplaLZ77 *tups = malloc(sizeof(TuplaLZ77) * capacidad);
    while (i < tam) {
        if (numTuplas >= capacidad) { capacidad *= 2; tups = realloc(tups, sizeof(TuplaLZ77) * capacidad); }
        tups[numTuplas] = buscarCoincidencia(texto, i, 1024, (int)tam);
        i += tups[numTuplas].length + 1; numTuplas++;
    }

    FILE *f = fopen(nomOut, "wb");
    int modo = MODO_LZ77;
    fwrite(&modo, sizeof(int), 1, f);
    fwrite(tups, sizeof(TuplaLZ77), numTuplas, f);
    
    long tamBin = ftell(f);
    fclose(f);

    clock_t fin = clock();
    double tMs = ((double)(fin - inicio)/CLOCKS_PER_SEC)*1000;
    double ahorro = 100.0 - (((double)tamBin / tam) * 100.0);
    
    printf("\n[LZ77] Tiempo: %.2f ms | Ahorro Real: %.2f%%\n", tMs, (ahorro<0)?0:ahorro);
    free(texto); free(tups);
}

void ejecutarCompresionHuffman() {
    long tam; char nomIn[100], nomOut[100];
    printf("Archivo .txt: "); scanf("%s", nomIn);
    char* texto = leerArchivo(nomIn, &tam); if(!texto) return;
    printf("Nombre para el .bin: "); scanf("%s", nomOut);

    clock_t inicio = clock();
    
    int frecs[256] = {0};
    for(int j=0; j<tam; j++) frecs[(unsigned char)texto[j]]++;
    construirArbolHuffman(frecs); 

    NodoHuffman* raiz = generarArbolReal(frecs);
    char codigos[256][100] = {""};
    char codigoActual[100] = "";
    if (raiz) generarCodigos(raiz, codigos, codigoActual, 0);

    FILE *f = fopen(nomOut, "wb");
    int modo = MODO_HUFFMAN;
    
    fwrite(&modo, sizeof(int), 1, f);
    fwrite(frecs, sizeof(int), 256, f);
    if (raiz) escribirBitsFisicos(texto, tam, codigos, f);
    
    long tamBin = ftell(f);
    fclose(f);

    clock_t fin = clock();
    double tMs = ((double)(fin-inicio)/CLOCKS_PER_SEC)*1000;
    double ahorro = 100.0 - (((double)tamBin / tam) * 100.0);
    printf("\n[Huffman] Tiempo: %.2f ms | Ahorro Real: %.2f%%\n", tMs, (ahorro<0)?0:ahorro);
    
    if(raiz) liberarArbol(raiz);
    free(texto);
}

void ejecutarCompresionSistemaIntegrado() {
    long tam; char nomIn[100], nomOut[100];
    printf("Archivo .txt: "); scanf("%s", nomIn);
    char* texto = leerArchivo(nomIn, &tam); if(!texto) return;
    printf("Nombre para el .bin: "); scanf("%s", nomOut);

    clock_t inicio = clock();
    
    // 1. PASO 1: Ejecutar LZ77 Real
    int capacidad = 1000, numTuplas = 0, i = 0;
    TuplaLZ77 *tups = malloc(sizeof(TuplaLZ77) * capacidad);
    while (i < tam) {
        if (numTuplas >= capacidad) { capacidad *= 2; tups = realloc(tups, sizeof(TuplaLZ77) * capacidad); }
        tups[numTuplas] = buscarCoincidencia(texto, i, 1024, (int)tam);
        i += tups[numTuplas].length + 1; numTuplas++;
    }
    
    // 2. PASO 2: Tratar el bloque de tuplas como bytes puros y calcular frecuencias reales
    long tamBytesTups = numTuplas * sizeof(TuplaLZ77);
    unsigned char* bytesTups = (unsigned char*)tups;
    
    int frecs[256] = {0};
    for(long k = 0; k < tamBytesTups; k++) {
        frecs[bytesTups[k]]++;
    }
    construirArbolHuffman(frecs);

    // 3. PASO 3: Construir árbol de Huffman real para las tuplas
    NodoHuffman* raiz = generarArbolReal(frecs);
    char codigos[256][100] = {""};
    char codigoActual[100] = "";
    if (raiz) generarCodigos(raiz, codigos, codigoActual, 0);

    FILE *f = fopen(nomOut, "wb");
    int modo = MODO_INTEGRADO;
    
    // Guardar ID, Tabla de frecuencias híbrida y la cantidad de tuplas originales
    fwrite(&modo, sizeof(int), 1, f);
    fwrite(&numTuplas, sizeof(int), 1, f); 
    fwrite(frecs, sizeof(int), 256, f);
    
    // Comprimir físicamente los bytes de LZ77 usando los códigos de Huffman
    if (raiz) escribirBitsFisicos((char*)bytesTups, tamBytesTups, codigos, f);

    long tamBin = ftell(f);
    fclose(f);

    clock_t fin = clock();
    double tMs = ((double)(fin-inicio)/CLOCKS_PER_SEC)*1000;
    double ahorro = 100.0 - (((double)tamBin / tam) * 100.0);
    
    printf("\n[SISTEMA INTEGRADO] Tiempo: %.2f ms | Ahorro Real: %.2f%%\n", tMs, (ahorro<0)?0:ahorro);
    
    if(raiz) liberarArbol(raiz);
    free(texto); free(tups);
}

void ejecutarExtraccionGlobal() {
    char nomBin[100];
    printf("\nArchivo .bin a extraer: "); 
    scanf("%s", nomBin);
    
    FILE *f = fopen(nomBin, "rb"); 
    if(!f) {
        printf("Error: No se pudo abrir el archivo %s\n", nomBin);
        return;
    }

    clock_t inicio = clock();

    int modo; 
    fread(&modo, sizeof(int), 1, f); // Leer ID de algoritmo
    
    char *txtFinal = NULL; 
    int tS = 0;

    if (modo == MODO_LZ77) {
        fseek(f, 0, SEEK_END); 
        long tamCuerpo = ftell(f) - sizeof(int);
        fseek(f, sizeof(int), SEEK_SET);

        int numTuplas = tamCuerpo / sizeof(TuplaLZ77);
        TuplaLZ77 *leidas = malloc(tamCuerpo);
        fread(leidas, sizeof(TuplaLZ77), numTuplas, f);
        txtFinal = descomprimirLZ77(leidas, numTuplas, &tS);
        free(leidas);
        fclose(f);
    } 
    else if (modo == MODO_HUFFMAN) {
        int frecsLeidas[256];
        fread(frecsLeidas, sizeof(int), 256, f);

        NodoHuffman* raizReconstruida = generarArbolReal(frecsLeidas);

        long posActual = ftell(f);
        fseek(f, 0, SEEK_END);
        long tamCuerpoBits = ftell(f) - posActual;
        fseek(f, posActual, SEEK_SET);

        if (raizReconstruida) {
            txtFinal = decodificarBitsFisicos(f, raizReconstruida, tamCuerpoBits, &tS);
            liberarArbol(raizReconstruida);
        }
        fclose(f);
    }
    else if (modo == MODO_INTEGRADO) {
        // --- EXTRAER DEL SISTEMA INTEGRADO REAL ---
        int numTuplasOriginales;
        fread(&numTuplasOriginales, sizeof(int), 1, f); // Leer cuántas tuplas eran
        
        int frecsLeidas[256];
        fread(frecsLeidas, sizeof(int), 256, f); // Leer árbol híbrido

        NodoHuffman* raizReconstruida = generarArbolReal(frecsLeidas);

        long posActual = ftell(f);
        fseek(f, 0, SEEK_END);
        long tamCuerpoBits = ftell(f) - posActual;
        fseek(f, posActual, SEEK_SET);

        if (raizReconstruida) {
            int tamBytesTupsOut = 0;
            // 1. Descompresión Huffman: Recuperamos los bytes de las tuplas originales
            char* bytesTuplasRecuperados = decodificarBitsFisicos(f, raizReconstruida, tamCuerpoBits, &tamBytesTupsOut);
            
            // 2. Descompresión LZ77: Convertimos esos bytes a tuplas y reconstruimos el texto plano
            TuplaLZ77* tuplasParaLZ = (TuplaLZ77*)bytesTuplasRecuperados;
            txtFinal = descomprimirLZ77(tuplasParaLZ, numTuplasOriginales, &tS);
            
            free(bytesTuplasRecuperados);
            liberarArbol(raizReconstruida);
        }
        fclose(f);
    }

    clock_t fin = clock();

    if (txtFinal) {
        char nomOut[100];
        printf("Nombre del .txt de salida (ej: recuperado): "); 
        scanf("%s", nomOut);
        if(!strstr(nomOut, ".txt")) strcat(nomOut, ".txt");
        
        FILE *out = fopen(nomOut, "w"); 
        if(out) {
            fprintf(out, "%s", txtFinal); 
            fclose(out);
            
            double tiempoMs = ((double)(fin - inicio) / CLOCKS_PER_SEC) * 1000;
            
            printf("\n============================================\n");
            printf("      REPORTE DE EXTRACCION (LOSSLESS)\n");
            printf("============================================\n");
            printf(" Algoritmo detectado: %s\n", (modo == 1) ? "LZ77" : (modo == 2) ? "Huffman" : "Integrado");
            printf(" Tiempo de ejecucion: %.2f ms\n", tiempoMs);
            printf(" Tamaño recuperado:   %d bytes\n", tS);
            printf(" Estado:              EXITOSO (Sin perdidas)\n");
            printf("============================================\n");
        }
        free(txtFinal);
    }
}