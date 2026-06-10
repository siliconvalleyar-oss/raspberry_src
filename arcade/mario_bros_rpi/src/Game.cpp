#include "../include/Game.h"
#include "../include/Renderer.h"
#include "../include/Sound.h"
#include "../include/Physics.h"
#include "../include/HardwareProfile.h"
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <cstdio>

// ============================================================
//  Funciones auxiliares para teclado no bloqueante
// ============================================================
static struct termios oldt, newt;

static void initKeyboard() {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

static void restoreKeyboard() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

static int kbhit() {
    char ch;
    int n = read(STDIN_FILENO, &ch, 1);
    if (n > 0) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

// ============================================================
//  Implementación de Game
// ============================================================
Game::Game() : player(nullptr), running(true), coinFrame(0) {
    player = new Player(32, 32);
    entities.push_back(player);
    entities.push_back(new Enemy(150, 150, EntityType::GOOMBA));
    initKeyboard();
}

Game::~Game() {
    for (auto e : entities) delete e;
    restoreKeyboard();
}

void Game::handleInput() {
    if (kbhit()) {
        char c = getchar();
        if (c == 'a' || c == 'A') player->move(Direction::LEFT);
        else if (c == 'd' || c == 'D') player->move(Direction::RIGHT);
        else if (c == 'w' || c == 'W') player->jump();
        else if (c == 'q' || c == 'Q') running = false;
        else player->stopMove();
    } else {
        player->stopMove();
    }
}

void Game::update(float dt) {
    for (auto e : entities) {
        if (e->active)
            e->update(dt, level.tiles);
    }

    // Colisiones jugador-enemigo
    for (auto e : entities) {
        if (e->type == EntityType::GOOMBA && e->active) {
            Enemy* enemy = static_cast<Enemy*>(e);
            if (Physics::AABBvsAABB(player->getAABB(), enemy->getAABB())) {
                if (player->vy > 0 && player->y + player->height < enemy->y + enemy->height/2) {
                    enemy->stomp();
                    player->vy = -3.0f;
                } else {
                    player->takeDamage();
                }
            }
        }
    }

    // Eliminar enemigos inactivos
    for (auto it = entities.begin(); it != entities.end(); ) {
        if (!(*it)->active && (*it)->type != EntityType::PLAYER) {
            delete *it;
            it = entities.erase(it);
        } else {
            ++it;
        }
    }

    if (!player->active) running = false;
}

void Game::draw() {
    Renderer::clearScreen(CYAN);   // o SKY_BLUE si prefieres
    level.draw();
    for (auto e : entities) e->draw();

    // HUD
    Renderer::drawText(2, 2, "MARIO", WHITE, 1);
    Renderer::drawNumber(50, 2, player->coins, YELLOW);
    Renderer::drawText(80, 2, "LIVES", WHITE);
    Renderer::drawNumber(130, 2, player->lives, RED);
}

void Game::run() {
    const int frameDelay = 16;  // ~60 FPS
    while (running) {
        handleInput();
        update(frameDelay / 1000.0f);
        draw();
        delay_ms(frameDelay);
    }

    Renderer::clearScreen(BLACK);
    Renderer::drawText(80, 100, "GAME OVER", RED, 2);
    delay_ms(2000);
}
