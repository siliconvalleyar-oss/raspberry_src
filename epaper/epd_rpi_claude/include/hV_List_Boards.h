///
/// @file hV_List_Boards.h
/// @brief Board pin definitions – Raspberry Pi 2W / Pi 4 port
///
/// BCM GPIO pin mapping for EPaper:
///   panelBusy  = GPIO7   (WIRE_RED,    PIN26, CE1)
///   panelDC    = GPIO8   (WIRE_ORANGE, PIN24, CE0)
///   panelReset = GPIO25  (WIRE_YELLOW, PIN22)
///   flashCS    = GPIO22  (WIRE_VIOLET, PIN15)
///   panelCS    = GPIO27  (WIRE_GRAY,   PIN13)
///   MOSI       = GPIO10  (WIRE_BLUE,   PIN19) hardware SPI
///   SCK        = GPIO11  (WIRE_BROWN,  PIN23) hardware SPI
///   MISO       = GPIO9   (WIRE_GREEN,  PIN21) hardware SPI
///
///   LED1       = GPIO20  (debug)
///   LED2       = GPIO6   (debug)
///   BUTTON1    = GPIO16  (active LOW, pull-up)
///   BUTTON2    = GPIO12  (active LOW, pull-up)
///

#include <stdint.h>
#include "hV_Configuration.h"

#ifndef hV_LIST_BOARDS_RELEASE
#define hV_LIST_BOARDS_RELEASE 812

#define NOT_CONNECTED (uint8_t)0xff

#if (USE_EXT_BOARD == BOARD_EXT3)

/// @brief EXT3 board pin configuration structure
struct pins_t
{
    uint8_t panelBusy;   ///< BUSY signal (input)
    uint8_t panelDC;     ///< Data/Command select
    uint8_t panelReset;  ///< Reset (active LOW)
    uint8_t flashCS;     ///< External Flash CS
    uint8_t panelCS;     ///< Panel CS
    uint8_t panelCSS;    ///< Panel CS secondary (large screens)
    uint8_t flashCSS;    ///< Flash CS secondary
    uint8_t touchInt;    ///< Touch interrupt
    uint8_t touchReset;  ///< Touch reset
    uint8_t panelPower;  ///< Optional power control
    uint8_t cardCS;      ///< SD-card CS
    uint8_t cardDetect;  ///< SD-card detect
};

///
/// @brief Raspberry Pi 2W / Pi 4 board configuration
/// @note All numbers are BCM GPIO numbers
///
const pins_t boardRaspberryPi2W =
{
    .panelBusy   = 7,              ///< GPIO7  – WIRE_RED    (PIN26, CE1)
    .panelDC     = 8,              ///< GPIO8  – WIRE_ORANGE (PIN24, CE0)
    .panelReset  = 25,             ///< GPIO25 – WIRE_YELLOW (PIN22)
    .flashCS     = 22,             ///< GPIO22 – WIRE_VIOLET (PIN15)
    .panelCS     = 27,             ///< GPIO27 – WIRE_GRAY   (PIN13)
    .panelCSS    = NOT_CONNECTED,  ///< Not used for 2.66" screen
    .flashCSS    = NOT_CONNECTED,  ///< Not used
    .touchInt    = NOT_CONNECTED,  ///< No touch
    .touchReset  = NOT_CONNECTED,  ///< No touch
    .panelPower  = NOT_CONNECTED,  ///< No external power circuit
    .cardCS      = NOT_CONNECTED,  ///< No SD-card
    .cardDetect  = NOT_CONNECTED,  ///< No SD-card
};

/// Alias
#define boardRaspberryPi4 boardRaspberryPi2W

#endif // USE_EXT_BOARD == BOARD_EXT3

#endif // hV_LIST_BOARDS_RELEASE
