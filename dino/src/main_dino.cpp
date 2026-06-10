// ============================================================
//  main_dino.cpp — Chrome Dino Arcade / RPi Zero 2W
//  Capa de hardware: /dev/spidev0.0 + /dev/gpiochip0 +
//  entrada unificada GPIO+teclado + bit-bang sound.
//
//  Compilar: make
//  Ejecutar: sudo ./bin/dino
// ============================================================

#include "DinoHardware.h"
#include "DinoGraphics.h"
#include "DinoEngine.h"
#include "DinoSound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/gpio.h>
#include <termios.h>
#include <sys/select.h>

// ============================================================
//  ESTADO GLOBAL DE HARDWARE
// ============================================================
static int spi_fd       = -1;
static int gpio_chip_fd = -1;
static int gpio_out_fd  = -1;
static int gpio_btn_fd  = -1;

#define N_OUT 3
static const uint32_t out_pins[N_OUT] = { PIN_DC, PIN_RST, PIN_BL };
static uint8_t        pin_state[N_OUT] = { 0, 1, 0 };

#define N_BTN 2
static const uint32_t btn_pins[N_BTN] = { BTN_JUMP_PIN, BTN_COLOR_PIN };

static struct termios orig_termios;

// ============================================================
//  GPIO
// ============================================================
static int gpio_init(void) {
    gpio_chip_fd = open(GPIO_CHIP, O_RDONLY);
    if(gpio_chip_fd < 0) { perror("open gpiochip0"); return -1; }

    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.lines = N_OUT;
    for(int i=0;i<N_OUT;i++){
        req.lineoffsets[i]    = out_pins[i];
        req.default_values[i] = pin_state[i];
    }
    strncpy(req.consumer_label, "dino_out", 15);
    if(ioctl(gpio_chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0){
        perror("GPIO salidas"); close(gpio_chip_fd); return -2;
    }
    gpio_out_fd = req.fd;

    memset(&req, 0, sizeof(req));
    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = N_BTN;
    for(int i=0;i<N_BTN;i++) req.lineoffsets[i] = btn_pins[i];
    strncpy(req.consumer_label, "dino_btn", 15);
    if(ioctl(gpio_chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) >= 0)
        gpio_btn_fd = req.fd;

    return 0;
}

void gpio_write(int pin, int val) {
    int idx = -1;
    for(int i=0;i<N_OUT;i++) if((int)out_pins[i]==pin){ idx=i; break; }
    if(idx < 0) return;
    pin_state[idx] = val ? 1 : 0;
    struct gpiohandle_data data;
    memset(&data, 0, sizeof(data));
    for(int i=0;i<N_OUT;i++) data.values[i] = pin_state[i];
    ioctl(gpio_out_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
}

int gpio_read(int pin) {
    if(gpio_btn_fd < 0) return 1;
    int idx = -1;
    for(int i=0;i<N_BTN;i++) if((int)btn_pins[i]==pin){ idx=i; break; }
    if(idx < 0) return 1;
    struct gpiohandle_data data;
    memset(&data, 0, sizeof(data));
    ioctl(gpio_btn_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
    return data.values[idx];
}

// ============================================================
//  SPI
// ============================================================
static int spi_init_dev(void) {
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if(spi_fd < 0){ perror("open spidev0.0"); return -1; }

    uint8_t  mode  = SPI_MODE_3;
    uint8_t  bits  = 8;
    uint32_t speed = SPI_SPEED_HZ;
    ioctl(spi_fd, SPI_IOC_WR_MODE,          &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if(ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0){
        speed = 32000000;
        ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    }
    fprintf(stderr, "SPI: %u MHz modo 3\n", speed/1000000);
    return 0;
}

void spi_write_byte(uint8_t d) {
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf        = (unsigned long)&d;
    tr.len           = 1;
    tr.speed_hz      = SPI_SPEED_HZ;
    tr.bits_per_word = 8;
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

void spi_write_buf(const uint8_t *buf, uint32_t len) {
    if(!len) return;
    const uint32_t CHUNK = 4096;
    for(uint32_t off=0; off<len; off+=CHUNK){
        uint32_t n = (len-off < CHUNK) ? len-off : CHUNK;
        struct spi_ioc_transfer tr;
        memset(&tr, 0, sizeof(tr));
        tr.tx_buf        = (unsigned long)(buf+off);
        tr.len           = n;
        tr.speed_hz      = SPI_SPEED_HZ;
        tr.bits_per_word = 8;
        ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
    }
}

// ============================================================
//  DELAY
// ============================================================
void delay_ms(uint32_t ms){
    struct timespec ts={ (time_t)(ms/1000), (long)(ms%1000)*1000000L };
    nanosleep(&ts, nullptr);
}
void delay_us(uint32_t us){
    struct timespec ts={ 0, (long)us*1000L };
    nanosleep(&ts, nullptr);
}

// ============================================================
//  PROTOCOLO ST7789
// ============================================================
void write_cmd(uint8_t c)  { DC_LOW();  spi_write_byte(c); }
void write_data(uint8_t d) { DC_HIGH(); spi_write_byte(d); }
void push_color(uint16_t c){ DC_HIGH();
    uint8_t b[2]={(uint8_t)(c>>8),(uint8_t)(c&0xFF)};
    spi_write_buf(b,2); }

void push_color_n(uint16_t color, uint32_t n){
    if(!n) return;
    DC_HIGH();
    const uint32_t BUF_PX=512;
    uint8_t buf[BUF_PX*2];
    uint8_t hi=color>>8, lo=color&0xFF;
    uint32_t fill=(n<BUF_PX)?n:BUF_PX;
    for(uint32_t i=0;i<fill;i++){buf[2*i]=hi;buf[2*i+1]=lo;}
    for(uint32_t left=n; left>0;){
        uint32_t c2=(left<BUF_PX)?left:BUF_PX;
        spi_write_buf(buf, c2*2);
        left-=c2;
    }
}

void set_window(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1){
    write_cmd(0x2A);
    write_data(x0>>8); write_data(x0&0xFF);
    write_data(x1>>8); write_data(x1&0xFF);
    write_cmd(0x2B);
    write_data(y0>>8); write_data(y0&0xFF);
    write_data(y1>>8); write_data(y1&0xFF);
    write_cmd(0x2C);
    DC_HIGH();
}

void reset_display(void){
    RST_HIGH(); delay_ms(10);
    RST_LOW();  delay_ms(20);
    RST_HIGH(); delay_ms(150);
}

void init_display(void){
    reset_display();
    write_cmd(0x11); delay_ms(120);
    write_cmd(0x36); write_data(0x00);
    write_cmd(0x3A); write_data(0x05);
    write_cmd(0x21);
    write_cmd(0x13);
    write_cmd(0xB2); write_data(0x0C); write_data(0x0C);
                     write_data(0x00); write_data(0x33); write_data(0x33);
    write_cmd(0xB7); write_data(0x35);
    write_cmd(0xBB); write_data(0x37);
    write_cmd(0xC0); write_data(0x2C);
    write_cmd(0xC2); write_data(0x01);
    write_cmd(0xC3); write_data(0x12);
    write_cmd(0xC4); write_data(0x20);
    write_cmd(0xC6); write_data(0x0F);
    write_cmd(0xD0); write_data(0xA4); write_data(0xA1);
    write_cmd(0xE0);
    { const uint8_t g[]={0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,
                         0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
      for(int i=0;i<14;i++) write_data(g[i]); }
    write_cmd(0xE1);
    { const uint8_t g[]={0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,
                         0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
      for(int i=0;i<14;i++) write_data(g[i]); }
    write_cmd(0x29); delay_ms(120);
    fprintf(stderr,"Display ST7789 OK\n");
}

// ============================================================
//  LIMPIEZA + SEÑALES
// ============================================================
static void cleanup(void){
    BL_LOW();
    if(gpio_out_fd>=0){ close(gpio_out_fd); gpio_out_fd=-1; }
    if(gpio_btn_fd>=0){ close(gpio_btn_fd); gpio_btn_fd=-1; }
    if(gpio_chip_fd>=0){ close(gpio_chip_fd); gpio_chip_fd=-1; }
    if(spi_fd>=0){ close(spi_fd); spi_fd=-1; }
    fprintf(stderr,"cleanup OK\n");
}
void hw_close(void){ cleanup(); }
static void sig_handler(int s){ (void)s; cleanup(); _exit(0); }

int hw_init(void){
    signal(SIGINT, sig_handler);
    signal(SIGTERM,sig_handler);
    if(gpio_init()    < 0) return -1;
    if(spi_init_dev() < 0){ cleanup(); return -2; }
    RST_HIGH(); DC_LOW(); BL_LOW();
    return 0;
}

// ============================================================
//  TECLADO (modo no canonico, sin echo)
// ============================================================
static int stdin_ready(void) {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO+1, &fds, NULL, NULL, &tv) > 0;
}

static int getch_nonblock(void) {
    if(!stdin_ready()) return -1;
    char c;
    if(read(STDIN_FILENO, &c, 1) != 1) return -1;
    return c;
}

void keyboard_init(void) {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN]  = 0;
    new_termios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

void keyboard_restore(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// ============================================================
//  LECTURA DE BOTONES UNIFICADA (GPIO + teclado)
//  Se llama desde las macros BTN_JUMP / BTN_COLOR
// ============================================================
int dino_read_btn(int pin) {
    // Intentar GPIO primero
    int gpio_val = gpio_read(pin);
    if(gpio_val == 0) return 1;  // activo bajo = presionado

    // Fallback a teclado
    int c = getch_nonblock();
    if(pin == BTN_JUMP_PIN)
        return (c == ' ' || c == 'w' || c == 'W' || c == 0x41);
    if(pin == BTN_COLOR_PIN)
        return (c == 'c' || c == 'C');
    return 0;
}

// ============================================================
//  MAIN
// ============================================================
int main(void){
    fprintf(stderr, "=== CHROME DINO ARCADE — RPi Zero 2W (teclado consola) ===\n");
    keyboard_init();

    if(hw_init() < 0){
        fprintf(stderr, "ERROR hw_init.\n"
                        "  1) sudo ./bin/dino\n"
                        "  2) dtparam=spi=on en /boot/firmware/config.txt\n");
        return 1;
    }

    init_display();
    delay_ms(50);
    BL_HIGH();
    fprintf(stderr, "Backlight ON\n");

    srand((unsigned)time(nullptr));
    dino_sound_init();
    dino_game_loop();

    hw_close();
    keyboard_restore();
    return 0;
}
