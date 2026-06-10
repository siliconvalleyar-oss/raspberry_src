#include "spi.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>

namespace SPI{

    SPI_t::SPI_t() : fd(-1) {
        std::cout<< "init Spi"<<std::endl;
    }

    int SPI_t::begin(const char* device, uint32_t speed) {
        fd = open(device, O_RDWR);
        if (fd < 0) {
            perror("Failed to open SPI device");
            return -1;
        }

        uint8_t mode = 0;
        uint8_t bits = 8;

        if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
            perror("Failed to set SPI mode");
            close(fd);
            return -1;
        }

        if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
            perror("Failed to set bits per word");
            close(fd);
            return -1;
        }

        if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
            perror("Failed to set max speed");
            close(fd);
            return -1;
        }

        return fd;
    }

    void SPI_t::transfer(uint8_t data) {
        struct spi_ioc_transfer tr{};
        tr.tx_buf = reinterpret_cast<__u64>(&data);
        tr.rx_buf = reinterpret_cast<__u64>(&data);
        tr.len = 1;

        if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
            perror("Failed to send SPI message");
        }
    }

    void SPI_t::end() {
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
    }

    SPI_t::~SPI_t() {
    end();
}


}