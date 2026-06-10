#include <iostream>
#include <memory>
#include <epaper/epaper.h>
#include <epaper/boards.h>
//#define SCREEN 213 //or 266
#define SCREEN 266
#include <graphics/userImageData.h>
#include <tyme/tyme.h>
#include <app/config.h>


void function(uint8_t* buff){

    const size_t bufferSize = 5624;


    // Llenar el buffer con el valor 0xff
    memset(buff, 0x00, bufferSize);

    // Para verificar que el buffer se llenó correctamente (opcional)
    for (size_t i = 0; i < 10; ++i) { // Imprimir los primeros 10 bytes
        //std::cout << std::hex << static_cast<int>(buff[i]) << " ";
    }
}


int main(){
    
    //std::unique_ptr<EPAPER::EPD_Driver>epaper {std::make_unique<EPAPER::EPD_Driver>(eScreen_EPD_213, EPAPER::boardRaspberryPiZero2W)};
    #ifdef CPU_32_BITS
        auto epaper {std::make_unique<EPAPER::EPD_Driver>(eScreen_EPD_266, EPAPER::boardRaspberryPiZero2W)};
    #else
        auto epaper {std::make_unique<EPAPER::EPD_Driver>(eScreen_EPD_266, EPAPER::boardRaspberryPi)};
        //auto eppaper {std::make_unique<EPAPER::hV_Board_t>(eScreen_EPD_266, EPAPER::boardRaspberryPi)};
    #endif
    
        epaper->COG_initial();
        epaper->printGpios();
        
        // Global Update Call
        // void EPD_Driver::globalUpdate(const uint8_t * data1s, const uint8_t * data2s)
        epaper->globalUpdate( BW_QrBuffer, BW_0x00Buffer);
        TYME::delay(1300);
        epaper->globalUpdate( BW_monoBuffer, BW_0x00Buffer);
        TYME::delay(300);

        //uint8_t buffer[5624];
        //function(buffer);
        //epaper->globalUpdate( buffer,buffer );

        TYME::delay(10);
        epaper->globalUpdate(BWR_blackBuffer,BWR_redBuffer);
        epaper->COG_powerOff();
        

    return 0;
}