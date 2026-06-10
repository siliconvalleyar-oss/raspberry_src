// ============================================================
//  TetrisGfx.cpp — Motor grafico del Tetris Arcade
//  Bloques con efecto 3D, brillo y sombra.
//  Pantalla 240x240, campo 10x20, bloque 11px.
// ============================================================
#include "TetrisGfx.h"
#include "Hardware.h"
#include <stdio.h>
#include <string.h>

// ============================================================
//  COLORES DE LAS 7 PIEZAS  I O T S Z J L  + ghost + borde
// ============================================================
const uint16_t PIECE_COLORS[9] = {
    CYAN,                       // 0 I — cian
    YELLOW,                     // 1 O — amarillo
    PURPLE,                     // 2 T — violeta
    GREEN,                      // 3 S — verde
    RED,                        // 4 Z — rojo
    BLUE,                       // 5 J — azul
    ORANGE,                     // 6 L — naranja
    COLOR565(40,40,40),         // 7 ghost (fantasma)
    WHITE                       // 8 special
};

// ============================================================
//  PRIMITIVAS
// ============================================================
void TGfx::fill_screen(uint16_t c){
    set_window(0,0,TFT_W-1,TFT_H-1);
    push_color_n(c,(uint32_t)TFT_W*TFT_H);
}

void TGfx::fill_rect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t c){
    if(x>=(int16_t)TFT_W||y>=(int16_t)TFT_H||w<=0||h<=0) return;
    if(x<0){w+=x;x=0;} if(y<0){h+=y;y=0;}
    if(x+w>(int16_t)TFT_W) w=TFT_W-x;
    if(y+h>(int16_t)TFT_H) h=TFT_H-y;
    set_window(x,y,x+w-1,y+h-1);
    push_color_n(c,(uint32_t)w*h);
}

void TGfx::draw_pixel(int16_t x,int16_t y,uint16_t c){
    if((uint16_t)x>=TFT_W||(uint16_t)y>=TFT_H) return;
    set_window(x,y,x,y); push_color(c);
}

void TGfx::draw_hline(int16_t x,int16_t y,int16_t len,uint16_t c){
    fill_rect(x,y,len,1,c);
}
void TGfx::draw_vline(int16_t x,int16_t y,int16_t len,uint16_t c){
    fill_rect(x,y,1,len,c);
}
void TGfx::draw_rect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t c){
    draw_hline(x,y,w,c); draw_hline(x,y+h-1,w,c);
    draw_vline(x,y,h,c); draw_vline(x+w-1,y,h,c);
}

// ============================================================
//  TEXTO  (fuente 5x7 de fonts.h)
// ============================================================
void TGfx::draw_char(int16_t x,int16_t y,char ch,
                      uint16_t fg,uint16_t bg,uint8_t sc){
    if(ch<0x20||ch>0x7E) ch=' ';
    uint8_t idx=ch-0x20;
    for(uint8_t col=0;col<5;col++){
        uint8_t line=font5x7[idx][col];
        for(uint8_t row=0;row<7;row++)
            fill_rect(x+col*sc,y+row*sc,sc,sc,(line&(1<<row))?fg:bg);
    }
}

void TGfx::draw_string(int16_t x,int16_t y,const char *s,
                        uint16_t fg,uint16_t bg,uint8_t sc){
    while(*s){ draw_char(x,y,*s++,fg,bg,sc); x+=6*sc; }
}

// ============================================================
//  BLOQUE 3D  (11x11 px)
//  Efecto: cara superior+izq mas clara (highlight)
//          cara inferior+der mas oscura (sombra)
//          interior relleno con color base
// ============================================================

// Aclarar un color RGB565
static uint16_t lighten(uint16_t c, uint8_t amount){
    uint8_t r=(c>>11)&0x1F;
    uint8_t g=(c>>5 )&0x3F;
    uint8_t b=(c>>0 )&0x1F;
    r=(uint8_t)((r+(amount>>3)) > 0x1F ? 0x1F : r+(amount>>3));
    g=(uint8_t)((g+(amount>>2)) > 0x3F ? 0x3F : g+(amount>>2));
    b=(uint8_t)((b+(amount>>3)) > 0x1F ? 0x1F : b+(amount>>3));
    return (uint16_t)((r<<11)|(g<<5)|b);
}

// Oscurecer un color RGB565
static uint16_t darken(uint16_t c, uint8_t amount){
    uint8_t r=(c>>11)&0x1F;
    uint8_t g=(c>>5 )&0x3F;
    uint8_t b=(c>>0 )&0x1F;
    uint8_t sub_r = amount>>3, sub_g = amount>>2, sub_b = amount>>3;
    r=(r>sub_r)?r-sub_r:0;
    g=(g>sub_g)?g-sub_g:0;
    b=(b>sub_b)?b-sub_b:0;
    return (uint16_t)((r<<11)|(g<<5)|b);
}

void TGfx::draw_block(int16_t bx,int16_t by,uint16_t color){
    // bx,by en coordenadas de BLOQUE (no pixel)
    // Convertir a pixel
    int16_t px = FIELD_X + bx * BLOCK_PX;
    int16_t py = FIELD_Y + by * BLOCK_PX;

    uint16_t hi  = lighten(color, 80);   // brillo
    uint16_t mid = color;
    uint16_t lo  = darken (color, 60);   // sombra
    uint16_t sh  = darken (color, 100);  // sombra profunda

    const int S = BLOCK_PX; // 11

    // Fondo base
    fill_rect(px, py, S, S, mid);

    // Highlight: borde superior (2px) e izquierdo (2px)
    draw_hline(px,   py,   S,   hi);
    draw_hline(px,   py+1, S-1, hi);
    draw_vline(px,   py,   S,   hi);
    draw_vline(px+1, py,   S-1, hi);

    // Sombra: borde inferior (2px) y derecho (2px)
    draw_hline(px,   py+S-1, S,   sh);
    draw_hline(px+1, py+S-2, S-1, lo);
    draw_vline(px+S-1, py,   S,   sh);
    draw_vline(px+S-2, py+1, S-1, lo);

    // Brillo interior (punto de luz en esquina sup-izq)
    fill_rect(px+2, py+2, 3, 2, lighten(color, 120));
}

void TGfx::clear_block(int16_t bx,int16_t by){
    int16_t px = FIELD_X + bx * BLOCK_PX;
    int16_t py = FIELD_Y + by * BLOCK_PX;
    fill_rect(px, py, BLOCK_PX, BLOCK_PX, COLOR_EMPTY);
    // Restaurar lineas de cuadricula
    draw_pixel(px,           py,           COLOR_GRID);
    draw_pixel(px+BLOCK_PX-1,py,           COLOR_GRID);
    draw_pixel(px,           py+BLOCK_PX-1,COLOR_GRID);
}

void TGfx::draw_ghost_block(int16_t bx,int16_t by,uint16_t color){
    int16_t px = FIELD_X + bx * BLOCK_PX;
    int16_t py = FIELD_Y + by * BLOCK_PX;
    uint16_t gc = darken(color, 90);
    // Solo el contorno (1px)
    draw_rect(px, py, BLOCK_PX, BLOCK_PX, gc);
    draw_pixel(px+1,py+1,darken(color,70));
}

// ============================================================
//  CAMPO
// ============================================================
void TGfx::draw_field_frame(void){
    // Fondo del campo (negro/azul oscuro)
    fill_rect(FIELD_X, FIELD_Y, FIELD_W, FIELD_H, COLOR_EMPTY);

    // Marco exterior (2px) con acento
    draw_rect(FIELD_X-2, FIELD_Y-2, FIELD_W+4, FIELD_H+4, COLOR_ACCENT);
    draw_rect(FIELD_X-1, FIELD_Y-1, FIELD_W+2, FIELD_H+2, darken(COLOR_ACCENT,40));
}

void TGfx::draw_grid(void){
    // Cuadricula sutil
    for(int col=1;col<FIELD_COLS;col++){
        int16_t x = FIELD_X + col * BLOCK_PX;
        draw_vline(x, FIELD_Y, FIELD_H, COLOR_GRID);
    }
    for(int row=1;row<FIELD_ROWS;row++){
        int16_t y = FIELD_Y + row * BLOCK_PX;
        draw_hline(FIELD_X, y, FIELD_W, COLOR_GRID);
    }
}

void TGfx::redraw_board(const uint8_t board[FIELD_ROWS][FIELD_COLS]){
    for(int r=0;r<FIELD_ROWS;r++){
        for(int c=0;c<FIELD_COLS;c++){
            if(board[r][c]){
                draw_block(c, r, PIECE_COLORS[board[r][c]-1]);
            } else {
                clear_block(c, r);
            }
        }
    }
}

void TGfx::flash_lines(const int *rows, int n){
    // Flash blanco 3 veces
    for(int f=0;f<3;f++){
        for(int i=0;i<n;i++){
            int16_t py = FIELD_Y + rows[i]*BLOCK_PX;
            fill_rect(FIELD_X, py, FIELD_W, BLOCK_PX, (f&1)?WHITE:GOLD);
        }
        delay_ms(60);
    }
}

void TGfx::clear_line_anim(int row){
    // Barrido desde el centro hacia afuera
    int16_t cy = FIELD_Y + row * BLOCK_PX;
    for(int i=FIELD_COLS/2;i>=0;i--){
        int16_t x1 = FIELD_X + i*BLOCK_PX;
        int16_t x2 = FIELD_X + (FIELD_COLS-1-i)*BLOCK_PX;
        fill_rect(x1, cy, BLOCK_PX, BLOCK_PX, COLOR_ACCENT);
        fill_rect(x2, cy, BLOCK_PX, BLOCK_PX, COLOR_ACCENT);
        delay_us(8000);
    }
    fill_rect(FIELD_X, cy, FIELD_W, BLOCK_PX, COLOR_EMPTY);
    // Restaurar grid en esa fila
    draw_hline(FIELD_X, cy, FIELD_W, COLOR_GRID);
}

// ============================================================
//  PANEL DERECHO
// ============================================================

// Dibuja una pieza miniatura en el panel
static void draw_mini_piece(int16_t px,int16_t py,
                              uint8_t type,uint8_t rot,
                              bool ghost){
    // Definicion de las 7 piezas, 4 rotaciones, 4 bloques cada una
    // [tipo][rot][bloque][col,fila]
    static const int8_t SHAPES[7][4][4][2] = {
        // I
        {{{0,1},{1,1},{2,1},{3,1}},{{2,0},{2,1},{2,2},{2,3}},
         {{0,2},{1,2},{2,2},{3,2}},{{1,0},{1,1},{1,2},{1,3}}},
        // O
        {{{0,0},{1,0},{0,1},{1,1}},{{0,0},{1,0},{0,1},{1,1}},
         {{0,0},{1,0},{0,1},{1,1}},{{0,0},{1,0},{0,1},{1,1}}},
        // T
        {{{0,1},{1,1},{2,1},{1,0}},{{1,0},{1,1},{1,2},{2,1}},
         {{0,1},{1,1},{2,1},{1,2}},{{1,0},{1,1},{1,2},{0,1}}},
        // S
        {{{1,0},{2,0},{0,1},{1,1}},{{1,0},{1,1},{2,1},{2,2}},
         {{1,1},{2,1},{0,2},{1,2}},{{0,0},{0,1},{1,1},{1,2}}},
        // Z
        {{{0,0},{1,0},{1,1},{2,1}},{{2,0},{1,1},{2,1},{1,2}},
         {{0,1},{1,1},{1,2},{2,2}},{{1,0},{0,1},{1,1},{0,2}}},
        // J
        {{{0,0},{0,1},{1,1},{2,1}},{{1,0},{2,0},{1,1},{1,2}},
         {{0,1},{1,1},{2,1},{2,2}},{{1,0},{1,1},{0,2},{1,2}}},
        // L
        {{{2,0},{0,1},{1,1},{2,1}},{{1,0},{1,1},{1,2},{2,2}},
         {{0,1},{1,1},{2,1},{0,2}},{{0,0},{1,0},{1,1},{1,2}}},
    };

    const int BS = 6; // mini bloque
    uint16_t c = ghost ? COLOR565(40,40,40) : PIECE_COLORS[type];

    for(int b=0;b<4;b++){
        int8_t bc = SHAPES[type][rot][b][0];
        int8_t br = SHAPES[type][rot][b][1];
        int16_t x = px + bc*BS;
        int16_t y = py + br*BS;
        if(ghost){
            TGfx::draw_rect(x,y,BS,BS,c);
        } else {
            uint16_t hi  = lighten(c,60);
            uint16_t lo  = darken (c,50);
            TGfx::fill_rect(x,y,BS,BS,c);
            TGfx::draw_hline(x,y,BS,hi);
            TGfx::draw_vline(x,y,BS,hi);
            TGfx::draw_hline(x,y+BS-1,BS,lo);
            TGfx::draw_vline(x+BS-1,y,BS,lo);
        }
    }
}

void TGfx::draw_panel_labels(void){
    fill_rect(PANEL_X, 0, PANEL_W, TFT_H, COLOR_PANEL);

    // Lineas decorativas
    draw_vline(PANEL_X-1, 0, TFT_H, COLOR_ACCENT);

    uint16_t lc = COLOR_ACCENT;
    uint16_t tc = WHITE;

    draw_string(PANEL_X+4,  22,  "NEXT",  lc, COLOR_PANEL, 1);
    draw_string(PANEL_X+4,  82,  "HOLD",  lc, COLOR_PANEL, 1);
    draw_string(PANEL_X+4,  132, "SCORE", lc, COLOR_PANEL, 1);
    draw_string(PANEL_X+4,  162, "BEST",  lc, COLOR_PANEL, 1);
    draw_string(PANEL_X+4,  192, "LEVEL", lc, COLOR_PANEL, 1);
    draw_string(PANEL_X+4,  212, "LINES", lc, COLOR_PANEL, 1);

    (void)tc;
}

void TGfx::draw_next_piece(uint8_t type, uint8_t rot){
    int16_t px = PANEL_X+6, py = 32;
    fill_rect(px-2, py-2, PANEL_W-8, 40, COLOR_PANEL);
    draw_mini_piece(px, py, type, rot, false);
}

void TGfx::draw_hold_piece(uint8_t type, uint8_t rot, bool can_hold){
    int16_t px = PANEL_X+6, py = 92;
    fill_rect(px-2, py-2, PANEL_W-8, 40, COLOR_PANEL);
    if(type < 7)
        draw_mini_piece(px, py, type, rot, !can_hold);
    else {
        draw_string(px, py+8, "----", COLOR565(80,80,80), COLOR_PANEL, 1);
    }
}

void TGfx::draw_stats(uint32_t score, uint32_t best,
                       uint8_t level, uint16_t lines,
                       uint32_t combo){
    char buf[12];
    uint16_t bg = COLOR_PANEL;
    uint16_t fg = WHITE;
    uint16_t ac = GOLD;

    int16_t x = PANEL_X+2;

    // Score
    fill_rect(x, 142, PANEL_W-4, 10, bg);
    snprintf(buf,sizeof(buf),"%7lu",(unsigned long)score);
    draw_string(x, 142, buf, fg, bg, 1);

    // Best
    fill_rect(x, 172, PANEL_W-4, 10, bg);
    snprintf(buf,sizeof(buf),"%7lu",(unsigned long)best);
    draw_string(x, 172, buf, ac, bg, 1);

    // Level
    fill_rect(x, 202, PANEL_W-4, 10, bg);
    snprintf(buf,sizeof(buf),"   %3u",level);
    draw_string(x, 202, buf, fg, bg, 1);

    // Lines
    fill_rect(x, 222, PANEL_W-4, 10, bg);
    snprintf(buf,sizeof(buf),"%6u",lines);
    draw_string(x, 222, buf, fg, bg, 1);

    // Combo (si > 1)
    if(combo >= 2){
        char cbuf[24];
        snprintf(cbuf,sizeof(cbuf),"x%lu COMBO",(unsigned long)combo);
        fill_rect(FIELD_X, FIELD_Y+2, FIELD_W, 12, COLOR_EMPTY);
        draw_string(FIELD_X+2, FIELD_Y+2, cbuf, NEON_GREEN, COLOR_EMPTY, 1);
    }
}

// ============================================================
//  HUD SUPERIOR
// ============================================================
void TGfx::draw_hud(uint32_t score,uint32_t best,
                     uint8_t level,uint16_t lines){
    fill_rect(0,0,TFT_W,HUD_H, COLOR565(5,5,20));
    draw_hline(0,HUD_H-1,TFT_W,COLOR_ACCENT);

    char buf[10];
    snprintf(buf,sizeof(buf),"%06lu",(unsigned long)score);
    draw_string(2,  4, "SC", COLOR_ACCENT, COLOR565(5,5,20), 1);
    draw_string(16, 4, buf,  WHITE,        COLOR565(5,5,20), 1);

    snprintf(buf,sizeof(buf),"%06lu",(unsigned long)best);
    draw_string(82, 4, "HI", COLOR_ACCENT, COLOR565(5,5,20), 1);
    draw_string(96, 4, buf,  GOLD,         COLOR565(5,5,20), 1);

    snprintf(buf,sizeof(buf),"L%02u",level);
    draw_string(166,4, buf, NEON_GREEN,   COLOR565(5,5,20), 1);

    snprintf(buf,sizeof(buf),"%04u",lines);
    draw_string(196,4, buf, WHITE,        COLOR565(5,5,20), 1);
}

// ============================================================
//  PANTALLA DE TITULO
// ============================================================
void TGfx::draw_title(void){
    fill_screen(COLOR565(5,5,20));

    // Titulo con bloques de colores
    const char *title = "TETRIS";
    const uint16_t tc[6]={CYAN,YELLOW,PURPLE,GREEN,RED,ORANGE};
    for(int i=0;i<6;i++){
        // Bloque decorativo
        int16_t bx = 14 + i*26;
        fill_rect(bx, 30, 22, 22, tc[i]);
        draw_hline(bx,30,22,lighten(tc[i],80));
        draw_vline(bx,30,22,lighten(tc[i],80));
        draw_hline(bx,51,22,darken(tc[i],80));
        draw_vline(bx+21,30,22,darken(tc[i],80));
        // Letra centrada
        char ch[2]={title[i],0};
        draw_string(bx+7, 37, ch, WHITE, tc[i], 1);
    }

    draw_string(12, 65, "ARCADE  EDITION", COLOR_ACCENT, COLOR565(5,5,20), 1);

    // Preview de piezas
    const uint16_t pc[7]={CYAN,YELLOW,PURPLE,GREEN,RED,BLUE,ORANGE};
    for(int i=0;i<7;i++){
        int16_t px2 = 8 + i*32;
        fill_rect(px2, 90, 26, 10, pc[i]);
        draw_rect(px2, 90, 26, 10, darken(pc[i],60));
    }

    draw_string(5,  115, "TECLAS  TECLADO:", WHITE,        COLOR565(5,5,20),1);
    draw_string(5,  127, "← → MOVER",       LIGHT_GRAY,   COLOR565(5,5,20),1);
    draw_string(5,  139, "↑ / Z   ROTAR",   LIGHT_GRAY,   COLOR565(5,5,20),1);
    draw_string(5,  151, "↓ BAJAR RAPIDO",  LIGHT_GRAY,   COLOR565(5,5,20),1);
    draw_string(5,  163, "ESPACIO HARD DROP",LIGHT_GRAY,   COLOR565(5,5,20),1);
    draw_string(5,  175, "P PAUSA  Q SALIR", LIGHT_GRAY,   COLOR565(5,5,20),1);

    draw_string(5,  191, "GPIO:  L=5 R=6",   GRAY,         COLOR565(5,5,20),1);
    draw_string(5,  201, "       ROT=13 D=19",GRAY,         COLOR565(5,5,20),1);

    // Parpadeo
    draw_string(18, 218, "PRESIONA ENTER / BTN", COLOR_ACCENT, COLOR565(5,5,20),1);
    draw_string(30, 228, "PARA COMENZAR",        COLOR_ACCENT, COLOR565(5,5,20),1);
}

// ============================================================
//  GAME OVER
// ============================================================
void TGfx::draw_gameover(uint32_t score,uint32_t best,
                           uint8_t level,uint16_t lines){
    // Oscurecer campo
    for(int r=0;r<FIELD_ROWS;r++){
        int16_t py=FIELD_Y+r*BLOCK_PX;
        fill_rect(FIELD_X,py,FIELD_W,BLOCK_PX,
                  COLOR565((uint8_t)(r*4),(0),(uint8_t)(r*2)));
        delay_ms(18);
    }
    delay_ms(300);

    fill_screen(COLOR565(5,5,20));
    draw_string(26,  40, "GAME",  RED,    COLOR565(5,5,20), 3);
    draw_string(26,  70, "OVER",  RED,    COLOR565(5,5,20), 3);

    char buf[20];
    snprintf(buf,sizeof(buf),"SCORE  %06lu",(unsigned long)score);
    draw_string(14,112, buf, WHITE,     COLOR565(5,5,20),1);
    snprintf(buf,sizeof(buf),"BEST   %06lu",(unsigned long)best);
    draw_string(14,124, buf, GOLD,      COLOR565(5,5,20),1);
    snprintf(buf,sizeof(buf),"NIVEL  %3u", level);
    draw_string(14,136, buf, NEON_GREEN,COLOR565(5,5,20),1);
    snprintf(buf,sizeof(buf),"LINEAS %4u", lines);
    draw_string(14,148, buf, CYAN,      COLOR565(5,5,20),1);

    draw_string(14,175,"ENTER/BTN CONTINUAR",COLOR_ACCENT,COLOR565(5,5,20),1);
}

// ============================================================
//  PAUSA
// ============================================================
void TGfx::draw_pause(void){
    int16_t x=FIELD_X+8, y=FIELD_Y+80;
    fill_rect(x-4,y-4, FIELD_W-8, 50, COLOR565(10,10,30));
    draw_rect(x-4,y-4, FIELD_W-8, 50, COLOR_ACCENT);
    draw_string(x+6, y+4,  "PAUSA",  YELLOW, COLOR565(10,10,30),2);
    draw_string(x+2, y+24, "P=CONTINUAR", LIGHT_GRAY,COLOR565(10,10,30),1);
}

// ============================================================
//  BANNERS
// ============================================================
void TGfx::draw_level_banner(uint8_t level){
    char buf[20];
    int16_t x=FIELD_X+2, y=FIELD_Y+88;
    fill_rect(x-2,y-4,FIELD_W+2,44,COLOR565(0,0,40));
    draw_rect(x-2,y-4,FIELD_W+2,44,GOLD);
    snprintf(buf,sizeof(buf)," NIVEL  %2u!",level);
    draw_string(x+1, y+2,  buf, GOLD,      COLOR565(0,0,40),1);
    draw_string(x+4, y+14, "VELOCIDAD+", NEON_GREEN, COLOR565(0,0,40),1);
    draw_string(x+4, y+24, "MAS PUNTOS!", NEON_CYAN, COLOR565(0,0,40),1);
}

void TGfx::draw_tetris_flash(void){
    // Flash azul-blanco del campo al hacer Tetris
    for(int f=0;f<4;f++){
        fill_rect(FIELD_X,FIELD_Y,FIELD_W,FIELD_H,(f&1)?WHITE:CYAN);
        delay_ms(50);
    }
}

void TGfx::draw_tspin_banner(void){
    char msg[] = "T-SPIN!";
    int16_t x=FIELD_X+12, y=FIELD_Y+100;
    fill_rect(x-2,y-2,82,18,COLOR565(50,0,80));
    draw_rect(x-2,y-2,82,18,MAGENTA);
    draw_string(x, y+2, msg, MAGENTA, COLOR565(50,0,80),1);
}
