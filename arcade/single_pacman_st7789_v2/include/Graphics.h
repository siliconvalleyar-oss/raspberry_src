#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "HardwareProfile.h"
#include "fonts.h"

namespace Graphics {

    // ---- Primitivas ----
    void fill_screen(uint16_t color);
    void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void draw_pixel(int16_t x, int16_t y, uint16_t color);
    void draw_hline(int16_t x, int16_t y, int16_t len, uint16_t color);
    void draw_vline(int16_t x, int16_t y, int16_t len, uint16_t color);
    void fill_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
    void draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color);

    // ---- Texto ----
    void draw_char  (int16_t x, int16_t y, char c,
                     uint16_t fg, uint16_t bg, uint8_t scale);
    void draw_string(int16_t x, int16_t y, const char *s,
                     uint16_t fg, uint16_t bg, uint8_t scale);

    // ---- Sprites de juego ----
    void draw_pacman(int16_t x, int16_t y, bool mouth_open, uint8_t dir);
    void draw_pacman_scaled(int16_t cx, int16_t cy, bool mouth_open,
                            uint8_t dir, uint8_t scale);

    void draw_ghost(int16_t x, int16_t y, uint16_t color, bool frightened);
    void draw_ghost_scaled(int16_t cx, int16_t cy, uint16_t color,
                           bool frightened, uint8_t scale);

    // ---- Laberinto ----
    void draw_dot  (int16_t cellPx, int16_t cellPy);
    void draw_power(int16_t cellPx, int16_t cellPy);
    void draw_wall_cell(int16_t px, int16_t py,
                        bool top, bool bot, bool lft, bool rgt);
}

#endif
