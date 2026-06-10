#pragma once

#include <stdint.h>
#include "spi.h"

#define SCREEN 213 // Tamaño de la pantalla en pulgadas (2.13")
#define WIDTH 212
#define HEIGHT 104


namespace EPAPER{

    struct EPaper_t {
    public:
        explicit EPaper_t(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy);
        void initialize();
        void sendCommand(uint8_t command);
        void sendData(uint8_t data);
        void reset();
        void waitUntilIdle();
        void displayFrame(const unsigned char* frame_buffer);

    private:
        void digitalWrite(uint8_t pin, uint8_t value);
        void delayMs(unsigned int ms);

        uint8_t cs_pin;
        uint8_t dc_pin;
        uint8_t rst_pin;
        uint8_t busy_pin;
        SPI::SPI_t spi;
        int fd; // File descriptor para SPI
        //const char device[3];
        uint32_t speed;
    };


}//end namespace EPAPER_t