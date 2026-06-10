#pragma once

#include <cstdint>

#define SPI_CH0_PORT0 "/dev/spidev0.0"
#define SPI_CH0_PORT1 "/dev/spidev0.1"


namespace SPI{

    struct SPI_t {
    public:
        SPI_t();
        ~SPI_t();
        int begin(const char*,uint32_t);
        void transfer(uint8_t data);
        void end();
    private:
        int fd;
    };

}


