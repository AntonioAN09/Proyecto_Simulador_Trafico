#include "Semaforo.h"

#include <chrono> // Proporciona las unidades de tiempo usadas por el semaforo.

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

