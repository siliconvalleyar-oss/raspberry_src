///
/// @file hV_Configuration.h
/// @brief Configuration for Raspberry Pi 2W / Pi 4 port
///
/// Screen: 2.66" EPD, 152 x 296 pixels, Film P (embedded fast update)
///
/// Pin mapping (BCM GPIO numbers):
///   panelBusy  = GPIO7   (WIRE_RED,    PIN26)
///   panelDC    = GPIO8   (WIRE_ORANGE, PIN24)
///   panelReset = GPIO25  (WIRE_YELLOW, PIN22)
///   flashCS    = GPIO22  (WIRE_VIOLET, PIN15)
///   panelCS    = GPIO27  (WIRE_GRAY,   PIN13)
///   SPI MOSI   = GPIO10  (WIRE_BLUE,   PIN19) – hardware SPI
///   SPI SCK    = GPIO11  (WIRE_BROWN,  PIN23) – hardware SPI
///   SPI MISO   = GPIO9   (WIRE_GREEN,  PIN21) – hardware SPI
///   LED1       = GPIO20
///   LED2       = GPIO6
///   Button1    = GPIO16
///   Button2    = GPIO12
///

#ifndef hV_CONFIGURATION_RELEASE
#define hV_CONFIGURATION_RELEASE 812

// ─── Board selection ──────────────────────────────────────────────────────────
#define BOARD_EXT3  1
#define BOARD_EXT4  2
#define USE_EXT_BOARD BOARD_EXT3

// ─── Screen configuration ─────────────────────────────────────────────────────
// Use FONT_TERMINAL (no external Flash memory needed)
#define FONT_MODE   USE_FONT_TERMINAL
#define USE_FONT_TERMINAL   1
#define USE_FONT_HEADER     2
#define USE_FONT_FLASH      3

// Maximum font size available:
// 0=6x8, 1=8x12, 2=12x16, 3=16x24
#define MAX_FONT_SIZE 4

// SRAM: use internal MCU memory
#define SRAM_MODE       USE_INTERNAL_MCU
#define USE_INTERNAL_MCU    1
#define USE_EXTERNAL_SPI    2

// Colour depth (EPD black/white = 1 bit per pixel in this driver)
#define SCREEN_EPD_COLOUR_BITS 1

// ─── Colour definitions (RGB565 compatible values used as tokens) ──────────────
#include <stdint.h>

struct Colours565
{
    const uint16_t black   = 0x0000;
    const uint16_t white   = 0xffff;
    const uint16_t grey    = 0x7bef;
    const uint16_t red     = 0xf800;
    const uint16_t green   = 0x07e0;
    const uint16_t blue    = 0x001f;
    const uint16_t yellow  = 0xffe0;
    const uint16_t orange  = 0xfd20;
    const uint16_t darkRed = 0x8000;
    const uint16_t clear   = 0xffff;
};

extern Colours565 myColours;

// ─── Frame-buffer type ────────────────────────────────────────────────────────
typedef uint8_t* FRAMEBUFFER_TYPE;

// ─── String type ──────────────────────────────────────────────────────────────
#include <string>
typedef std::string STRING_TYPE;
typedef const std::string& STRING_CONST_TYPE;

// ─── Power profile defaults ───────────────────────────────────────────────────
// These are defined in hV_List_Constants.h but repeated here for clarity

#endif // hV_CONFIGURATION_RELEASE
