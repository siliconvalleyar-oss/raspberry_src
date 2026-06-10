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
#define DINO_GROUND_Y   (GROUND_LINE - DINO_H)

// Fisica del salto
#define JUMP_VEL        (-14)
#define GRAVITY_INT     2
#define GRAVITY_FRAC    10

// Obstaculos
#define MAX_OBS         4
#define OBS_CACTUS_S    0
#define OBS_CACTUS_M    1
#define OBS_CACTUS_D    2
#define OBS_CACTUS_T    3
#define OBS_PTERO_LOW   4
#define OBS_PTERO_MID   5
#define OBS_ROCK        6

// Dimensiones de obstaculos (definidos en DinoEngine.cpp)
extern const int16_t OBS_W[7];
extern const int16_t OBS_H[7];
extern const int16_t OBS_YOFF[7];

// Nubes
#define MAX_CLOUDS      4

// Estrellas
#define MAX_STARS       12

// Power-ups
#define MAX_POWERUPS    2
#define PU_SHIELD       0
#define PU_SLOW         1

// Niveles
#define MAX_LEVEL       10
extern const int16_t  LEVEL_SPEED[MAX_LEVEL+1];
extern const uint8_t  LEVEL_GAP[MAX_LEVEL+1];
extern const uint32_t LEVEL_SCORE[MAX_LEVEL+1];

// ============================================================
//  ESTRUCTURAS
// ============================================================
struct Obstacle {
    int16_t  x;
    uint8_t  type;
    uint8_t  frame;
    bool     active;
};

struct Cloud {
    int16_t x, y;
    uint8_t speed;
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
