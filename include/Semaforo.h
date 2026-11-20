#pragma once

#include <condition_variable> // Permite esperar cambios de estado sin revisar continuamente.
#include <mutex>
#include <thread> // Permite ejecutar el cambio de luces en paralelo.

enum class EstadoSemaforo
{
    // Estados posibles del semaforo.
    Verde,
    Amarillo,
    Rojo
};

class Semaforo
{
public:
    Semaforo(int segundosVerde = 2, int segundosAmarillo = 1, int segundosRojo = 3);
    ~Semaforo();

    void iniciar();
    EstadoSemaforo obtenerEstado() const;
    bool permitePaso() const;
    static bool permitePasoHorizontal();
    static bool permitePasoVertical();
    static EstadoSemaforo obtenerEstadoHorizontal();
    static EstadoSemaforo obtenerEstadoVertical();

private:
    // Hilo que cambia el estado del semaforo.
    void cambiarEstado();
    bool esperar(int segundos);

    // Duracion de cada color en segundos.
    int segundosVerde_;
    int segundosAmarillo_;
    int segundosRojo_;
    // Protege el estado y la bandera de detencion.
    mutable std::mutex mutex_;
    std::condition_variable condicion_;
    // Estado actual y control para detener el hilo.
    EstadoSemaforo estado_;
    bool detener_;
    std::thread hilo_;
};

