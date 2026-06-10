#include "../include/Renderer.h"
#include "../include/HardwareProfile.h"
#include "../include/Graphics.h"      // <--- AÑADIR ESTE INCLUDE
#include "../include/fonts.h"
#include <cstdio>
#include <cstring>

void Renderer::init() {
    Graphics::fill_screen(BLACK);
}

void Renderer::clearScreen(uint16_t color) {
    Graphics::fill_screen(color);
}

void Renderer::drawTile(int x, int y, TileType type) {
    uint16_t color = BLACK;
    switch(type) {
        case TileType::GROUND: color = BROWN; break;
        case TileType::BRICK:  color = COLOR565(200,100,50); break;
        case TileType::QUESTION: color = COLOR565(255,200,0); break;
        case TileType::USED_BLOCK: color = COLOR565(150,100,50); break;
        case TileType::HIDDEN_BLOCK: color = COLOR565(100,100,200); break;
        case TileType::PIPE_TOP_LEFT: case TileType::PIPE_TOP_RIGHT:
        case TileType::PIPE_BOTTOM_LEFT: case TileType::PIPE_BOTTOM_RIGHT:
            color = GREEN; break;
        case TileType::COIN_BRICK: color = COLOR565(200,100,50); break;
        default: return;
    }
    Graphics::fill_rect(x, y, TILE_SIZE, TILE_SIZE, color);
    // Bordes simples (usando líneas)
    if(type != TileType::EMPTY && type != TileType::GROUND) {
        Graphics::draw_hline(x, y, TILE_SIZE, BLACK);
        Graphics::draw_hline(x, y+TILE_SIZE-1, TILE_SIZE, BLACK);
        Graphics::draw_vline(x, y, TILE_SIZE, BLACK);
        Graphics::draw_vline(x+TILE_SIZE-1, y, TILE_SIZE, BLACK);
    }
    if(type == TileType::QUESTION) {
        Graphics::draw_string(x+4, y+4, "?", WHITE, BLACK, 1);
    }
}

void Renderer::drawPlayer(float x, float y, bool big, Direction facing, bool invincible) {
    uint16_t color = invincible ? WHITE : RED;
    int w = big ? 16 : 14;
    int h = big ? 20 : 16;
    Graphics::fill_rect((int)x, (int)y, w, h, color);
    Graphics::fill_rect((int)x+2, (int)y+4, 4, 4, WHITE);
    Graphics::fill_rect((int)x+8, (int)y+4, 4, 4, WHITE);
    Graphics::draw_hline((int)x+4, (int)y+10, 6, BLACK);
    Graphics::draw_hline((int)x+4, (int)y+11, 6, BLACK);
}

void Renderer::drawGoomba(float x, float y, bool stomped) {
    uint16_t color = stomped ? DARK_BLUE : BROWN;
    Graphics::fill_rect((int)x, (int)y, 14, 14, color);
    Graphics::fill_rect((int)x+2, (int)y+4, 4, 4, WHITE);
    Graphics::fill_rect((int)x+8, (int)y+4, 4, 4, WHITE);
}

void Renderer::drawCoin(float x, float y, int frame) {
    uint16_t color = (frame%2) ? YELLOW : WHITE;
    Graphics::fill_rect((int)x, (int)y, 8, 12, color);
}

void Renderer::drawText(int x, int y, const char* text, uint16_t color, uint8_t scale) {
    Graphics::draw_string(x, y, text, color, BLACK, scale);
}

void Renderer::drawNumber(int x, int y, int number, uint16_t color, uint8_t scale) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", number);
    drawText(x, y, buf, color, scale);
}

void Renderer::drawRect(int x, int y, int w, int h, uint16_t color) {
    Graphics::draw_hline(x, y, w, color);
    Graphics::draw_hline(x, y+h-1, w, color);
    Graphics::draw_vline(x, y, h, color);
    Graphics::draw_vline(x+w-1, y, h, color);
}

void Renderer::fillRect(int x, int y, int w, int h, uint16_t color) {
    Graphics::fill_rect(x, y, w, h, color);
}
