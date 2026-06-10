#ifndef LEVEL_H
#define LEVEL_H

#include "Types.h"
#include <vector>

class Level {
public:
    uint8_t tiles[MAP_WIDTH][MAP_HEIGHT];
    
    Level();
    void loadTestLevel();
    TileType getTile(int x, int y) const;
    void setTile(int x, int y, TileType type);
    void removeTile(int x, int y);
    void draw() const;
};

#endif
