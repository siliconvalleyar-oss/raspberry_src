#include "../include/Entity.h"
#include "../include/Physics.h"


Entity::Entity(float x, float y, float w, float h, EntityType t)
    : x(x), y(y), vx(0), vy(0), width(w), height(h), prevX(x), prevY(y),
      type(t), state(EntityState::IDLE), facing(Direction::RIGHT),
      onGround(false), active(true) {}


// Ya no es necesario definir el destructor aquí porque está inline en el .h

AABB Entity::getAABB() const {
    return {x, y, width, height};
}

void Entity::onTileCollision(TileType tile, int tileX, int tileY, bool fromTop) {
    // Por defecto no hace nada
}
