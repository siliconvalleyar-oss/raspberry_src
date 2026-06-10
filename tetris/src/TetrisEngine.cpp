// ============================================================
//  TetrisEngine.cpp — Motor de juego Tetris completo
//  SRS rotation + wall kicks + T-spin + combo + B2B + 7-bag
// ============================================================
#include "TetrisEngine.h"
#include "TetrisSound.h"
#include "TetrisGfx.h"
#include "Hardware.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// ============================================================
//  DEFINICION DE PIEZAS — SRS  (col,fila relativo al pivot)
//  4 rotaciones x 4 bloques x [col,fila]
// ============================================================
static const int8_t PIECE_DATA[7][4][4][2] = {
    // I  (pivot en col 1.5,fila 1.5 → se ajusta en offsets SRS)
    {{{-1,0},{0,0},{1,0},{2,0}},
     {{1,-1},{1,0},{1,1},{1,2}},
     {{-1,1},{0,1},{1,1},{2,1}},
     {{0,-1},{0,0},{0,1},{0,2}}},
    // O
    {{{0,-1},{1,-1},{0,0},{1,0}},
     {{0,-1},{1,-1},{0,0},{1,0}},
     {{0,-1},{1,-1},{0,0},{1,0}},
     {{0,-1},{1,-1},{0,0},{1,0}}},
    // T
    {{{-1,0},{0,0},{1,0},{0,-1}},
     {{0,-1},{0,0},{0,1},{1,0}},
     {{-1,0},{0,0},{1,0},{0,1}},
     {{0,-1},{0,0},{0,1},{-1,0}}},
    // S
    {{{-1,0},{0,0},{0,-1},{1,-1}},
     {{0,-1},{0,0},{1,0},{1,1}},
     {{-1,1},{0,1},{0,0},{1,0}},
     {{-1,-1},{-1,0},{0,0},{0,1}}},
    // Z
    {{{-1,-1},{0,-1},{0,0},{1,0}},
     {{1,-1},{0,0},{1,0},{0,1}},
     {{-1,0},{0,0},{0,1},{1,1}},
     {{0,-1},{-1,0},{0,0},{-1,1}}},
    // J
    {{{-1,-1},{-1,0},{0,0},{1,0}},
     {{0,-1},{1,-1},{0,0},{0,1}},
     {{-1,0},{0,0},{1,0},{1,1}},
     {{0,-1},{0,0},{-1,1},{0,1}}},
    // L
    {{{1,-1},{-1,0},{0,0},{1,0}},
     {{0,-1},{0,0},{0,1},{1,1}},
     {{-1,0},{0,0},{1,0},{-1,1}},
     {{-1,-1},{0,-1},{0,0},{0,1}}},
};

// ============================================================
//  WALL KICKS SRS
//  JLSTZ: tablas estandar
//  I:     tablas propias
//  Formato: [rot_from*4+rot_to][kick_index][dx,dy]
// ============================================================
// 0->1, 1->2, 2->3, 3->0  y sus inversas
static const int8_t WK_JLSTZ[8][5][2] = {
    {{0,0},{-1,0},{-1,-1},{0, 2},{-1, 2}}, // 0->1
    {{0,0},{ 1,0},{ 1, 1},{0,-2},{ 1,-2}}, // 1->0
    {{0,0},{ 1,0},{ 1, 1},{0,-2},{ 1,-2}}, // 1->2
    {{0,0},{-1,0},{-1,-1},{0, 2},{-1, 2}}, // 2->1
    {{0,0},{ 1,0},{ 1,-1},{0, 2},{ 1, 2}}, // 2->3
    {{0,0},{-1,0},{-1, 1},{0,-2},{-1,-2}}, // 3->2
    {{0,0},{-1,0},{-1, 1},{0,-2},{-1,-2}}, // 3->0
    {{0,0},{ 1,0},{ 1,-1},{0, 2},{ 1, 2}}, // 0->3
};
static const int8_t WK_I[8][5][2] = {
    {{0,0},{-2,0},{ 1,0},{-2, 1},{ 1,-2}}, // 0->1
    {{0,0},{ 2,0},{-1,0},{ 2,-1},{-1, 2}}, // 1->0
    {{0,0},{-1,0},{ 2,0},{-1,-2},{ 2, 1}}, // 1->2
    {{0,0},{ 1,0},{-2,0},{ 1, 2},{-2,-1}}, // 2->1
    {{0,0},{ 2,0},{-1,0},{ 2,-1},{-1, 2}}, // 2->3
    {{0,0},{-2,0},{ 1,0},{-2, 1},{ 1,-2}}, // 3->2
    {{0,0},{ 1,0},{-2,0},{ 1, 2},{-2,-1}}, // 3->0
    {{0,0},{-1,0},{ 2,0},{-1,-2},{ 2, 1}}, // 0->3
};

static int wk_index(int from, int to){
    if(from==0&&to==1) return 0;
    if(from==1&&to==0) return 1;
    if(from==1&&to==2) return 2;
    if(from==2&&to==1) return 3;
    if(from==2&&to==3) return 4;
    if(from==3&&to==2) return 5;
    if(from==3&&to==0) return 6;
    if(from==0&&to==3) return 7;
    return 0;
}

// ============================================================
//  GRAVEDAD POR NIVEL  (frames entre caidas automaticas)
//  Guideline: nivel 1 = cada 48 frames (a 60fps)
//  Aqui usamos 30fps  → ajustado
// ============================================================
static const uint8_t GRAVITY_FRAMES[16] = {
    0,  // nivel 0 (no usado)
    30, 24, 20, 16, 12,  // 1-5
    10,  8,  6,  5,  4,  // 6-10
     3,  3,  2,  2,  1   // 11-15
};

// Puntos base por lineas borradas (Guideline)
static const uint32_t LINE_SCORE[5] = {0, 100, 300, 500, 800};
// Multiplicador B2B (Back-to-Back Tetris o T-spin)
#define B2B_BONUS 150

// ============================================================
//  ESTADO DEL JUEGO
// ============================================================
static uint8_t  board[FIELD_ROWS][FIELD_COLS];  // 0=vacío, 1-7=color+1
static uint8_t  prev_board[FIELD_ROWS][FIELD_COLS]; // para dibujo diferencial

// Pieza activa
static int8_t   cur_type, cur_rot;
static int16_t  cur_x, cur_y;
static int8_t   ghost_y;

// Cola de piezas (7-bag)
static uint8_t  bag[7];
static uint8_t  bag_pos;
static uint8_t  next_type;

// Hold
static uint8_t  hold_type;
static bool     can_hold;

// Estadísticas
static uint32_t score, best_score;
static uint16_t lines_cleared;
static uint8_t  level;
static uint32_t combo;
static bool     b2b;        // back-to-back activo
static bool     paused;
static bool     game_running;

// Lock delay
static uint8_t  lock_timer;
static bool     on_floor;
#define LOCK_DELAY_FRAMES 15  // a 30fps = 500ms

// Auto-repeat para izq/der
static int8_t   ar_key;
static uint8_t  ar_timer;
static uint8_t  ar_phase;   // 0=espera inicial, 1=repeticion

// Frame timing
static uint8_t  gravity_timer;

// T-spin detection
static bool     last_was_tspin;

// ============================================================
//  7-BAG RANDOMIZER
// ============================================================
static void fill_bag(void){
    for(int i=0;i<7;i++) bag[i]=i;
    // Fisher-Yates shuffle
    for(int i=6;i>0;i--){
        int j=rand()%(i+1);
        uint8_t tmp=bag[i]; bag[i]=bag[j]; bag[j]=tmp;
    }
    bag_pos=0;
}

static uint8_t next_from_bag(void){
    if(bag_pos >= 7) fill_bag();
    return bag[bag_pos++];
}

// ============================================================
//  VALIDACION — verifica si la pieza cabe en (x,y,rot)
// ============================================================
static bool piece_fits(int8_t type,int8_t rot,int16_t x,int16_t y){
    for(int b=0;b<4;b++){
        int16_t bc = x + PIECE_DATA[type][rot][b][0];
        int16_t br = y + PIECE_DATA[type][rot][b][1];
        if(bc<0||bc>=FIELD_COLS) return false;
        if(br>=FIELD_ROWS)       return false;
        if(br>=0 && board[br][bc]) return false;
    }
    return true;
}

// ============================================================
//  GHOST PIECE — posicion mas baja donde cabe
// ============================================================
static int8_t compute_ghost(void){
    int16_t gy = cur_y;
    while(piece_fits(cur_type,cur_rot,cur_x,gy+1)) gy++;
    return (int8_t)gy;
}

// ============================================================
//  DIBUJO DIFERENCIAL — solo redibuja lo que cambio
// ============================================================
static int8_t  prev_ghost_y = -1;
static int16_t prev_ghost_x = -1;
static int8_t  prev_ghost_rot = -1;
static int16_t prev_cur_x = -1;
static int16_t prev_cur_y = -1;
static int8_t  prev_cur_rot = -1;

static void erase_piece_at(int8_t type,int8_t rot,int16_t x,int16_t y,bool ghost){
    for(int b=0;b<4;b++){
        int16_t bc=x+PIECE_DATA[type][rot][b][0];
        int16_t br=y+PIECE_DATA[type][rot][b][1];
        if(br<0||br>=FIELD_ROWS||bc<0||bc>=FIELD_COLS) continue;
        if(board[br][bc]) TGfx::draw_block(bc,br,PIECE_COLORS[board[br][bc]-1]);
        else {
            if(ghost) TGfx::clear_block(bc,br);
            else      TGfx::clear_block(bc,br);
        }
    }
}

static void draw_piece_at(int8_t type,int8_t rot,int16_t x,int16_t y,bool ghost){
    uint16_t c = PIECE_COLORS[type];
    for(int b=0;b<4;b++){
        int16_t bc=x+PIECE_DATA[type][rot][b][0];
        int16_t br=y+PIECE_DATA[type][rot][b][1];
        if(br<0||br>=FIELD_ROWS||bc<0||bc>=FIELD_COLS) continue;
        if(ghost) TGfx::draw_ghost_block(bc,br,c);
        else      TGfx::draw_block(bc,br,c);
    }
}

static void update_display(void){
    // 1) Borrar ghost anterior si cambio
    if(prev_ghost_y>=0 &&
       (prev_ghost_y!=ghost_y||prev_cur_x!=cur_x||prev_ghost_rot!=cur_rot)){
        erase_piece_at(cur_type,prev_ghost_rot,(int16_t)prev_cur_x,prev_ghost_y,true);
    }
    // 2) Borrar pieza activa anterior
    if(prev_cur_y>=0){
        erase_piece_at(cur_type,prev_cur_rot,prev_cur_x,prev_cur_y,false);
    }
    // 3) Dibujar ghost nuevo
    if(ghost_y != cur_y){
        draw_piece_at(cur_type,cur_rot,cur_x,ghost_y,true);
    }
    // 4) Dibujar pieza activa
    draw_piece_at(cur_type,cur_rot,cur_x,cur_y,false);

    prev_ghost_y  = ghost_y;
    prev_cur_x    = cur_x;
    prev_cur_y    = cur_y;
    prev_cur_rot  = cur_rot;
    prev_ghost_rot= cur_rot;
    prev_ghost_x  = cur_x;
}

// ============================================================
//  LOCK PIECE — fijar pieza en el tablero
// ============================================================
static bool lock_piece(void){
    for(int b=0;b<4;b++){
        int16_t bc=cur_x+PIECE_DATA[cur_type][cur_rot][b][0];
        int16_t br=cur_y+PIECE_DATA[cur_type][cur_rot][b][1];
        if(br<0) { return false; }  // game over: pieza fuera del campo
        if(bc>=0&&bc<FIELD_COLS&&br>=0&&br<FIELD_ROWS)
            board[br][bc] = (uint8_t)(cur_type+1);
    }
    return true;
}

// ============================================================
//  LIMPIAR LINEAS COMPLETAS
// ============================================================
static int clear_lines(int cleared_rows[4]){
    int n=0;
    for(int r=FIELD_ROWS-1;r>=0;r--){
        bool full=true;
        for(int c=0;c<FIELD_COLS&&full;c++)
            if(!board[r][c]) full=false;
        if(full) cleared_rows[n++]=r;
    }
    if(n==0) return 0;

    // Animacion flash
    TGfx::flash_lines(cleared_rows,n);

    // Animacion borrado linea a linea
    for(int i=0;i<n;i++) TGfx::clear_line_anim(cleared_rows[i]);

    // Compactar tablero (caer filas de arriba)
    for(int i=0;i<n;i++){
        int row=cleared_rows[i];
        for(int r=row;r>0;r--)
            memcpy(board[r],board[r-1],FIELD_COLS);
        memset(board[0],0,FIELD_COLS);
        // Ajustar indices de lineas pendientes
        for(int j=i+1;j<n;j++)
            if(cleared_rows[j]<=row) cleared_rows[j]++;
    }
    return n;
}

// ============================================================
//  T-SPIN DETECTION (simplificado: 3-corner rule)
// ============================================================
static bool detect_tspin(void){
    if(cur_type != PIECE_T) return false;
    // Contar esquinas bloqueadas alrededor del pivot T
    int corners=0;
    const int8_t cx[4]={-1,-1,1,1};
    const int8_t cy2[4]={-1,1,-1,1};
    for(int i=0;i<4;i++){
        int16_t nc=cur_x+cx[i];
        int16_t nr=cur_y+cy2[i];
        if(nc<0||nc>=FIELD_COLS||nr<0||nr>=FIELD_ROWS||board[nr][nc])
            corners++;
    }
    return corners>=3;
}

// ============================================================
//  ROTAR CON SRS WALL KICKS
// ============================================================
static bool rotate_piece(int dir){  // dir=+1 o -1
    int8_t new_rot=(int8_t)((cur_rot+4+dir)%4);
    int wi=wk_index(cur_rot,new_rot);
    const int8_t (*wk)[2] = (cur_type==PIECE_I) ?
                              WK_I[wi] : WK_JLSTZ[wi];
    for(int k=0;k<5;k++){
        int16_t nx=cur_x+wk[k][0];
        int16_t ny=cur_y+wk[k][1];
        if(piece_fits(cur_type,new_rot,nx,ny)){
            cur_rot=new_rot;
            cur_x=nx; cur_y=ny;
            return true;
        }
    }
    return false;
}

// ============================================================
//  SPAWNAR NUEVA PIEZA
// ============================================================
static bool spawn_piece(uint8_t type){
    cur_type = (int8_t)type;
    cur_rot  = 0;
    cur_x    = FIELD_COLS/2 - 1;
    cur_y    = 0;
    // Intentar aparecer 1 fila mas arriba si no cabe
    if(!piece_fits(cur_type,0,cur_x,cur_y)){
        cur_y = -1;
        if(!piece_fits(cur_type,0,cur_x,cur_y)) return false; // game over
    }
    ghost_y        = compute_ghost();
    lock_timer     = 0;
    on_floor       = false;
    prev_cur_y     = -1;
    prev_ghost_y   = -1;
    last_was_tspin = false;
    return true;
}

// ============================================================
//  SCORING  (Guideline oficial)
// ============================================================
static void add_score(int n_lines, bool tspin){
    if(n_lines==0 && !tspin) return;

    uint32_t pts=0;
    bool is_difficult = (n_lines==4) || tspin;

    if(tspin){
        // T-spin mini / normal
        const uint32_t ts_base[4]={400,800,1200,1600};
        pts = ts_base[n_lines<4?n_lines:3];
    } else {
        pts = LINE_SCORE[n_lines<5?n_lines:4];
    }

    pts *= level;

    // B2B bonus
    if(is_difficult){
        if(b2b) pts += (pts * B2B_BONUS)/100;
        b2b=true;
    } else if(n_lines>0){
        b2b=false;
    }

    // Combo bonus
    if(n_lines>0 && combo>0)
        pts += 50 * combo * level;
    if(n_lines>0) combo++;
    else          combo=0;

    score += pts;
    if(score>best_score) best_score=score;
}

// ============================================================
//  SUBIDA DE NIVEL
// ============================================================
static void check_level_up(uint16_t prev_lines){
    // Cada 10 lineas sube de nivel
    uint8_t new_level=(uint8_t)(lines_cleared/10+1);
    if(new_level>15) new_level=15;
    if(new_level > level){
        level=new_level;
        snd_levelup();
        TGfx::draw_level_banner(level);
        delay_ms(1200);
        TGfx::redraw_board(board);
        TGfx::draw_grid();
    }
    (void)prev_lines;
}

// ============================================================
//  AUTO-REPEAT (DAS — Delayed Auto-Shift)
// ============================================================
static bool process_ar(int key, bool &moved){
    moved=false;
    if(key!=KEY_LEFT && key!=KEY_RIGHT){
        ar_key=-1; ar_timer=0; ar_phase=0;
        return false;
    }
    if(ar_key != key){
        ar_key=key; ar_timer=0; ar_phase=0;
        return true; // primer disparo
    }
    if(ar_phase==0){
        ar_timer++;
        if(ar_timer >= AR_DELAY_FRAMES){ ar_phase=1; ar_timer=0; }
        return false;
    } else {
        ar_timer++;
        if(ar_timer >= AR_RATE_FRAMES){ ar_timer=0; moved=true; return true; }
        return false;
    }
}

// ============================================================
//  REDIBUJADO COMPLETO DEL PANEL
// ============================================================
static void refresh_panel(void){
    TGfx::draw_next_piece(next_type,0);
    TGfx::draw_hold_piece(hold_type,0,can_hold);
    TGfx::draw_stats(score,best_score,level,lines_cleared,combo);
    TGfx::draw_hud(score,best_score,level,lines_cleared);
}

// ============================================================
//  LOOP PRINCIPAL DEL JUEGO
// ============================================================
static void play_game(void){
    // Init estado
    memset(board,0,sizeof(board));
    memset(prev_board,0,sizeof(prev_board));
    score=0; lines_cleared=0; level=1;
    combo=0; b2b=false; paused=false;
    hold_type=PIECE_NONE; can_hold=true;
    ar_key=-1; ar_timer=0; ar_phase=0;
    gravity_timer=0;
    prev_cur_y=-1; prev_ghost_y=-1;
    game_running=true;

    fill_bag();
    next_type=next_from_bag();

    // Pantalla inicial del campo
    TGfx::fill_screen(COLOR565(5,5,20));
    TGfx::draw_field_frame();
    TGfx::draw_grid();
    TGfx::draw_panel_labels();
    refresh_panel();

    if(!spawn_piece(next_type)){ game_running=false; return; }
    next_type=next_from_bag();
    TGfx::draw_next_piece(next_type,0);

    // Frame timing: 30fps
    struct timespec ts_last, ts_now;
    clock_gettime(CLOCK_MONOTONIC, &ts_last);
    const long FRAME_NS = 33333333L;  // 33.3ms = 30fps

    while(game_running){

        // ---- PAUSA ----
        if(paused){
            int k=input_read();
            if(k==KEY_PAUSE||k==KEY_START) paused=false;
            delay_ms(50);
            continue;
        }

        // ---- INPUT ----
        int key = input_read();
        bool ar_fire=false, ar_moved=false;

        if(key==KEY_LEFT||key==KEY_RIGHT){
            ar_fire = process_ar(key, ar_moved);
        } else if(key!=KEY_NONE){
            ar_key=-1; ar_timer=0; ar_phase=0;
        }

        // Procesar tecla
        if(key==KEY_QUIT){ game_running=false; break; }

        if(key==KEY_PAUSE){ paused=true; TGfx::draw_pause(); continue; }

        // Mover izquierda/derecha
        if(ar_fire || ar_moved){
            int dx = (key==KEY_LEFT)?-1:1;
            if(piece_fits(cur_type,cur_rot,cur_x+dx,cur_y)){
                cur_x+=dx;
                ghost_y=compute_ghost();
                snd_move();
                lock_timer=0; // resetear lock delay al mover
            }
        }

        // Rotar
        if(key==KEY_ROT){
            if(rotate_piece(+1)){
                ghost_y=compute_ghost();
                snd_rotate();
                lock_timer=0;
                last_was_tspin=detect_tspin();
            }
        }

        // Soft drop
        if(key==KEY_DOWN){
            if(piece_fits(cur_type,cur_rot,cur_x,cur_y+1)){
                cur_y++; score++; gravity_timer=0;
            }
        }

        // Hard drop
        if(key==KEY_DROP){
            int drop_dist=0;
            while(piece_fits(cur_type,cur_rot,cur_x,cur_y+1)){
                cur_y++; drop_dist++;
            }
            score += (uint32_t)(drop_dist*2);
            snd_harddrop();
            // Forzar lock inmediato
            lock_timer=LOCK_DELAY_FRAMES+1;
        }

        // Hold
        if(key==KEY_START && can_hold){
            erase_piece_at(cur_type,cur_rot,cur_x,cur_y,false);
            erase_piece_at(cur_type,cur_rot,cur_x,ghost_y,true);
            prev_cur_y=-1; prev_ghost_y=-1;

            uint8_t save=hold_type;
            hold_type=cur_type;
            can_hold=false;
            snd_hold();

            if(save==PIECE_NONE) save=next_from_bag();
            // Hacer la pieza guardada la siguiente del bag
            uint8_t tmp=next_type;
            // Invertir: la nueva activa es 'save', el next queda igual
            spawn_piece(save);
            // next_type se mantiene
            (void)tmp;

            TGfx::draw_hold_piece(hold_type,0,can_hold);
            ghost_y=compute_ghost();
        }

        // ---- GRAVEDAD ----
        gravity_timer++;
        uint8_t gframes=GRAVITY_FRAMES[level<16?level:15];
        if(gravity_timer >= gframes){
            gravity_timer=0;
            if(piece_fits(cur_type,cur_rot,cur_x,cur_y+1)){
                cur_y++;
                on_floor=false;
                lock_timer=0;
            } else {
                on_floor=true;
            }
        }

        // ---- LOCK DELAY ----
        if(on_floor){
            lock_timer++;
            if(lock_timer >= LOCK_DELAY_FRAMES){
                // Fijar pieza
                bool ok=lock_piece();
                snd_lock();

                if(!ok){
                    game_running=false;
                    break;
                }

                // Detectar T-spin antes de borrar lineas
                bool tspin=last_was_tspin && detect_tspin();
                if(tspin) TGfx::draw_tspin_banner();

                // Borrar lineas completas
                int cleared[4]={0};
                int n=clear_lines(cleared);

                if(n==4){
                    TGfx::draw_tetris_flash();
                    snd_tetris();
                } else if(tspin && n>0){
                    snd_tspin();
                } else {
                    switch(n){
                        case 1: snd_clear1(); break;
                        case 2: snd_clear2(); break;
                        case 3: snd_clear3(); break;
                    }
                }

                uint16_t prev_l=lines_cleared;
                lines_cleared+=(uint16_t)n;
                add_score(n,tspin);
                check_level_up(prev_l);

                // Redibujar campo tras caida de filas
                TGfx::redraw_board(board);
                TGfx::draw_grid();

                // Siguiente pieza
                can_hold=true;
                bool spawned=spawn_piece(next_type);
                if(!spawned){ game_running=false; break; }
                next_type=next_from_bag();

                refresh_panel();
                prev_cur_y=-1; prev_ghost_y=-1;
                on_floor=false; lock_timer=0;
            }
        }

        // ---- DIBUJO DIFERENCIAL ----
        update_display();

        // ---- HUD cada 2 frames ----
        static uint8_t hud_cnt=0;
        if(++hud_cnt>=2){ hud_cnt=0; refresh_panel(); }

        // ---- FRAME TIMING ----
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        long elapsed=(ts_now.tv_sec-ts_last.tv_sec)*1000000000L
                    +(ts_now.tv_nsec-ts_last.tv_nsec);
        if(elapsed<FRAME_NS){
            struct timespec sl={0, FRAME_NS-elapsed};
            nanosleep(&sl,nullptr);
        }
        ts_last=ts_now;
    }
}

// ============================================================
//  ENTRY POINT
// ============================================================
extern void keyboard_restore(void);

void tetris_run(void){
    best_score=0;

title:
    TGfx::draw_title();
    snd_start();

    // Esperar ENTER o boton
    while(true){
        int k=input_read();
        if(k==KEY_START||k==KEY_DROP||k==KEY_ROT) break;
        if(gpio_btn(BTN_LEFT_PIN)||gpio_btn(BTN_RIGHT_PIN)||
           gpio_btn(BTN_ROT_PIN) ||gpio_btn(BTN_DOWN_PIN)) break;
        delay_ms(30);
    }
    delay_ms(200); // debounce

play:
    play_game();

    // Game over
    snd_gameover();
    TGfx::draw_gameover(score,best_score,level,lines_cleared);

    // Esperar accion
    delay_ms(500);
    while(true){
        int k=input_read();
        if(k==KEY_START||k==KEY_DROP||k==KEY_QUIT||k==KEY_ROT) break;
        if(gpio_btn(BTN_LEFT_PIN)||gpio_btn(BTN_ROT_PIN)) break;
        delay_ms(30);
    }
    delay_ms(200);

    // Preguntar si jugar de nuevo (presionando Q va a titulo)
    {
        int k=input_read();
        if(k==KEY_QUIT) goto title;
    }
    goto play;
}
