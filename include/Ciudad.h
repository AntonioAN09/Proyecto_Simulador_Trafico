#pragma once

#include <memory> // Permite administrar intersecciones sin copiar objetos con mutex.
#include <vector>

#include "Interseccion.h"

class Ciudad
{
public:
    // Crea una ciudad con el numero indicado de filas y columnas.
    Ciudad(
        int filas,
        int columnas,
        int tiempoVerde = 2,
        int tiempoAmarillo = 1,
        int tiempoRojo = 3);
    // Devuelve la interseccion ubicada en una coordenada.
    Interseccion& obtenerInterseccion(int fila, int columna);
    int obtenerFilas() const;
    int obtenerColumnas() const;
    int obtenerSentidoHorizontal(int fila) const;
    int obtenerSentidoVertical(int columna) const;
    bool interseccionTieneSemaforo(int fila, int columna) const;

private:
    // Guarda las dimensiones de la cuadricula.
    int filas_;
    int columnas_;
    // Cada posicion contiene una interseccion independiente.
    std::vector<std::vector<std::unique_ptr<Interseccion>>> intersecciones_;
    std::vector<int> sentidosHorizontal_;
    std::vector<int> sentidosVertical_;
    std::vector<std::vector<bool>> semaforos_;
};

