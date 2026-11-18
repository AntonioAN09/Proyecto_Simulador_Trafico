#include "Vehiculo.h"

#include <chrono> // Permite simular el tiempo que tarda el vehiculo en cruzar.
#include <iostream>
#include <mutex>
#include <thread> // Permite ejecutar la conduccion de cada vehiculo en paralelo.

namespace
{
// Evita que los mensajes de los hilos se mezclen en la consola.
std::mutex mutexConsola;
}

Vehiculo::Vehiculo(
    int identificador,
    Ciudad& ciudad,
    Metricas& metricas,
    Coordenada inicio,
    Coordenada destino)
    : identificador_(identificador), ciudad_(ciudad), metricas_(metricas),
      inicio_(inicio), destino_(destino)
{
    posicionActual_ = inicio_;
    esperando_ = false;
}

Coordenada Vehiculo::obtenerPosicionActual() const
{
    std::lock_guard<std::mutex> bloqueo(mutexPosicion_);
    return posicionActual_;
}

bool Vehiculo::estaEsperando() const
{
    std::lock_guard<std::mutex> bloqueo(mutexPosicion_);
    return esperando_;
}

std::vector<Coordenada> Vehiculo::calcularRuta() const
{
    PlanificadorRutas planificador;
    return planificador.calcularRuta(ciudad_, inicio_, destino_);
}

void Vehiculo::conducir()
{
    metricas_.iniciarViaje(identificador_);

    // Cada vehiculo recorre su propia copia de la ruta.
    const std::vector<Coordenada> ruta = calcularRuta();

    for (const Coordenada& coordenada : ruta)
    {
        {
            std::lock_guard<std::mutex> bloqueo(mutexPosicion_);
            posicionActual_ = coordenada;
        }

        Interseccion& interseccion =
            ciudad_.obtenerInterseccion(coordenada.fila, coordenada.columna);

        bool esperando = false;
        while (!interseccion.permitePaso() || !interseccion.intentarEntrar())
        {
            if (!esperando)
            {
                metricas_.iniciarEspera(identificador_, coordenada);
                esperando = true;
                std::lock_guard<std::mutex> bloqueo(mutexPosicion_);
                esperando_ = true;
            }

            // Aqui el vehiculo espera a que el semaforo y la interseccion esten libres.
            std::lock_guard<std::mutex> bloqueo(mutexConsola);
            std::cout << "Vehiculo " << identificador_
                      << " espera en (" << coordenada.fila << ","
                      << coordenada.columna << ")\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (esperando)
        {
            metricas_.terminarEspera(identificador_);
            std::lock_guard<std::mutex> bloqueo(mutexPosicion_);
            esperando_ = false;
        }

        {
            std::lock_guard<std::mutex> bloqueo(mutexConsola);
            std::cout << "Vehiculo " << identificador_
                      << " esta en (" << coordenada.fila << ","
                      << coordenada.columna << ")\n";
        }
        // Tiempo visible en cada interseccion para que el movimiento sea lento.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Libera la interseccion antes de avanzar a la siguiente.
        interseccion.salir();
    }

    {
        std::lock_guard<std::mutex> bloqueo(mutexConsola);
        std::cout << "Vehiculo " << identificador_ << " llego a su destino\n";
    }

    metricas_.finalizarViaje(identificador_);
}

