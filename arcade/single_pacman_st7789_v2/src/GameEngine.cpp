// ============================================================
//  GameEngine.cpp — Motor de Pac-Man para Raspberry Pi + ST7789
//  Nuevas funcionalidades:
//   * Laberinto 9x9 celdas 24px -> pasillo interior 16px (Pac-Man cabe)
//   * Power pellets (puntos grandes) -> fantasmas asustados (azul, huyen)
//   * Comer fantasma asustado suma 200 pts
//   * Niveles: al vaciar el laberinto sube nivel (velocidad incrementa)
//   * High score por sesion
//   * Pantalla de titulo con sprites escala 3x
//   * Animacion "Level Clear"
//   * Anti-flicker: borrar bbox completo antes de redibujar sprite
//   * Laberinto con offset centrado (MAZE_OX / MAZE_OY)
//   * Bordes de pared con biseladointeligente
// ============================================================

#include "../include/GameEngine.h"
#include "../include/Sound.h"
#include <cstdio>
#include <stdlib.h>
#include <time.h>

namespace GameEngine {

// ============================================================
//  LABERINTO 9x9
//  0=pasillo  1=pared  2=punto  3=power pellet
// ============================================================
uint8_t maze[MAZE_W][MAZE_H] = {
    {1,1,1,1,1,1,1,1,1},
    {1,3,2,2,1,2,2,3,1},
    {1,2,1,2,2,2,1,2,1},
    {1,2,1,2,1,2,1,2,1},
    {1,2,2,2,1,2,2,2,1},
    {1,2,1,2,1,2,1,2,1},
    {1,2,1,2,2,2,1,2,1},
    {1,3,2,2,1,2,2,3,1},
    {1,1,1,1,1,1,1,1,1}
};

static uint8_t mazeTemplate[MAZE_W][MAZE_H];

// ---- Pac-Man ----
int16_t pacX, pacY;
static int16_t prevPacX, prevPacY;
int8_t  pacDir       = 3;
int8_t  requestedDir = 3;
static bool    mouthOpen    = true;
static uint8_t mouthCounter = 0;

// ---- Fantasmas ----
int16_t ghostX[4], ghostY[4];
static int16_t prevGhostX[4], prevGhostY[4];
uint8_t ghostDir[4];
bool    ghostFrightened[4];
uint8_t frightenTimer = 0;

static const uint16_t GHOST_COLORS[4] = { RED, PINK, CYAN, ORANGE };

// ---- Estado ----
uint16_t score     = 0;
uint16_t highScore = 0;
uint8_t  lives     = 3;
uint8_t  level     = 1;
bool     gameOver  = false;

// ---- Velocidad ----
static uint8_t pacSpeed   = 4;
static uint8_t ghostSpeed = 2;

// ============================================================
//  COORDENADAS
// ============================================================
static inline int16_t cellPX(uint8_t cx) { return MAZE_OX + (int16_t)cx * CELL_W; }
static inline int16_t cellPY(uint8_t cy) { return MAZE_OY + (int16_t)cy * CELL_H; }

static inline uint8_t pixToCellX(int16_t px) {
    int16_t v = (px - MAZE_OX) / CELL_W;
    if(v < 0) v = 0;
    if(v >= MAZE_W) v = MAZE_W-1;
    return (uint8_t)v;
}
static inline uint8_t pixToCellY(int16_t py) {
    int16_t v = (py - MAZE_OY) / CELL_H;
    if(v < 0) v = 0;
    if(v >= MAZE_H) v = MAZE_H-1;
    return (uint8_t)v;
}

// ============================================================
//  REDIBUJA todas las celdas solapadas con (rx,ry,rw,rh)
// ============================================================
static void redraw_cells_in_rect(int16_t rx, int16_t ry,
                                  int16_t rw, int16_t rh)
{
    int16_t cx0 = (rx - MAZE_OX) / CELL_W;
    int16_t cy0 = (ry - MAZE_OY) / CELL_H;
    int16_t cx1 = (rx + rw - 1 - MAZE_OX) / CELL_W;
    int16_t cy1 = (ry + rh - 1 - MAZE_OY) / CELL_H;

    if(cx0 < 0)        cx0 = 0;
    if(cy0 < 0)        cy0 = 0;
    if(cx1 >= MAZE_W)  cx1 = MAZE_W-1;
    if(cy1 >= MAZE_H)  cy1 = MAZE_H-1;

    for(int16_t cx = cx0; cx <= cx1; cx++) {
        for(int16_t cy = cy0; cy <= cy1; cy++) {
            int16_t px = cellPX((uint8_t)cx);
            int16_t py = cellPY((uint8_t)cy);
            uint8_t v  = maze[cx][cy];
            if(v == 1) {
                bool wT = (cy > 0)        && (maze[cx][cy-1]==1);
                bool wB = (cy < MAZE_H-1) && (maze[cx][cy+1]==1);
                bool wL = (cx > 0)        && (maze[cx-1][cy]==1);
                bool wR = (cx < MAZE_W-1) && (maze[cx+1][cy]==1);
                Graphics::draw_wall_cell(px, py, wT, wB, wL, wR);
            } else {
                Graphics::fill_rect(px, py, CELL_W, CELL_H, FLOOR_COLOR);
                if(v == 2) Graphics::draw_dot  (px, py);
                if(v == 3) Graphics::draw_power(px, py);
            }
        }
    }
}

// ============================================================
//  INIT
// ============================================================
static void save_template(void) {
    for(uint8_t i=0;i<MAZE_W;i++)
        for(uint8_t j=0;j<MAZE_H;j++)
            mazeTemplate[i][j] = maze[i][j];
}

void init_game(void) {
    // Restaurar laberinto desde plantilla
    for(uint8_t i=0;i<MAZE_W;i++)
        for(uint8_t j=0;j<MAZE_H;j++)
            maze[i][j] = mazeTemplate[i][j];

    pacX = cellPX(1); pacY = cellPY(1);
    prevPacX = pacX;  prevPacY = pacY;
    pacDir = 3; requestedDir = 3;
    mouthOpen = true; mouthCounter = 0;

    ghostX[0]=cellPX(7); ghostY[0]=cellPY(7);
    ghostX[1]=cellPX(7); ghostY[1]=cellPY(1);
    ghostX[2]=cellPX(1); ghostY[2]=cellPY(7);
    ghostX[3]=cellPX(4); ghostY[3]=cellPY(4);
    for(int i=0;i<4;i++){
        prevGhostX[i]=ghostX[i]; prevGhostY[i]=ghostY[i];
        ghostDir[i]=(i<2)?2:3;
        ghostFrightened[i]=false;
    }
    frightenTimer = 0;
    gameOver = false;

    pacSpeed   = (level<=3)?4:(level<=6)?5:6;
    ghostSpeed = (level<=2)?2:(level<=5)?3:4;
}

// ============================================================
//  DRAW MAZE
// ============================================================
void draw_maze(void) {
    Graphics::fill_screen(BLACK);
    for(uint8_t cx=0;cx<MAZE_W;cx++) {
        for(uint8_t cy=0;cy<MAZE_H;cy++) {
            int16_t px = cellPX(cx);
            int16_t py = cellPY(cy);
            uint8_t v  = maze[cx][cy];
            if(v==1) {
                bool wT=(cy>0)       &&(maze[cx][cy-1]==1);
                bool wB=(cy<MAZE_H-1)&&(maze[cx][cy+1]==1);
                bool wL=(cx>0)       &&(maze[cx-1][cy]==1);
                bool wR=(cx<MAZE_W-1)&&(maze[cx+1][cy]==1);
                Graphics::draw_wall_cell(px,py,wT,wB,wL,wR);
            } else if(v==2) {
                Graphics::draw_dot  (px,py);
            } else if(v==3) {
                Graphics::draw_power(px,py);
            }
        }
    }
}

// ============================================================
//  COLISION CON PAREDES (verifica las 4 esquinas del sprite)
// ============================================================
static bool canMovePac(int16_t px, int16_t py, int8_t dir) {
    int16_t nx=px, ny=py;
    switch(dir){
        case 0: ny-=pacSpeed; break;
        case 1: ny+=pacSpeed; break;
        case 2: nx-=pacSpeed; break;
        case 3: nx+=pacSpeed; break;
    }
    const int16_t M=2, S=PACMAN_SIZE-2*M-1;
    for(uint8_t c=0;c<4;c++){
        int16_t tx = nx + M + ((c&1)?S:0);
        int16_t ty = ny + M + ((c>>1)?S:0);
        uint8_t cx2 = pixToCellX(tx);
        uint8_t cy2 = pixToCellY(ty);
        if(maze[cx2][cy2]==1) return false;
    }
    return true;
}

// ============================================================
//  UPDATE PAC-MAN
// ============================================================
void update_pacman(void) {
    static uint8_t lastCX=0xFF, lastCY=0xFF;

    prevPacX = pacX;
    prevPacY = pacY;

    if(++mouthCounter >= 5){ mouthOpen=!mouthOpen; mouthCounter=0; }

    if(canMovePac(pacX,pacY,requestedDir)) pacDir=requestedDir;
    if(canMovePac(pacX,pacY,pacDir)){
        switch(pacDir){
            case 0: pacY-=pacSpeed; break;
            case 1: pacY+=pacSpeed; break;
            case 2: pacX-=pacSpeed; break;
            case 3: pacX+=pacSpeed; break;
        }
    }

    // Comer puntos
    uint8_t cx=pixToCellX(pacX+PACMAN_SIZE/2);
    uint8_t cy=pixToCellY(pacY+PACMAN_SIZE/2);
    if(maze[cx][cy]==2){
        maze[cx][cy]=0; score+=10; sound_eat();
    } else if(maze[cx][cy]==3){
        maze[cx][cy]=0; score+=50;
        frightenTimer=150;
        for(int i=0;i<4;i++) ghostFrightened[i]=true;
        sound_ghost();
    }

    // Anti-flicker: borrar posicion anterior
    redraw_cells_in_rect(prevPacX, prevPacY, PACMAN_SIZE, PACMAN_SIZE);

    // Dibujar Pac-Man
    Graphics::draw_pacman(pacX, pacY, mouthOpen, pacDir);

    uint8_t ncx=pixToCellX(pacX+PACMAN_SIZE/2);
    uint8_t ncy=pixToCellY(pacY+PACMAN_SIZE/2);
    if(ncx!=lastCX||ncy!=lastCY){ lastCX=ncx; lastCY=ncy; draw_status(); }
}

// ============================================================
//  UPDATE FANTASMAS
// ============================================================
void update_ghosts(void) {
    if(frightenTimer>0){
        frightenTimer--;
        if(frightenTimer==0)
            for(int i=0;i<4;i++) ghostFrightened[i]=false;
    }

    for(int i=0;i<4;i++){
        prevGhostX[i]=ghostX[i];
        prevGhostY[i]=ghostY[i];

        int16_t dx=pacX-ghostX[i];
        int16_t dy=pacY-ghostY[i];

        // Elegir direccion
        static uint8_t rnd[4]={0,8,16,24};
        rnd[i]++;
        bool randomTurn = (rnd[i] > 20+i*4);
        if(randomTurn) rnd[i]=0;

        if(ghostFrightened[i]){
            if(abs(dx)>abs(dy)) ghostDir[i]=(dx>0)?2:3;
            else                 ghostDir[i]=(dy>0)?0:1;
        } else if(randomTurn){
            if((rand()%10)<3) ghostDir[i]=rand()%4;
            else { if(abs(dx)>abs(dy)) ghostDir[i]=(dx>0)?3:2;
                   else                 ghostDir[i]=(dy>0)?1:0; }
        }

        int16_t spd=ghostFrightened[i]?(ghostSpeed>1?ghostSpeed-1:1):ghostSpeed;

        for(int tries=0;tries<4;tries++){
            int16_t nx=ghostX[i], ny=ghostY[i];
            switch(ghostDir[i]){
                case 0: ny-=spd; break;
                case 1: ny+=spd; break;
                case 2: nx-=spd; break;
                case 3: nx+=spd; break;
            }
            uint8_t cx2=pixToCellX(nx+GHOST_SIZE/2);
            uint8_t cy2=pixToCellY(ny+GHOST_SIZE/2);
            if(maze[cx2][cy2]!=1){
                ghostX[i]=nx; ghostY[i]=ny; break;
            }
            ghostDir[i]=(ghostDir[i]+1)%4;
        }

        // Anti-flicker
        redraw_cells_in_rect(prevGhostX[i], prevGhostY[i],
                              GHOST_SIZE, GHOST_SIZE);

        Graphics::draw_ghost(ghostX[i], ghostY[i],
                              GHOST_COLORS[i], ghostFrightened[i]);
    }
}

// ============================================================
//  COLISIONES PAC-MAN / FANTASMA
// ============================================================
void check_collisions(void) {
    for(int i=0;i<4;i++){
        if(abs(pacX-ghostX[i])<PACMAN_SIZE-4 &&
           abs(pacY-ghostY[i])<PACMAN_SIZE-4){
            if(ghostFrightened[i]){
                ghostFrightened[i]=false;
                score+=200;
                sound_ghost();
                ghostX[i]=cellPX(4); ghostY[i]=cellPY(4);
                prevGhostX[i]=ghostX[i]; prevGhostY[i]=ghostY[i];
            } else {
                if(lives>0) lives--;
                sound_death();
                draw_maze();
                pacX=cellPX(1); pacY=cellPY(1);
                prevPacX=pacX; prevPacY=pacY;
                ghostX[0]=cellPX(7); ghostY[0]=cellPY(7);
                ghostX[1]=cellPX(7); ghostY[1]=cellPY(1);
                ghostX[2]=cellPX(1); ghostY[2]=cellPY(7);
                ghostX[3]=cellPX(4); ghostY[3]=cellPY(4);
                for(int j=0;j<4;j++){
                    prevGhostX[j]=ghostX[j]; prevGhostY[j]=ghostY[j];
                    ghostFrightened[j]=false;
                }
                frightenTimer=0;
                Graphics::draw_pacman(pacX,pacY,true,pacDir);
                draw_status();
                delay_ms(1200);
                if(lives==0){ gameOver=true; }
                return;
            }
        }
    }
}

// ============================================================
//  HUD
// ============================================================
void draw_status(void) {
    static uint16_t ls=0xFFFF;
    static uint8_t  ll=0xFF, llv=0xFF;
    if(score==ls&&lives==ll&&level==llv) return;
    ls=score; ll=lives; llv=level;

    if(score>highScore) highScore=score;

    Graphics::fill_rect(0, HUD_Y, TFT_W, HUD_H, BLACK);

    // Mini pac-mans por vida
    for(uint8_t i=0;i<lives&&i<5;i++)
        Graphics::draw_pacman_scaled(8+i*14, HUD_Y+HUD_H/2, true, 3, 1);

    char buf[28];
    sprintf(buf,"S:%u H:%u L%u", score, highScore, level);
    Graphics::draw_string(52, HUD_Y+2, buf, YELLOW, BLACK, 1);
    Graphics::draw_hline(0, HUD_Y-1, TFT_W, WALL_EDGE);
}

// ============================================================
//  TITULO
// ============================================================
void show_title(void) {
    Graphics::fill_screen(BLACK);

    Graphics::draw_string(28, 8,  "PAC-MAN",    YELLOW, BLACK, 3);
    Graphics::draw_string(38, 40, "PIC32MX795", CYAN,   BLACK, 2);

    // Pac-Man grande centrado
    Graphics::draw_pacman_scaled(120, 105, true, 3, 3);

    // 4 fantasmas grandes con colores diferentes
    const uint16_t gc[4]={RED,PINK,CYAN,ORANGE};
    for(int i=0;i<4;i++)
        Graphics::draw_ghost_scaled(28+i*48, 158, gc[i], false, 3);

    Graphics::draw_string(8,  200, "POWER DOT=GHOSTS BLUE!", WHITE,  BLACK, 1);
    Graphics::draw_string(8,  212, "EAT THEM FOR 200 PTS!",  YELLOW, BLACK, 1);
    Graphics::draw_string(28, 224, "* STARTING IN 3 SEC *",  GRAY,   BLACK, 1);

    sound_start();
    delay_ms(3000);
    Graphics::fill_screen(BLACK);
}

// ============================================================
//  LEVEL CLEAR
// ============================================================
void show_level_clear(void) {
    for(uint8_t f=0;f<6;f++){
        Graphics::fill_rect(MAZE_OX,MAZE_OY,
                             MAZE_W*CELL_W,MAZE_H*CELL_H,
                             (f&1)?WALL_EDGE:BLACK);
        delay_ms(120);
    }
    char buf[18];
    sprintf(buf,"LEVEL %u CLEAR!", level);
    // Centrar texto (~12 chars * 12px = 144px)
    Graphics::draw_string((TFT_W-144)/2, TFT_H/2-10,
                           buf, YELLOW, BLACK, 2);
    sound_start();
    delay_ms(2000);
    level++;
}

// ============================================================
//  GAME OVER
// ============================================================
void show_game_over(void) {
    Graphics::fill_screen(BLACK);
    Graphics::draw_string((TFT_W-108)/2, TFT_H/2-40,
                           "GAME OVER",   RED,    BLACK, 2);
    char buf[20];
    sprintf(buf,"SCORE: %u",   score);
    Graphics::draw_string((TFT_W-96)/2, TFT_H/2-10,  buf, YELLOW,BLACK,2);
    sprintf(buf,"BEST:  %u",   highScore);
    Graphics::draw_string((TFT_W-96)/2, TFT_H/2+14,  buf, CYAN,  BLACK,2);
    Graphics::draw_string((TFT_W-78)/2, TFT_H/2+46,
                           "PRESS RESET", WHITE, BLACK, 1);
    sound_death();
}

// ============================================================
//  GAME LOOP
// ============================================================
void game_loop(void) {
    save_template();
    show_title();

    score=0; lives=3; level=1; highScore=0;

restart_game:
    init_game();
    draw_maze();
    draw_status();
    Graphics::draw_pacman(pacX, pacY, true, pacDir);
    for(int i=0;i<4;i++)
        Graphics::draw_ghost(ghostX[i],ghostY[i],GHOST_COLORS[i],false);

    // Frame timing con POSIX (igual lógica que PIC32, distinta implementación)
    // PIC32: ft ciclos a 80 MHz = ft/80000 ms
    // RPi:   usamos clock_gettime(CLOCK_MONOTONIC) con resolución de ns
    struct timespec ts_last, ts_now;
    clock_gettime(CLOCK_MONOTONIC, &ts_last);

    while(true){

#ifdef USE_BUTTONS
        if(BTN_UP)         requestedDir=0;
        else if(BTN_DOWN)  requestedDir=1;
        else if(BTN_LEFT)  requestedDir=2;
        else if(BTN_RIGHT) requestedDir=3;
#else
        static uint8_t ac=0;
        if(++ac>25){ ac=0; requestedDir=rand()%4; }
#endif

        update_pacman();
        update_ghosts();
        check_collisions();

        if(gameOver){
            show_game_over();
            while(1);
        }

        // Verificar nivel completo
        bool done=true;
        for(uint8_t ci=0;ci<MAZE_W&&done;ci++)
            for(uint8_t cj=0;cj<MAZE_H&&done;cj++)
                if(maze[ci][cj]==2||maze[ci][cj]==3) done=false;

        if(done){
            show_level_clear();
            goto restart_game;
        }

        // Frame timing equivalente al PIC32:
        //   PIC32 ft = 55000 ciclos / nivel  @80 MHz →  ~687 us / nivel
        //   Usamos el mismo valor en microsegundos directamente
        uint32_t frame_us = 55000UL / (level < 8 ? level : 8);
        if(frame_us < 18000) frame_us = 18000;  // mínimo ~18 ms (~55 fps max)

        // Busy-wait preciso (igual que PIC32) usando System Timer
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        long elapsed_us = (ts_now.tv_sec  - ts_last.tv_sec)  * 1000000L
                        + (ts_now.tv_nsec - ts_last.tv_nsec) / 1000L;
        if(elapsed_us < (long)frame_us)
            delay_us((uint32_t)(frame_us - elapsed_us));
        clock_gettime(CLOCK_MONOTONIC, &ts_last);
    }
}

} // namespace GameEngine
