// ============================================================
//  Graphics.cpp — Raspberry Pi
//  Idéntico en lógica al PIC32, pero fill_rect usa push_color_n
//  que envía N píxeles en un solo bloque SPI (mucho más rápido).
// ============================================================

#include "../include/Graphics.h"
#include "../include/HardwareProfile.h"

// ============================================================
//  PRIMITIVAS
// ============================================================

void Graphics::fill_screen(uint16_t color) {
    set_window(0, 0, TFT_W-1, TFT_H-1);
    push_color_n(color, (uint32_t)TFT_W * TFT_H);
}

void Graphics::fill_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                          uint16_t color) {
    if(x>=(int16_t)TFT_W || y>=(int16_t)TFT_H || w<=0 || h<=0) return;
    if(x<0){ w+=x; x=0; }
    if(y<0){ h+=y; y=0; }
    if(x+w>(int16_t)TFT_W) w = TFT_W - x;
    if(y+h>(int16_t)TFT_H) h = TFT_H - y;
    set_window(x, y, x+w-1, y+h-1);
    push_color_n(color, (uint32_t)w * h);
}

void Graphics::draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if((uint16_t)x >= TFT_W || (uint16_t)y >= TFT_H) return;
    set_window(x, y, x, y);
    push_color(color);
}

void Graphics::draw_hline(int16_t x, int16_t y, int16_t len, uint16_t c) {
    fill_rect(x, y, len, 1, c);
}
void Graphics::draw_vline(int16_t x, int16_t y, int16_t len, uint16_t c) {
    fill_rect(x, y, 1, len, c);
}

// sqrt entera rápida
static int16_t isqrt(int32_t v) {
    if(v <= 0) return 0;
    int16_t x = (int16_t)(v < 65536L ? (int16_t)v : 256);
    for(int i=0;i<8;i++) { int16_t nx=(int16_t)((x + v/x)>>1); if(nx>=x) break; x=nx; }
    while((int32_t)x*x > v) x--;
    return x;
}

void Graphics::fill_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    for(int16_t dy=-r; dy<=r; dy++) {
        int16_t dx = isqrt((int32_t)r*r - (int32_t)dy*dy);
        fill_rect(cx-dx, cy+dy, 2*dx+1, 1, color);
    }
}

void Graphics::draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    int16_t x=0, y=r, d=3-2*r;
    while(x<=y) {
        draw_pixel(cx+x,cy+y,color); draw_pixel(cx-x,cy+y,color);
        draw_pixel(cx+x,cy-y,color); draw_pixel(cx-x,cy-y,color);
        draw_pixel(cx+y,cy+x,color); draw_pixel(cx-y,cy+x,color);
        draw_pixel(cx+y,cy-x,color); draw_pixel(cx-y,cy-x,color);
        if(d<0) d+=4*x+6; else { d+=4*(x-y)+10; y--; }
        x++;
    }
}

// ============================================================
//  TEXTO
// ============================================================
void Graphics::draw_char(int16_t x, int16_t y, char c,
                          uint16_t fg, uint16_t bg, uint8_t scale) {
    if(c < 0x20 || c > 0x7E) c = ' ';
    uint8_t idx = c - 0x20;
    for(uint8_t col=0; col<5; col++) {
        uint8_t line = font5x7[idx][col];
        for(uint8_t row=0; row<7; row++) {
            fill_rect(x+col*scale, y+row*scale, scale, scale,
                      (line&(1<<row)) ? fg : bg);
        }
    }
}

void Graphics::draw_string(int16_t x, int16_t y, const char *s,
                             uint16_t fg, uint16_t bg, uint8_t scale) {
    while(*s) { draw_char(x, y, *s++, fg, bg, scale); x += 6*scale; }
}

// ============================================================
//  PAC-MAN
// ============================================================
void Graphics::draw_pacman(int16_t x, int16_t y, bool mouth_open, uint8_t dir) {
    draw_pacman_scaled(x + PACMAN_R, y + PACMAN_R, mouth_open, dir, 1);
}

void Graphics::draw_pacman_scaled(int16_t cx, int16_t cy, bool mouth_open,
                                   uint8_t dir, uint8_t sc) {
    int16_t r = PACMAN_R * sc;

    // 1) Borrar bounding box completo (1 operación SPI)
    fill_rect(cx-r, cy-r, 2*r+1, 2*r+1, BLACK);

    // 2) Cuerpo circular (scanline a scanline)
    for(int16_t dy=-r; dy<=r; dy++) {
        int16_t dx = isqrt((int32_t)r*r - (int32_t)dy*dy);
        fill_rect(cx-dx, cy+dy, 2*dx+1, 1, YELLOW);
    }

    // 3) Boca triangular según dirección
    if(mouth_open) {
        for(int16_t i=0; i<=r; i++) {
            int16_t w = (int16_t)((int32_t)i*7/10) + 1;
            switch(dir) {
                case 3: fill_rect(cx+i-1,  cy-w,   1, 2*w+1, BLACK); break;
                case 2: fill_rect(cx-i,    cy-w,   1, 2*w+1, BLACK); break;
                case 0: fill_rect(cx-w,    cy-i,   2*w+1, 1, BLACK); break;
                case 1: fill_rect(cx-w,    cy+i-1, 2*w+1, 1, BLACK); break;
            }
        }
    }

    // 4) Ojo
    int16_t er = (sc<=1) ? 1 : sc;
    fill_circle(cx+r/3, cy-r/2, er, BLACK);
}

// ============================================================
//  FANTASMA
// ============================================================
void Graphics::draw_ghost(int16_t x, int16_t y, uint16_t color, bool frightened) {
    draw_ghost_scaled(x+GHOST_SIZE/2, y+GHOST_SIZE/2, color, frightened, 1);
}

void Graphics::draw_ghost_scaled(int16_t cx, int16_t cy, uint16_t color,
                                   bool frightened, uint8_t sc) {
    uint16_t bodyColor = frightened ? COLOR565(0,0,200) : color;
    int16_t r   = (GHOST_SIZE/2) * sc;
    int16_t bot = cy + r;

    // 1) Borrar bounding box
    fill_rect(cx-r, cy-r, 2*r+1, 2*r+1, BLACK);

    // 2) Cúpula superior semicircular
    for(int16_t dy=-r; dy<=0; dy++) {
        int16_t dx = isqrt((int32_t)r*r - (int32_t)dy*dy);
        fill_rect(cx-dx, cy+dy, 2*dx+1, 1, bodyColor);
    }
    // 3) Cuerpo rectangular
    fill_rect(cx-r, cy, 2*r+1, r, bodyColor);

    // 4) Faldón dentado (3 huecos triangulares)
    int16_t dw  = (2*r) / 3;
    int16_t dh  = r/3 + 1;
    int16_t sx  = cx - r;
    for(int8_t t=0; t<3; t++)
        fill_rect(sx + t*dw + dw/2, bot-dh, dw/2+1, dh+1, BLACK);

    // 5) Ojos / expresión
    if(!frightened) {
        int16_t ew  = (sc<=1) ? 2 : 3*sc/2;
        int16_t ex1 = cx - r/3 - ew/2;
        int16_t ex2 = cx + r/3 - ew/2;
        int16_t ey  = cy - r/3;
        fill_rect(ex1, ey, ew, ew, WHITE);
        fill_rect(ex2, ey, ew, ew, WHITE);
        fill_rect(ex1+ew/2, ey+ew/2, ew/2+1, ew/2+1, BLUE);
        fill_rect(ex2+ew/2, ey+ew/2, ew/2+1, ew/2+1, BLUE);
    } else {
        int16_t ew  = sc+1;
        int16_t ex1 = cx - r/3;
        int16_t ex2 = cx + r/3;
        int16_t ey  = cy - r/4;
        for(int16_t k=0; k<ew; k++) {
            draw_pixel(ex1-k, ey+k, WHITE); draw_pixel(ex1+k, ey+k, WHITE);
            draw_pixel(ex2-k, ey+k, WHITE); draw_pixel(ex2+k, ey+k, WHITE);
        }
        for(int16_t bx=cx-r+1; bx<cx+r; bx++)
            draw_pixel(bx, cy+r/3+((bx/2)&1), WHITE);
    }
}

// ============================================================
//  LABERINTO
// ============================================================
void Graphics::draw_dot(int16_t px, int16_t py) {
    fill_rect(px+CELL_W/2-1, py+CELL_H/2-1, 3, 3, DOT_COLOR);
}

void Graphics::draw_power(int16_t px, int16_t py) {
    fill_circle(px+CELL_W/2, py+CELL_H/2, 4, POWER_COLOR);
}

void Graphics::draw_wall_cell(int16_t px, int16_t py,
                               bool top, bool bot, bool lft, bool rgt) {
    fill_rect(px, py, CELL_W, CELL_H, WALL_COLOR);
    if(!top) draw_hline(px,          py,          CELL_W, WALL_EDGE);
    if(!bot) draw_hline(px,          py+CELL_H-1, CELL_W, WALL_EDGE);
    if(!lft) draw_vline(px,          py,          CELL_H, WALL_EDGE);
    if(!rgt) draw_vline(px+CELL_W-1, py,          CELL_H, WALL_EDGE);
}
