#include "../include/Enemy.h"
#include "../include/Physics.h"
#include "../include/Renderer.h"
#include "../include/Sound.h"

Enemy::Enemy(float x, float y, EntityType t) : Entity(x, y, 14, 14, t) {
    stomped = false;
    stompTimer = 0;
    vx = -ENEMY_SPEED;
    facing = Direction::LEFT;
}

void Enemy::update(float dt, const uint8_t tileMap[MAP_WIDTH][MAP_HEIGHT]) {
    if(stomped) {
        stompTimer -= dt;
        if(stompTimer <= 0) active = false;
        return;
    }
    
    vy += GRAVITY;
    if(vy > MAX_FALL_SPEED) vy = MAX_FALL_SPEED;
    
    x += vx;
    AABB aabb = getAABB();                    // <-- variable local
    Physics::resolveTileCollision(aabb, vx, vy, tileMap);
    x = aabb.x;                               // actualizar posición
    y = aabb.y;
    
    y += vy;
    aabb = getAABB();
    onGround = false;
    if(Physics::resolveTileCollision(aabb, vx, vy, tileMap)) {
        if(vy >= 0) onGround = true;
        else vy = 0;
    }
    x = aabb.x;
    y = aabb.y;
    
    // ... resto igual
/*

//void Enemy::update(float dt, const uint8_t tileMap[MAP_WIDTH][MAP_HEIGHT]) {
    if(stomped) {
        stompTimer -= dt;
        if(stompTimer <= 0) active = false;
        return;
    }
    
    vy += GRAVITY;
    if(vy > MAX_FALL_SPEED) vy = MAX_FALL_SPEED;
    
    x += vx;
    Physics::resolveTileCollision(getAABB(), vx, vy, tileMap);
    y += vy;
    onGround = false;
    if(Physics::resolveTileCollision(getAABB(), vx, vy, tileMap)) {
        if(vy >= 0) onGround = true;
        else vy = 0;
    }
  */  
    // Cambiar dirección en bordes o paredes
    if(vx == 0 || x <= 0 || x + width >= SCREEN_WIDTH) {
        vx = -vx;
        facing = (vx > 0) ? Direction::RIGHT : Direction::LEFT;
    }
}

void Enemy::draw() const {
    if(!active) return;
    if(type == EntityType::GOOMBA)
        Renderer::drawGoomba(x, y, stomped);
}

void Enemy::stomp() {
    if(stomped) return;
    stomped = true;
    stompTimer = 0.5f;
    vx = 0;
    vy = 0;
    sound_stomp();
}
