#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

constexpr int SCREEN_WIDTH  = 240;
constexpr int SCREEN_HEIGHT = 240;

constexpr int TILE_SIZE     = 16;
constexpr int MAP_WIDTH     = SCREEN_WIDTH  / TILE_SIZE;   // 15
constexpr int MAP_HEIGHT    = SCREEN_HEIGHT / TILE_SIZE;   // 15

// Gravedad y física
constexpr float GRAVITY          = 0.4f;
constexpr float MAX_FALL_SPEED   = 6.0f;
constexpr float PLAYER_SPEED     = 1.8f;
constexpr float PLAYER_JUMP_FORCE = -5.2f;
constexpr float ENEMY_SPEED      = 1.0f;

enum class EntityType {
    PLAYER,
    GOOMBA,
    KOOPA,
    COIN,
    MUSHROOM
};

enum class EntityState {
    IDLE,
    WALKING,
    JUMPING,
    FALLING,
    DEAD,
    SHELL,
    SLIDING
};

enum class Direction {
    LEFT,
    RIGHT,
    NONE
};

enum class TileType : uint8_t {
    EMPTY = 0,
    BRICK,          // ladrillo rompible
    QUESTION,       // bloque ? (contiene item)
    USED_BLOCK,     // bloque ? ya usado
    HIDDEN_BLOCK,   // bloque invisible
    PIPE_TOP_LEFT,
    PIPE_TOP_RIGHT,
    PIPE_BOTTOM_LEFT,
    PIPE_BOTTOM_RIGHT,
    GROUND,
    COIN_BRICK      // ladrillo con moneda (al romper suelta moneda)
};

#endif
