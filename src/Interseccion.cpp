#include "Interseccion.h"

Interseccion::Interseccion(
    int tiempoVerde,
    int tiempoAmarillo,
    int tiempoRojo,
    bool tieneSemaforo)
    : semaforo_(tiempoVerde, tiempoAmarillo, tiempoRojo),
      tieneSemaforo_(tieneSemaforo)
{
    // Solo las avenidas principales tienen semaforo activo.
    if (tieneSemaforo_)
    {
        semaforo_.iniciar();
    }
}

bool Interseccion::intentarEntrar()
{
    // Intenta entrar sin quedarse bloqueado indefinidamente.
    return mutex_.try_lock();
}

void Interseccion::salir()
{
    // Libera la interseccion para el siguiente vehiculo.
    mutex_.unlock();
}

bool Interseccion::permitePaso() const
{
    if (!tieneSemaforo_)
    {
        return true;
    }
    return semaforo_.permitePaso();
}

bool Interseccion::permitePasoHorizontal() const
{
    if (!tieneSemaforo_)
    {
        return true;
    }
    return Semaforo::permitePasoHorizontal();
}

bool Interseccion::permitePasoVertical() const
{
    if (!tieneSemaforo_)
    {
        return true;
    }
    return Semaforo::permitePasoVertical();
}

EstadoSemaforo Interseccion::obtenerEstadoHorizontal() const
{
    return Semaforo::obtenerEstadoHorizontal();
}

EstadoSemaforo Interseccion::obtenerEstadoVertical() const
{
    return Semaforo::obtenerEstadoVertical();
}

bool Interseccion::tieneSemaforo() const
{
    return tieneSemaforo_;
}

EstadoSemaforo Interseccion::obtenerEstadoSemaforo() const
{
    return semaforo_.obtenerEstado();
}

