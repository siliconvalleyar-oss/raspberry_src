#pragma once

    #if (defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 4))//es de 32 bits?
        #define  CPU_32_BITS
        #else
        #define  CPU_64_BITS
    #endif

#define IN_INTERRUPT    23    //GPIO INTERRUPT 
#define OUT_INTERRUPT   12    //GPIO LED DBG
#define READING_STEPS   2     // originalmente
#define SCREEN 213

namespace CONFIG{

#define eScreen_EPD_213 (uint32_t)0x2100 ///< reference xE2213CSxxx//SELECT

/*
        case 0x21: // 2.13"

            screenSizeV = 212; // vertical = wide size
            screenSizeH = 104; // horizontal = small size
            screenDiagonal = 213;
            refreshTime = 15;
            break;



*/
}