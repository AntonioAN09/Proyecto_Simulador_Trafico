#pragma once

#include <mutex>

#include "Semaforo.h"

class Interseccion
{
public:
    Interseccion(
        int tiempoVerde = 2,
        int tiempoAmarillo = 1,
        int tiempoRojo = 3,
        bool tieneSemaforo = true);
    bool intentarEntrar();
    void salir();
    bool permitePaso() const;
    bool permitePasoHorizontal() const;
    bool permitePasoVertical() const;
    EstadoSemaforo obtenerEstadoHorizontal() const;
    EstadoSemaforo obtenerEstadoVertical() const;
    bool tieneSemaforo() const;
    EstadoSemaforo obtenerEstadoSemaforo() const;

private:
    // Evita que dos vehiculos ocupen la interseccion al mismo tiempo.
    std::mutex mutex_;
    // Controla si los vehiculos pueden pasar.
    Semaforo semaforo_;
    bool tieneSemaforo_;
};

