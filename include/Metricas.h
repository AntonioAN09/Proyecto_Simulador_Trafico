#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <vector>

#include "Coordenada.h"

struct ResultadoVehiculo
{
    int identificador;
    long long tiempoViajeMilisegundos;
    long long tiempoEsperaMilisegundos;
    double porcentajeDetenido;
};

struct ResultadoCongestion
{
    Coordenada coordenada;
    int maximoVehiculosEsperando;
};

class Metricas
{
public:
    void iniciarViaje(int identificador);
    void iniciarEspera(int identificador, Coordenada coordenada);
    void terminarEspera(int identificador);
    void finalizarViaje(int identificador);
    std::vector<ResultadoVehiculo> obtenerResultados() const;
    std::vector<ResultadoCongestion> obtenerCongestion() const;
    void registrarChoqueEvitado();
    int obtenerChoquesEvitados() const;

private:
    struct Registro
    {
        std::chrono::steady_clock::time_point inicioViaje;
        std::chrono::steady_clock::time_point inicioEspera;
        Coordenada coordenadaEspera{-1, -1};
        long long tiempoViajeMilisegundos = 0;
        long long tiempoEsperaMilisegundos = 0;
        bool estaEsperando = false;
        bool finalizado = false;
    };

    mutable std::mutex mutex_;
    int choquesEvitados_ = 0;
    std::map<int, Registro> registros_;
    std::map<std::pair<int, int>, int> esperandoActual_;
    std::map<std::pair<int, int>, int> congestionMaxima_;
};

