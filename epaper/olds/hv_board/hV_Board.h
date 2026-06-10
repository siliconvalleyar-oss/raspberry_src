    
#pragma once 
    
#include <iostream>
#include <gpio/gpio.h>
#include <epaper/epaper.h>
#include <spi/spi.h>
#include <memory>

//#define     FSM_GPIO_MASK    0
//#define     NOT_CONNECTED    0

#define FILM_C 'C'
#define FILM_H 'H'
#define FILM_E 'E'
#define FILM_F 'F'
#define FILM_G 'G'
#define FILM_J 'J'



//#define FAMILY_SMALL 0
//#define MODE_MANUAL 0
//#define SCOPE_NONE 0
//#define MODE_AUTO 0  
//#define SCOPE_GPIO_ONLY 0

#if (FONT_MODE == USE_FONT_TERMINAL)
#define MAX_FONT_SIZE 4
#else
#define MAX_FONT_SIZE 64
#endif

#define SCREEN_SIZE(X) ((uint16_t)((X >> 16) & 0x0fff)) ///< Get size
#define SCREEN_FILM(X) ((uint8_t)((X >> 8) & 0xff)) ///< Get family
#define SCREEN_DRIVER(X) ((uint8_t)(X & 0xff)) ///< Get type
#define SCREEN_EXTRA(X) ((uint8_t)((X >> 28) & 0x0f)) ///< Get extra

#define SIZE_213 213 ///< 2.13"
#define SIZE_266 266 ///< 2.66"

//#define FSM_OFF 0
//#define OUTPUT "out"
//#define INPUT "in"

namespace EPAPER{

struct pins_t;
//struct SPI::Spi_t;
//struct GPIO::Gpio_t;

//uint8_t register_data[12];

//[[maybe_unused]] const uint8_t register_data_mid[] = { 0x00, 0x0e, 0x19, 0x02, 0x0f, 0x89 };	// 4.2"
//[[maybe_unused]] const uint8_t register_data_sm[] = { 0x00, 0x0e, 0x19, 0x02, 0xcf, 0x8d };	// other sizes



struct font_s
{
    uint8_t kind; ///< font description
    uint8_t height; ///< general height in pixels
    uint8_t maxWidth; ///< maximum width in pixels from *width array
    uint8_t first; ///< number of first character, usually 32
    uint8_t number; ///< number of characters, usually 96 or 224
};


    struct hV_Board_t:public GPIO::Gpio_t {
    public :
            
            explicit hV_Board_t(uint32_t , pins_t );
            ~hV_Board_t ()  =   default;

            void        COG_SmallCJ_initial();
            void        COG_SmallCJ_powerOff();
            void        COG_SmallCJ_update();
            void        COG_SmallCJ_sendImageData();
            void        COG_powerOff();
            void        COG_SmallCJ_getDataOTP();
            void        COG_SmallCJ_reset();
            void        s_getDataOTP();
            

//protected:
            void        b_waitBusy(){}
            void        b_waitBusy(bool);
            void        b_sendCommand8( uint8_t );
            void        b_sendCommandData8(uint8_t , uint8_t );
            void        b_sendIndexFixedSelect(uint8_t , uint8_t , uint32_t, uint8_t );
            void        b_sendIndexData(uint8_t , const uint8_t * , uint32_t );
            void        b_sendIndexFixed(uint8_t , const uint8_t * , uint32_t );
            void        b_select(const uint8_t){return ;}
            void        b_begin( pins_t& ,  uint8_t ,const  uint16_t );
            void        b_resume();
            void        b_reset(uint32_t, uint32_t , uint32_t , uint32_t , uint32_t );
            void        suspend(uint8_t );
            void        debugVariant(uint8_t){}
            void        begin();
            void        setTemperatureC(int8_t );
            void        resume();
            void        setPowerProfile(uint8_t , uint8_t );

            uint8_t     f_fontMax();
            void        f_selectFont(uint8_t );
            void        setOrientation(uint8_t );
            void        s_setOrientation(uint8_t);
            void        s_flush(uint8_t );
            uint16_t    screenSizeX();
            uint16_t    screenSizeY();
            uint8_t     hV_HAL_SPI_transfer(uint8_t);

        private:
            uint8_t     b_fsmPowerScreen;
            pins_t      b_pin;
            [[maybe_unused]] uint8_t b_family ;
            [[maybe_unused]] uint16_t b_delayCS ;
            
            uint8_t COG_data[128];
            int8_t u_temperature {25};
            bool        u_flagOTP;
            uint8_t     blackBuffer         {0};
            uint8_t     u_pageColourSize    {0};                                    
            bool        enable;
            uint8_t *   s_newImage;            
            std::unique_ptr <SPI::Spi_t>spi_ptr;
            uint16_t u_codeSize;
            uint16_t u_codeFilm;
            uint16_t u_codeDriver;
            uint16_t u_codeExtra;
            uint16_t v_screenColourBits;
            uint32_t u_eScreen_EPD;        
            uint16_t v_screenSizeV ;
            uint16_t v_screenSizeH ;
            uint16_t v_screenDiagonal ;
            uint16_t u_bufferDepth ;
            uint16_t u_bufferSizeV; 
            uint16_t u_bufferSizeH;         
            uint8_t u_suspendMode;
            uint8_t u_suspendScope;        
            font_s f_font; 
            uint8_t f_fontSize;
            uint8_t v_orientation;
            bool f_fontSolid ;
            bool v_penSolid ;
            bool u_invert ;            
    };
}