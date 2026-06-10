#pragma once
#include <cstdint>

// Definición de pines
constexpr uint8_t SSD1963_LCD_D0  = 12;
constexpr uint8_t SSD1963_LCD_D1  = 13;
constexpr uint8_t SSD1963_LCD_D2  = 14;
constexpr uint8_t SSD1963_LCD_D3  = 15;
constexpr uint8_t SSD1963_LCD_D4  = 16;
constexpr uint8_t SSD1963_LCD_D5  = 17;
constexpr uint8_t SSD1963_LCD_D6  = 18;
constexpr uint8_t SSD1963_LCD_D7  = 19;
constexpr uint8_t SSD1963_LCD_D8  = 20;
constexpr uint8_t SSD1963_LCD_D9  = 21;
constexpr uint8_t SSD1963_LCD_D10 = 22;
constexpr uint8_t SSD1963_LCD_D11 = 23;
constexpr uint8_t SSD1963_LCD_D12 = 24;
constexpr uint8_t SSD1963_LCD_D13 = 25;
constexpr uint8_t SSD1963_LCD_D14 = 26;
constexpr uint8_t SSD1963_LCD_D15 = 27;
constexpr uint8_t SSD1963_LCD_WR    = 3;  // Write strobe
constexpr uint8_t SSD1963_LCD_RS    = 4;  // Data/Command selection
constexpr uint8_t SSD1963_LCD_CS    = 5;  // Chip select
constexpr uint8_t SSD1963_LCD_RESET = 6;  // Reset
constexpr uint8_t SSD1963_LCD_BACKLIGHT = 0; // Backlight control
// Dimensiones del display
constexpr unsigned int LCD_WIDTH  = 480;
constexpr unsigned int LCD_HEIGHT = 272;
// Sincronización horizontal
constexpr uint16_t TFT_HSYNC_PULSE        = 41;
constexpr uint16_t TFT_HSYNC_BACK_PORCH   = 2;
constexpr uint16_t TFT_HSYNC_FRONT_PORCH  = 2;
constexpr uint16_t TFT_HSYNC_PERIOD = LCD_WIDTH + TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH + TFT_HSYNC_FRONT_PORCH;
// Sincronización vertical
constexpr uint16_t TFT_VSYNC_PULSE        = 10;
constexpr uint16_t TFT_VSYNC_BACK_PORCH   = 2;
constexpr uint16_t TFT_VSYNC_FRONT_PORCH  = 2;
constexpr uint16_t TFT_VSYNC_PERIOD = LCD_HEIGHT + TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH + TFT_VSYNC_FRONT_PORCH;
// Frecuencia de píxel: este valor se puede ajustar
constexpr uint32_t LCD_FPR = 0x01E848; // 2.0 MHz para SYSCLK = 100MHz


// Macros para control de pines
#define RS_LOW()    bcm2835_gpio_write(SSD1963_LCD_RS, LOW)
#define RS_HIGH()   bcm2835_gpio_write(SSD1963_LCD_RS, HIGH)
#define WR_LOW()    bcm2835_gpio_write(SSD1963_LCD_WR, LOW)
#define WR_HIGH()   bcm2835_gpio_write(SSD1963_LCD_WR, HIGH)
#define CS_LOW()    bcm2835_gpio_write(SSD1963_LCD_CS, LOW)
#define CS_HIGH()   bcm2835_gpio_write(SSD1963_LCD_CS, HIGH)
#define RESET_LOW() bcm2835_gpio_write(SSD1963_LCD_RESET, LOW)
#define RESET_HIGH() bcm2835_gpio_write(SSD1963_LCD_RESET, HIGH)
#define BACKLIGHT_ON() bcm2835_gpio_write(SSD1963_LCD_BACKLIGHT, HIGH)
#define BACKLIGHT_OFF() bcm2835_gpio_write(SSD1963_LCD_BACKLIGHT, LOW)

