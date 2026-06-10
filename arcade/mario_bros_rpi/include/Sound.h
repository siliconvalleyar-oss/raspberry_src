#ifndef SOUND_H
#define SOUND_H

#include <cstdint>

void sound_init();
void sound_beep(uint16_t freq_hz, uint16_t duration_ms);
void sound_jump();
void sound_coin();
void sound_stomp();
void sound_powerup();
void sound_mario_die();
void sound_stop();

#endif
