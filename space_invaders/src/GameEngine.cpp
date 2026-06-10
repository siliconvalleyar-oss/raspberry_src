// ============================================================
//  GameEngine.cpp — Space Shooter para Raspberry Pi + ST7789
//  VERSIÓN CORREGIDA - Sin bucles infinitos
// ============================================================

#include "../include/GameEngine.h"
#include "../include/Sound.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <time.h>

namespace GameEngine {
using namespace Graphics;

// ============================================================
//  ESTADO GLOBAL
// ============================================================
int16_t playerX, playerY;
uint8_t playerDir;
int8_t  playerMoveX, playerMoveY;

Bullet   bullets[MAX_BULLETS];
Enemy    enemies[MAX_ENEMIES];
Explosion explosions[MAX_EXPLOSIONS];
Star     stars[NUM_STARS];

uint16_t score     = 0;
uint16_t highScore = 0;
uint8_t  lives     = 3;
uint8_t  wave      = 1;
bool     gameOver  = false;
GameState gameState = STATE_TITLE;

uint8_t  fireCooldown    = 0;
uint8_t  enemySpawnTimer = 0;
uint8_t  enemiesAlive    = 0;
uint8_t  enemiesPerWave  = 5;
uint16_t waveDelay       = 0;

// ---- Power-up ----
static bool  powerUpActive   = false;
static uint8_t powerUpTimer  = 0;
static int16_t powerUpX      = 0;
static int16_t powerUpY      = 0;
static bool  powerUpVisible  = false;

// ---- Para evitar flicker ----
static int16_t prevPlayerX, prevPlayerY;
static int16_t prevEnemyX[MAX_ENEMIES], prevEnemyY[MAX_ENEMIES];
static bool    prevEnemyActive[MAX_ENEMIES];
static int16_t prevBulletX[MAX_BULLETS], prevBulletY[MAX_BULLETS];
static bool    prevBulletActive[MAX_BULLETS];

// ---- Posiciones anteriores de estrellas (anti-flicker estelar) ----
static int16_t prevStarX[NUM_STARS];
static int16_t prevStarY[NUM_STARS];
static bool    prevStarValid[NUM_STARS];

// ============================================================
//  INICIALIZAR ESTRELLAS
// ============================================================
void init_stars(void) {
    for(uint8_t i = 0; i < NUM_STARS; i++) {
        stars[i].x = rand() % TFT_W;
        stars[i].y = rand() % TFT_H;
        stars[i].speed = 1 + (rand() % 3);
        stars[i].brightness = 1 + (rand() % 3);
    }
}

// ============================================================
//  ACTUALIZAR ESTRELLAS
// ============================================================
void update_stars(void) {
    for(uint8_t i = 0; i < NUM_STARS; i++) {
        stars[i].y += stars[i].speed;
        if(stars[i].y >= TFT_H) {
            stars[i].y = 0;
            stars[i].x = rand() % TFT_W;
            stars[i].speed = 1 + (rand() % 3);
        }
    }
}

// ============================================================
//  DIBUJAR ESTRELLAS
// ============================================================
void draw_stars(void) {
    for(uint8_t i = 0; i < NUM_STARS; i++) {
        uint16_t color;
        switch(stars[i].brightness) {
            case 1: color = GRAY;       break;
            case 2: color = COLOR565(180,180,200); break;
            default: color = WHITE;     break;
        }
        if(stars[i].brightness >= 2)
            fill_rect(stars[i].x, stars[i].y,
                      stars[i].brightness - 1, stars[i].brightness - 1, color);
        else
            draw_pixel(stars[i].x, stars[i].y, color);
    }
}

// ============================================================
//  INICIALIZAR JUEGO
// ============================================================
void init_game(void) {
    playerX = (TFT_W - PLAYER_W) / 2;
    playerY = TFT_H - 40;
    prevPlayerX = playerX;
    prevPlayerY = playerY;
    playerDir = 0;
    playerMoveX = 0;
    playerMoveY = 0;

    for(int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
        prevBulletActive[i] = false;
    }
    for(int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
        prevEnemyActive[i] = false;
    }
    for(int i = 0; i < MAX_EXPLOSIONS; i++) {
        explosions[i].active = false;
    }

    fireCooldown = 0;
    enemySpawnTimer = 0;
    enemiesAlive = 0;
    enemiesPerWave = 5 + wave;
    if(enemiesPerWave > 25) enemiesPerWave = 25;
    waveDelay = 0;

    powerUpActive = false;
    powerUpTimer = 0;
    powerUpVisible = false;

    score = 0;
    lives = 3;
    gameOver = false;
    gameState = STATE_PLAYING;

    init_stars();

    for(int i = 0; i < MAX_ENEMIES; i++) {
        prevEnemyX[i] = 0;
        prevEnemyY[i] = 0;
    }
    for(int i = 0; i < MAX_BULLETS; i++) {
        prevBulletX[i] = 0;
        prevBulletY[i] = 0;
    }
    for(uint8_t i = 0; i < NUM_STARS; i++) {
        prevStarValid[i] = false;
        prevStarX[i] = 0;
        prevStarY[i] = 0;
    }
}

// ============================================================
//  DISPARAR
// ============================================================
static void fire_bullet(void) {
    if(fireCooldown > 0) return;
    for(int i = 0; i < MAX_BULLETS; i++) {
        if(!bullets[i].active) {
            bullets[i].x = playerX + PLAYER_W/2 - BULLET_W/2;
            bullets[i].y = playerY - BULLET_H;
            bullets[i].active = true;
            fireCooldown = 8;
            sound_eat();
            break;
        }
    }
}

// ============================================================
//  SPAWNEAR ENEMIGO
// ============================================================
void spawn_enemy(void) {
    if(enemiesAlive >= enemiesPerWave) return;
    if(enemySpawnTimer > 0) { enemySpawnTimer--; return; }

    for(int i = 0; i < MAX_ENEMIES; i++) {
        if(!enemies[i].active) {
            enemies[i].x = 10 + (rand() % (TFT_W - ENEMY_W - 20));
            enemies[i].y = -ENEMY_H;

            uint8_t r = rand() % 100;
            if(wave <= 2) {
                enemies[i].type = (r < 70) ? 0 : 1;
            } else if(wave <= 5) {
                enemies[i].type = (r < 50) ? 0 : (r < 80) ? 1 : 2;
            } else {
                enemies[i].type = (r < 40) ? 0 : (r < 65) ? 1 : 2;
            }

            enemies[i].hp = (enemies[i].type == 2) ? 2 : 1;
            enemies[i].dirX = (rand() % 3) - 1;
            if(enemies[i].dirX == 0 && (rand() % 2)) enemies[i].dirX = 1;
            enemies[i].active = true;
            enemies[i].animFrame = 0;

            enemiesAlive++;
            enemySpawnTimer = 15 + (rand() % 20);
            if(wave > 3) enemySpawnTimer -= 5;
            if(enemySpawnTimer < 8) enemySpawnTimer = 8;
            break;
        }
    }
}

// ============================================================
//  SPAWNEAR POWER-UP
// ============================================================
static void spawn_powerup(void) {
    if(powerUpVisible || powerUpActive) return;
    if((rand() % 100) > 5) return;
    powerUpX = 20 + (rand() % (TFT_W - 40));
    powerUpY = -10;
    powerUpVisible = true;
}

// ============================================================
//  UPDATE JUGADOR
// ============================================================
void update_player(void) {
    prevPlayerX = playerX;
    prevPlayerY = playerY;

    const uint8_t SPEED = 3;

    int16_t newX = playerX + playerMoveX * SPEED;
    int16_t newY = playerY + playerMoveY * SPEED;

    if(newX < 0) newX = 0;
    if(newX > TFT_W - PLAYER_W) newX = TFT_W - PLAYER_W;
    if(newY < 0) newY = 0;
    if(newY > TFT_H - PLAYER_H - 12) newY = TFT_H - PLAYER_H - 12;

    playerX = newX;
    playerY = newY;

    if(fireCooldown > 0) fireCooldown--;
    fire_bullet();
}

// ============================================================
//  UPDATE BALAS
// ============================================================
void update_bullets(void) {
    for(int i = 0; i < MAX_BULLETS; i++) {
        prevBulletX[i] = bullets[i].x;
        prevBulletY[i] = bullets[i].y;
        prevBulletActive[i] = bullets[i].active;

        if(!bullets[i].active) continue;

        bullets[i].y -= 6;

        if(bullets[i].y + BULLET_H < 0) {
            bullets[i].active = false;
        }
    }
}

// ============================================================
//  UPDATE ENEMIGOS
// ============================================================
void update_enemies(void) {
    for(int i = 0; i < MAX_ENEMIES; i++) {
        prevEnemyX[i] = enemies[i].x;
        prevEnemyY[i] = enemies[i].y;
        prevEnemyActive[i] = enemies[i].active;

        if(!enemies[i].active) continue;

        enemies[i].animFrame = (enemies[i].animFrame + 1) % 8;

        uint8_t speed = (enemies[i].type == 1) ? 2 : 1;

        if(enemies[i].y < 10) {
            enemies[i].y += speed;
        } else {
            enemies[i].x += enemies[i].dirX * speed;
            enemies[i].y += speed;

            if(enemies[i].x <= 0) {
                enemies[i].x = 0;
                enemies[i].dirX = 1;
            }
            if(enemies[i].x >= TFT_W - ENEMY_W) {
                enemies[i].x = TFT_W - ENEMY_W;
                enemies[i].dirX = -1;
            }

            if((rand() % 100) < 2) {
                enemies[i].dirX = (rand() % 3) - 1;
            }
        }

        if(enemies[i].y > TFT_H + 5) {
            enemies[i].active = false;
            enemiesAlive--;
        }
    }

    if(enemiesAlive < enemiesPerWave) {
        spawn_enemy();
    }
}

// ============================================================
//  UPDATE EXPLOSIONES
// ============================================================
void update_explosions(void) {
    for(int i = 0; i < MAX_EXPLOSIONS; i++) {
        if(!explosions[i].active) continue;
        explosions[i].frame++;
        if(explosions[i].frame >= 8) {
            explosions[i].active = false;
        }
    }
}

// ============================================================
//  CREAR EXPLOSIÓN
// ============================================================
static void create_explosion(int16_t x, int16_t y) {
    for(int i = 0; i < MAX_EXPLOSIONS; i++) {
        if(!explosions[i].active) {
            explosions[i].x = x;
            explosions[i].y = y;
            explosions[i].frame = 0;
            explosions[i].active = true;
            break;
        }
    }
}

// ============================================================
//  COLISIONES
// ============================================================
void check_collisions(void) {
    for(int b = 0; b < MAX_BULLETS; b++) {
        if(!bullets[b].active) continue;

        for(int e = 0; e < MAX_ENEMIES; e++) {
            if(!enemies[e].active) continue;

            if(bullets[b].x < enemies[e].x + ENEMY_W &&
               bullets[b].x + BULLET_W > enemies[e].x &&
               bullets[b].y < enemies[e].y + ENEMY_H &&
               bullets[b].y + BULLET_H > enemies[e].y) {

                bullets[b].active = false;
                enemies[e].hp--;

                if(enemies[e].hp == 0) {
                    create_explosion(enemies[e].x + ENEMY_W/2,
                                     enemies[e].y + ENEMY_H/2);
                    enemies[e].active = false;
                    enemiesAlive--;

                    switch(enemies[e].type) {
                        case 0: score += 10; break;
                        case 1: score += 15; break;
                        case 2: score += 25; break;
                    }
                    sound_ghost();
                }
                break;
            }
        }
    }

    for(int e = 0; e < MAX_ENEMIES; e++) {
        if(!enemies[e].active) continue;

        if(playerX < enemies[e].x + ENEMY_W &&
           playerX + PLAYER_W > enemies[e].x &&
           playerY < enemies[e].y + ENEMY_H &&
           playerY + PLAYER_H > enemies[e].y) {

            create_explosion(playerX + PLAYER_W/2, playerY + PLAYER_H/2);
            create_explosion(enemies[e].x + ENEMY_W/2, enemies[e].y + ENEMY_H/2);

            enemies[e].active = false;
            enemiesAlive--;

            if(powerUpActive) {
                powerUpActive = false;
                powerUpTimer = 0;
                sound_ghost();
            } else {
                lives--;
                sound_death();
                if(lives == 0) {
                    gameOver = true;
                    gameState = STATE_GAME_OVER;
                    return;
                }
                playerX = (TFT_W - PLAYER_W) / 2;
                playerY = TFT_H - 40;
                prevPlayerX = playerX;
                prevPlayerY = playerY;
                delay_ms(500);
            }
        }
    }

    if(powerUpVisible) {
        powerUpY += 1;
        if(playerX < powerUpX + 8 &&
           playerX + PLAYER_W > powerUpX &&
           playerY < powerUpY + 8 &&
           playerY + PLAYER_H > powerUpY) {
            powerUpVisible = false;
            powerUpActive = true;
            powerUpTimer = 180;
            score += 50;
            sound_start();
        }
        if(powerUpY > TFT_H) {
            powerUpVisible = false;
        }
    }

    if(powerUpActive) {
        if(powerUpTimer > 0) powerUpTimer--;
        else powerUpActive = false;
    }

    bool anyActive = false;
    for(int i = 0; i < MAX_ENEMIES; i++) {
        if(enemies[i].active) {
            anyActive = true;
            break;
        }
    }
    if(!anyActive && enemiesAlive == 0 && enemySpawnTimer == 0) {
        waveDelay++;
        if(waveDelay > 60) {
            wave++;
            show_wave_clear();
            enemiesPerWave = 5 + wave;
            if(enemiesPerWave > 25) enemiesPerWave = 25;
            waveDelay = 0;
            enemiesAlive = 0;
            enemySpawnTimer = 30;
        }
    } else {
        waveDelay = 0;
    }
}

// ============================================================
//  HUD
// ============================================================
void draw_hud(void) {
    static uint16_t lastScore = 0xFFFF;
    static uint8_t  lastLives = 0xFF;
    static uint8_t  lastWave  = 0xFF;

    if(score == lastScore && lives == lastLives && wave == lastWave) return;
    lastScore = score; lastLives = lives; lastWave = wave;

    if(score > highScore) highScore = score;

    fill_rect(0, TFT_H - 10, TFT_W, 10, BLACK);
    draw_hline(0, TFT_H - 10, TFT_W, COLOR565(0, 80, 150));

    char buf[24];
    sprintf(buf, "S:%u W%u", score, wave);
    draw_string(2, TFT_H - 8, buf, YELLOW, BLACK, 1);

    for(uint8_t i = 0; i < lives && i < 5; i++) {
        int16_t lx = TFT_W - 8 - i * 10;
        int16_t ly = TFT_H - 8;
        fill_rect(lx, ly, 5, 2, CYAN);
        fill_rect(lx+1, ly+2, 3, 2, CYAN);
        fill_rect(lx+2, ly+4, 1, 2, CYAN);
    }
}

// ============================================================
//  TITULO
// ============================================================
void show_title(void) {
    fill_screen(BLACK);
    init_stars();
    draw_stars();

    draw_string(20, 20, "SPACE",   CYAN,   BLACK, 3);
    draw_string(10, 50, "SHOOTER",  WHITE,  BLACK, 3);
    draw_player_scaled(TFT_W/2, 105, 0, true, 3);
    draw_enemy_scaled(60,  170, 0, 0, 2);
    draw_enemy_scaled(120, 170, 1, 0, 2);
    draw_enemy_scaled(180, 170, 2, 0, 2);
    draw_string(18, 200, "BASIC   FAST   TANK", GRAY, BLACK, 1);
    draw_string(5,  215, "ARROW KEYS TO MOVE",  WHITE, BLACK, 1);
    draw_string(15, 225, "AUTO-FIRE ACTIVE",    YELLOW, BLACK, 1);

    sound_start();
    delay_ms(3000);
    fill_screen(BLACK);
}

// ============================================================
//  WAVE CLEAR
// ============================================================
void show_wave_clear(void) {
    for(uint8_t f = 0; f < 4; f++) {
        fill_rect(0, 0, TFT_W, TFT_H - 10, (f & 1) ? WHITE : BLACK);
        delay_ms(80);
    }

    char buf[20];
    sprintf(buf, "WAVE %u CLEAR!", wave);
    draw_string((TFT_W - (strlen(buf) * 12)) / 2, TFT_H/2 - 10,
                buf, YELLOW, BLACK, 2);
    sound_start();
    delay_ms(1500);
    fill_rect(0, 0, TFT_W, TFT_H - 10, BLACK);

    if(score > highScore) highScore = score;
}

// ============================================================
//  GAME OVER - VERSIÓN CORREGIDA (sin bucle infinito)
// ============================================================
void show_game_over(void) {
    fill_screen(BLACK);
    
    char buf[24];
    int centerX = TFT_W / 2;
    int centerY = TFT_H / 2;
    
    draw_string(centerX - 54, centerY - 40, "GAME OVER", RED, BLACK, 2);
    
    sprintf(buf, "SCORE: %u", score);
    draw_string(centerX - 48, centerY - 10, buf, YELLOW, BLACK, 2);
    
    sprintf(buf, "WAVE:  %u", wave);
    draw_string(centerX - 48, centerY + 10, buf, CYAN, BLACK, 2);
    
    if(score > highScore) highScore = score;
    sprintf(buf, "BEST:  %u", highScore);
    draw_string(centerX - 48, centerY + 30, buf, GREEN, BLACK, 2);
    
    draw_string(centerX - 60, centerY + 55, "RESTARTING...", WHITE, BLACK, 1);
    
    sound_death();
    delay_ms(3000);
}

// ============================================================
//  DIBUJAR TODO EL FRAME
// ============================================================
static void draw_frame(void) {
    // Limpiar estrellas anteriores (anti-flicker estelar)
    for(uint8_t i = 0; i < NUM_STARS; i++) {
        if(prevStarValid[i]) {
            int16_t sx = prevStarX[i];
            int16_t sy = prevStarY[i];
            if(sx >= 0 && sy >= 0 && sx < (int16_t)TFT_W && sy < (int16_t)(TFT_H - 10)) {
                uint8_t sz = stars[i].brightness - 1;
                if(sz > 0)
                    fill_rect(sx, sy, sz, sz, BLACK);
                else
                    draw_pixel(sx, sy, BLACK);
            }
        }
    }
    // Guardar posiciones actuales para próximo frame
    for(uint8_t i = 0; i < NUM_STARS; i++) {
        prevStarX[i] = stars[i].x;
        prevStarY[i] = stars[i].y;
        prevStarValid[i] = true;
    }

    fill_rect(prevPlayerX - 1, prevPlayerY - 1,
              PLAYER_W + 2, PLAYER_H + 2, BLACK);

    for(int i = 0; i < MAX_ENEMIES; i++) {
        if(prevEnemyActive[i]) {
            fill_rect(prevEnemyX[i] - 1, prevEnemyY[i] - 1,
                      ENEMY_W + 2, ENEMY_H + 2, BLACK);
        }
    }

    for(int i = 0; i < MAX_BULLETS; i++) {
        if(prevBulletActive[i]) {
            fill_rect(prevBulletX[i] - 1, prevBulletY[i] - 1,
                      BULLET_W + 2, BULLET_H + 2, BLACK);
        }
    }

    draw_stars();

    if(powerUpVisible) {
        fill_circle(powerUpX + 4, powerUpY + 4, 5, YELLOW);
        fill_circle(powerUpX + 4, powerUpY + 4, 3, WHITE);
        draw_string(powerUpX - 4, powerUpY - 10, "S", GREEN, BLACK, 1);
    }

    for(int i = 0; i < MAX_BULLETS; i++) {
        if(bullets[i].active) {
            draw_bullet(bullets[i].x, bullets[i].y, true);
        }
    }

    for(int i = 0; i < MAX_ENEMIES; i++) {
        if(enemies[i].active) {
            draw_enemy(enemies[i].x, enemies[i].y,
                       enemies[i].type, enemies[i].animFrame);
        }
    }

    for(int i = 0; i < MAX_EXPLOSIONS; i++) {
        if(explosions[i].active) {
            draw_explosion(explosions[i].x, explosions[i].y,
                           explosions[i].frame);
        }
    }

    bool thrust = (playerMoveX != 0 || playerMoveY != 0);
    if(powerUpActive) {
        draw_player(playerX, playerY, playerDir, thrust);
        fill_rect(playerX, playerY, PLAYER_W, PLAYER_H, COLOR565(255, 215, 0));
        draw_player(playerX, playerY, playerDir, thrust);
    } else {
        draw_player(playerX, playerY, playerDir, thrust);
    }

    draw_hud();
}

// ============================================================
//  GAME LOOP PRINCIPAL - VERSIÓN CORREGIDA
// ============================================================
void game_loop(void) {
    bool running = true;
    
    while(running) {
        show_title();
        score = 0; lives = 3; wave = 1; highScore = 0;
        srand((unsigned)(time(nullptr)));
        init_game();
        draw_frame();

        struct timespec ts_last, ts_now;
        clock_gettime(CLOCK_MONOTONIC, &ts_last);

        while(!gameOver) {
#ifdef USE_BUTTONS
            if(BTN_UP)         { playerMoveX = 0; playerMoveY = -1; playerDir = 0; }
            else if(BTN_DOWN)  { playerMoveX = 0; playerMoveY = 1;  playerDir = 1; }
            else if(BTN_LEFT)  { playerMoveX = -1; playerMoveY = 0; playerDir = 2; }
            else if(BTN_RIGHT) { playerMoveX = 1;  playerMoveY = 0; playerDir = 3; }
            else               { playerMoveX = 0;  playerMoveY = 0; }
#else
            static uint8_t ac = 0;
            static int8_t demoDir = 1;
            if(++ac > 30) {
                ac = 0;
                playerMoveX = demoDir;
                playerMoveY = 0;
                if(playerX >= TFT_W - PLAYER_W - 10) demoDir = -1;
                if(playerX <= 10) demoDir = 1;
            }
            fire_bullet();
#endif

            update_stars();
            update_player();
            update_bullets();
            update_enemies();
            update_explosions();
            spawn_powerup();
            check_collisions();
            draw_frame();

            uint32_t frame_us = 30000UL;
            clock_gettime(CLOCK_MONOTONIC, &ts_now);
            long elapsed_us = (ts_now.tv_sec  - ts_last.tv_sec)  * 1000000L
                            + (ts_now.tv_nsec - ts_last.tv_nsec) / 1000L;
            if(elapsed_us < (long)frame_us)
                delay_us((uint32_t)(frame_us - elapsed_us));
            clock_gettime(CLOCK_MONOTONIC, &ts_last);
        }
        
        show_game_over();
        
        gameOver = false;
        gameState = STATE_TITLE;
        score = 0;
        lives = 3;
        wave = 1;
        init_game();
    }
}

} // namespace GameEngine
