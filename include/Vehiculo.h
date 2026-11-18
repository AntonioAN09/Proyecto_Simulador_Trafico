#pragma once

#include <mutex> // Protege la posicion mostrada por Raylib.
#include <vector>

#include "Ciudad.h"
#include "Coordenada.h"
#include "Metricas.h"
#include "PlanificadorRutas.h"

class Vehiculo
{
public:
    Vehiculo(
        int identificador,
        Ciudad& ciudad,
        Metricas& metricas,
        Coordenada inicio,
        Coordenada destino);
    void conducir();
    Coordenada obtenerPosicionActual() const;
    bool estaEsperando() const;

private:
    // Genera una ruta usando el algoritmo A*.
    std::vector<Coordenada> calcularRuta() const;

    int identificador_;
    Ciudad& ciudad_;
    Metricas& metricas_;
    Coordenada inicio_;
    Coordenada destino_;
    mutable std::mutex mutexPosicion_;
    Coordenada posicionActual_;
    bool esperando_;
};

