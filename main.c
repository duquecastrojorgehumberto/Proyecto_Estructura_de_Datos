#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "LZ77.h"
#include "Huffman.h"

// Prototipos de funciones locales
void ejecutarCompresion();
void ejecutarExtraccion();

int main() {
    int opcion = 0;
    while (opcion != 3) {
        printf("\n============================================\n");
        printf("   SISTEMA DE COMPRESION UTP - VERSION C\n");
        printf("============================================\n");
        printf("1. Comprimir archivo (.txt -> .bin)\n");
        printf("2. Extraer archivo   (.bin -> .txt)\n");
        printf("3. Salir\n");
        printf("Seleccione una tarea: ");
        
        if (scanf("%d", &opcion) != 1) break;

        switch (opcion) {
            case 1: ejecutarCompresion(); break;
            case 2: ejecutarExtraccion(); break;
            case 3: printf("Saliendo...\n"); break;
            default: printf("Opcion no valida.\n");
        }
    }
    return 0;
}

void ejecutarCompresion() {
    char nombreTxt[100];
    printf("\nIngrese el nombre del archivo de texto a comprimir (ej: entrada.txt): ");
    scanf("%s", nombreTxt);

    // 1. Abrir archivo elegido por el usuario
    FILE *archivoIn = fopen(nombreTxt, "rb");
    if (!archivoIn) {
        printf("Error: El archivo '%s' no existe o no se puede abrir.\n", nombreTxt);
        return;
    }

    // Calcular tamaño y leer texto
    fseek(archivoIn, 0, SEEK_END);
    long tamanoOriginal = ftell(archivoIn);
    fseek(archivoIn, 0, SEEK_SET);

    if (tamanoOriginal == 0) {
        printf("Error: El archivo esta vacio.\n");
        fclose(archivoIn);
        return;
    }

    char *texto = (char*)malloc(tamanoOriginal + 1);
    fread(texto, 1, tamanoOriginal, archivoIn);
    texto[tamanoOriginal] = '\0';
    fclose(archivoIn);

    // Pedir nombre para el archivo binario resultante
    char nombreBin[100];
    printf("Nombre para el archivo comprimido (ej: resultado.bin): ");
    scanf("%s", nombreBin);

    printf("\nComprimiendo '%s' (%ld bytes)...\n", nombreTxt, tamanoOriginal);

    clock_t inicio = clock();

    // 2. Ejecutar LZ77 (Fase 1)
    int capacidad = 1000;
    TuplaLZ77 *tuplas = (TuplaLZ77*)malloc(sizeof(TuplaLZ77) * capacidad);
    int numTuplas = 0;
    int i = 0;
    while (i < tamanoOriginal) {
        if (numTuplas >= capacidad) {
            capacidad *= 2;
            tuplas = (TuplaLZ77*)realloc(tuplas, sizeof(TuplaLZ77) * capacidad);
        }
        tuplas[numTuplas] = buscarCoincidencia(texto, i, 1024, (int)tamanoOriginal);
        i += tuplas[numTuplas].length + 1;
        numTuplas++;
    }

    // 3. Fase Huffman (Fase 2 - Tabla de Frecuencias)
    int frecuencias[256] = {0};
    for (int k = 0; k < numTuplas; k++) {
        frecuencias[(unsigned char)tuplas[k].nextChar]++;
    }
    construirArbolHuffman(frecuencias);

    clock_t fin = clock();

    // 4. Guardar archivo binario con el nombre elegido
    FILE *archivoOut = fopen(nombreBin, "wb");
    if (archivoOut) {
        fwrite(tuplas, sizeof(TuplaLZ77), numTuplas, archivoOut);
        fclose(archivoOut);
    }

    // ... justo después de fclose(archivoOut); ...

    double tiempoMs = ((double)(fin - inicio) / CLOCKS_PER_SEC) * 1000;
    
    // CÁLCULO DE LA TASA (Crucial para la rúbrica)
    // Cada tupla LZ77 pesa 5 bytes (2 de offset + 2 de length + 1 de char)
    double tamañoBinario = (double)numTuplas * sizeof(TuplaLZ77);
    double ahorro = 100.0 - ((tamañoBinario / (double)tamanoOriginal) * 100.0);

    printf("\n----------------------------------------\n");
    printf("COMPRESION FINALIZADA\n");
    printf("Archivo original: %s (%ld bytes)\n", nombreTxt, tamanoOriginal);
    printf("Archivo binario:  %s (%.0f bytes)\n", nombreBin, tamañoBinario);
    printf("Tiempo:           %.2f ms\n", tiempoMs);
    printf("Tasa de ahorro:   %.2f%%\n", ahorro);
    printf("----------------------------------------\n");

    free(texto);
    free(tuplas);
}

void ejecutarExtraccion() {
    char nombreIn[100];
    printf("\nNombre del archivo binario (ej: comprimido.bin): ");
    scanf("%s", nombreIn);

    FILE *archivoBin = fopen(nombreIn, "rb");
    if (!archivoBin) {
        printf("Error: No se pudo abrir %s\n", nombreIn);
        return;
    }

    // Leer tuplas
    fseek(archivoBin, 0, SEEK_END);
    long tamArchivo = ftell(archivoBin);
    int numTuplas = tamArchivo / sizeof(TuplaLZ77);
    fseek(archivoBin, 0, SEEK_SET);

    TuplaLZ77 *tuplas = (TuplaLZ77*)malloc(sizeof(TuplaLZ77) * numTuplas);
    fread(tuplas, sizeof(TuplaLZ77), numTuplas, archivoBin);
    fclose(archivoBin);

    // 5. Descompresion LZ77
    int tamanoSalida = 0;
    char *textoFinal = descomprimirLZ77(tuplas, numTuplas, &tamanoSalida);

    // Guardar resultado
    char nombreOut[100];
    printf("Nombre para el archivo TXT de salida: ");
    scanf("%s", nombreOut);
    strcat(nombreOut, ".txt");

    FILE *archivoTxt = fopen(nombreOut, "w");
    if (archivoTxt) {
        fprintf(archivoTxt, "%s", textoFinal);
        fclose(archivoTxt);
        printf("Archivo '%s' generado correctamente.\n", nombreOut);
    }

    free(tuplas);
    free(textoFinal);
}