#include "Vehiculo.h"

#include <chrono> // Permite simular el tiempo que tarda el vehiculo en cruzar.
#include <thread> // Permite ejecutar la conduccion de cada vehiculo en paralelo.

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

    Coordenada anterior = ruta.empty() ? inicio_ : ruta.front();

    for (const Coordenada& coordenada : ruta)
    {
        {
            std::lock_guard<std::mutex> bloqueo(mutexPosicion_);
            posicionActual_ = coordenada;
        }

        Interseccion& interseccion =
            ciudad_.obtenerInterseccion(coordenada.fila, coordenada.columna);

        bool esperando = false;
        while (true)
        {
            const bool movimientoHorizontal =
                coordenada.fila == anterior.fila;
            const bool puedeAvanzar =
                interseccion.permitePaso() &&
                (movimientoHorizontal
                    ? interseccion.permitePasoHorizontal()
                    : interseccion.permitePasoVertical());

            if (!puedeAvanzar)
            {
                if (!esperando)
                {
                    metricas_.iniciarEspera(identificador_, coordenada);
                    esperando = true;
                    std::lock_guard<std::mutex> bloqueo(mutexPosicion_);
                    esperando_ = true;
                }
            }
            else if (!interseccion.intentarEntrar())
            {
                metricas_.registrarChoqueEvitado();
                if (!esperando)
                {
                    metricas_.iniciarEspera(identificador_, coordenada);
                    esperando = true;
                    std::lock_guard<std::mutex> bloqueo(mutexPosicion_);
                    esperando_ = true;
                }
            }
            else
            {
                break;
            }

            // Espera sin repetir el mismo mensaje en cada ciclo.
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (esperando)
        {
            metricas_.terminarEspera(identificador_);
            std::lock_guard<std::mutex> bloqueo(mutexPosicion_);
            esperando_ = false;
        }

        // Tiempo visible en cada interseccion para que el movimiento sea lento.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Libera la interseccion antes de avanzar a la siguiente.
        interseccion.salir();
        anterior = coordenada;
    }

    metricas_.finalizarViaje(identificador_);
}

