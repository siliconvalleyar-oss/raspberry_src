// DinoSound.h
#ifndef DINO_SOUND_H
#define DINO_SOUND_H
#include <stdint.h>

void dino_sound_init(void);
void dino_sound_beep(uint16_t freq_hz, uint16_t duration_ms);

// Efectos
void snd_jump(void);      // salto
void snd_land(void);      // aterrizaje
void snd_die(void);       // muerte
void snd_point(void);     // milestone 100pts
void snd_levelup(void);   // subida de nivel
void snd_start(void);     // intro
void snd_pterodactyl(void); // enemigo aereo aparece
void snd_powerup(void);   // item especial

#endif
