#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <fstream>
#include <sstream>
#include "LZ77.h"
#include "Huffman.h"

// --- PROTOTIPOS DE FUNCIONES ---
// Esto le dice al programa que estas funciones existen abajo
void mostrarMenu();
void ejecutarCompresion();
void ejecutarExtraccion();
std::vector<TuplaLZ77> leerArchivoBinario(std::string nombreArchivo);

int main() {
    int opcion = 0;
    while (opcion != 3) {
        mostrarMenu();
        if (!(std::cin >> opcion)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }

        switch (opcion) {
            case 1:
                ejecutarCompresion();
                break;
            case 2:
                ejecutarExtraccion();
                break;
            case 3:
                std::cout << "Saliendo del programa..." << std::endl;
                break;
            default:
                std::cout << "Opcion no valida. Intente de nuevo." << std::endl;
        }
    }
    return 0;
}

// --- IMPLEMENTACIÓN DE LAS FUNCIONES ---

void mostrarMenu() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   SISTEMA DE COMPRESION UTP (V1.0)     " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Comprimir archivo (.txt -> .bin)" << std::endl;
    std::cout << "2. Extraer archivo   (.bin -> .txt)" << std::endl;
    std::cout << "3. Salir" << std::endl;
    std::cout << "Seleccione una tarea: ";
}

void ejecutarCompresion() {
    std::ifstream archivoEntrada("entrada.txt");
    std::string textoOriginal;
    
    if (archivoEntrada.is_open()) {
        std::stringstream buffer;
        buffer << archivoEntrada.rdbuf();
        textoOriginal = buffer.str();
        archivoEntrada.close();
    } else {
        std::cerr << "Error: No se encontro 'entrada.txt' en la carpeta." << std::endl;
        return;
    }

    std::cout << "\nComprimiendo..." << std::endl;
    auto inicio = std::chrono::high_resolution_clock::now();

    // Fase LZ77
    std::vector<TuplaLZ77> comprimido;
    int i = 0;
    while (i < (int)textoOriginal.length()) {
        TuplaLZ77 tupla = buscarCoincidencia(textoOriginal, i, 1024);
        comprimido.push_back(tupla);
        i += tupla.length + 1;
    }

    // Fase Huffman
    std::map<char, int> frecuencias;
    for (const auto& t : comprimido) {
        if (t.nextChar != '\0') frecuencias[t.nextChar]++;
    }
    construirArbolHuffman(frecuencias); // Genera el árbol internamente

    auto fin = std::chrono::high_resolution_clock::now();

    // Guardar Binario
    std::ofstream archivoBin("comprimido.bin", std::ios::binary);
    for (const auto& t : comprimido) {
        archivoBin.write(reinterpret_cast<const char*>(&t.offset), sizeof(t.offset));
        archivoBin.write(reinterpret_cast<const char*>(&t.length), sizeof(t.length));
        archivoBin.write(&t.nextChar, sizeof(t.nextChar));
    }
    archivoBin.close();

    // Reporte de Desempeño (Rúbrica: Tasa y Tiempo)
    std::chrono::duration<double, std::milli> tiempo = fin - inicio;
    std::ifstream f1("entrada.txt", std::ios::ate | std::ios::binary);
    std::ifstream f2("comprimido.bin", std::ios::ate | std::ios::binary);
    long long p1 = f1.tellg();
    long long p2 = f2.tellg();

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "¡EXITO! Archivo 'comprimido.bin' creado." << std::endl;
    std::cout << "Tiempo: " << tiempo.count() << " ms" << std::endl;
    std::cout << "Ahorro: " << (1.0 - (double)p2/p1) * 100 << "%" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

void ejecutarExtraccion() {
    // 1. Pedir al usuario el nombre del archivo binario (Entrada)
    std::string nombreEntrada;
    std::cout << "\nIngrese el nombre del archivo binario a extraer (ej: comprimido.bin): ";
    std::cin >> nombreEntrada;

    std::cout << "Leyendo '" << nombreEntrada << "'..." << std::endl;
    
    // 2. Cargamos las tuplas usando el nombre que el usuario escribió
    std::vector<TuplaLZ77> tuplasLeidas = leerArchivoBinario(nombreEntrada);
    
    // Verificamos si el archivo existe o tiene datos
    if (tuplasLeidas.empty()) {
        std::cout << "Error: El archivo '" << nombreEntrada << "' no existe o esta vacio." << std::endl;
        return;
    }

    // 3. Proceso de descompresión LZ77
    std::string textoRecuperado = descomprimirLZ77(tuplasLeidas);

    // 4. Pedir nombre para el archivo de texto (Salida)
    std::string nombreSalida;
    std::cout << "Nombre para el archivo de salida (sin extension): ";
    std::cin >> nombreSalida;
    nombreSalida += ".txt";

    // 5. Guardar el resultado en el disco
    std::ofstream archivoSalida(nombreSalida);
    if (archivoSalida.is_open()) {
        archivoSalida << textoRecuperado;
        archivoSalida.close();
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "EXTRACCION EXITOSA" << std::endl;
        std::cout << "Archivo '" << nombreSalida << "' generado correctamente." << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    } else {
        std::cout << "Error al crear el archivo de texto." << std::endl;
    }
}

std::vector<TuplaLZ77> leerArchivoBinario(std::string nombreArchivo) {
    std::vector<TuplaLZ77> tuplas;
    std::ifstream archivo(nombreArchivo, std::ios::binary);
    if (archivo.is_open()) {
        TuplaLZ77 t;
        while (archivo.read(reinterpret_cast<char*>(&t.offset), sizeof(t.offset))) {
            archivo.read(reinterpret_cast<char*>(&t.length), sizeof(t.length));
            archivo.read(&t.nextChar, sizeof(t.nextChar));
            tuplas.push_back(t);
        }
        archivo.close();
    }
    return tuplas;
}