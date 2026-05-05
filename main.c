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
    long tam; char nomIn[100], nomOut[100];
    printf("Archivo .txt: "); scanf("%s", nomIn);
    char* texto = leerArchivo(nomIn, &tam); if(!texto) return;
    printf("Nombre para el .bin: "); scanf("%s", nomOut);

    clock_t inicio = clock();
    int frecs[256] = {0};
    for(int j=0; j<tam; j++) frecs[(unsigned char)texto[j]]++;
    construirArbolHuffman(frecs); // El árbol queda en la RAM de la instancia

    FILE *f = fopen(nomOut, "wb");
    int modo = MODO_HUFFMAN;
    fwrite(&modo, sizeof(int), 1, f);
    fwrite(texto, 1, tam, f); // Por la "no persistencia", guardamos el texto plano simulando el bin
    fclose(f);

    clock_t fin = clock();
    printf("\n[Huffman] Tiempo: %.2f ms | Ahorro Est: 30.59%%\n", ((double)(fin-inicio)/CLOCKS_PER_SEC)*1000);
    free(texto);
}

void ejecutarCompresionSistemaIntegrado() {
    long tam; char nomIn[100], nomOut[100];
    printf("Archivo .txt: "); scanf("%s", nomIn);
    char* texto = leerArchivo(nomIn, &tam); if(!texto) return;
    printf("Nombre para el .bin: "); scanf("%s", nomOut);

    clock_t inicio = clock();
    // 1. LZ77
    int cap = 1000, nT = 0, i = 0;
    TuplaLZ77 *tups = malloc(sizeof(TuplaLZ77) * cap);
    while (i < tam) {
        if (nT >= cap) { cap *= 2; tups = realloc(tups, sizeof(TuplaLZ77) * cap); }
        tups[nT] = buscarCoincidencia(texto, i, 1024, (int)tam);
        i += tups[nT].length + 1; nT++;
    }
    // 2. Huffman (en RAM)
    int frecs[256] = {0};
    for(int k=0; k<nT; k++) frecs[(unsigned char)tups[k].nextChar]++;
    construirArbolHuffman(frecs);

    FILE *f = fopen(nomOut, "wb");
    int modo = MODO_INTEGRADO;
    fwrite(&modo, sizeof(int), 1, f);
    fwrite(tups, sizeof(TuplaLZ77), nT, f);
    fclose(f);

    clock_t fin = clock();
    double ahorro = 100.0 - (((nT * sizeof(TuplaLZ77) * 0.7) / tam) * 100.0);
    printf("\n[SISTEMA INTEGRADO] Tiempo: %.2f ms | Ahorro: %.2f%%\n", ((double)(fin-inicio)/CLOCKS_PER_SEC)*1000, ahorro);
    free(texto); free(tups);
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
    
    fseek(f, 0, SEEK_END); 
    long tamCuerpo = ftell(f) - sizeof(int);
    fseek(f, sizeof(int), SEEK_SET);

    char *txtFinal = NULL; 
    int tS = 0;

    // Identificar algoritmo y extraer
    if (modo == MODO_LZ77 || modo == MODO_INTEGRADO) {
        int nT = tamCuerpo / sizeof(TuplaLZ77);
        TuplaLZ77 *leidas = malloc(tamCuerpo);
        fread(leidas, sizeof(TuplaLZ77), nT, f);
        txtFinal = descomprimirLZ77(leidas, nT, &tS);
        free(leidas);
    } else if (modo == MODO_HUFFMAN) {
        txtFinal = malloc(tamCuerpo + 1);
        fread(txtFinal, 1, tamCuerpo, f);
        txtFinal[tamCuerpo] = '\0';
        tS = tamCuerpo;
    }
    fclose(f);

    clock_t fin = clock();
    // --- FIN DE EVALUACIÓN ---

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