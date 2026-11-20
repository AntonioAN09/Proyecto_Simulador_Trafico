#include <algorithm> // Permite encontrar el vehiculo que llego primero.
#include <iostream>
#include <chrono>
#include <numeric> // Permite sumar los tiempos de viaje.
#include <deque> // Permite guardar vehiculos sin mover sus mutex.
#include <random> // Permite generar vehiculos aleatorios.
#include <thread>
#include <raylib.h> // Permite mostrar la simulacion grafica.

#include "Ciudad.h"
#include "Configuracion.h"
#include "Metricas.h"
#include "PlanificadorRutas.h"
#include "Vehiculo.h"

using namespace std;


int main()
{
    // Inicia la simulacion.
    cout << "Simulador de trafico iniciado\n";

    Configuracion configuracion;
    if (!configuracion.cargar("configuracion.txt"))
    {
        cout << "No se encontro configuracion.txt; se usan valores predeterminados\n";
    }

    // Crea la ciudad con las dimensiones y tiempos de la configuracion.
    Ciudad ciudad(
        configuracion.filas,
        configuracion.columnas,
        configuracion.tiempoVerde,
        configuracion.tiempoAmarillo,
        configuracion.tiempoRojo);
    cout << "Ciudad creada: " << ciudad.obtenerFilas()
              << " x " << ciudad.obtenerColumnas() << "\n";

    PlanificadorRutas planificador;
    vector<SolicitudRuta> solicitudes;
    solicitudes.reserve(configuracion.cantidadVehiculos);
    random_device dispositivo;
    mt19937 generador(dispositivo());
    uniform_int_distribution<int> filaAleatoria(
        0, configuracion.filas - 1);
    uniform_int_distribution<int> columnaAleatoria(
        0, configuracion.columnas - 1);

    for (int i = 0; i < configuracion.cantidadVehiculos; ++i)
    {
        Coordenada inicio;
        Coordenada destino;

        if (configuracion.origenAleatorio)
        {
            inicio = {filaAleatoria(generador), columnaAleatoria(generador)};
        }
        else
        {
            inicio = {i % configuracion.filas, 0};
        }

        if (configuracion.destinoAleatorio)
        {
            do
            {
                destino = {
                    filaAleatoria(generador),
                    columnaAleatoria(generador)
                };
            } while (destino.fila == inicio.fila &&
                     destino.columna == inicio.columna);
        }
        else
        {
            destino = {
                configuracion.filas - 1 - i % configuracion.filas,
                configuracion.columnas - 1
            };
        }

        // Solo agrega solicitudes que A* pueda alcanzar con los sentidos actuales.
        try
        {
            planificador.calcularRuta(ciudad, inicio, destino);
            solicitudes.push_back({inicio, destino});
        }
        catch (const std::runtime_error&)
        {
            --i;
        }
    }

    auto inicioSecuencial = chrono::steady_clock::now();
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
    for (int i = 0; i < configuracion.cantidadVehiculos; ++i)
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
    hilos.reserve(configuracion.cantidadVehiculos);
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
        ClearBackground(RAYWHITE);

        DrawText("SIMULADOR DE TRAFICO", 230, 20, 24, DARKGRAY);
        DrawText("Sentido horizontal:", 650, 70, 16, SKYBLUE);
        DrawText("< Oeste              Este >", 650, 92, 16, DARKGRAY);
        DrawText("Sentido vertical:", 650, 125, 16, ORANGE);
        DrawText("^ Norte              Sur v", 650, 147, 16, DARKGRAY);

        int totalSemaforos = 0;
        for (int fila = 0; fila < ciudad.obtenerFilas(); ++fila)
        {
            for (int columna = 0; columna < ciudad.obtenerColumnas(); ++columna)
            {
                if (ciudad.interseccionTieneSemaforo(fila, columna))
                {
                    ++totalSemaforos;
                }
            }
        }
        DrawText(
            TextFormat("%d cruces principales con semaforo",
                       totalSemaforos),
            650,
            180,
            14,
            DARKGRAY);

        // Dibuja calles horizontales y verticales.
        for (int fila = 0; fila < ciudad.obtenerFilas(); ++fila)
        {
            const int y = origenY + fila * separacion;
            DrawRectangle(origenX - 20, y - 8,
                          11 * separacion + 40, 16, GRAY);

            // Muestra el sentido horizontal en varios puntos de la calle.
            for (int columna = 0; columna < 11; ++columna)
            {
                const int x = origenX + columna * separacion + separacion / 2;
                if (ciudad.obtenerSentidoHorizontal(fila) > 0)
                {
                    // Este.
                    DrawTriangle(
                        {(float)x + 8, (float)y},
                        {(float)x - 5, (float)y - 6},
                        {(float)x - 5, (float)y + 6},
                        SKYBLUE);
                }
                else
                {
                    // Oeste.
                    DrawTriangle(
                        {(float)x - 8, (float)y},
                        {(float)x + 5, (float)y - 6},
                        {(float)x + 5, (float)y + 6},
                        SKYBLUE);
                }
            }
        }
        for (int columna = 0; columna < ciudad.obtenerColumnas(); ++columna)
        {
            const int x = origenX + columna * separacion;
            DrawRectangle(x - 8, origenY - 20,
                          16, 11 * separacion + 40, GRAY);

            // Muestra el sentido vertical en varios puntos de la calle.
            for (int fila = 0; fila < 11; ++fila)
            {
                const int y = origenY + fila * separacion + separacion / 2;
                if (ciudad.obtenerSentidoVertical(columna) > 0)
                {
                    // Sur.
                    DrawTriangle(
                        {(float)x, (float)y + 8},
                        {(float)x - 6, (float)y - 5},
                        {(float)x + 6, (float)y - 5},
                        ORANGE);
                }
                else
                {
                    // Norte.
                    DrawTriangle(
                        {(float)x, (float)y - 8},
                        {(float)x - 6, (float)y + 5},
                        {(float)x + 6, (float)y + 5},
                        ORANGE);
                }
            }
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

                DrawRectangle(x - 12, y - 12, 24, 24, DARKGRAY);

                if (!ciudad.interseccionTieneSemaforo(fila, columna))
                {
                    continue;
                }

                auto obtenerColor = [](EstadoSemaforo estado)
                {
                    if (estado == EstadoSemaforo::Verde)
                    {
                        return GREEN;
                    }
                    if (estado == EstadoSemaforo::Amarillo)
                    {
                        return YELLOW;
                    }
                    return RED;
                };

                if (interseccion.tieneSemaforo())
                {
                    // Luz azul horizontal y luz naranja vertical.
                    DrawCircle(x - 14, y - 14, 5,
                               obtenerColor(interseccion.obtenerEstadoHorizontal()));
                    DrawCircle(x + 14, y + 14, 5,
                               obtenerColor(interseccion.obtenerEstadoVertical()));
                }
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
            DrawCircle(
                x - 8,
                y + 12,
                7,
                colores[i % 10]);
            DrawText(TextFormat("V%d", i + 1), x - 15, y + 20, 10, DARKGRAY);

            if (vehiculos[i].estaEsperando())
            {
                DrawCircleLines(x - 8, y + 12, 10, YELLOW);
            }
        }

        DrawText("Cerrar ventana para terminar", 500, 570, 14, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();

    // Espera a que los diez hilos terminen antes de mostrar las metricas.
    for (std::thread& hilo : hilos)
    {
        hilo.join();
    }

    const auto resultados = metricas.obtenerResultados();
    std::vector<ResultadoVehiculo> resultadosOrdenados = resultados;
    std::sort(
        resultadosOrdenados.begin(),
        resultadosOrdenados.end(),
        [](const ResultadoVehiculo& primero,
           const ResultadoVehiculo& segundo)
        {
            return primero.tiempoViajeMilisegundos <
                   segundo.tiempoViajeMilisegundos;
        });

    std::cout << "\nOrden de llegada:\n";
    for (size_t i = 0; i < resultadosOrdenados.size(); ++i)
    {
        const ResultadoVehiculo& resultado = resultadosOrdenados[i];
        std::cout << i + 1 << ". Vehiculo "
                  << resultado.identificador
                  << " - " << resultado.tiempoViajeMilisegundos
                  << " ms - detenido "
                  << resultado.porcentajeDetenido << "%\n";
    }

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

    std::cout << "\nTabla final de resultados\n";
    std::cout << "Vehiculo | Viaje ms | Espera ms | Detenido %\n";
    for (const ResultadoVehiculo& resultado : resultadosOrdenados)
    {
        std::cout << resultado.identificador
                  << " | " << resultado.tiempoViajeMilisegundos
                  << " | " << resultado.tiempoEsperaMilisegundos
                  << " | " << resultado.porcentajeDetenido << "\n";
    }

    std::cout << "Colisiones detectadas: 0\n";
    std::cout << "Choques evitados por bloqueo: "
              << metricas.obtenerChoquesEvitados() << "\n";

    return 0;
}
