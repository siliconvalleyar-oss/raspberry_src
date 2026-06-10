#include "epaper.h"
#include "config.h"
#include <memory>
#include <iostream>


int main() {
    // Inicializar el ePaper con los pines correctos
    
    auto epaper = std::make_unique<EPAPER::EPaper_t>(8, 25, 17, 24);

#ifdef CPU_64_BITS
std::cout<< "es 64 bits"<<std::endl;
#else
std::cout<< "es 32 bits"<<std::endl;
#endif

    // Inicializar el ePaper
    epaper->initialize();

    // Restablecer el ePaper
    epaper->reset();

    // Esperar hasta que el ePaper esté listo
    epaper->waitUntilIdle();

    // Enviar un comando de ejemplo
    epaper->sendCommand(0x01);

    // Enviar datos de ejemplo
    epaper->sendData(0x0F);

    // Visualizar un frame (ejemplo)
    unsigned char frame_buffer[200] = {0};  // Ejemplo de buffer
    epaper->displayFrame(frame_buffer);

    return 0;
}
