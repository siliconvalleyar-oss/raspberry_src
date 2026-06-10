#include "epaper.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

#include <chrono>
#include <thread>

namespace EPAPER{

    EPaper_t::EPaper_t(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy)
        : cs_pin(cs), dc_pin(dc), rst_pin(rst), busy_pin(busy), spi(), fd(-1) ,speed(100000) //,device{SPI_1}
    {
        // Configuración inicial de pines o SPI si es necesario
        fd = spi.begin(SPI_CH0_PORT1,  speed);
    }

    void EPaper_t::digitalWrite(uint8_t pin, uint8_t value) {
    // Implementación básica de digitalWrite utilizando el sistema de GPIO de Linux
    char path[35];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);

    int gpio_fd = open(path, O_WRONLY);
    if (gpio_fd < 0) {
        perror("Failed to open GPIO value file");
        return;
    }

    if (write(gpio_fd, value ? "1" : "0", 1) < 0) {
        perror("Failed to write to GPIO value file");
    }

    close(gpio_fd);
}


    void EPaper_t::initialize() {
        // Ejemplo de inicialización del ePaper_t
        reset();
        waitUntilIdle();
        sendCommand(0x01);  // POWER_SETTING command
        // Más comandos de inicialización aquí

            if (SCREEN == 213) {
            // Código de inicialización específico para el ePaper_t de 2.13"
        }
    }

    void EPaper_t::sendCommand(uint8_t command) {
        digitalWrite(dc_pin, 0);  // DC a LOW para comandos
        spi.transfer(command);
    }

    void EPaper_t::sendData(uint8_t data) {
        digitalWrite(dc_pin, 1);  // DC a HIGH para datos
        spi.transfer(data);
    }

    void EPaper_t::reset() {
        digitalWrite(rst_pin, 0);  // RST a LOW para reset
        delayMs(200);
        digitalWrite(rst_pin, 1);  // RST a HIGH para salir del reset
        delayMs(200);
    }

    void EPaper_t::waitUntilIdle() {
        // Implementar la espera leyendo el pin BUSY
        char path[35];
        snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", busy_pin);

        int gpio_fd = open(path, O_RDONLY);
        if (gpio_fd < 0) {
            perror("Failed to open GPIO value file");
            return;
        }

        char value;
        do {
            lseek(gpio_fd, 0, SEEK_SET);  // Mover el puntero de archivo al inicio
            read(gpio_fd, &value, 1);
        } while (value == '0');  // Asume que '0' significa ocupado

        close(gpio_fd);
    }

    void EPaper_t::displayFrame(const unsigned char* frame_buffer) {
        // Implementar la visualización del frame
        sendCommand(0x24);  // WRITE_RAM command
        for (int i = 0; i < WIDTH * HEIGHT / 8; i++) {
            sendData(frame_buffer[i]);
        }
        sendCommand(0x22);  // DISPLAY_UPDATE command
        sendData(0xC7);
        sendCommand(0x20);  // TRIGGER_DISPLAY_UPDATE
        waitUntilIdle();
    }

    void EPaper_t::delayMs(unsigned int ms){
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }





}