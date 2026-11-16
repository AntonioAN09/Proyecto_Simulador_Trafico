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
        Interseccion& interseccion =
            ciudad_.obtenerInterseccion(coordenada.fila, coordenada.columna);

        bool esperando = false;
        while (!interseccion.permitePaso() || !interseccion.intentarEntrar())
        {
            if (!esperando)
            {
                metricas_.iniciarEspera(identificador_, coordenada);
                esperando = true;
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
        }

        {
            std::lock_guard<std::mutex> bloqueo(mutexConsola);
            std::cout << "Vehiculo " << identificador_
                      << " esta en (" << coordenada.fila << ","
                      << coordenada.columna << ")\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        // Libera la interseccion antes de avanzar a la siguiente.
        interseccion.salir();
    }

    {
        std::lock_guard<std::mutex> bloqueo(mutexConsola);
        std::cout << "Vehiculo " << identificador_ << " llego a su destino\n";
    }

    metricas_.finalizarViaje(identificador_);
}

