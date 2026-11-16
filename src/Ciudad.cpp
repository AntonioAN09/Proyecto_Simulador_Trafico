#include "Ciudad.h"

#include <stdexcept> // Permite informar dimensiones o coordenadas invalidas.

Ciudad::Ciudad(int filas, int columnas)
    : filas_(filas), columnas_(columnas),
      intersecciones_(filas)
{
    // Evita crear una ciudad con dimensiones invalidas.
    if (filas <= 0 || columnas <= 0)
    {
        throw std::invalid_argument("La ciudad debe tener filas y columnas positivas");
    }

    for (auto& fila : intersecciones_)
    {
        fila.reserve(columnas_);
        for (int columna = 0; columna < columnas_; ++columna)
        {
            // Cada interseccion tiene su propio mutex y semaforo.
            fila.push_back(std::make_unique<Interseccion>());
        }
    }
}

Interseccion& Ciudad::obtenerInterseccion(int fila, int columna)
{
    if (fila < 0 || fila >= filas_ || columna < 0 || columna >= columnas_)
    {
        // Impide acceder a una posicion que no existe.
        throw std::out_of_range("Coordenada fuera de la ciudad");
    }

    return *intersecciones_[fila][columna];
}

int Ciudad::obtenerFilas() const
{
    return filas_;
}

int Ciudad::obtenerColumnas() const
{
    return columnas_;
}

