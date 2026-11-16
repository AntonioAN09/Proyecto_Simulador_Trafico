#pragma once

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

private:
    // Genera una ruta usando el algoritmo A*.
    std::vector<Coordenada> calcularRuta() const;

    int identificador_;
    Ciudad& ciudad_;
    Metricas& metricas_;
    Coordenada inicio_;
    Coordenada destino_;
};

