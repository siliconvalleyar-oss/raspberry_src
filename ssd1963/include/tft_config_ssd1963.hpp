#pragma once
#include <cstdint>

static constexpr uint16_t LCD_WIDTH = 480;
static constexpr uint16_t LCD_HEIGHT = 272;


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

//constexpr uint32_t LCD_FPR = 0x01E848;
