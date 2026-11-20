#pragma once

#include <string>

class Configuracion
{
public:
    bool cargar(const std::string& rutaArchivo);

    int cantidadVehiculos = 10;
    int filas = 12;
    int columnas = 12;
    int tiempoVerde = 2;
    int tiempoAmarillo = 1;
    int tiempoRojo = 3;
    bool origenAleatorio = true;
    bool destinoAleatorio = true;

private:
    static std::string leerValor(
        const std::string& linea,
        const std::string& clave);
};
