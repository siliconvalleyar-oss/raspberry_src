#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"

class Enemy : public Entity {
public:
    Enemy(float x, float y, EntityType t);
    void update(float dt, const uint8_t tileMap[MAP_WIDTH][MAP_HEIGHT]) override;
    void draw() const override;
    void stomp();
    
protected:
    float stompTimer;
    bool stomped;
};

#endif
