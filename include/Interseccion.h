#pragma once

#include <mutex>

#include "Semaforo.h"

class Interseccion
{
public:
    Interseccion();
    bool intentarEntrar();
    void salir();
    bool permitePaso() const;
    EstadoSemaforo obtenerEstadoSemaforo() const;

private:
    // Evita que dos vehiculos ocupen la interseccion al mismo tiempo.
    std::mutex mutex_;
    // Controla si los vehiculos pueden pasar.
    Semaforo semaforo_;
};

