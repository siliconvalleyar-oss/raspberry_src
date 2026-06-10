#ifndef RENDERER_H
#define RENDERER_H

#include "Types.h"
#include <cstdint>

class Renderer {
public:
    static void init();
    static void clearScreen(uint16_t color);
    static void drawTile(int x, int y, TileType type);
    static void drawPlayer(float x, float y, bool big, Direction facing, bool invincible);
    static void drawGoomba(float x, float y, bool stomped);
    static void drawCoin(float x, float y, int frame);
    static void drawText(int x, int y, const char* text, uint16_t color, uint8_t scale = 1);
    static void drawNumber(int x, int y, int number, uint16_t color, uint8_t scale = 1);
    static void drawRect(int x, int y, int w, int h, uint16_t color);
    static void fillRect(int x, int y, int w, int h, uint16_t color);
};

#endif
