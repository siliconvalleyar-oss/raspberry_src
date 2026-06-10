// ============================================================
//  DinoGraphics.cpp — Chrome Dino Arcade
//  Sprites pixel-art completos para pantalla 240x240 ST7789
// ============================================================
#include "DinoGraphics.h"
#include "DinoHardware.h"
#include <stdio.h>
#include <string.h>

// ============================================================
//  MODO COLOR / BN
// ============================================================
bool g_color_mode = true;

// Convierte un color RGB565 a escala de grises BT.601
static uint16_t to_gray(uint16_t c){
    uint8_t r = (c >> 11) & 0x1F;
    uint8_t g = (c >>  5) & 0x3F;
    uint8_t b = (c >>  0) & 0x1F;
    // Expandir a 8 bits
    uint8_t R = (r << 3) | (r >> 2);
    uint8_t G = (g << 2) | (g >> 4);
    uint8_t B = (b << 3) | (b >> 2);
    // Luminancia BT.601
    uint8_t Y = (uint8_t)(0.299f*R + 0.587f*G + 0.114f*B);
    // Convertir de vuelta a RGB565 gris
    uint8_t r5 = Y >> 3;
    uint8_t g6 = Y >> 2;
    uint8_t b5 = Y >> 3;
    return (uint16_t)((r5<<11)|(g6<<5)|b5);
}

// Devuelve color_val en modo color, gray_val en modo BN
// (gray_val puede ser WHITE, BLACK, GRAY, etc.)
uint16_t gc(uint16_t color_val, uint16_t gray_val){
    if(g_color_mode) return color_val;
    (void)gray_val;
    return to_gray(color_val);
}

// ============================================================
//  SQRT ENTERA
// ============================================================
static int16_t isqrt(int32_t v){
    if(v<=0) return 0;
    int16_t x=(int16_t)(v<65536L?v:256);
    for(int i=0;i<8;i++){int16_t n=(int16_t)((x+v/x)>>1);if(n>=x)break;x=n;}
    while((int32_t)x*x>v) x--;
    return x;
}

// ============================================================
//  PRIMITIVAS
// ============================================================
void DG::fill_screen(uint16_t c){
    set_window(0,0,TFT_W-1,TFT_H-1);
    push_color_n(c,(uint32_t)TFT_W*TFT_H);
}

void DG::fill_rect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t c){
    if(x>=(int16_t)TFT_W||y>=(int16_t)TFT_H||w<=0||h<=0) return;
    if(x<0){w+=x;x=0;} if(y<0){h+=y;y=0;}
    if(x+w>(int16_t)TFT_W) w=TFT_W-x;
    if(y+h>(int16_t)TFT_H) h=TFT_H-y;
    set_window(x,y,x+w-1,y+h-1);
    push_color_n(c,(uint32_t)w*h);
}

void DG::draw_pixel(int16_t x,int16_t y,uint16_t c){
    if((uint16_t)x>=TFT_W||(uint16_t)y>=TFT_H) return;
    set_window(x,y,x,y); push_color(c);
}

void DG::draw_hline(int16_t x,int16_t y,int16_t len,uint16_t c){
    fill_rect(x,y,len,1,c);
}
void DG::draw_vline(int16_t x,int16_t y,int16_t len,uint16_t c){
    fill_rect(x,y,1,len,c);
}

void DG::fill_circle(int16_t cx,int16_t cy,int16_t r,uint16_t c){
    for(int16_t dy=-r;dy<=r;dy++){
        int16_t dx=isqrt((int32_t)r*r-(int32_t)dy*dy);
        fill_rect(cx-dx,cy+dy,2*dx+1,1,c);
    }
}

// ============================================================
//  TEXTO
// ============================================================
void DG::draw_char(int16_t x,int16_t y,char ch,
                    uint16_t fg,uint16_t bg,uint8_t sc){
    if(ch<0x20||ch>0x7E) ch=' ';
    uint8_t idx=ch-0x20;
    for(uint8_t col=0;col<5;col++){
        uint8_t line=font5x7[idx][col];
        for(uint8_t row=0;row<7;row++)
            fill_rect(x+col*sc,y+row*sc,sc,sc,(line&(1<<row))?fg:bg);
    }
}

void DG::draw_string(int16_t x,int16_t y,const char *s,
                      uint16_t fg,uint16_t bg,uint8_t sc){
    while(*s){ draw_char(x,y,*s++,fg,bg,sc); x+=6*sc; }
}

int DG::text_width(const char *s,uint8_t sc){
    int n=0; while(*s++){n++;} return n*6*sc;
}

// ============================================================
//  DINO SPRITE  32x34 px
//  El dino tiene 3 frames: 0=parado, 1=pata_izq, 2=pata_der
//  Dibuja sobre fondo de cielo (se borra antes)
// ============================================================
//  Bitmap del cuerpo del dino (32 columnas x 34 filas, 1=pixel)
//  Se define como mascara de bits por fila de 32px (uint32_t)
static const uint32_t DINO_BODY[34] = {
    0b00000000000011111110000000000000,
    0b00000000001111111111000000000000,
    0b00000000011111111111100000000000,
    0b00000000111111111111110000000000,
    0b00000000111111111111111000000000,
    0b00000001111111111111111100000000,
    0b00000011111111111111111110000000,
    0b00000011111111111111111110000000,
    0b00000011111111111111111110000000,
    0b00000001111111111111110000000000,
    0b00000001111111111111100000000000,
    0b00000011111111111111100000000000,
    0b00000111111111111111110000000000,
    0b00001111111111111111111000000000,
    0b00011111111111111111111000000000,
    0b00011111111111111111111000000000,
    0b00011111111111111111111000000000,
    0b00001111111111111111110000000000,
    0b00000111111111111110000000000000,
    0b00000011111111110000000000000000,
    0b00000011111111110000000000000000,
    0b00000011111111110000000000000000,
    0b00000011111111110000000000000000,
    0b00000011111111110000000000000000,
    0b00000011111111110000000000000000,
    0b00000011001111110000000000000000,
    0b00000011001111110000000000000000,
    0b00000011001111110000000000000000,
    0b00000011001111110000000000000000,
    0b00000011001111110000000000000000,
    0b00000110000111100000000000000000,
    0b00000110000111100000000000000000,
    0b00001100000011110000000000000000,
    0b00001100000011110000000000000000,
};
// Patas frame 1 y 2 (solo las 8 filas inferiores)
static const uint32_t DINO_LEGS[2][8] = {
    {  // frame 1: pata izquierda avanza
        0b00000110000111100000000000000000,
        0b00001100000011000000000000000000,
        0b00001100000000000000000000000000,
        0b00001100000000000000000000000000,
        0b00001110000000000000000000000000,
        0b00000110000000000000000000000000,
        0b00000011000000000000000000000000,
        0b00000011000000000000000000000000,
    },
    {  // frame 2: pata derecha avanza
        0b00000110000111100000000000000000,
        0b00000110000011100000000000000000,
        0b00000000000011100000000000000000,
        0b00000000000011100000000000000000,
        0b00000000000011110000000000000000,
        0b00000000000001110000000000000000,
        0b00000000000000110000000000000000,
        0b00000000000000110000000000000000,
    },
};

void DG::draw_dino(int16_t x, int16_t y, uint8_t frame, bool dead) {
    uint16_t body_c = gc(DINO_GREEN, DARK_GRAY);
    uint16_t eye_c  = gc(WHITE,      WHITE);
    uint16_t pupil_c= gc(BLACK,      BLACK);
    uint16_t bg_c   = gc(SKY_BLUE,   WHITE);
    if(dead){ body_c = gc(COLOR565(180,40,40), GRAY); }

    const int W = 32, H = 34;
    // Borrar bbox
    fill_rect(x, y, W, H, bg_c);

    // Dibujar cuerpo
    for(int row = 0; row < 26; row++) {
        uint32_t mask = DINO_BODY[row];
        for(int col = 0; col < W; col++)
            if(mask & (1u<<(31-col)))
                draw_pixel(x+col, y+row, body_c);
    }
    // Patas segun frame
    const uint32_t *legs = (frame==0) ? DINO_BODY+26 :
                           DINO_LEGS[(frame-1)&1];
    for(int row=0; row<8; row++){
        uint32_t mask = legs[row];
        for(int col=0;col<W;col++)
            if(mask&(1u<<(31-col)))
                draw_pixel(x+col, y+26+row, body_c);
    }
    // Ojo: fila 3, columna 24-25
    draw_pixel(x+24, y+3, eye_c);
    draw_pixel(x+25, y+3, eye_c);
    draw_pixel(x+25, y+4, pupil_c);
    if(dead){
        // X en el ojo
        draw_pixel(x+23,y+3,body_c);
        draw_pixel(x+25,y+3,body_c);
        draw_pixel(x+24,y+4,body_c);
        draw_pixel(x+24,y+3,body_c);
        draw_pixel(x+23,y+4,COLOR565(255,0,0));
        draw_pixel(x+25,y+4,COLOR565(255,0,0));
        draw_pixel(x+23,y+5,COLOR565(255,0,0));
        draw_pixel(x+25,y+5,COLOR565(255,0,0));
    }
}

// ============================================================
//  CACTUS  tipos 0,1,2,3
//  type 0: cactus pequeño (1 palo)  14x30
//  type 1: cactus mediano (1 palo + brazos)  20x36
//  type 2: cactus doble  32x30
//  type 3: cactus triple grande  46x36
// ============================================================
void DG::draw_cactus(int16_t x, int16_t y, uint8_t type) {
    uint16_t c  = gc(CACTUS_GR, DARK_GRAY);
    uint16_t bg = gc(SKY_BLUE,  WHITE);

    if(type == 0) {
        // Tallo central 4px ancho, 30px alto
        fill_rect(x+5, y,    4, 30, c);
        // Punta redondeada
        fill_rect(x+4, y+2,  6, 2,  c);
        fill_rect(x+3, y+4,  8, 1,  c);
        // Brazo izquierdo
        fill_rect(x,   y+8,  5,  4, c);
        fill_rect(x,   y+4,  4, 10, c);
        // Brazo derecho
        fill_rect(x+9, y+10, 5,  4, c);
        fill_rect(x+10,y+6,  4, 10, c);
        // Base
        fill_rect(x+4, y+28, 6,  4, c);
    } else if(type == 1) {
        // Tallo 6px, 36px alto
        fill_rect(x+7,  y,    6, 36, c);
        fill_rect(x+6,  y+2,  8,  2, c);
        // Brazo izquierdo grande
        fill_rect(x,    y+6,  7,  5, c);
        fill_rect(x,    y+2, 5,  14, c);
        fill_rect(x,    y+2,  6,  2, c);
        // Brazo derecho
        fill_rect(x+13, y+8,  7,  5, c);
        fill_rect(x+15, y+4, 5,  14, c);
        fill_rect(x+14, y+4,  6,  2, c);
        // Base
        fill_rect(x+6,  y+34, 8,  4, c);
    } else if(type == 2) {
        // Doble: dos cactus tipo 0 juntos
        fill_rect(x+4,  y+2,  4, 28, c);
        fill_rect(x,    y+8,  4,  4, c);
        fill_rect(x,    y+4,  4,  8, c);
        fill_rect(x+8,  y+10, 4,  4, c);
        fill_rect(x+8,  y+6,  4, 10, c);
        fill_rect(x+4,  y+28, 4,  4, c);
        // Segundo cactus a la derecha
        fill_rect(x+20, y+4,  4, 26, c);
        fill_rect(x+16, y+10, 4,  4, c);
        fill_rect(x+16, y+6,  4,  8, c);
        fill_rect(x+24, y+12, 4,  4, c);
        fill_rect(x+24, y+8,  4,  8, c);
        fill_rect(x+20, y+28, 4,  4, c);
    } else {
        // Triple grande: 3 tallos
        for(int k=0;k<3;k++){
            int ox = x + k*14;
            fill_rect(ox+4,  y+2+(k%2)*6, 5, 28, c);
            fill_rect(ox,    y+8+(k%2)*4, 4, 5,  c);
            fill_rect(ox,    y+4+(k%2)*4, 4, 8,  c);
            fill_rect(ox+9,  y+10,        4, 5,  c);
            fill_rect(ox+9,  y+6,         4, 8,  c);
        }
    }
    (void)bg; // bg usado en draw_dino para borrar, aqui no hace falta
}

// ============================================================
//  PTERODACTILO  40x22 px, 2 frames (alas arriba/abajo)
// ============================================================
void DG::draw_ptero(int16_t x, int16_t y, uint8_t frame) {
    uint16_t c  = gc(PTERO_GRAY, DARK_GRAY);
    uint16_t bg = gc(SKY_BLUE,   WHITE);
    fill_rect(x, y, 40, 22, bg);

    // Cuerpo central
    fill_rect(x+14, y+8,  14, 8,  c);
    fill_rect(x+12, y+6,  18, 10, c);
    // Cabeza y pico
    fill_rect(x+26, y+5,   8, 6,  c);
    fill_rect(x+34, y+6,   6, 2,  c);
    // Ojo
    draw_pixel(x+28, y+6, WHITE);
    // Cola
    fill_rect(x+4,  y+10,  8, 4,  c);
    fill_rect(x,    y+12,  6, 3,  c);

    if(frame == 0){
        // Alas arriba
        fill_rect(x+6,  y,    16, 7, c);
        fill_rect(x+2,  y+2,  10, 4, c);
        fill_rect(x+20, y+1,  14, 6, c);
    } else {
        // Alas abajo
        fill_rect(x+6,  y+14, 16, 7, c);
        fill_rect(x+2,  y+15, 10, 4, c);
        fill_rect(x+20, y+14, 14, 6, c);
    }
}

// ============================================================
//  ROCA  28x16 px
// ============================================================
void DG::draw_rock(int16_t x, int16_t y) {
    uint16_t c  = gc(ROCK_BROWN,    GRAY);
    uint16_t lc = gc(COLOR565(180,150,110), LIGHT_GRAY);
    fill_circle(x+14, y+10, 10, c);
    fill_circle(x+6,  y+12,  7, c);
    fill_circle(x+22, y+11,  8, c);
    // Brillo
    fill_rect(x+10, y+4, 5, 3, lc);
}

// ============================================================
//  NUBE  46x18 px
// ============================================================
void DG::draw_cloud(int16_t x, int16_t y) {
    uint16_t c  = gc(CLOUD_WHITE, WHITE);
    uint16_t bg = gc(SKY_BLUE,    WHITE);
    fill_rect(x, y, 46, 18, bg);
    fill_circle(x+10, y+12, 8,  c);
    fill_circle(x+20, y+8,  10, c);
    fill_circle(x+32, y+10,  9, c);
    fill_circle(x+40, y+13,  6, c);
    fill_rect(x+2, y+12, 40, 6, c);
}

// ============================================================
//  LUNA  4 fases
// ============================================================
void DG::draw_moon(int16_t x, int16_t y, uint8_t phase) {
    uint16_t mc = gc(MOON_YELLOW, WHITE);
    uint16_t bg = gc(gc(COLOR565(15,15,40), BLACK), BLACK);
    fill_circle(x+12, y+12, 12, mc);
    // Sombra segun fase para simular cuarto/luna llena
    if(phase == 0) { // cuarto creciente
        fill_circle(x+6,  y+12, 12, bg);
    } else if(phase == 1) { // media
        fill_circle(x+8,  y+12, 12, bg);
    } else if(phase == 2) { // gibosa
        fill_circle(x+16, y+12, 10, bg);
    }
    // fase 3: luna llena, no tapa nada
}

// ============================================================
//  ESTRELLA  3px
// ============================================================
void DG::draw_star(int16_t x, int16_t y) {
    uint16_t c = gc(STAR_WHITE, WHITE);
    draw_pixel(x,   y,   c);
    draw_pixel(x+1, y,   c);
    draw_pixel(x,   y+1, c);
    draw_pixel(x+1, y+1, c);
}

// ============================================================
//  ESCUDO (power-up)  16x16
// ============================================================
void DG::draw_shield(int16_t x, int16_t y) {
    uint16_t c = gc(CYAN, WHITE);
    uint16_t i = gc(COLOR565(0,200,255), LIGHT_GRAY);
    // Forma de escudo
    fill_rect(x+2, y,   12, 2, c);
    fill_rect(x,   y+2, 16, 8, c);
    fill_rect(x+2, y+10, 12, 3, c);
    fill_rect(x+5, y+13,  6, 2, c);
    fill_rect(x+7, y+15,  2, 1, c);
    // Cruz interior
    fill_rect(x+7, y+3,  2, 8, i);
    fill_rect(x+4, y+6,  8, 2, i);
}

// ============================================================
//  SUELO  — linea con textura de guijarros
// ============================================================
void DG::draw_ground(int16_t gy, uint32_t scroll) {
    uint16_t gc1 = gc(SAND_COLOR,        DARK_GRAY);
    uint16_t gc2 = gc(COLOR565(180,150,100), GRAY);
    uint16_t gc3 = gc(COLOR565(160,130,80),  DARK_GRAY);

    // Linea principal de suelo
    draw_hline(0, gy,     TFT_W, gc1);
    draw_hline(0, gy+1,   TFT_W, gc2);
    draw_hline(0, gy+2,   TFT_W, gc1);

    // Guijarros animados con scroll
    for(int i=0; i<8; i++){
        int16_t gx = (int16_t)(((uint32_t)(i*30 + 5) - scroll) % TFT_W);
        if(gx < 0) gx += TFT_W;
        draw_pixel(gx,    gy+1, gc3);
        draw_pixel(gx+2,  gy+2, gc3);
        draw_pixel(gx+10, gy+1, gc3);
    }
}

// ============================================================
//  HUD
// ============================================================
void DG::draw_hud(uint32_t score, uint32_t hi,
                   uint8_t lives, uint8_t level,
                   bool color_mode) {
    uint16_t bg  = gc(SKY_BLUE, WHITE);
    uint16_t fg  = gc(BLACK,    BLACK);
    uint16_t hfc = gc(GRAY,     DARK_GRAY);

    fill_rect(0, 0, TFT_W, 18, bg);
    draw_hline(0, 18, TFT_W, gc(DARK_GRAY, DARK_GRAY));

    char buf[32];
    // HI score
    snprintf(buf, sizeof(buf), "HI%06lu", (unsigned long)hi);
    draw_string(2, 2, buf, hfc, bg, 1);
    // Score actual
    snprintf(buf, sizeof(buf), "%06lu", (unsigned long)score);
    draw_string(80, 2, buf, fg, bg, 1);
    // Nivel
    snprintf(buf, sizeof(buf), "LV%u", level);
    draw_string(152, 2, buf, gc(ORANGE, DARK_GRAY), bg, 1);
    // Vidas (iconos de dino mini)
    for(uint8_t i=0;i<lives&&i<3;i++)
        fill_rect(188+i*14, 4, 10, 10, gc(DINO_GREEN, DARK_GRAY));
    // Indicador modo BN/Color
    draw_string(228, 2, color_mode?"C":"B", gc(CYAN, DARK_GRAY), bg, 1);
}

// ============================================================
//  TITULO
// ============================================================
void DG::draw_title(void) {
    uint16_t bg   = gc(SKY_BLUE, WHITE);
    uint16_t fg   = gc(BLACK,    BLACK);
    uint16_t acc  = gc(ORANGE,   DARK_GRAY);
    uint16_t sub  = gc(GRAY,     GRAY);

    fill_screen(bg);

    // Titulo
    draw_string(10,  22, "CHROME  DINO",  fg,  bg, 2);
    draw_string(28,  42, "A R C A D E",   acc, bg, 2);

    // Dino grande central
    draw_dino(96, 70, 1, false);
    // Cactus a los lados
    draw_cactus(40,  88, 1);
    draw_cactus(160, 88, 0);
    // Nube
    draw_cloud(80, 60);

    draw_string(15, 140, "SALTAR: boton GPIO5", fg,  bg, 1);
    draw_string(15, 152, "COLOR:  boton GPIO6", sub, bg, 1);
    draw_string(15, 164, "O presiona ESPACIO  ", sub, bg, 1);
    draw_string(8,  180, "CACTUS / PTERO / ROCA", fg, bg, 1);
    draw_string(8,  192, "ESCUDO = INVENCIBLE!",  acc,bg, 1);

    draw_string(20, 215, "PRESIONA PARA JUGAR", gc(DARK_GRAY,GRAY), bg, 1);
}

// ============================================================
//  GAME OVER
// ============================================================
void DG::draw_gameover(uint32_t score, uint32_t hi) {
    uint16_t bg  = gc(SKY_BLUE,  WHITE);
    uint16_t fg  = gc(BLACK,     BLACK);
    uint16_t rc  = gc(RED,       DARK_GRAY);
    uint16_t acc = gc(ORANGE,    GRAY);

    fill_screen(bg);
    draw_string(28, 70,  "GAME  OVER",    rc,  bg, 2);
    draw_dino(96, 100, 0, true);

    char buf[24];
    snprintf(buf,sizeof(buf),"SCORE  %06lu",(unsigned long)score);
    draw_string(20, 150, buf, fg, bg, 1);
    snprintf(buf,sizeof(buf),"BEST   %06lu",(unsigned long)hi);
    draw_string(20, 162, buf, acc,bg, 1);

    draw_string(28, 190, "PRESIONA PARA", fg,  bg, 1);
    draw_string(40, 202, "CONTINUAR",     fg,  bg, 1);
}

// ============================================================
//  LEVEL BANNER
// ============================================================
void DG::draw_level_banner(uint8_t level) {
    uint16_t bg  = gc(SKY_BLUE, WHITE);
    uint16_t acc = gc(YELLOW,   DARK_GRAY);
    uint16_t fg  = gc(BLACK,    BLACK);

    fill_rect(30, 90, 180, 60, bg);
    fill_rect(30, 90, 180,  2, acc);
    fill_rect(30,148, 180,  2, acc);
    char buf[20];
    snprintf(buf,sizeof(buf)," NIVEL  %u ! ", level);
    draw_string(36, 105, buf, acc, bg, 2);
    draw_string(50, 128, "VELOCIDAD+", fg, bg, 1);
}
