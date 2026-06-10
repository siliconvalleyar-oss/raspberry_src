#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"

class Player : public Entity {
public:
    Player(float x, float y);
    virtual ~Player();                     // <--- DECLARACIÓN EXPLÍCITA

    void update(float dt, const uint8_t tileMap[MAP_WIDTH][MAP_HEIGHT]) override;
    void draw() const override;

    void jump();
    void move(Direction dir);
    void stopMove();
    void takeDamage();

    int coins;
    int lives;
    bool big;
    float invincibleTimer;

private:
    Direction moveDir;
    bool jumpPressed;
    float jumpBuffer;
};

#endif
