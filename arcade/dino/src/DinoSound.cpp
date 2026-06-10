// ============================================================
//  DinoSound.cpp — Sonido bit-bang GPIO 12
//  Codigo identico al Sound.cpp que funciono en el Pac-Man.
//  Buzzer pasivo entre GPIO 12 (Pin 32) y GND.
// ============================================================
#include "DinoSound.h"
#include "DinoHardware.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

static int snd_gpio_fd = -1;

void dino_sound_init(void) {
    int fd = open("/dev/gpiochip0", O_RDONLY);
    if(fd < 0){
        fprintf(stderr,"Sound: no se pudo abrir gpiochip0\n");
        return;
    }
    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = PIN_SOUND;
    req.lines          = 1;
    req.flags          = GPIOHANDLE_REQUEST_OUTPUT;
    strncpy(req.consumer_label, "dino_sound", 15);

    if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) == 0){
        snd_gpio_fd = req.fd;
        fprintf(stderr,"Sound: GPIO %d OK\n", PIN_SOUND);
    } else {
        fprintf(stderr,"Sound: error GPIO %d\n", PIN_SOUND);
    }
    close(fd);
}

// ---- Nucleo del bit-bang (identico al Pac-Man que funciono) ----
void dino_sound_beep(uint16_t freq_hz, uint16_t duration_ms) {
    if(snd_gpio_fd < 0){
        if(duration_ms) usleep((uint32_t)duration_ms * 1000);
        return;
    }
    if(freq_hz == 0){
        if(duration_ms) usleep((uint32_t)duration_ms * 1000);
        return;
    }

    uint32_t half_us = 500000u / freq_hz;
    uint32_t cycles  = ((uint32_t)duration_ms * 1000u) / (half_us * 2u);
    if(cycles < 1) cycles = 1;

    struct gpiohandle_data data;
    memset(&data, 0, sizeof(data));

    for(uint32_t i = 0; i < cycles; i++){
        data.values[0] = 1;
        ioctl(snd_gpio_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
        delay_us(half_us);
        data.values[0] = 0;
        ioctl(snd_gpio_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
        delay_us(half_us);
    }
}

// ============================================================
//  EFECTOS DE SONIDO ARCADE
// ============================================================

// Salto — glissando rapido ascendente
void snd_jump(void) {
    for(int f = 300; f <= 700; f += 40)
        dino_sound_beep((uint16_t)f, 12);
}

// Aterrizaje — golpe sordo
void snd_land(void) {
    dino_sound_beep(180, 25);
    dino_sound_beep(120, 20);
}

// Muerte — descenso dramatico
void snd_die(void) {
    dino_sound_beep(880, 80);
    delay_ms(20);
    dino_sound_beep(660, 80);
    delay_ms(20);
    dino_sound_beep(440, 100);
    delay_ms(20);
    dino_sound_beep(220, 200);
    delay_ms(20);
    dino_sound_beep(110, 300);
}

// Milestone 100 puntos — fanfarria corta
void snd_point(void) {
    dino_sound_beep(523, 60);   // C5
    dino_sound_beep(659, 60);   // E5
    dino_sound_beep(784, 90);   // G5
}

// Subida de nivel — arpegio ascendente
void snd_levelup(void) {
    const uint16_t notes[] = {523,659,784,1047};
    for(int i=0;i<4;i++){
        dino_sound_beep(notes[i], 70);
        delay_ms(10);
    }
}

// Intro — melodia de arranque
void snd_start(void) {
    // Do-Mi-Sol-Do (C4-E4-G4-C5)
    dino_sound_beep(262, 100); delay_ms(30);
    dino_sound_beep(330, 100); delay_ms(30);
    dino_sound_beep(392, 100); delay_ms(30);
    dino_sound_beep(523, 200);
}

// Pterodactilo — chirrido descendente amenazante
void snd_pterodactyl(void) {
    for(int f=900; f>=400; f-=50)
        dino_sound_beep((uint16_t)f, 10);
}

// Power-up (escudo/moneda) — efecto magico
void snd_powerup(void) {
    for(int f=400; f<=1200; f+=80)
        dino_sound_beep((uint16_t)f, 8);
    dino_sound_beep(1200, 80);
}
