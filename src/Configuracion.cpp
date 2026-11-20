#include "Configuracion.h"

#include <algorithm>
#include <fstream>

std::string Configuracion::leerValor(
    const std::string& linea,
    const std::string& clave)
{
    const std::string prefijo = clave + "=";
    if (linea.rfind(prefijo, 0) == 0)
    {
        return linea.substr(prefijo.size());
    }

    return "";
}

bool Configuracion::cargar(const std::string& rutaArchivo)
{
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open())
    {
        return false;
    }

    std::string linea;
    while (std::getline(archivo, linea))
    {
        if (linea.find("vehiculos=") == 0)
        {
            cantidadVehiculos = std::clamp(
                std::stoi(linea.substr(10)), 1, 200);
        }
        else if (linea.find("filas=") == 0)
        {
            filas = std::max(1, std::stoi(linea.substr(6)));
        }
        else if (linea.find("columnas=") == 0)
        {
            columnas = std::max(1, std::stoi(linea.substr(9)));
        }
        else if (linea.find("tiempoVerde=") == 0)
        {
            tiempoVerde = std::max(1, std::stoi(linea.substr(12)));
        }
        else if (linea.find("tiempoAmarillo=") == 0)
        {
            tiempoAmarillo = std::max(1, std::stoi(linea.substr(15)));
        }
        else if (linea.find("tiempoRojo=") == 0)
        {
            tiempoRojo = std::max(1, std::stoi(linea.substr(11)));
        }
        else if (linea.find("origenAleatorio=") == 0)
        {
            origenAleatorio =
                linea.substr(16) == "true";
        }
        else if (linea.find("destinoAleatorio=") == 0)
        {
            destinoAleatorio =
                linea.substr(17) == "true";
        }
    }

    return true;
}
