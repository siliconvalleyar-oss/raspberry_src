///
/// @file hV_HAL_Peripherals.h
/// @brief Hardware Abstraction Layer for Raspberry Pi (bcm2835)
///
/// @details Ported from Arduino/Energia SDK to bcm2835 for Raspberry Pi 2W / Pi 4
///
/// Pin mapping for EPaper display:
///   WIRE_GRAY   -> Panel_CS   -> GPIO27 (PIN13)
///   WIRE_BLUE   -> MOSI       -> GPIO10 (PIN19)
///   WIRE_BROWN  -> SCK        -> GPIO11 (PIN23)
///   WIRE_GREEN  -> MISO       -> GPIO9  (PIN21)
///   WIRE_VIOLET -> Flash_CS   -> GPIO22 (PIN15)
///   WIRE_YELLOW -> RESET      -> GPIO25 (PIN22)
///   WIRE_ORANGE -> D/C        -> GPIO8  (PIN24, CE0)
///   WIRE_RED    -> BUSY       -> GPIO7  (PIN26, CE1)
///
/// Extra GPIOs:
///   GPIO20 -> LED debug 1
///   GPIO6  -> LED debug 2
///   GPIO16 -> Button 1 (with internal pull-up)
///   GPIO12 -> Button 2 (with internal pull-up)
///

#ifndef hV_HAL_PERIPHERALS_RELEASE
#define hV_HAL_PERIPHERALS_RELEASE 812

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include <bcm2835.h>

// ─── Arduino-compat types ─────────────────────────────────────────────────────
using String = std::string;

#define HIGH    1
#define LOW     0
#define INPUT   BCM2835_GPIO_FSEL_INPT
#define OUTPUT  BCM2835_GPIO_FSEL_OUTP
#define INPUT_PULLUP  0xFE  // custom marker, handled in pinMode()

#define MSBFIRST    1
#define SPI_MODE0   BCM2835_SPI_MODE0

// ─── Serial stub (maps to stdout) ─────────────────────────────────────────────
struct SerialClass {
    void print(const char* s)   { fputs(s, stdout); fflush(stdout); }
    void print(int v)           { printf("%d", v); fflush(stdout); }
    void println(const char* s) { printf("%s\n", s); fflush(stdout); }
    void println(int v)         { printf("%d\n", v); fflush(stdout); }
    void println()              { printf("\n"); fflush(stdout); }
    void println(const String& s) { printf("%s\n", s.c_str()); fflush(stdout); }
    void flush() { fflush(stdout); }
    void print(const String& s)   { fputs(s.c_str(), stdout); fflush(stdout); }
};
extern SerialClass Serial;
#define mySerial Serial

// ─── Arduino pin/time functions ────────────────────────────────────────────────
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int  digitalRead(uint8_t pin);
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);
uint32_t millis();

// Bit manipulation
#define bitRead(val, bit)     (((val) >> (bit)) & 0x01)
#define bitSet(val, bit)      ((val) |=  (1U << (bit)))
#define bitClear(val, bit)    ((val) &= ~(1U << (bit)))

// Arduino map()
long map(long x, long in_min, long in_max, long out_min, long out_max);

// ─── HAL functions ─────────────────────────────────────────────────────────────
void hV_HAL_begin();
void waitFor(uint8_t pin, uint8_t state = HIGH);

// 4-wire SPI
void    hV_HAL_SPI_begin(uint32_t speed = 8000000);
void    hV_HAL_SPI_end();
uint8_t hV_HAL_SPI_transfer(uint8_t data);

// 3-wire SPI (bit-bang)
void    hV_HAL_SPI3_begin();
void    hV_HAL_SPI3_define(uint8_t pinClock, uint8_t pinData);
uint8_t hV_HAL_SPI3_read();
void    hV_HAL_SPI3_write(uint8_t value);

// I2C (Wire stub — not used for basic EPD)
void hV_HAL_Wire_begin();
void hV_HAL_Wire_end();
void hV_HAL_Wire_transfer(uint8_t address, uint8_t* dataWrite, size_t sizeWrite,
                           uint8_t* dataRead = nullptr, size_t sizeRead = 0);

// ─── Utility macros ────────────────────────────────────────────────────────────
#define hV_HAL_min(a, b)  ((a) < (b) ? (a) : (b))
#define hV_HAL_max(a, b)  ((a) > (b) ? (a) : (b))
#define hV_HAL_swap(x, y) do { __typeof__(x) _W = (x); (x) = (y); (y) = _W; } while (0)

// ─── Debug LEDs and Buttons ───────────────────────────────────────────────────
#define GPIO_LED1    20   ///< Debug LED 1
#define GPIO_LED2     6   ///< Debug LED 2
#define GPIO_BUTTON1 16   ///< Push-button 1 (active LOW, pull-up)
#define GPIO_BUTTON2 12   ///< Push-button 2 (active LOW, pull-up)

void debug_leds_init();
void debug_led1_set(bool on);
void debug_led2_set(bool on);
bool debug_button1_pressed();
bool debug_button2_pressed();

#endif // hV_HAL_PERIPHERALS_RELEASE
