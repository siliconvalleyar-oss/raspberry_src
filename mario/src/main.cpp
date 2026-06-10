#include "../include/HardwareProfile.h"
#include "../include/Renderer.h"
#include "../include/Game.h"
#include "../include/Sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/gpio.h>

static int spi_fd = -1;
static int gpio_fd = -1;
static int gpio_out_fd = -1;
//static int gpio_in_fd = -1;

#define IDX_DC     0
#define IDX_RST    1
#define IDX_BL     2
#define N_OUT_PINS 3

static const uint32_t out_pins[N_OUT_PINS] = { PIN_DC, PIN_RST, PIN_BL };
static uint8_t pin_state[N_OUT_PINS] = {0, 1, 0};

static int gpio_init() {
    gpio_fd = open(GPIO_CHIP, O_RDONLY);
    if(gpio_fd < 0) return -1;
    struct gpiohandle_request req_out;
    memset(&req_out, 0, sizeof(req_out));
    req_out.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req_out.lines = N_OUT_PINS;
    for(int i=0;i<N_OUT_PINS;i++) {
        req_out.lineoffsets[i] = out_pins[i];
        req_out.default_values[i] = pin_state[i];
    }
    strncpy(req_out.consumer_label, "mario_out", 15);
    if(ioctl(gpio_fd, GPIO_GET_LINEHANDLE_IOCTL, &req_out) < 0) {
        close(gpio_fd); return -2;
    }
    gpio_out_fd = req_out.fd;
    return 0;
}

void gpio_write(int pin, int val) {
    int idx = -1;
    for(int i=0;i<N_OUT_PINS;i++) if((int)out_pins[i]==pin) { idx=i; break; }
    if(idx<0) return;
    pin_state[idx] = val ? 1 : 0;
    struct gpiohandle_data data;
    memset(&data, 0, sizeof(data));
    for(int i=0;i<N_OUT_PINS;i++) data.values[i] = pin_state[i];
    ioctl(gpio_out_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
}

int gpio_read(int pin) { return 0; }

static int spi_init_dev() {
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if(spi_fd < 0) return -1;
    uint8_t mode = SPI_MODE_3;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    uint8_t bits = 8;
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    uint32_t speed = SPI_SPEED_HZ;
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    return 0;
}

void spi_write_byte(uint8_t d) {
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (unsigned long)&d;
    tr.len = 1;
    tr.speed_hz = SPI_SPEED_HZ;
    tr.bits_per_word = 8;
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

void spi_write_buf(const uint8_t *buf, uint32_t len) {
    const uint32_t CHUNK = 4096;
    uint32_t offset = 0;
    while(offset < len) {
        uint32_t chunk = (len - offset < CHUNK) ? (len - offset) : CHUNK;
        struct spi_ioc_transfer tr;
        memset(&tr, 0, sizeof(tr));
        tr.tx_buf = (unsigned long)(buf + offset);
        tr.len = chunk;
        tr.speed_hz = SPI_SPEED_HZ;
        tr.bits_per_word = 8;
        ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
        offset += chunk;
    }
}

void delay_ms(uint32_t ms) {
    struct timespec ts = { (time_t)(ms/1000), (long)(ms%1000)*1000000L };
    nanosleep(&ts, nullptr);
}

void delay_us(uint32_t us) {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    long target_ns = start.tv_nsec + (long)us * 1000L;
    time_t target_sec = start.tv_sec + target_ns / 1000000000L;
    target_ns %= 1000000000L;
    do {
        clock_gettime(CLOCK_MONOTONIC, &now);
    } while (now.tv_sec < target_sec || 
            (now.tv_sec == target_sec && now.tv_nsec < target_ns));
}

/*
void delay_us(uint32_t us) {
    struct timespec ts = { 0, (long)us * 1000L };
    nanosleep(&ts, nullptr);
}
*/
void write_cmd(uint8_t c) { DC_LOW(); spi_write_byte(c); }
void write_data(uint8_t d) { DC_HIGH(); spi_write_byte(d); }
void push_color(uint16_t color) {
    DC_HIGH();
    uint8_t buf[2] = { (uint8_t)(color>>8), (uint8_t)(color&0xFF) };
    spi_write_buf(buf, 2);
}
void push_color_n(uint16_t color, uint32_t n) {
    if(!n) return;
    DC_HIGH();
    const uint32_t BUF_PX = 512;
    uint8_t buf[BUF_PX*2];
    uint8_t hi = color>>8, lo = color&0xFF;
    for(uint32_t i=0;i<BUF_PX;i++) { buf[2*i]=hi; buf[2*i+1]=lo; }
    uint32_t left = n;
    while(left>0) {
        uint32_t chunk = (left<BUF_PX)?left:BUF_PX;
        spi_write_buf(buf, chunk*2);
        left -= chunk;
    }
}
void set_window(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1) {
    write_cmd(0x2A); write_data(x0>>8); write_data(x0&0xFF);
    write_data(x1>>8); write_data(x1&0xFF);
    write_cmd(0x2B); write_data(y0>>8); write_data(y0&0xFF);
    write_data(y1>>8); write_data(y1&0xFF);
    write_cmd(0x2C); DC_HIGH();
}
void reset_display() {
    RST_HIGH(); delay_ms(10);
    RST_LOW();  delay_ms(20);
    RST_HIGH(); delay_ms(150);
}
void init_display() {
    reset_display();
    write_cmd(0x11); delay_ms(120);
    write_cmd(0x36); write_data(0x00);
    write_cmd(0x3A); write_data(0x05);
    write_cmd(0x21);
    write_cmd(0x13);
    write_cmd(0xB2); write_data(0x0C); write_data(0x0C); write_data(0x00); write_data(0x33); write_data(0x33);
    write_cmd(0xB7); write_data(0x35);
    write_cmd(0xBB); write_data(0x37);
    write_cmd(0xC0); write_data(0x2C);
    write_cmd(0xC2); write_data(0x01);
    write_cmd(0xC3); write_data(0x12);
    write_cmd(0xC4); write_data(0x20);
    write_cmd(0xC6); write_data(0x0F);
    write_cmd(0xD0); write_data(0xA4); write_data(0xA1);
    write_cmd(0xE0); const uint8_t g1[]={0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
    for(int i=0;i<14;i++) write_data(g1[i]);
    write_cmd(0xE1); const uint8_t g2[]={0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
    for(int i=0;i<14;i++) write_data(g2[i]);
    write_cmd(0x29); delay_ms(120);
}
static void cleanup() {
    BL_LOW();
    if(gpio_out_fd>=0) close(gpio_out_fd);
    if(gpio_fd>=0) close(gpio_fd);
    if(spi_fd>=0) close(spi_fd);
}
static void sig_handler(int) { cleanup(); _exit(0); }
int hw_init() {
    signal(SIGINT, sig_handler); signal(SIGTERM, sig_handler);
    if(gpio_init()<0) return -1;
    if(spi_init_dev()<0) { cleanup(); return -2; }
    RST_HIGH(); DC_LOW(); BL_LOW();
    return 0;
}
void hw_close() { cleanup(); }

int main() {
    if(hw_init()<0) { fprintf(stderr,"HW init failed\n"); return 1; }
    init_display();
    delay_ms(50);
    BL_HIGH();
    sound_init();
    Renderer::init();
    Game game;
    game.run();
    hw_close();
    return 0;
}
