#pragma once
#include <cstdint>


#define RGB565CONVERT(red, green, blue) (uint16_t) (((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3))


/*********************************************************************
* Overview: Some basic colors definitions.
*********************************************************************/



// Colores básicos en formato RGB565
constexpr uint16_t BLACK = 0x0000;
constexpr uint16_t BLUE = 0x001F;
constexpr uint16_t RED = 0xF800;
constexpr uint16_t GREEN = 0x07E0;
constexpr uint16_t CYAN = 0x07FF;
constexpr uint16_t MAGENTA = 0xF81F;
constexpr uint16_t YELLOW = 0xFFE0;
constexpr uint16_t WHITE = 0xFFFF;


/*
#define BLACK               RGB565CONVERT(0,    0,      0)
#define BRIGHTBLUE          RGB565CONVERT(0,    0,      255)
#define BRIGHTGREEN         RGB565CONVERT(0,    255,    0)
#define BRIGHTCYAN          RGB565CONVERT(0,    255,    255)
#define BRIGHTRED           RGB565CONVERT(255,  0,      0)
#define BRIGHTMAGENTA       RGB565CONVERT(255,  0,      255)
#define BRIGHTYELLOW        RGB565CONVERT(255,  255,    0)
#define BLUE                RGB565CONVERT(0,    0,      128)
#define GREEN               RGB565CONVERT(0,    128,    0)
#define CYAN                RGB565CONVERT(0,    128,    128)
#define RED                 RGB565CONVERT(128,  0,      0)
#define MAGENTA             RGB565CONVERT(128,  0,      128)
#define BROWN               RGB565CONVERT(255,  128,    0)
#define LIGHTGRAY           RGB565CONVERT(128,  128,    128)
#define DARKGRAY            RGB565CONVERT(64,   64,     64)
#define LIGHTBLUE           RGB565CONVERT(128,  128,    255)
#define LIGHTGREEN          RGB565CONVERT(128,  255,    128)
#define LIGHTCYAN           RGB565CONVERT(128,  255,    255)
#define LIGHTRED            RGB565CONVERT(255,  128,    128)
#define LIGHTMAGENTA        RGB565CONVERT(255,  128,    255)
#define YELLOW              RGB565CONVERT(255,  255,    128)
#define WHITE               RGB565CONVERT(255,  255,    255)
#define SKY               RGB565CONVERT(200,  255,    255)
#define CLOUD               RGB565CONVERT(254,254,254)
                            
#define GRAY0       	    RGB565CONVERT(224,  224,    224)
#define GRAY1         	    RGB565CONVERT(192,  192,    192)   
#define GRAY2               RGB565CONVERT(160,  160,    160)   
#define GRAY3               RGB565CONVERT(128,  128,    128)
#define GRAY4               RGB565CONVERT(96,   96,     96)
#define GRAY5               RGB565CONVERT(64,   64,     64)
#define GRAY6	            RGB565CONVERT(32,   32,     32)

*/
