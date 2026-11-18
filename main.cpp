#include <algorithm> // Permite encontrar el vehiculo que llego primero.
#include <iostream>
#include <chrono>
#include <numeric> // Permite sumar los tiempos de viaje.
#include <deque> // Permite guardar vehiculos sin mover sus mutex.
#include <thread>
#include <raylib.h> // Permite mostrar la simulacion grafica.

#include "Ciudad.h"
#include "Metricas.h"
#include "PlanificadorRutas.h"
#include "Vehiculo.h"


int main()
{
    // Inicia la simulacion.
    std::cout << "Simulador de trafico iniciado\n";

    // Crea una ciudad de doce filas por doce columnas.
    Ciudad ciudad(12, 12);
    std::cout << "Ciudad creada: " << ciudad.obtenerFilas()
              << " x " << ciudad.obtenerColumnas() << "\n";

    PlanificadorRutas planificador;
    std::vector<SolicitudRuta> solicitudes = {
        {{0, 0}, {11, 11}}, {{11, 0}, {0, 11}},
        {{0, 11}, {11, 0}}, {{11, 11}, {0, 0}},
        {{0, 5}, {11, 6}}, {{11, 5}, {0, 6}},
        {{5, 0}, {6, 11}}, {{6, 11}, {5, 0}},
        {{2, 0}, {9, 11}}, {{9, 11}, {2, 0}}
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

    std::deque<Vehiculo> vehiculos;
    for (int i = 0; i < 10; ++i)
    {
        vehiculos.emplace_back(
            i + 1,
            ciudad,
            metricas,
            solicitudes[i].inicio,
            solicitudes[i].destino);
    }

    // Cada vehiculo se ejecuta en su propio hilo.
    std::vector<std::thread> hilos;
    hilos.reserve(10);
    for (Vehiculo& vehiculo : vehiculos)
    {
        hilos.emplace_back(&Vehiculo::conducir, &vehiculo);
    }

    const int anchoVentana = 1000;
    const int altoVentana = 700;
    const int origenX = 70;
    const int origenY = 70;
    const int separacion = 48;

    InitWindow(anchoVentana, altoVentana,
               "Simulador concurrente de trafico");
    SetTargetFPS(60);

    // Raylib dibuja la ventana mientras los hilos continúan la simulación.
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        DrawText("SIMULADOR DE TRAFICO", 230, 20, 24, RAYWHITE);

        // Dibuja calles horizontales y verticales.
        for (int fila = 0; fila < ciudad.obtenerFilas(); ++fila)
        {
            const int y = origenY + fila * separacion;
            DrawLine(origenX - 20, y, origenX + 11 * separacion + 20, y, LIGHTGRAY);

            // Filas pares hacia la derecha y filas impares hacia la izquierda.
            const char* direccion =
                fila % 2 == 0 ? ">" : "<";
            DrawText(direccion, origenX + 11 * separacion + 12, y - 6, 12, SKYBLUE);
        }
        for (int columna = 0; columna < ciudad.obtenerColumnas(); ++columna)
        {
            const int x = origenX + columna * separacion;
            DrawLine(x, origenY - 20, x, origenY + 11 * separacion + 20, LIGHTGRAY);

            // Columnas pares hacia abajo y columnas impares hacia arriba.
            const char* direccion =
                columna % 2 == 0 ? "v" : "^";
            DrawText(direccion, x - 4, origenY + 11 * separacion + 12, 12, ORANGE);
        }

        // Dibuja cada intersección y el color de su semáforo.
        for (int fila = 0; fila < ciudad.obtenerFilas(); ++fila)
        {
            for (int columna = 0; columna < ciudad.obtenerColumnas(); ++columna)
            {
                Interseccion& interseccion =
                    ciudad.obtenerInterseccion(fila, columna);
                const int x = origenX + columna * separacion;
                const int y = origenY + fila * separacion;

                DrawRectangle(x - 12, y - 12, 24, 24, GRAY);

                Color colorSemaforo = RED;
                if (interseccion.obtenerEstadoSemaforo() ==
                    EstadoSemaforo::Verde)
                {
                    colorSemaforo = GREEN;
                }
                else if (interseccion.obtenerEstadoSemaforo() ==
                         EstadoSemaforo::Amarillo)
                {
                    colorSemaforo = YELLOW;
                }

                DrawCircle(x + 14, y - 14, 5, colorSemaforo);
            }
        }

        // Dibuja los vehículos usando la posición protegida por su mutex.
        for (int i = 0; i < static_cast<int>(vehiculos.size()); ++i)
        {
            const Coordenada posicion =
                vehiculos[i].obtenerPosicionActual();
            const int x = origenX + posicion.columna * separacion;
            const int y = origenY + posicion.fila * separacion;

            const Color colores[] = {
                SKYBLUE, ORANGE, GREEN, YELLOW, PINK,
                PURPLE, RED, LIME, GOLD, VIOLET
            };
            DrawCircle(x - 8, y + 12, 7, colores[i]);
            DrawText(TextFormat("V%d", i + 1), x - 15, y + 20, 10, RAYWHITE);

            if (vehiculos[i].estaEsperando())
            {
                DrawCircleLines(x - 8, y + 12, 10, YELLOW);
            }
        }

        DrawText("Cerrar ventana para terminar", 500, 570, 14, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();

    // Espera a que los diez hilos terminen antes de mostrar las metricas.
    for (std::thread& hilo : hilos)
    {
        hilo.join();
    }

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
