#ifndef DINO_GRAPHICS_H
#define DINO_GRAPHICS_H

#include "DinoHardware.h"
#include "fonts.h"

// ---- Modo de color ----
extern bool g_color_mode;   // true=color, false=blanco y negro
uint16_t gc(uint16_t color_val, uint16_t gray_val);  // elige segun modo

namespace DG {   // DinoGraphics namespace

    // Primitivas
    void fill_screen (uint16_t color);
    void fill_rect   (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void draw_pixel  (int16_t x, int16_t y, uint16_t color);
    void draw_hline  (int16_t x, int16_t y, int16_t len, uint16_t color);
    void draw_vline  (int16_t x, int16_t y, int16_t len, uint16_t color);
    void fill_circle (int16_t cx, int16_t cy, int16_t r, uint16_t color);

    // Texto bitmap 5x7
    void draw_char  (int16_t x, int16_t y, char c,
                     uint16_t fg, uint16_t bg, uint8_t sc);
    void draw_string(int16_t x, int16_t y, const char *s,
                     uint16_t fg, uint16_t bg, uint8_t sc);
    int  text_width (const char *s, uint8_t sc);

    // Sprites del juego
    void draw_dino  (int16_t x, int16_t y, uint8_t frame, bool dead);
    void draw_cactus(int16_t x, int16_t y, uint8_t type);
    void draw_ptero (int16_t x, int16_t y, uint8_t frame);
    void draw_rock  (int16_t x, int16_t y);
    void draw_cloud (int16_t x, int16_t y);
    void draw_moon  (int16_t x, int16_t y, uint8_t phase);
    void draw_star  (int16_t x, int16_t y);
    void draw_shield(int16_t x, int16_t y); // power-up escudo
    void draw_ground(int16_t y, uint32_t scroll_offset);

    // HUD
    void draw_hud(uint32_t score, uint32_t hi, uint8_t lives,
                  uint8_t level, bool color_mode);
    void draw_title(void);
    void draw_gameover(uint32_t score, uint32_t hi);
    void draw_level_banner(uint8_t level);
}

#endif
