#ifndef PHYSICS_H
#define PHYSICS_H

#include "Types.h"

struct AABB {
    float x, y;
    float w, h;
};

class Physics {
public:
    static bool AABBvsAABB(const AABB& a, const AABB& b);
    static bool resolveTileCollision(AABB& entity, float& velX, float& velY,
                                     const uint8_t tileMap[MAP_WIDTH][MAP_HEIGHT],
                                     bool isPlayer = false);
};

#endif
