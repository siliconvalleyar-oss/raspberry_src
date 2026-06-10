#ifndef DINO_ENGINE_H
#define DINO_ENGINE_H

#include "DinoHardware.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================
//  CONSTANTES DEL JUEGO
// ============================================================
#define SCREEN_W        240
#define SCREEN_H        240

// Zona de juego (debajo del HUD)
#define HUD_H           19
#define PLAY_TOP        HUD_H
#define PLAY_H          (SCREEN_H - HUD_H)

// Suelo
#define GROUND_Y        188    // Y del suelo (relativo a pantalla)
#define GROUND_LINE     (GROUND_Y)

// Dino
#define DINO_X          24     // X fija del dino
#define DINO_W          32
#define DINO_H          34
#define DINO_GROUND_Y   (GROUND_LINE - DINO_H)   // Y cuando esta en el suelo

// Fisica del salto
#define JUMP_VEL        (-14)  // velocidad inicial
#define GRAVITY_INT     2      // pixels/frame^2 entero (se acumula)
#define GRAVITY_FRAC    10     // x/10 para suavizar

// Obstaculos
#define MAX_OBS         4
// Tipos de obstaculo
#define OBS_CACTUS_S    0
#define OBS_CACTUS_M    1
#define OBS_CACTUS_D    2
#define OBS_CACTUS_T    3
#define OBS_PTERO_LOW   4   // pterodactilo bajo (a la altura del dino)
#define OBS_PTERO_MID   5   // pterodactilo medio (saltar por arriba o pasar por abajo)
#define OBS_ROCK        6   // roca

// Dimensiones de obstaculos [tipo][w,h,ycenter_offset_from_ground]
// ycenter_offset: cuantos px sobre el suelo esta el centro
static const int16_t OBS_W[7]      = {14, 20, 32, 46, 40, 40, 28};
static const int16_t OBS_H[7]      = {30, 36, 30, 36, 22, 22, 16};
static const int16_t OBS_YOFF[7]   = {30, 36, 30, 36, 30, 50, 16};
//                                     ^suelo    ^vuela bajo  ^vuela medio

// Nubes
#define MAX_CLOUDS      4

// Estrellas
#define MAX_STARS       12

// Power-ups
#define MAX_POWERUPS    2
#define PU_SHIELD       0   // escudo: 5 segundos de invencibilidad
#define PU_SLOW         1   // slow-mo: reduce velocidad 3 segundos

// Niveles
#define MAX_LEVEL       10
// Velocidad base por nivel (px/frame * 10 para precision)
static const int16_t LEVEL_SPEED[MAX_LEVEL+1] = {
    0, 30, 36, 42, 48, 55, 62, 70, 78, 87, 96
};
// Frames entre obstaculos (decrecen con nivel)
static const uint8_t LEVEL_GAP[MAX_LEVEL+1] = {
    0, 80, 72, 64, 56, 50, 44, 38, 32, 28, 24
};

// Score para subir de nivel
static const uint32_t LEVEL_SCORE[MAX_LEVEL+1] = {
    0,0,100,250,450,700,1000,1400,1900,2500,3200
};

// ============================================================
//  ESTRUCTURAS
// ============================================================
struct Obstacle {
    int16_t  x;
    uint8_t  type;
    uint8_t  frame;     // animacion ptero
    bool     active;
};

struct Cloud {
    int16_t x, y;
    uint8_t speed;  // 1 o 2
    bool    active;
};

struct Star {
    int16_t x, y;
    bool    active;
};

struct PowerUp {
    int16_t  x, y;
    uint8_t  type;
    bool     active;
};

// ============================================================
//  API
// ============================================================
void dino_game_loop(void);

#endif
