#include "../include/Level.h"
#include "../include/Renderer.h"
#include "../include/Graphics.h"        // <-- AÑADIDO
#include "../include/HardwareProfile.h" // <-- AÑADIDO
#include <cstring>

Level::Level() {
    memset(tiles, 0, sizeof(tiles));
    loadTestLevel();
}

void Level::loadTestLevel() {
    // Suelo
    for(int x=0; x<MAP_WIDTH; x++) {
        tiles[x][MAP_HEIGHT-2] = (uint8_t)TileType::GROUND;
        tiles[x][MAP_HEIGHT-1] = (uint8_t)TileType::GROUND;
    }
    // Algunos bloques ?
    tiles[5][10] = (uint8_t)TileType::QUESTION;
    tiles[8][10] = (uint8_t)TileType::BRICK;
    tiles[9][10] = (uint8_t)TileType::BRICK;
    tiles[10][10] = (uint8_t)TileType::QUESTION;
    tiles[7][7]  = (uint8_t)TileType::HIDDEN_BLOCK;
    // Tubería
    tiles[12][11] = (uint8_t)TileType::PIPE_TOP_LEFT;
    tiles[13][11] = (uint8_t)TileType::PIPE_TOP_RIGHT;
    tiles[12][12] = (uint8_t)TileType::PIPE_BOTTOM_LEFT;
    tiles[13][12] = (uint8_t)TileType::PIPE_BOTTOM_RIGHT;
}

TileType Level::getTile(int x, int y) const {
    if(x<0||x>=MAP_WIDTH||y<0||y>=MAP_HEIGHT) return TileType::EMPTY;
    return (TileType)tiles[x][y];
}

void Level::setTile(int x, int y, TileType type) {
    if(x>=0&&x<MAP_WIDTH&&y>=0&&y<MAP_HEIGHT)
        tiles[x][y] = (uint8_t)type;
}

void Level::removeTile(int x, int y) {
    setTile(x, y, TileType::EMPTY);
}
/*
void Level::draw() const {
    // Fondo de cielo (evita el parpadeo)
    Graphics::fill_screen(SKY_BLUE);
    
    // Dibujar solo los tiles no vacíos
    for(int y = 0; y < MAP_HEIGHT; y++) {
        for(int x = 0; x < MAP_WIDTH; x++) {
            TileType t = (TileType)tiles[x][y];
            if(t != TileType::EMPTY)
                Renderer::drawTile(x * TILE_SIZE, y * TILE_SIZE, t);
        }
    }
}
*/
void Level::draw() const {
    // Ya NO limpiamos toda la pantalla. Solo dibujamos los tiles.
    for(int y = 0; y < MAP_HEIGHT; y++) {
        for(int x = 0; x < MAP_WIDTH; x++) {
            TileType t = (TileType)tiles[x][y];
            if(t != TileType::EMPTY)
                Renderer::drawTile(x * TILE_SIZE, y * TILE_SIZE, t);
        }
    }
}
