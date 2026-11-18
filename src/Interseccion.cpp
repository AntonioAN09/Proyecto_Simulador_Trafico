#include "Interseccion.h"

Interseccion::Interseccion()
{
    // Cada interseccion inicia su propio hilo de semaforo.
    semaforo_.iniciar();
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
    return semaforo_.permitePaso();
}

EstadoSemaforo Interseccion::obtenerEstadoSemaforo() const
{
    return semaforo_.obtenerEstado();
}

