// ============================================================
//  TetrisSound.cpp — bit-bang GPIO 12
//  Identico al codigo que funciono en Pac-Man y Dino.
// ============================================================
#include "TetrisSound.h"
#include "Hardware.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

static int snd_fd = -1;

void tetris_sound_init(void){
    int fd = open("/dev/gpiochip0", O_RDONLY);
    if(fd < 0){ fprintf(stderr,"Sound: no gpiochip0\n"); return; }
    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = PIN_SOUND;
    req.lines          = 1;
    req.flags          = GPIOHANDLE_REQUEST_OUTPUT;
    strncpy(req.consumer_label,"tetris_snd",15);
    if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req)==0){
        snd_fd = req.fd;
        fprintf(stderr,"Sound: GPIO %d OK\n", PIN_SOUND);
    }
    close(fd);
}

void tetris_beep(uint16_t freq, uint16_t ms){
    if(snd_fd<0){ if(ms) usleep((uint32_t)ms*1000); return; }
    if(!freq)    { if(ms) usleep((uint32_t)ms*1000); return; }
    uint32_t half = 500000u / freq;
    uint32_t n    = ((uint32_t)ms * 1000u) / (half*2u);
    if(n<1) n=1;
    struct gpiohandle_data d; memset(&d,0,sizeof(d));
    for(uint32_t i=0;i<n;i++){
        d.values[0]=1; ioctl(snd_fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&d);
        delay_us(half);
        d.values[0]=0; ioctl(snd_fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&d);
        delay_us(half);
    }
}

// Nota musical — silencio si freq=0
static void note(uint16_t freq, uint16_t ms){
    if(freq) tetris_beep(freq, ms);
    else     delay_ms(ms);
}

// ============================================================
//  EFECTOS
// ============================================================
void snd_move(void){
    tetris_beep(880, 8);
}

void snd_rotate(void){
    tetris_beep(660, 10);
    tetris_beep(880, 10);
}

void snd_lock(void){
    tetris_beep(300, 18);
    tetris_beep(200, 14);
}

void snd_harddrop(void){
    for(int f=800;f>=200;f-=60) tetris_beep((uint16_t)f, 6);
}

void snd_hold(void){
    tetris_beep(523,15); tetris_beep(659,15); tetris_beep(784,20);
}

// 1 linea — click satisfactorio
void snd_clear1(void){
    tetris_beep(523, 40);
    tetris_beep(659, 50);
}

// 2 lineas
void snd_clear2(void){
    tetris_beep(523,30); tetris_beep(659,30); tetris_beep(784,50);
}

// 3 lineas
void snd_clear3(void){
    tetris_beep(523,25); tetris_beep(659,25);
    tetris_beep(784,25); tetris_beep(1047,70);
}

// TETRIS! 4 lineas — fanfarria completa
void snd_tetris(void){
    // Re-Mi-Re-Si-Do-La
    note(587,80); note(659,80); note(587,80);
    note(494,80); note(523,80); note(440,160);
    delay_ms(20);
    note(659,80); note(698,80); note(659,80);
    note(587,80); note(659,80); note(784,160);
}

// T-spin — efecto especial
void snd_tspin(void){
    tetris_beep(1047,30); delay_ms(10);
    tetris_beep(1319,30); delay_ms(10);
    tetris_beep(1047,30); delay_ms(10);
    tetris_beep(1568,80);
}

// Subida de nivel
void snd_levelup(void){
    note(523,60); note(659,60); note(784,60); note(1047,120);
}

// Game over — descendente melancólico
void snd_gameover(void){
    note(523,120); note(440,120); note(370,120);
    note(330,120); note(294,120); note(220,300);
}

// ============================================================
//  INTRO — fragmento de Korobeiniki (tema A de Tetris)
//  Re-Mi-Fa-La | Sol-Fa-Mi-Do | Re-Do-Re-Mi-Fa
// ============================================================
void snd_start(void){
    // Korobeiniki — tema clásico del Tetris
    struct{ uint16_t f; uint16_t ms; } melody[] = {
        {587,120},{0,20},{740,60},{0,20},{659,120},{0,20},
        {554,60}, {0,20},{587,120},{0,20},{440,60},{0,20},
        {440,180},{0,40},
        {494,120},{0,20},{587,60},{0,20},{740,120},{0,20},
        {698,60}, {0,20},{659,180},{0,40},
        {523,120},{0,20},{659,60},{0,20},{587,120},{0,20},
        {523,60}, {0,20},{494,120},{0,20},{440,60},{0,20},
        {440,240},{0,60},
    };
    int len = (int)(sizeof(melody)/sizeof(melody[0]));
    for(int i=0;i<len;i++) note(melody[i].f, melody[i].ms);
}
