#pragma once

#include <future> // Permite calcular rutas en hilos independientes.
#include <vector>

#include "Ciudad.h"
#include "Coordenada.h"

struct SolicitudRuta
{
    Coordenada inicio;
    Coordenada destino;
};

class PlanificadorRutas
{
public:
    std::vector<Coordenada> calcularRuta(
        const Ciudad& ciudad,
        Coordenada inicio,
        Coordenada destino) const;
    std::vector<std::vector<Coordenada>> calcularRutasSecuencial(
        const Ciudad& ciudad,
        const std::vector<SolicitudRuta>& solicitudes) const;
    std::vector<std::vector<Coordenada>> calcularRutasParalelo(
        const Ciudad& ciudad,
        const std::vector<SolicitudRuta>& solicitudes) const;

private:
    int calcularHeuristica(Coordenada actual, Coordenada destino) const;
    bool movimientoPermitido(
        const Ciudad& ciudad,
        Coordenada actual,
        Coordenada vecino) const;
};

