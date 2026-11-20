#include "PlanificadorRutas.h"

#include <algorithm> // Permite invertir la ruta reconstruida desde el destino.
#include <cmath> // Permite calcular la distancia absoluta de la heuristica.
#include <limits> // Proporciona el valor maximo para representar un costo infinito.
#include <queue> // Proporciona la cola de prioridad usada por A*.
#include <stdexcept> // Permite informar coordenadas invalidas o rutas inexistentes.

namespace
{
struct Nodo
{
    // Interseccion que representa este nodo.
    Coordenada coordenada;
    // Costo acumulado desde el inicio.
    int costoReal;
    // Costo real mas la estimacion hasta el destino.
    int costoTotal;
};

struct CompararNodos
{
    // Coloca primero el nodo con menor costo total.
    bool operator()(const Nodo& primero, const Nodo& segundo) const
    {
        return primero.costoTotal > segundo.costoTotal;
    }
};

bool mismaCoordenada(Coordenada primero, Coordenada segundo)
{
    // Comprueba si dos nodos representan la misma posicion.
    return primero.fila == segundo.fila &&
           primero.columna == segundo.columna;
}
}

int PlanificadorRutas::calcularHeuristica(
    Coordenada actual,
    Coordenada destino) const
{
    // La distancia Manhattan estima cuantos pasos faltan.
    return std::abs(actual.fila - destino.fila) +
           std::abs(actual.columna - destino.columna);
}

bool PlanificadorRutas::movimientoPermitido(
    const Ciudad& ciudad,
    Coordenada actual,
    Coordenada vecino) const
{
    if (vecino.fila == actual.fila)
    {
        if (std::abs(vecino.columna - actual.columna) != 1)
        {
            return false;
        }
        return ciudad.obtenerSentidoHorizontal(actual.fila) ==
               (vecino.columna - actual.columna);
    }

    if (vecino.columna == actual.columna)
    {
        if (std::abs(vecino.fila - actual.fila) != 1)
        {
            return false;
        }
        return ciudad.obtenerSentidoVertical(actual.columna) ==
               (vecino.fila - actual.fila);
    }

    return false;
}

std::vector<Coordenada> PlanificadorRutas::calcularRuta(
    const Ciudad& ciudad,
    Coordenada inicio,
    Coordenada destino) const
{
    const int filas = ciudad.obtenerFilas();
    const int columnas = ciudad.obtenerColumnas();
    const int infinito = std::numeric_limits<int>::max();

    auto esValida = [filas, columnas](Coordenada coordenada)
    {
        return coordenada.fila >= 0 && coordenada.fila < filas &&
               coordenada.columna >= 0 && coordenada.columna < columnas;
    };

    if (!esValida(inicio) || !esValida(destino))
    {
        throw std::out_of_range("El inicio o destino esta fuera de la ciudad");
    }

    std::vector<std::vector<int>> costos(filas,
        std::vector<int>(columnas, infinito));
    std::vector<std::vector<Coordenada>> padres(filas,
        std::vector<Coordenada>(columnas, {-1, -1}));
    std::priority_queue<Nodo, std::vector<Nodo>, CompararNodos> pendientes;

    costos[inicio.fila][inicio.columna] = 0;
    pendientes.push({
        inicio,
        0,
        calcularHeuristica(inicio, destino)
    });

    const int movimientos[4][2] = {
        {-1, 0},
        {1, 0},
        {0, -1},
        {0, 1}
    };

    while (!pendientes.empty())
    {
        const Nodo actual = pendientes.top();
        pendientes.pop();

        if (mismaCoordenada(actual.coordenada, destino))
        {
            break;
        }

        // A* recorre las cuatro direcciones del mapa.
        for (const auto& movimiento : movimientos)
        {
            Coordenada vecino{
                actual.coordenada.fila + movimiento[0],
                actual.coordenada.columna + movimiento[1]
            };

            if (!esValida(vecino))
            {
                continue;
            }

            // Evita usar calles en contra de su sentido establecido.
            if (!movimientoPermitido(ciudad, actual.coordenada, vecino))
            {
                continue;
            }

            const int nuevoCosto = actual.costoReal + 1;
            if (nuevoCosto < costos[vecino.fila][vecino.columna])
            {
                costos[vecino.fila][vecino.columna] = nuevoCosto;
                padres[vecino.fila][vecino.columna] = actual.coordenada;
                pendientes.push({
                    vecino,
                    nuevoCosto,
                    nuevoCosto + calcularHeuristica(vecino, destino)
                });
            }
        }
    }

    if (costos[destino.fila][destino.columna] == infinito)
    {
        throw std::runtime_error("No se encontro una ruta hacia el destino");
    }

    // Reconstruye la ruta siguiendo los padres desde el destino.
    std::vector<Coordenada> ruta;
    Coordenada actual = destino;
    while (!mismaCoordenada(actual, inicio))
    {
        ruta.push_back(actual);
        actual = padres[actual.fila][actual.columna];
    }
    ruta.push_back(inicio);
    std::reverse(ruta.begin(), ruta.end());

    return ruta;
}

std::vector<std::vector<Coordenada>>
PlanificadorRutas::calcularRutasSecuencial(
    const Ciudad& ciudad,
    const std::vector<SolicitudRuta>& solicitudes) const
{
    std::vector<std::vector<Coordenada>> rutas;
    rutas.reserve(solicitudes.size());

    // Calcula cada ruta una despues de otra.
    for (const SolicitudRuta& solicitud : solicitudes)
    {
        rutas.push_back(calcularRuta(
            ciudad, solicitud.inicio, solicitud.destino));
    }

    return rutas;
}

std::vector<std::vector<Coordenada>>
PlanificadorRutas::calcularRutasParalelo(
    const Ciudad& ciudad,
    const std::vector<SolicitudRuta>& solicitudes) const
{
    std::vector<std::future<std::vector<Coordenada>>> futuros;
    futuros.reserve(solicitudes.size());

    // Inicia una tarea independiente para cada ruta.
    for (const SolicitudRuta& solicitud : solicitudes)
    {
        futuros.push_back(std::async(
            std::launch::async,
            [this, &ciudad, solicitud]
            {
                return calcularRuta(
                    ciudad, solicitud.inicio, solicitud.destino);
            }));
    }

    std::vector<std::vector<Coordenada>> rutas;
    rutas.reserve(futuros.size());

    // Espera y recoge el resultado de cada tarea.
    for (std::future<std::vector<Coordenada>>& futuro : futuros)
    {
        rutas.push_back(futuro.get());
    }

    return rutas;
}

