#include <algorithm> // Permite encontrar el vehiculo que llego primero.
#include <iostream>
#include <chrono>
#include <numeric> // Permite sumar los tiempos de viaje.
#include <thread>

#include "Ciudad.h"
#include "Metricas.h"
#include "PlanificadorRutas.h"
#include "Vehiculo.h"

int main()
{
    // Inicia la simulacion.
    std::cout << "Simulador de trafico iniciado\n";

    // Crea una ciudad de tres filas por tres columnas.
    Ciudad ciudad(3, 3);
    std::cout << "Ciudad creada: " << ciudad.obtenerFilas()
              << " x " << ciudad.obtenerColumnas() << "\n";

    PlanificadorRutas planificador;
    std::vector<SolicitudRuta> solicitudes = {
        {{0, 0}, {2, 2}},
        {{2, 0}, {0, 2}},
        {{0, 2}, {2, 0}}
    };

    auto inicioSecuencial = std::chrono::steady_clock::now();
    const auto rutasSecuenciales =
        planificador.calcularRutasSecuencial(ciudad, solicitudes);
    auto finSecuencial = std::chrono::steady_clock::now();

    auto inicioParalelo = std::chrono::steady_clock::now();
    const auto rutasParalelas =
        planificador.calcularRutasParalelo(ciudad, solicitudes);
    auto finParalelo = std::chrono::steady_clock::now();

    const auto tiempoSecuencial =
        std::chrono::duration_cast<std::chrono::microseconds>(
            finSecuencial - inicioSecuencial).count();
    const auto tiempoParalelo =
        std::chrono::duration_cast<std::chrono::microseconds>(
            finParalelo - inicioParalelo).count();

    std::cout << "Rutas calculadas: " << rutasSecuenciales.size() << "\n";
    std::cout << "Tiempo secuencial: " << tiempoSecuencial << " microsegundos\n";
    std::cout << "Tiempo paralelo: " << tiempoParalelo << " microsegundos\n";

    Metricas metricas;

    // Crea dos vehiculos con la misma ruta para probar la espera.
    Vehiculo vehiculo1(1, ciudad, metricas, {0, 0}, {2, 2});
    Vehiculo vehiculo2(2, ciudad, metricas, {0, 0}, {2, 2});

    // Cada vehiculo se ejecuta en su propio hilo.
    std::thread hilo1(&Vehiculo::conducir, &vehiculo1);
    std::thread hilo2(&Vehiculo::conducir, &vehiculo2);

    // Espera a que los dos vehiculos terminen.
    hilo1.join();
    hilo2.join();

    const auto resultados = metricas.obtenerResultados();
    for (const ResultadoVehiculo& resultado : resultados)
    {
        std::cout << "Vehiculo " << resultado.identificador
                  << " - viaje: " << resultado.tiempoViajeMilisegundos
                  << " ms, espera: "
                  << resultado.tiempoEsperaMilisegundos << " ms\n";
    }

    if (!resultados.empty())
    {
        const auto primero = std::min_element(
            resultados.begin(),
            resultados.end(),
            [](const ResultadoVehiculo& primero,
               const ResultadoVehiculo& segundo)
            {
                return primero.tiempoViajeMilisegundos <
                       segundo.tiempoViajeMilisegundos;
            });

        const long long sumaViajes = std::accumulate(
            resultados.begin(),
            resultados.end(),
            0LL,
            [](long long suma, const ResultadoVehiculo& resultado)
            {
                return suma + resultado.tiempoViajeMilisegundos;
            });

        const double promedioViaje =
            static_cast<double>(sumaViajes) / resultados.size();

        std::cout << "Vehiculo que llego primero: "
                  << primero->identificador << "\n";
        std::cout << "Tiempo promedio de viaje: "
                  << promedioViaje << " ms\n";
    }

    const auto congestiones = metricas.obtenerCongestion();
    for (const ResultadoCongestion& congestion : congestiones)
    {
        std::cout << "Congestion en ("
                  << congestion.coordenada.fila << ","
                  << congestion.coordenada.columna << "): "
                  << congestion.maximoVehiculosEsperando
                  << " vehiculos esperando\n";
    }

    return 0;
}
