#ifndef TETRIS_ENGINE_H
#define TETRIS_ENGINE_H

#include "Hardware.h"
#include "TetrisGfx.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================
//  MOTOR TETRIS — Mecanicas completas
//
//  Piezas: I O T S Z J L  (7 tetrominos + rotaciones SRS)
//  Sistema de rotacion: Super Rotation System (SRS)
//  Wall-kick: tablas completas SRS JLSTZ e I
//  Hold: reservar pieza
//  Ghost piece: muestra donde cae la pieza
//  Lock delay: 500ms antes de fijar (permite maniobrar)
//  T-spin detection
//  Combo: bonus por lineas consecutivas
//  B2B (Back-to-Back): bonus por Tetris/T-spin consecutivos
//  7-bag randomizer: bolsa de 7 piezas sin repeticion
//  Niveles 1-15 con gravity creciente
//  Scoring: Guideline oficial
// ============================================================

// ---- Piezas ----
#define PIECE_I  0
#define PIECE_O  1
#define PIECE_T  2
#define PIECE_S  3
#define PIECE_Z  4
#define PIECE_J  5
#define PIECE_L  6
#define PIECE_NONE 255

// ---- Tipos de entrada (agrupadas por AutoRepeat) ----
#define AR_DELAY_FRAMES  10   // frames antes de auto-repeat
#define AR_RATE_FRAMES    2   // frames entre repeticiones

void tetris_run(void);

#endif
