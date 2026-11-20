#include "Metricas.h"

#include <algorithm> // Permite guardar el mayor nivel de congestion.

void Metricas::iniciarViaje(int identificador)
{
    std::lock_guard<std::mutex> bloqueo(mutex_);
    registros_[identificador].inicioViaje = std::chrono::steady_clock::now();
}

void Metricas::registrarChoqueEvitado()
{
    std::lock_guard<std::mutex> bloqueo(mutex_);
    ++choquesEvitados_;
}

int Metricas::obtenerChoquesEvitados() const
{
    std::lock_guard<std::mutex> bloqueo(mutex_);
    return choquesEvitados_;
}

void Metricas::iniciarEspera(int identificador, Coordenada coordenada)
{
    std::lock_guard<std::mutex> bloqueo(mutex_);
    Registro& registro = registros_[identificador];

    if (!registro.estaEsperando)
    {
        registro.inicioEspera = std::chrono::steady_clock::now();
        registro.coordenadaEspera = coordenada;
        registro.estaEsperando = true;

        const std::pair<int, int> clave{coordenada.fila, coordenada.columna};
        const int esperando = ++esperandoActual_[clave];
        congestionMaxima_[clave] =
            std::max(congestionMaxima_[clave], esperando);
    }
}

void Metricas::terminarEspera(int identificador)
{
    std::lock_guard<std::mutex> bloqueo(mutex_);
    Registro& registro = registros_[identificador];

    if (registro.estaEsperando)
    {
        const auto ahora = std::chrono::steady_clock::now();
        registro.tiempoEsperaMilisegundos +=
            std::chrono::duration_cast<std::chrono::milliseconds>(
                ahora - registro.inicioEspera).count();
        registro.estaEsperando = false;

        const std::pair<int, int> clave{
            registro.coordenadaEspera.fila,
            registro.coordenadaEspera.columna
        };
        --esperandoActual_[clave];
    }
}

void Metricas::finalizarViaje(int identificador)
{
    std::lock_guard<std::mutex> bloqueo(mutex_);
    Registro& registro = registros_[identificador];

    if (registro.estaEsperando)
    {
        const auto ahora = std::chrono::steady_clock::now();
        registro.tiempoEsperaMilisegundos +=
            std::chrono::duration_cast<std::chrono::milliseconds>(
                ahora - registro.inicioEspera).count();
        registro.estaEsperando = false;

        const std::pair<int, int> clave{
            registro.coordenadaEspera.fila,
            registro.coordenadaEspera.columna
        };
        --esperandoActual_[clave];
    }

    const auto ahora = std::chrono::steady_clock::now();
    registro.tiempoViajeMilisegundos =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            ahora - registro.inicioViaje).count();
    registro.finalizado = true;
}

std::vector<ResultadoVehiculo> Metricas::obtenerResultados() const
{
    std::lock_guard<std::mutex> bloqueo(mutex_);
    std::vector<ResultadoVehiculo> resultados;

    for (const auto& [identificador, registro] : registros_)
    {
        if (!registro.finalizado)
        {
            continue;
        }

        resultados.push_back({
            identificador,
            registro.tiempoViajeMilisegundos,
            registro.tiempoEsperaMilisegundos,
            registro.tiempoViajeMilisegundos > 0
                ? (static_cast<double>(registro.tiempoEsperaMilisegundos) *
                   100.0 / registro.tiempoViajeMilisegundos)
                : 0.0
        });
    }

    return resultados;
}

std::vector<ResultadoCongestion> Metricas::obtenerCongestion() const
{
    std::lock_guard<std::mutex> bloqueo(mutex_);
    std::vector<ResultadoCongestion> resultados;

    for (const auto& [clave, maximo] : congestionMaxima_)
    {
        resultados.push_back({
            {clave.first, clave.second},
            maximo
        });
    }

    return resultados;
}

