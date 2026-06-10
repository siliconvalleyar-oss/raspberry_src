#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "HardwareProfile.h"
#include "fonts.h"
#include "Sprite.h"   // <-- AÑADIR ESTA LÍNEA

namespace Graphics {
    void fill_screen(uint16_t color);
    void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void draw_pixel(int16_t x, int16_t y, uint16_t color);
    void draw_hline(int16_t x, int16_t y, int16_t len, uint16_t color);
    void draw_vline(int16_t x, int16_t y, int16_t len, uint16_t color);
    void fill_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
    void draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
    void draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale);
    void draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale);
    
	void drawPNG(int x, int y, const char* filename, uint16_t bgColor = BLACK);
    void drawSprite(int x, int y, const Sprite& sprite, uint16_t bgColor);
	//void drawPNG(int x, int y, const char* filename, uint16_t bgColor = BLACK);
}

#endif
