#include "../include/Sound.h"
#include "../include/HardwareProfile.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

static int gpio_fd = -1;

void sound_init() {
    int fd = open("/dev/gpiochip0", O_RDONLY);
    if(fd < 0) return;
    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = PIN_SOUND;
    req.lines = 1;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    strncpy(req.consumer_label, "mario_sound", 15);
    if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) == 0)
        gpio_fd = req.fd;
    close(fd);
}

static void gpio_beep(uint16_t freq_hz, uint16_t duration_ms) {
    if(gpio_fd < 0) { if(duration_ms) delay_ms(duration_ms); return; }
    if(freq_hz == 0) { delay_ms(duration_ms); return; }
    uint32_t half_period_us = 500000 / freq_hz;
    uint32_t cycles = (duration_ms * 1000) / (half_period_us * 2);
    if(cycles < 1) cycles = 1;
    struct gpiohandle_data data;
    for(uint32_t i=0; i<cycles; i++) {
        data.values[0] = 1;
        ioctl(gpio_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
        delay_us(half_period_us);
        data.values[0] = 0;
        ioctl(gpio_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
        delay_us(half_period_us);
    }
}

void sound_beep(uint16_t freq, uint16_t dur) { gpio_beep(freq, dur); }
void sound_jump()   { gpio_beep(600, 80); }
void sound_coin()   { gpio_beep(988, 50); delay_ms(30); gpio_beep(1319, 50); }
void sound_stomp()  { gpio_beep(200, 100); }
void sound_powerup(){ gpio_beep(660, 80); delay_ms(40); gpio_beep(880, 80); delay_ms(40); gpio_beep(1100, 120); }
void sound_mario_die(){ gpio_beep(440, 200); delay_ms(60); gpio_beep(350, 200); delay_ms(60); gpio_beep(260, 350); }
void sound_stop()   {}
