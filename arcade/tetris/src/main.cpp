// ============================================================
//  main.cpp — Tetris Arcade / RPi Zero 2W
//  Hardware identico al Dino/Pac-Man que funciono.
//  Agrega: teclado raw via termios (SSH o consola local)
//          + GPIO botones simultaneamente.
// ============================================================

#include "Hardware.h"
#include "TetrisEngine.h"
#include "TetrisSound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <linux/spi/spidev.h>
#include <linux/gpio.h>
#include <pthread.h>

// ============================================================
//  SPI + GPIO ESTADO GLOBAL
// ============================================================
static int spi_fd       = -1;
static int gpio_chip_fd = -1;
static int gpio_out_fd  = -1;   // DC, RST, BL (salidas display)
static int gpio_btn_fd  = -1;   // botones entrada

#define N_OUT 3
static const uint32_t out_pins[N_OUT] = { PIN_DC, PIN_RST, PIN_BL };
static uint8_t        pin_state[N_OUT]= { 0, 1, 0 };

#define N_BTN 5
static const uint32_t btn_pins[N_BTN] = {
    BTN_LEFT_PIN, BTN_RIGHT_PIN, BTN_ROT_PIN,
    BTN_DOWN_PIN, BTN_PAUSE_PIN
};

// ============================================================
//  GPIO
// ============================================================
static int gpio_init_hw(void) {
    gpio_chip_fd = open(GPIO_CHIP, O_RDONLY);
    if(gpio_chip_fd < 0){ perror("open gpiochip0"); return -1; }

    // Salidas: DC, RST, BL
    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.lines = N_OUT;
    for(int i=0;i<N_OUT;i++){
        req.lineoffsets[i]    = out_pins[i];
        req.default_values[i] = pin_state[i];
    }
    strncpy(req.consumer_label, "tetris_out", 15);
    if(ioctl(gpio_chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0){
        perror("GPIO salidas"); close(gpio_chip_fd); return -2;
    }
    gpio_out_fd = req.fd;

    // Entradas: botones
    memset(&req, 0, sizeof(req));
    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = N_BTN;
    for(int i=0;i<N_BTN;i++) req.lineoffsets[i] = btn_pins[i];
    strncpy(req.consumer_label, "tetris_btn", 15);
    if(ioctl(gpio_chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) >= 0)
        gpio_btn_fd = req.fd;
    else
        fprintf(stderr,"GPIO botones: no disponibles (solo teclado)\n");

    return 0;
}

void gpio_write(int pin, int val) {
    int idx = -1;
    for(int i=0;i<N_OUT;i++) if((int)out_pins[i]==pin){idx=i;break;}
    if(idx<0) return;
    pin_state[idx] = val ? 1 : 0;
    struct gpiohandle_data data;
    memset(&data, 0, sizeof(data));
    for(int i=0;i<N_OUT;i++) data.values[i] = pin_state[i];
    ioctl(gpio_out_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
}

int gpio_read(int pin) {
    (void)pin; return 1;
}

bool gpio_btn(int pin) {
    if(gpio_btn_fd < 0) return false;
    int idx = -1;
    for(int i=0;i<N_BTN;i++) if((int)btn_pins[i]==pin){idx=i;break;}
    if(idx<0) return false;
    struct gpiohandle_data data;
    memset(&data, 0, sizeof(data));
    ioctl(gpio_btn_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
    return data.values[idx] == 0; // activo bajo
}

// ============================================================
//  SPI
// ============================================================
static int spi_init_hw(void) {
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if(spi_fd<0){ perror("open spidev0.0"); return -1; }
    uint8_t  mode  = SPI_MODE_3;
    uint8_t  bits  = 8;
    uint32_t speed = SPI_SPEED_HZ;
    ioctl(spi_fd, SPI_IOC_WR_MODE,          &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if(ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0){
        speed = 32000000;
        ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    }
    fprintf(stderr,"SPI: %u MHz modo 3\n", speed/1000000);
    return 0;
}

void spi_write_byte(uint8_t d){
    struct spi_ioc_transfer tr;
    memset(&tr,0,sizeof(tr));
    tr.tx_buf=(unsigned long)&d; tr.len=1;
    tr.speed_hz=SPI_SPEED_HZ; tr.bits_per_word=8;
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

void spi_write_buf(const uint8_t *buf, uint32_t len){
    if(!len) return;
    const uint32_t CHUNK=4096;
    for(uint32_t off=0;off<len;off+=CHUNK){
        uint32_t n=(len-off<CHUNK)?len-off:CHUNK;
        struct spi_ioc_transfer tr;
        memset(&tr,0,sizeof(tr));
        tr.tx_buf=(unsigned long)(buf+off); tr.len=n;
        tr.speed_hz=SPI_SPEED_HZ; tr.bits_per_word=8;
        ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
    }
}

// ============================================================
//  DELAY
// ============================================================
void delay_ms(uint32_t ms){
    struct timespec ts={(time_t)(ms/1000),(long)(ms%1000)*1000000L};
    nanosleep(&ts,nullptr);
}
void delay_us(uint32_t us){
    struct timespec ts={0,(long)us*1000L};
    nanosleep(&ts,nullptr);
}

// ============================================================
//  PROTOCOLO ST7789 (identico al Dino/Pac-Man)
// ============================================================
void write_cmd(uint8_t c) { DC_LOW();  spi_write_byte(c); }
void write_data(uint8_t d){ DC_HIGH(); spi_write_byte(d); }
void push_color(uint16_t c){
    DC_HIGH();
    uint8_t b[2]={(uint8_t)(c>>8),(uint8_t)(c&0xFF)};
    spi_write_buf(b,2);
}
void push_color_n(uint16_t color,uint32_t n){
    if(!n) return;
    DC_HIGH();
    const uint32_t BUF_PX=512;
    uint8_t buf[BUF_PX*2];
    uint8_t hi=color>>8,lo=color&0xFF;
    uint32_t fill=(n<BUF_PX)?n:BUF_PX;
    for(uint32_t i=0;i<fill;i++){buf[2*i]=hi;buf[2*i+1]=lo;}
    for(uint32_t left=n;left>0;){
        uint32_t c2=(left<BUF_PX)?left:BUF_PX;
        spi_write_buf(buf,c2*2); left-=c2;
    }
}
void set_window(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1){
    write_cmd(0x2A);
    write_data(x0>>8);write_data(x0&0xFF);
    write_data(x1>>8);write_data(x1&0xFF);
    write_cmd(0x2B);
    write_data(y0>>8);write_data(y0&0xFF);
    write_data(y1>>8);write_data(y1&0xFF);
    write_cmd(0x2C); DC_HIGH();
}
void reset_display(void){
    RST_HIGH();delay_ms(10);
    RST_LOW(); delay_ms(20);
    RST_HIGH();delay_ms(150);
}
void init_display(void){
    reset_display();
    write_cmd(0x11);delay_ms(120);
    write_cmd(0x36);write_data(0x00);
    write_cmd(0x3A);write_data(0x05);
    write_cmd(0x21); write_cmd(0x13);
    write_cmd(0xB2);write_data(0x0C);write_data(0x0C);
    write_data(0x00);write_data(0x33);write_data(0x33);
    write_cmd(0xB7);write_data(0x35);
    write_cmd(0xBB);write_data(0x37);
    write_cmd(0xC0);write_data(0x2C);
    write_cmd(0xC2);write_data(0x01);
    write_cmd(0xC3);write_data(0x12);
    write_cmd(0xC4);write_data(0x20);
    write_cmd(0xC6);write_data(0x0F);
    write_cmd(0xD0);write_data(0xA4);write_data(0xA1);
    write_cmd(0xE0);
    {const uint8_t g[]={0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,
                        0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
     for(int i=0;i<14;i++)write_data(g[i]);}
    write_cmd(0xE1);
    {const uint8_t g[]={0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,
                        0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
     for(int i=0;i<14;i++)write_data(g[i]);}
    write_cmd(0x29);delay_ms(120);
    fprintf(stderr,"Display ST7789 OK\n");
}

// ============================================================
//  TECLADO RAW (termios) — no bloquea, devuelve char o 0
// ============================================================
static struct termios orig_termios;
static bool termios_active = false;

static void termios_restore(void){
    if(termios_active){
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        termios_active = false;
    }
}

static int termios_init(void){
    if(!isatty(STDIN_FILENO)){
        fprintf(stderr,"stdin no es tty — solo GPIO\n");
        return 0;
    }
    if(tcgetattr(STDIN_FILENO, &orig_termios) < 0) return 0;
    struct termios raw = orig_termios;
    raw.c_lflag    &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag    &= ~(IXON | ICRNL);
    raw.c_cc[VMIN]  = 0;   // no bloquea
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    termios_active = true;
    // Poner stdin en modo no-bloqueante
    int fl = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, fl | O_NONBLOCK);
    fprintf(stderr,"Teclado raw OK\n");
    return 1;
}

// Leer hasta 4 bytes (para secuencias de escape de flechas)
static int read_key_raw(void){
    uint8_t buf[8]; int n;
    n = (int)read(STDIN_FILENO, buf, sizeof(buf));
    if(n <= 0) return 0;
    if(n == 1){
        switch(buf[0]){
            case 'q': case 'Q': return KEY_QUIT;
            case 'p': case 'P': return KEY_PAUSE;
            case ' ':           return KEY_DROP;
            case 's': case 'S':
            case 'k': case 'K': return KEY_DOWN;
            case 'a': case 'A':
            case 'j': case 'J': return KEY_LEFT;
            case 'd': case 'D':
            case 'l': case 'L': return KEY_RIGHT;
            case 'w': case 'W':
            case 'i': case 'I':
            case 'z': case 'Z':
            case 'x': case 'X': return KEY_ROT;
            case '\r': case '\n': return KEY_START;
            default: return KEY_NONE;
        }
    }
    // Secuencias de escape (flechas): ESC [ A/B/C/D
    if(n >= 3 && buf[0]==0x1B && buf[1]=='['){
        switch(buf[2]){
            case 'A': return KEY_ROT;   // flecha arriba
            case 'B': return KEY_DOWN;  // flecha abajo
            case 'C': return KEY_RIGHT; // flecha derecha
            case 'D': return KEY_LEFT;  // flecha izquierda
        }
    }
    return KEY_NONE;
}

// ============================================================
//  INPUT: FUSION TECLADO + GPIO
//  Prioridad: cualquier fuente activa gana
// ============================================================
int input_init(void){
    termios_init();
    return 0;
}
void input_close(void){ termios_restore(); }

// Debounce simple por GPIO
static uint8_t btn_debounce[N_BTN] = {0};
#define DEBOUNCE_THRESH 3

int input_read(void){
    // 1) Teclado (no bloquea)
    int k = read_key_raw();
    if(k != KEY_NONE) return k;

    // 2) GPIO botones con debounce
    if(gpio_btn_fd < 0) return KEY_NONE;

    struct gpiohandle_data data;
    memset(&data, 0, sizeof(data));
    ioctl(gpio_btn_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);

    static const int btn_key[N_BTN] = {
        KEY_LEFT, KEY_RIGHT, KEY_ROT, KEY_DOWN, KEY_PAUSE
    };
    for(int i=0;i<N_BTN;i++){
        if(data.values[i] == 0){   // activo bajo
            if(++btn_debounce[i] == DEBOUNCE_THRESH)
                return btn_key[i];
        } else {
            btn_debounce[i] = 0;
        }
    }
    return KEY_NONE;
}

// ============================================================
//  LIMPIEZA
// ============================================================
static void cleanup(void){
    input_close();
    BL_LOW();
    if(gpio_out_fd>=0){ close(gpio_out_fd); gpio_out_fd=-1; }
    if(gpio_btn_fd>=0){ close(gpio_btn_fd); gpio_btn_fd=-1; }
    if(gpio_chip_fd>=0){ close(gpio_chip_fd); gpio_chip_fd=-1; }
    if(spi_fd>=0){ close(spi_fd); spi_fd=-1; }
    fprintf(stderr,"cleanup OK\n");
}
void hw_close(void){ cleanup(); }
static void sig_handler(int s){(void)s; cleanup(); _exit(0);}

int hw_init(void){
    signal(SIGINT, sig_handler);
    signal(SIGTERM,sig_handler);
    if(gpio_init_hw() < 0) return -1;
    if(spi_init_hw()  < 0){ cleanup(); return -2; }
    RST_HIGH(); DC_LOW(); BL_LOW();
    return 0;
}

// ============================================================
//  MAIN
// ============================================================
int main(void){
    fprintf(stderr,"=== TETRIS ARCADE — RPi Zero 2W ===\n");
    fprintf(stderr,"Teclas: ←→ mover | ↑/Z rotar | ↓ bajar | SPC drop | P pausa | Q salir\n");
    fprintf(stderr,"GPIO:   L=5 R=6 Rot=13 Down=19 Pause=26\n\n");

    if(hw_init() < 0){
        fprintf(stderr,"ERROR: hw_init. Ejecutar con sudo. dtparam=spi=on\n");
        return 1;
    }
    init_display();
    delay_ms(50);
    BL_HIGH();
    fprintf(stderr,"Display ON\n");

    srand((unsigned)time(nullptr));
    tetris_sound_init();
    input_init();
    tetris_run();

    hw_close();
    return 0;
}
