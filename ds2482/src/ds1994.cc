#include <iostream>
#include <fstream>
#include <string>

int main() {
    // Ruta al archivo del dispositivo 1-Wire
    std::string path = "/sys/bus/w1/devices/04-000000532bd4/w1_slave"; // Cambia la ruta si es diferente

    // Abrir el archivo para lectura
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo del dispositivo 1-Wire: " << path << std::endl;
        return 1;
    }

    // Leer el contenido del archivo
    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }

    // Cerrar el archivo
    file.close();

    return 0;
}
