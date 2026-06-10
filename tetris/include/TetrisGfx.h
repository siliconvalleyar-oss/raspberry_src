#ifndef TETRIS_GFX_H
#define TETRIS_GFX_H

#include "Hardware.h"
#include "fonts.h"
#include <stdint.h>

// ============================================================
//  LAYOUT DE PANTALLA  240x240
//
//   0          239
//   ┌──────────────┐  0
//   │  HUD (score) │  (18px alto)
//   ├──┬───────┬───┤  18
//   │  │       │N  │
//   │  │ CAMPO │E  │
//   │  │10x20  │X  │
//   │  │       │T  │
//   │  │       │   │
//   └──┴───────┴───┘  240
//
//  Campo: 10 columnas x 20 filas, cada bloque = 11px
//  → campo = 110px ancho x 220px alto
//  Posicion X del campo: (240-110-52)/2 = 39px  → se centra con panel derecho
//  Panel derecho (52px): NEXT, HOLD, NIVEL, LINEAS
// ============================================================

// Bloque en pixeles
#define BLOCK_PX    11

// Campo de juego
#define FIELD_COLS  10
#define FIELD_ROWS  20
#define FIELD_W     (FIELD_COLS * BLOCK_PX)   // 110
#define FIELD_H     (FIELD_ROWS * BLOCK_PX)   // 220

// Posicion del campo en pantalla
#define FIELD_X     2
#define FIELD_Y     20    // debajo del HUD

// Panel derecho
#define PANEL_X     (FIELD_X + FIELD_W + 4)   // 116
#define PANEL_W     (TFT_W - PANEL_X - 1)     // 123

// HUD
#define HUD_H       19

// Colores de las 7 tetrominos (I O T S Z J L) + phantom + borde
extern const uint16_t PIECE_COLORS[9];
#define COLOR_EMPTY  BLACK
#define COLOR_BORDER COLOR565(40,40,60)
#define COLOR_GRID   COLOR565(20,20,35)
#define COLOR_PANEL  COLOR565(10,10,25)
#define COLOR_ACCENT COLOR565(0,180,255)

namespace TGfx {
    // Primitivas
    void fill_screen(uint16_t c);
    void fill_rect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t c);
    void draw_pixel(int16_t x,int16_t y,uint16_t c);
    void draw_hline(int16_t x,int16_t y,int16_t len,uint16_t c);
    void draw_vline(int16_t x,int16_t y,int16_t len,uint16_t c);
    void draw_rect (int16_t x,int16_t y,int16_t w,int16_t h,uint16_t c);

    // Texto
    void draw_char  (int16_t x,int16_t y,char ch,uint16_t fg,uint16_t bg,uint8_t sc);
    void draw_string(int16_t x,int16_t y,const char *s,uint16_t fg,uint16_t bg,uint8_t sc);

    // Bloque Tetris (con efecto 3D y brillo)
    void draw_block (int16_t bx,int16_t by,uint16_t color);
    void clear_block(int16_t bx,int16_t by);          // borra a negro+grid
    void draw_ghost_block(int16_t bx,int16_t by,uint16_t color); // fantasma translucido

    // Campo
    void draw_field_frame(void);                       // marco + fondo
    void draw_grid(void);                              // cuadricula
    void redraw_board(const uint8_t board[FIELD_ROWS][FIELD_COLS]); // redibujar todo
    void flash_lines(const int *rows,int n);           // flash antes de borrar
    void clear_line_anim(int row);                     // animacion de linea

    // Panel derecho
    void draw_panel_labels(void);
    void draw_next_piece(uint8_t type,uint8_t rot);
    void draw_hold_piece(uint8_t type,uint8_t rot,bool can_hold);
    void draw_stats(uint32_t score,uint32_t best,uint8_t level,
                    uint16_t lines,uint32_t combo);

    // Pantallas
    void draw_hud(uint32_t score,uint32_t best,uint8_t level,uint16_t lines);
    void draw_title(void);
    void draw_gameover(uint32_t score,uint32_t best,uint8_t level,uint16_t lines);
    void draw_pause(void);
    void draw_level_banner(uint8_t level);
    void draw_tetris_flash(void);
    void draw_tspin_banner(void);
}

#endif
