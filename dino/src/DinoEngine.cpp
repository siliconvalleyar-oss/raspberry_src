// ============================================================
//  DinoEngine.cpp — Chrome Dino Arcade
//  Motor de juego completo:
//   * Fisica de salto con gravedad suavizada
//   * 7 tipos de obstaculos (cactus S/M/D/T, ptero bajo/medio, roca)
//   * Power-ups: escudo (invencible 5s) y slow-mo (3s)
//   * 10 niveles con velocidad y dificultad crecientes
//   * Ciclo dia/noche (nivel par=dia, impar=noche)
//   * Nubes y estrellas animadas con parallax
//   * Toggle color / escala de grises
//   * HUD: score, hi-score, nivel, vidas, modo color
//   * 3 vidas, game-over con pantalla dedicada
//   * Anti-flicker: borrar bbox exacto antes de redibujar
// ============================================================

#include "DinoEngine.h"
#include "DinoGraphics.h"
#include "DinoSound.h"
#include "DinoHardware.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================
//  TABLAS DE CONSTANTES (referenciadas desde DinoEngine.h)
// ============================================================
const int16_t  OBS_W[7]      = {14, 20, 32, 46, 40, 40, 28};
const int16_t  OBS_H[7]      = {30, 36, 30, 36, 22, 22, 16};
const int16_t  OBS_YOFF[7]   = {30, 36, 30, 36, 30, 50, 16};

const int16_t  LEVEL_SPEED[MAX_LEVEL+1]  = {0, 30, 36, 42, 48, 55, 62, 70, 78, 87, 96};
const uint8_t  LEVEL_GAP[MAX_LEVEL+1]    = {0, 80, 72, 64, 56, 50, 44, 38, 32, 28, 24};
const uint32_t LEVEL_SCORE[MAX_LEVEL+1]  = {0,0,100,250,450,700,1000,1400,1900,2500,3200};

// ============================================================
//  ESTADO GLOBAL
// ============================================================
static uint32_t g_score   = 0;
static uint32_t g_hi      = 0;
static uint8_t  g_lives   = 3;
static uint8_t  g_level   = 1;
static bool     g_running = false;
static bool     g_dead    = false;

static int16_t  dino_y;
static int16_t  dino_vy;
static bool     dino_jumping;
static bool     dino_on_ground;
static uint8_t  dino_frame;
static uint8_t  dino_anim_cnt;
static bool     dino_shield;
static uint8_t  shield_timer;
static bool     dino_slow;
static uint8_t  slow_timer;

static int16_t  g_speed;
static int16_t  speed_accum;
static uint32_t ground_scroll;

static Obstacle obs[MAX_OBS];
static uint8_t  obs_timer;

static Cloud    clouds[MAX_CLOUDS];
static uint8_t  cloud_timer;

static Star     stars[MAX_STARS];

static PowerUp  powerups[MAX_POWERUPS];
static uint8_t  pu_timer;

static uint32_t next_milestone;
static bool     g_night;

// Scroll parallax
static uint32_t mountain_scroll;
static uint32_t dune_scroll;

// ============================================================
//  HELPERS
// ============================================================
static inline int16_t obs_screen_y(const Obstacle &o) {
    return GROUND_LINE - OBS_YOFF[o.type];
}

static uint16_t sky_color(void) {
    if(g_night) return gc(COLOR565(15,15,40), BLACK);
    return gc(SKY_BLUE, WHITE);
}

static uint8_t random_obs_type(void) {
    int r = rand() % 100;
    if(g_level <= 2){
        if(r < 60) return OBS_CACTUS_S;
        if(r < 85) return OBS_CACTUS_M;
        return OBS_CACTUS_D;
    } else if(g_level <= 4){
        if(r < 30) return OBS_CACTUS_S;
        if(r < 55) return OBS_CACTUS_M;
        if(r < 70) return OBS_CACTUS_D;
        if(r < 80) return OBS_CACTUS_T;
        if(r < 90) return OBS_PTERO_LOW;
        return OBS_ROCK;
    } else if(g_level <= 7){
        if(r < 20) return OBS_CACTUS_S;
        if(r < 38) return OBS_CACTUS_M;
        if(r < 52) return OBS_CACTUS_D;
        if(r < 64) return OBS_CACTUS_T;
        if(r < 76) return OBS_PTERO_LOW;
        if(r < 88) return OBS_PTERO_MID;
        return OBS_ROCK;
    } else {
        if(r < 15) return OBS_CACTUS_S;
        if(r < 28) return OBS_CACTUS_M;
        if(r < 40) return OBS_CACTUS_D;
        if(r < 52) return OBS_CACTUS_T;
        if(r < 64) return OBS_PTERO_LOW;
        if(r < 80) return OBS_PTERO_MID;
        return OBS_ROCK;
    }
}

// ============================================================
//  INICIALIZAR JUEGO
// ============================================================
static void game_init(bool full_reset) {
    if(full_reset){
        g_score  = 0;
        g_lives  = 3;
        g_level  = 1;
    }
    g_running    = true;
    g_dead       = false;
    g_night      = (g_level % 2 == 0);
    g_speed      = LEVEL_SPEED[g_level];
    speed_accum  = 0;
    ground_scroll  = 0;
    mountain_scroll = 0;
    dune_scroll     = 0;

    dino_y         = DINO_GROUND_Y;
    dino_vy        = 0;
    dino_jumping   = false;
    dino_on_ground = true;
    dino_frame     = 0;
    dino_anim_cnt  = 0;
    dino_shield    = false;
    shield_timer   = 0;
    dino_slow      = false;
    slow_timer     = 0;

    memset(obs,      0, sizeof(obs));
    memset(clouds,   0, sizeof(clouds));
    memset(stars,    0, sizeof(stars));
    memset(powerups, 0, sizeof(powerups));

    obs_timer   = LEVEL_GAP[g_level];
    cloud_timer = 20;
    pu_timer    = 200 + rand()%100;

    next_milestone = (g_score / 100 + 1) * 100;

    if(g_night){
        for(int i=0;i<MAX_STARS;i++){
            stars[i].x = rand() % SCREEN_W;
            stars[i].y = HUD_H + 4 + rand() % 60;
            stars[i].active = true;
        }
    }
    for(int i=0;i<MAX_CLOUDS;i++){
        clouds[i].x = 20 + i * (SCREEN_W/MAX_CLOUDS) + rand()%30;
        clouds[i].y = HUD_H + 5 + rand()%40;
        clouds[i].speed = 1 + rand()%2;
        clouds[i].active = true;
    }
}

// ============================================================
//  DIBUJAR FONDO COMPLETO
// ============================================================
static void draw_background(void) {
    uint16_t sky = sky_color();
    DG::fill_rect(0, HUD_H, SCREEN_W, SCREEN_H - HUD_H, sky);

    DG::draw_mountains(GROUND_LINE - 2, mountain_scroll, g_night);
    DG::draw_dunes(GROUND_LINE - 2, dune_scroll, g_night);

    if(g_night){
        DG::draw_moon(196, HUD_H+8, (g_level/2)%4);
        for(int i=0;i<MAX_STARS;i++)
            if(stars[i].active)
                DG::draw_star(stars[i].x, stars[i].y);
    }
    for(int i=0;i<MAX_CLOUDS;i++)
        if(clouds[i].active)
            DG::draw_cloud(clouds[i].x, clouds[i].y);

    DG::draw_ground(GROUND_LINE, ground_scroll);
}

static void erase_rect(int16_t x, int16_t y, int16_t w, int16_t h) {
    DG::fill_rect(x, y, w, h, sky_color());
}

// ============================================================
//  FISICA DEL DINO
// ============================================================
static void update_dino_physics(bool jump_pressed) {
    if(jump_pressed && dino_on_ground){
        dino_vy = JUMP_VEL * GRAVITY_FRAC;
        dino_jumping   = true;
        dino_on_ground = false;
        snd_jump();
    }

    int grav = dino_slow ? GRAVITY_INT : GRAVITY_INT*2;

    if(!dino_on_ground){
        dino_vy += grav;
        dino_y  += dino_vy / GRAVITY_FRAC;

        if(dino_y >= DINO_GROUND_Y){
            dino_y         = DINO_GROUND_Y;
            dino_vy        = 0;
            dino_jumping   = false;
            dino_on_ground = true;
            if(!dino_jumping) snd_land();
        }
    }

    if(dino_on_ground){
        if(++dino_anim_cnt >= 8){
            dino_anim_cnt = 0;
            dino_frame = (dino_frame == 1) ? 2 : 1;
        }
    } else {
        dino_frame = 0;
    }

    if(dino_shield && shield_timer > 0){
        shield_timer--;
        if(shield_timer == 0) dino_shield = false;
    }
    if(dino_slow && slow_timer > 0){
        slow_timer--;
        if(slow_timer == 0){
            dino_slow = false;
            g_speed = LEVEL_SPEED[g_level];
        }
    }
}

// ============================================================
//  ACTUALIZAR OBSTACULOS
// ============================================================
static void update_obstacles(int16_t move_px) {
    for(int i=0;i<MAX_OBS;i++){
        if(!obs[i].active) continue;
        erase_rect(obs[i].x, obs_screen_y(obs[i]),
                   OBS_W[obs[i].type], OBS_H[obs[i].type]);
        obs[i].x -= move_px;
        if(obs[i].type == OBS_PTERO_LOW || obs[i].type == OBS_PTERO_MID){
            if(++obs[i].frame >= 16) obs[i].frame = 0;
        }
        if(obs[i].x + OBS_W[obs[i].type] < 0) obs[i].active = false;
    }

    if(--obs_timer <= 0){
        obs_timer = LEVEL_GAP[g_level] + (int16_t)(rand()%30) - 10;
        if(obs_timer < 12) obs_timer = 12;
        for(int i=0;i<MAX_OBS;i++){
            if(!obs[i].active){
                obs[i].active = true;
                obs[i].x      = SCREEN_W + 4;
                obs[i].type   = (uint8_t)random_obs_type();
                obs[i].frame  = 0;
                break;
            }
        }
    }

    for(int i=0;i<MAX_OBS;i++){
        if(!obs[i].active) continue;
        int16_t sy = obs_screen_y(obs[i]);
        if(obs[i].type == OBS_PTERO_LOW || obs[i].type == OBS_PTERO_MID)
            DG::draw_ptero(obs[i].x, sy, obs[i].frame/8);
        else if(obs[i].type == OBS_ROCK)
            DG::draw_rock(obs[i].x, sy);
        else
            DG::draw_cactus(obs[i].x, sy, obs[i].type);
    }
}

// ============================================================
//  ACTUALIZAR NUBES
// ============================================================
static void update_clouds(void) {
    for(int i=0;i<MAX_CLOUDS;i++){
        if(!clouds[i].active) continue;
        erase_rect(clouds[i].x, clouds[i].y, 46, 18);
        clouds[i].x -= clouds[i].speed;
        if(clouds[i].x + 46 < 0){
            clouds[i].x = SCREEN_W + 4 + rand()%60;
            clouds[i].y = HUD_H + 5 + rand()%45;
            clouds[i].speed = 1 + rand()%2;
        }
        DG::draw_cloud(clouds[i].x, clouds[i].y);
    }

    if(--cloud_timer <= 0){
        cloud_timer = 30 + rand()%40;
        for(int i=0;i<MAX_CLOUDS;i++){
            if(!clouds[i].active){
                clouds[i].x = SCREEN_W + 4;
                clouds[i].y = HUD_H + 5 + rand()%45;
                clouds[i].speed = 1 + rand()%2;
                clouds[i].active = true;
                break;
            }
        }
    }
}

// ============================================================
//  ACTUALIZAR POWER-UPS
// ============================================================
static void update_powerups(int16_t move_px) {
    for(int i=0;i<MAX_POWERUPS;i++){
        if(!powerups[i].active) continue;
        erase_rect(powerups[i].x, powerups[i].y, 16, 16);
        powerups[i].x -= move_px;
        if(powerups[i].x + 16 < 0) powerups[i].active = false;
        else DG::draw_shield(powerups[i].x, powerups[i].y);
    }

    if(--pu_timer <= 0){
        pu_timer = 200 + rand()%200;
        for(int i=0;i<MAX_POWERUPS;i++){
            if(!powerups[i].active){
                powerups[i].active = true;
                powerups[i].x = SCREEN_W + 4;
                powerups[i].y = GROUND_LINE - 24 - rand()%20;
                powerups[i].type = rand()%2;
                break;
            }
        }
    }
}

// ============================================================
//  COLISIONES (AABB con margen de tolerancia)
// ============================================================
static bool aabb(int16_t ax,int16_t ay,int16_t aw,int16_t ah,
                 int16_t bx,int16_t by,int16_t bw,int16_t bh,
                 int16_t margin){
    return (ax+margin < bx+bw-margin) &&
           (ax+aw-margin > bx+margin) &&
           (ay+margin < by+bh-margin) &&
           (ay+ah-margin > by+margin);
}

static bool check_collisions(void) {
    int16_t dx = DINO_X + 4;
    int16_t dy = dino_y + 2;
    int16_t dw = DINO_W - 8;
    int16_t dh = DINO_H - 4;

    for(int i=0;i<MAX_OBS;i++){
        if(!obs[i].active) continue;
        int16_t sy = obs_screen_y(obs[i]);
        if(aabb(dx,dy,dw,dh,
                obs[i].x, sy,
                OBS_W[obs[i].type], OBS_H[obs[i].type], 4)){
            if(!dino_shield) return true;
            erase_rect(obs[i].x, sy, OBS_W[obs[i].type], OBS_H[obs[i].type]);
            obs[i].active = false;
            g_score += 20;
        }
    }

    for(int i=0;i<MAX_POWERUPS;i++){
        if(!powerups[i].active) continue;
        if(aabb(dx,dy,dw,dh,
                powerups[i].x, powerups[i].y, 16, 16, 2)){
            erase_rect(powerups[i].x, powerups[i].y, 16, 16);
            powerups[i].active = false;
            if(powerups[i].type == PU_SHIELD){
                dino_shield  = true;
                shield_timer = 25*5;
            } else {
                dino_slow  = true;
                slow_timer = 25*3;
                g_speed    = g_speed * 6 / 10;
            }
            snd_powerup();
        }
    }
    return false;
}

// ============================================================
//  LEER INPUT
// ============================================================
static bool read_jump(void) {
    return BTN_JUMP;
}

static bool read_color_toggle(void) {
    static bool last = false;
    bool cur = BTN_COLOR;
    if(cur && !last){ last=cur; return true; }
    last = cur;
    return false;
}

// ============================================================
//  LOOP PRINCIPAL
// ============================================================
void dino_game_loop(void) {
    g_color_mode = true;
    DG::draw_title();
    snd_start();

    while(!read_jump()) delay_ms(20);
    delay_ms(200);

    bool full_reset = true;

restart:
    game_init(full_reset);
    full_reset = false;

    DG::fill_rect(0, HUD_H, SCREEN_W, SCREEN_H - HUD_H, sky_color());
    draw_background();
    DG::draw_hud(g_score, g_hi, g_lives, g_level, g_color_mode);
    DG::draw_dino(DINO_X, dino_y, dino_frame, false);

    struct timespec ts_last, ts_now;
    clock_gettime(CLOCK_MONOTONIC, &ts_last);
    const long FRAME_NS = 40000000L;

    uint32_t frame_cnt = 0;

    while(g_running) {

        bool jump = read_jump();

        if(read_color_toggle()){
            g_color_mode = !g_color_mode;
            DG::fill_rect(0, HUD_H, SCREEN_W, SCREEN_H-HUD_H, sky_color());
            draw_background();
            DG::draw_hud(g_score, g_hi, g_lives, g_level, g_color_mode);
        }

        speed_accum += g_speed;
        int16_t move_px = speed_accum / 10;
        speed_accum   %= 10;
        if(move_px < 1) move_px = 1;

        ground_scroll  += move_px;
        mountain_scroll += move_px;
        dune_scroll     += move_px;

        int16_t prev_dino_y = dino_y;
        update_dino_physics(jump);

        g_score += 1;
        if(g_score > g_hi) g_hi = g_score;

        if(g_score >= next_milestone){
            next_milestone += 100;
            snd_point();
        }

        if(g_level < MAX_LEVEL &&
           g_score >= LEVEL_SCORE[g_level+1]){
            g_level++;
            g_speed = LEVEL_SPEED[g_level];
            g_night = (g_level % 2 == 0);
            snd_levelup();

            DG::draw_level_banner(g_level);
            delay_ms(1500);

            DG::fill_rect(0, HUD_H, SCREEN_W, SCREEN_H-HUD_H, sky_color());
            draw_background();

            if(g_night){
                for(int i=0;i<MAX_STARS;i++){
                    stars[i].x = rand()%SCREEN_W;
                    stars[i].y = HUD_H+4+rand()%60;
                    stars[i].active = true;
                }
            }
        }

        // Fondo parallax: montañas y dunas se dibujan cada frame
        DG::draw_mountains(GROUND_LINE - 2, mountain_scroll, g_night);
        DG::draw_dunes(GROUND_LINE - 2, dune_scroll, g_night);

        if(dino_y != prev_dino_y || (frame_cnt % 8 == 0)){
            erase_rect(DINO_X, prev_dino_y, DINO_W, DINO_H);
            DG::draw_ground(GROUND_LINE, ground_scroll);
        }

        update_clouds();
        update_obstacles(move_px);
        update_powerups(move_px);

        DG::draw_ground(GROUND_LINE, ground_scroll);

        bool show = true;
        if(dino_shield && (frame_cnt % 4 < 2)) show = false;
        if(show) DG::draw_dino(DINO_X, dino_y, dino_frame, false);

        if(check_collisions()){
            g_dead = true;
            g_running = false;
        }

        if(frame_cnt % 4 == 0)
            DG::draw_hud(g_score, g_hi, g_lives, g_level, g_color_mode);

        frame_cnt++;

        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        long elapsed = (ts_now.tv_sec  - ts_last.tv_sec )*1000000000L
                      +(ts_now.tv_nsec - ts_last.tv_nsec);
        if(elapsed < FRAME_NS){
            struct timespec sl = {0, FRAME_NS - elapsed};
            nanosleep(&sl, nullptr);
        }
        ts_last = ts_now;
    }

    // ---- MUERTE ----
    if(g_dead){
        snd_die();

        erase_rect(DINO_X, dino_y, DINO_W, DINO_H);
        DG::draw_dino(DINO_X, dino_y, 0, true);
        delay_ms(800);

        g_lives--;
        if(g_lives == 0){
            DG::draw_gameover(g_score, g_hi);
            delay_ms(1000);
            while(!read_jump()) delay_ms(20);
            delay_ms(200);
            g_lives    = 3;
            g_score    = 0;
            g_level    = 1;
            full_reset = true;
            goto restart;
        } else {
            delay_ms(500);
            goto restart;
        }
    }
}
