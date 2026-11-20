#include "Semaforo.h"

#include <chrono> // Proporciona las unidades de tiempo usadas por el semaforo.

namespace
{
const std::chrono::steady_clock::time_point inicioCoordinado =
    std::chrono::steady_clock::now();
}

Semaforo::Semaforo(int segundosVerde, int segundosAmarillo, int segundosRojo)
    : segundosVerde_(segundosVerde),
      segundosAmarillo_(segundosAmarillo),
      segundosRojo_(segundosRojo),
      estado_(EstadoSemaforo::Verde),
      detener_(false)
{
}

Semaforo::~Semaforo()
{
    // Solicita detener el hilo del semaforo.
    {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        detener_ = true;
    }
    condicion_.notify_all();

    if (hilo_.joinable())
    {
        hilo_.join();
    }
}

void Semaforo::iniciar()
{
    // Inicia el hilo solamente una vez.
    if (!hilo_.joinable())
    {
        hilo_ = std::thread(&Semaforo::cambiarEstado, this);
    }
}

EstadoSemaforo Semaforo::obtenerEstado() const
{
    std::lock_guard<std::mutex> bloqueo(mutex_);
    return estado_;
}

bool Semaforo::permitePaso() const
{
    // Solo el color verde permite avanzar.
    return obtenerEstado() == EstadoSemaforo::Verde;
}

bool Semaforo::permitePasoHorizontal()
{
    return obtenerEstadoHorizontal() == EstadoSemaforo::Verde;
}

bool Semaforo::permitePasoVertical()
{
    return obtenerEstadoVertical() == EstadoSemaforo::Verde;
}

EstadoSemaforo Semaforo::obtenerEstadoHorizontal()
{
    const auto transcurrido =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - inicioCoordinado).count();
    const int fase = transcurrido % 6000;

    if (fase < 2000)
    {
        return EstadoSemaforo::Verde;
    }
    if (fase < 3000)
    {
        return EstadoSemaforo::Amarillo;
    }
    if (fase < 5000)
    {
        return EstadoSemaforo::Rojo;
    }
    return EstadoSemaforo::Amarillo;
}

EstadoSemaforo Semaforo::obtenerEstadoVertical()
{
    const auto transcurrido =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - inicioCoordinado).count();
    const int fase = transcurrido % 6000;

    if (fase < 2000)
    {
        return EstadoSemaforo::Rojo;
    }
    if (fase < 3000)
    {
        return EstadoSemaforo::Amarillo;
    }
    if (fase < 5000)
    {
        return EstadoSemaforo::Verde;
    }
    return EstadoSemaforo::Amarillo;
}

bool Semaforo::esperar(int segundos)
{
    std::unique_lock<std::mutex> bloqueo(mutex_);
    return condicion_.wait_for(
        bloqueo,
        std::chrono::seconds(segundos),
        [this] { return detener_; });
}

void Semaforo::cambiarEstado()
{
    while (true)
    {
        // Obtiene cuanto tiempo debe durar el color actual.
        int duracion = 0;
        {
            std::lock_guard<std::mutex> bloqueo(mutex_);
            if (detener_)
            {
                return;
            }

            if (estado_ == EstadoSemaforo::Verde)
            {
                duracion = segundosVerde_;
            }
            else if (estado_ == EstadoSemaforo::Amarillo)
            {
                duracion = segundosAmarillo_;
            }
            else
            {
                duracion = segundosRojo_;
            }
        }

        if (esperar(duracion))
        {
            return;
        }

        std::lock_guard<std::mutex> bloqueo(mutex_);
        // Cambia al siguiente color del ciclo.
        if (estado_ == EstadoSemaforo::Verde)
        {
            estado_ = EstadoSemaforo::Amarillo;
        }
        else if (estado_ == EstadoSemaforo::Amarillo)
        {
            estado_ = EstadoSemaforo::Rojo;
        }
        else
        {
            estado_ = EstadoSemaforo::Verde;
        }
    }
}

