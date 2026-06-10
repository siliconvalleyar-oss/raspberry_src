//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   spi.h
//          License             :   GNU 
//          Author              :   Lio
//          Change History      :
//          Processor           :   ARM
//          Hardware            :		
//          Complier            :   ARM
//          Company             :
//          Dependencies        :
//          Description         :   Librarie SPI
//          brief               :	
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
//#ifdef __linux__
#include <features.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
//#endif

#include <vector>
#include <memory>

#define SPI_SPEED   1600000 

#define     STATUS_BUSY         0x1 
    
#define     SECTOR_SIZE_4       4
#define     SECTOR_SIZE_8       8
#define     LARGE_SECTOR_SIZE   256 // Tamaño de un sector grande (puede ser modificado)


namespace SPI{
  
  struct  Spi_t{

explicit  Spi_t();
          ~Spi_t();

    void init();
    void settings_spi();
    void spi_close();
    const uint8_t Transfer1bytes(const uint8_t cmd);
    const uint8_t Transfer2bytes(const uint16_t address);
    const uint8_t Transfer3bytes(const uint32_t address);
    void printDBGSpi();
    void msj_fail();  
    uint32_t get_spi_speed();
  private:
    //[[maybe_unused]] uint8_t m_tx_buffer[LARGE_SECTOR_SIZE]{ 0 };
    //[[maybe_unused]] uint8_t m_rx_buffer[LARGE_SECTOR_SIZE]{ 0 };
    uint8_t m_tx_buffer[LARGE_SECTOR_SIZE];
    uint8_t m_rx_buffer[LARGE_SECTOR_SIZE];

    const uint32_t m_len_data  { 32 };
    //const uint32_t m_spi_speed { 0 } ;
    const uint32_t m_spi_speed {};
    int fs{0};
    int ret{0};
        
    uint8_t looper{0};
    uint32_t scratch32{0};
    std::unique_ptr<struct spi_ioc_transfer >spi{nullptr} ;
    
  };

}//END namespace SPI_H
