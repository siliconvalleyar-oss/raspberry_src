#include "../include/Physics.h"
#include "../include/Types.h"
#include <cmath>

bool Physics::AABBvsAABB(const AABB& a, const AABB& b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x &&
            a.y < b.y + b.h && a.y + a.h > b.y);
}

bool Physics::resolveTileCollision(AABB& entity, float& velX, float& velY,
                                   const uint8_t tileMap[MAP_WIDTH][MAP_HEIGHT],
                                   bool isPlayer) {
    bool collided = false;
    int leftTile   = (int)(entity.x / TILE_SIZE);
    int rightTile  = (int)((entity.x + entity.w) / TILE_SIZE);
    int topTile    = (int)(entity.y / TILE_SIZE);
    int bottomTile = (int)((entity.y + entity.h) / TILE_SIZE);

    leftTile   = (leftTile < 0) ? 0 : leftTile;
    rightTile  = (rightTile >= MAP_WIDTH) ? MAP_WIDTH-1 : rightTile;
    topTile    = (topTile < 0) ? 0 : topTile;
    bottomTile = (bottomTile >= MAP_HEIGHT) ? MAP_HEIGHT-1 : bottomTile;

    for(int ty = topTile; ty <= bottomTile; ++ty) {
        for(int tx = leftTile; tx <= rightTile; ++tx) {
            if(tileMap[tx][ty] == (uint8_t)TileType::EMPTY) continue;
            TileType tile = (TileType)tileMap[tx][ty];
            if(tile == TileType::COIN_BRICK) continue; // sin colisión sólida

            AABB tileAABB = { (float)(tx*TILE_SIZE), (float)(ty*TILE_SIZE),
                              (float)TILE_SIZE, (float)TILE_SIZE };
            if(AABBvsAABB(entity, tileAABB)) {
                collided = true;
                float overlapLeft   = (entity.x + entity.w) - tileAABB.x;
                float overlapRight  = (tileAABB.x + tileAABB.w) - entity.x;
                float overlapTop    = (entity.y + entity.h) - tileAABB.y;
                float overlapBottom = (tileAABB.y + tileAABB.h) - entity.y;

                float minOverlap = overlapLeft;
                if(overlapRight < minOverlap) minOverlap = overlapRight;
                if(overlapTop < minOverlap) minOverlap = overlapTop;
                if(overlapBottom < minOverlap) minOverlap = overlapBottom;

                if(minOverlap == overlapLeft)      { entity.x = tileAABB.x - entity.w; velX = 0; }
                else if(minOverlap == overlapRight){ entity.x = tileAABB.x + tileAABB.w; velX = 0; }
                else if(minOverlap == overlapTop)  { entity.y = tileAABB.y - entity.h; velY = 0; }
                else if(minOverlap == overlapBottom){ entity.y = tileAABB.y + tileAABB.h; velY = 0; }
            }
        }
    }
    return collided;
}
