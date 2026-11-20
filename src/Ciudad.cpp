#include "Ciudad.h"

#include <algorithm>
#include <random>
#include <stdexcept> // Permite informar dimensiones o coordenadas invalidas.

Ciudad::Ciudad(
    int filas,
    int columnas,
    int tiempoVerde,
    int tiempoAmarillo,
    int tiempoRojo)
    : filas_(filas), columnas_(columnas),
      intersecciones_(filas)
{
    // Evita crear una ciudad con dimensiones invalidas.
    if (filas <= 0 || columnas <= 0)
    {
        throw std::invalid_argument("La ciudad debe tener filas y columnas positivas");
    }

    std::mt19937 generador(static_cast<unsigned int>(
        std::random_device{}()));
    std::uniform_int_distribution<int> sentido(0, 1);

    sentidosHorizontal_.resize(filas_);
    for (int fila = 0; fila < filas_; ++fila)
    {
        sentidosHorizontal_[fila] = sentido(generador) == 0 ? -1 : 1;
    }

    sentidosVertical_.resize(columnas_);
    for (int columna = 0; columna < columnas_; ++columna)
    {
        sentidosVertical_[columna] = sentido(generador) == 0 ? -1 : 1;
    }

    semaforos_.assign(
        filas_,
        std::vector<bool>(columnas_, false));

    const int cantidadSemaforos =
        std::clamp(filas_ * columnas_ / 9, 10, 20);
    int semaforosCreados = 0;

    while (semaforosCreados < cantidadSemaforos)
    {
        const int fila =
            std::uniform_int_distribution<int>(0, filas_ - 1)(generador);
        const int columna =
            std::uniform_int_distribution<int>(0, columnas_ - 1)(generador);

        if (!semaforos_[fila][columna])
        {
            semaforos_[fila][columna] = true;
            ++semaforosCreados;
        }
    }

    for (int indiceFila = 0; indiceFila < filas_; ++indiceFila)
    {
        auto& fila = intersecciones_[indiceFila];
        fila.reserve(columnas_);
        for (int columna = 0; columna < columnas_; ++columna)
        {
            const bool tieneSemaforo =
                semaforos_[indiceFila][columna];
            fila.push_back(std::make_unique<Interseccion>(
                tiempoVerde,
                tiempoAmarillo,
                tiempoRojo,
                tieneSemaforo));
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

int Ciudad::obtenerSentidoHorizontal(int fila) const
{
    if (fila < 0 || fila >= filas_)
    {
        throw std::out_of_range("Fila fuera de la ciudad");
    }
    return sentidosHorizontal_[fila];
}

int Ciudad::obtenerSentidoVertical(int columna) const
{
    if (columna < 0 || columna >= columnas_)
    {
        throw std::out_of_range("Columna fuera de la ciudad");
    }
    return sentidosVertical_[columna];
}

bool Ciudad::interseccionTieneSemaforo(int fila, int columna) const
{
    if (fila < 0 || fila >= filas_ ||
        columna < 0 || columna >= columnas_)
    {
        return false;
    }
    return semaforos_[fila][columna];
}

