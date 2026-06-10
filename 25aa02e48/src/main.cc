#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "main.h"
#include <iostream>
#define SPI_DEVICE "/dev/spidev0.0"
#define SPI_BITS_PER_WORD 8

int main(int argc, char *argv[]) {
struct spi_ioc_transfer spi;
    int fd;
    unsigned char tx_buf[4];
    unsigned char rx_buf[4];


uint16_t tx_buffer[255];
uint16_t rx_buffer[255];

spi.tx_buf = (unsigned long)tx_buffer;
spi.rx_buf = (unsigned long)rx_buffer;
spi.bits_per_word = SPI_BITS_PER_WORD;
spi.speed_hz = 1000000;
spi.delay_usecs = 0;
spi.len = 3;//despues se modifica

spi.cs_change = 0;

    // Abrir el dispositivo SPI
    if ((fd = open(SPI_DEVICE, O_RDWR)) < 0) {
        perror("Error al abrir el dispositivo SPI");
        exit(1);
    }

    // Configurar los parámetros SPI
    int mode = SPI_MODE_0;
    int bits = 8;
    int speed = 1000000; // 1 MHz

for(int i =0;i<255;++i){
	rx_buffer[i]=0xff;
	tx_buffer[i]=0xff;
}

    if (
	ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0 ||
	ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0 ||
        ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Error al configurar los parámetros SPI");
        exit(1);
    }

//std::cout<<"fd : "<< fd <<std::endl;
/*
    // Escribir datos en la memoria EEPROM
    tx_buf[0] = READ; // Comando de escritura
    tx_buf[1] = 0xFC; // Dirección de memoria (byte alto)
    tx_buf[2] = 0x00; // Datos a escribir
*/
/*
  if (write(fd, tx_buf, sizeof(tx_buf)) != sizeof(tx_buf)) {
        perror("Error al escribir en la memoria EEPROM");
        exit(1);
    }
*/
    // Esperar un tiempo para que se complete la escritura
    usleep(10000); // 10 ms

    // Leer datos de la memoria EEPROM
    //tx_buf[0] = RDSR; // Comando de lectura
    tx_buf[0] = READ; // Comando de lectura
    tx_buf[1] = 0xFC; // Dirección de memoria (byte alto)
    tx_buf[2] = 0x00; // Bytes a leer

 /*   if (write(fd, tx_buf, sizeof(tx_buf)) != sizeof(tx_buf)) {
        perror("Error al enviar el comando de lectura\n");
        exit(1);
    }
*/
/*
if (read(fd, rx_buf, sizeof(rx_buf)) != sizeof(rx_buf)){
        perror("Error al leer los datos de la memoria EEPROM\n");
        exit(1);
    }
*/
int ret =ioctl(fd,SPI_IOC_MESSAGE(1),&spi);

if(ret!=0){
	perror("Error al leer los datos de la memoria EEPROM\n");
        exit(1);
    }


//printf ("SPI_IOC_MESSAGE(0) : 0x%x\n ",SPI_IOC_MESSAGE(0)); 

// Mostrar los datos leídos
    printf("Datos leídos: 0x%02X 0x%02X 0x%02X\n",  rx_buf[0], rx_buf[1], rx_buf[2]);
    printf("Datos RX Buffer : 0x%02X 0x%02X 0x%02X \n", rx_buffer[0], rx_buffer[1], rx_buffer[2]);

for(int i =0;i<255;++i)if(rx_buffer[i]!=0xff)std::cout <<"buffer ["<<i<<"] : 0x"<<std::hex<<rx_buffer[i]<<std::endl;


// Cerrar el dispositivo SPI
    close(fd);

    return 0;
}
