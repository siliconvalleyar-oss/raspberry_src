#include "../include/Player.h"
#include "../include/Physics.h"
#include "../include/Renderer.h"
#include "../include/Sound.h"

Player::Player(float x, float y) : Entity(x, y, 14, 16, EntityType::PLAYER) {
    coins = 0;
    lives = 3;
    big = false;
    invincibleTimer = 0;
    moveDir = Direction::NONE;
    jumpPressed = false;
    jumpBuffer = 0;
    state = EntityState::FALLING;
}

Player::~Player() {}   // <--- DEFINICIÓN CORRECTA (con declaración previa en .h)

void Player::update(float dt, const uint8_t tileMap[MAP_WIDTH][MAP_HEIGHT]) {
    if (invincibleTimer > 0) invincibleTimer -= dt;

    if (moveDir == Direction::LEFT)       vx = -PLAYER_SPEED;
    else if (moveDir == Direction::RIGHT) vx = PLAYER_SPEED;
    else                                  vx = 0;

    if (jumpPressed) {
        if (onGround) {
            vy = PLAYER_JUMP_FORCE;
            onGround = false;
            sound_jump();
        } else {
            jumpBuffer = 0.15f;
        }
        jumpPressed = false;
    }
    if (jumpBuffer > 0) {
        if (onGround) {
            vy = PLAYER_JUMP_FORCE;
            onGround = false;
            sound_jump();
            jumpBuffer = 0;
        }
        jumpBuffer -= dt;
    }

    vy += GRAVITY;
    if (vy > MAX_FALL_SPEED) vy = MAX_FALL_SPEED;

    x += vx;
    AABB aabb = getAABB();
    Physics::resolveTileCollision(aabb, vx, vy, tileMap, true);
    x = aabb.x;
    y += vy;
    aabb = getAABB();
    onGround = false;
    if (Physics::resolveTileCollision(aabb, vx, vy, tileMap, true)) {
        if (vy >= 0) onGround = true;
        else vy = 0;
    }
    x = aabb.x;
    y = aabb.y;

    if (!onGround) {
        state = (vy < 0) ? EntityState::JUMPING : EntityState::FALLING;
    } else {
        state = (vx != 0) ? EntityState::WALKING : EntityState::IDLE;
    }

    if (vx > 0) facing = Direction::RIGHT;
    else if (vx < 0) facing = Direction::LEFT;

    if (x < 0) x = 0;
    if (x + width > SCREEN_WIDTH) x = SCREEN_WIDTH - width;
    if (y < 0) { y = 0; vy = 0; }
    if (y + height > SCREEN_HEIGHT) {
        takeDamage();
    }
}

void Player::draw() const {
    bool inv = (invincibleTimer > 0 && ((int)(invincibleTimer * 10) % 2));
    Renderer::drawPlayer(x, y, big, facing, inv);
}

void Player::jump() { jumpPressed = true; }
void Player::move(Direction dir) { moveDir = dir; }
void Player::stopMove() { moveDir = Direction::NONE; }

void Player::takeDamage() {
    if (invincibleTimer > 0) return;
    if (big) {
        big = false;
        invincibleTimer = 2.0f;
        sound_powerup();
    } else {
        lives--;
        sound_mario_die();
        if (lives <= 0) {
            active = false;
        } else {
            x = 32; y = 32;
            vx = vy = 0;
            invincibleTimer = 2.0f;
        }
    }
}
