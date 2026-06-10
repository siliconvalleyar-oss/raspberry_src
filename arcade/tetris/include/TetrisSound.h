// TetrisSound.h
#ifndef TETRIS_SOUND_H
#define TETRIS_SOUND_H
#include <stdint.h>

void tetris_sound_init(void);
void tetris_beep(uint16_t freq, uint16_t ms);

void snd_move(void);        // mover pieza
void snd_rotate(void);      // rotar
void snd_lock(void);        // pieza encajada
void snd_clear1(void);      // 1 linea
void snd_clear2(void);      // 2 lineas
void snd_clear3(void);      // 3 lineas
void snd_tetris(void);      // 4 lineas (TETRIS!)
void snd_levelup(void);     // subida de nivel
void snd_gameover(void);    // fin de juego
void snd_start(void);       // intro
void snd_harddrop(void);    // hard drop
void snd_hold(void);        // hold piece
void snd_tspin(void);       // T-spin bonus

#endif
