#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "HardwareProfile.h"
#include "Graphics.h"

namespace GameEngine {

    // ---- Estado del laberinto ----
    // Valores de celda:
    //   0 = pasillo vacío
    //   1 = pared
    //   2 = punto normal
    //   3 = power pellet (punto grande)
    extern uint8_t maze[MAZE_W][MAZE_H];

    // ---- Pac-Man ----
    extern int16_t pacX, pacY;       // posición en píxeles (esquina sup-izq)
    extern int8_t  pacDir;           // 0=arr 1=abaj 2=izq 3=der
    extern int8_t  requestedDir;

    // ---- Fantasmas ----
    extern int16_t ghostX[4], ghostY[4];
    extern uint8_t ghostDir[4];
    extern bool    ghostFrightened[4];   // modo asustado
    extern uint8_t frightenTimer;        // frames restantes asustado

    // ---- Puntuación / estado ----
    extern uint16_t score;
    extern uint8_t  lives;
    extern uint8_t  level;           // nivel actual (aumenta velocidad)
    extern bool     gameOver;
    extern uint16_t highScore;       // récord de sesión

    // ---- API ----
    void init_game(void);
    void draw_maze(void);
    void update_pacman(void);
    void update_ghosts(void);
    void check_collisions(void);
    void draw_status(void);
    void game_loop(void);
    void show_title(void);
    void show_game_over(void);
    void show_level_clear(void);
}

#endif
