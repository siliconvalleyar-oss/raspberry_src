#ifndef ENTITY_H
#define ENTITY_H

#include "Types.h"
#include <cstdint>

struct AABB;

class Entity {
public:
    float x, y;
    float vx, vy;
    float width, height;
    EntityType type;
    EntityState state;
    Direction facing;
    bool onGround;
    bool active;

    Entity(float x, float y, float w, float h, EntityType t);
    virtual ~Entity() {}

    virtual void update(float dt, const uint8_t tileMap[MAP_WIDTH][MAP_HEIGHT]) = 0;
    virtual void draw() const = 0;
    virtual void onTileCollision(TileType tile, int tileX, int tileY, bool fromTop);

    AABB getAABB() const;
};

#endif
