// ============================================================
//  main_arcade.cpp — Tetris Arcade / RPi Zero 2W
//  Capa de hardware: /dev/spidev0.0 + /dev/gpiochip0 +
//  entrada unificada GPIO+teclado + bit-bang sound.
//
//  Compilar: make
//  Ejecutar: sudo ./bin/tetris
// ============================================================

#include "Hardware.h"
#include "TetrisGfx.h"
#include "TetrisEngine.h"
#include "TetrisSound.h"

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

#define N_BTN 5
static const uint32_t btn_pins[N_BTN] = {
    BTN_LEFT_PIN, BTN_RIGHT_PIN, BTN_ROT_PIN, BTN_DOWN_PIN, BTN_PAUSE_PIN
};

static struct termios orig_termios;

// Flag para saber si el teclado estaba en modo raw
static bool keyboard_raw = false;

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
    strncpy(req.consumer_label, "tetris_out", 15);
    if(ioctl(gpio_chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0){
        perror("GPIO salidas"); close(gpio_chip_fd); return -2;
    }
    gpio_out_fd = req.fd;

    memset(&req, 0, sizeof(req));
    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = N_BTN;
    for(int i=0;i<N_BTN;i++) req.lineoffsets[i] = btn_pins[i];
    strncpy(req.consumer_label, "tetris_btn", 15);
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

bool gpio_btn(int pin) {
    return gpio_read(pin) == 0;  // activo bajo
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
//  LIMPIEZA + SEÑALES (CORREGIDO - Restaura teclado)
// ============================================================
static void cleanup(void){
    BL_LOW();
    if(gpio_out_fd>=0){ close(gpio_out_fd); gpio_out_fd=-1; }
    if(gpio_btn_fd>=0){ close(gpio_btn_fd); gpio_btn_fd=-1; }
    if(gpio_chip_fd>=0){ close(gpio_chip_fd); gpio_chip_fd=-1; }
    if(spi_fd>=0){ close(spi_fd); spi_fd=-1; }
    
    // RESTAURAR TECLADO si estaba en modo raw
    if(keyboard_raw) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        keyboard_raw = false;
        fprintf(stderr, "Teclado restaurado\n");
    }
    fprintf(stderr,"cleanup OK\n");
}

void hw_close(void){ cleanup(); }

static void sig_handler(int s){ 
    (void)s; 
    cleanup(); 
    _exit(0); 
}

int hw_init(void){
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGQUIT, sig_handler);  // Capturar Ctrl+\ también
    if(gpio_init()    < 0) return -1;
    if(spi_init_dev() < 0){ cleanup(); return -2; }
    RST_HIGH(); DC_LOW(); BL_LOW();
    return 0;
}

// ============================================================
//  TECLADO (modo no canonico, sin echo)
// ============================================================
void keyboard_init(void) {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN]  = 0;
    new_termios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    keyboard_raw = true;
}

void keyboard_restore(void) {
    if(keyboard_raw) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        keyboard_raw = false;
    }
}

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
    return (unsigned char)c;
}

// Buffer para secuencias escape (flechas)
static int escape_buf[3];
static int escape_pos = 0;

static int read_keyboard(void) {
    // Si estamos en medio de una secuencia escape, leer mas bytes
    if(escape_pos > 0) {
        while(escape_pos < 3 && stdin_ready()) {
            int c = getch_nonblock();
            if(c < 0) break;
            escape_buf[escape_pos++] = c;
        }
        // Secuencia completa: ESC [ A (arriba), B (abajo), C (der), D (izq)
        if(escape_pos >= 3) {
            if(escape_buf[0] == 0x1B && escape_buf[1] == '[') {
                int result = KEY_NONE;
                switch(escape_buf[2]) {
                    case 'A': result = KEY_ROT;    break;  // ↑ rotar
                    case 'B': result = KEY_DOWN;   break;  // ↓ soft drop
                    case 'C': result = KEY_RIGHT;  break;  // → derecha
                    case 'D': result = KEY_LEFT;   break;  // ← izquierda
                }
                escape_pos = 0;
                return result;
            }
            escape_pos = 0;
            return KEY_NONE;
        }
        // Esperando mas bytes de la secuencia
        return KEY_NONE;
    }

    int c = getch_nonblock();
    if(c < 0) return KEY_NONE;

    switch(c) {
        case 0x1B:  // ESC — inicio de secuencia escape (flechas)
            escape_buf[0] = 0x1B;
            escape_pos = 1;
            return KEY_NONE;
        case ' ': case 0x0A: case 0x0D:  // SPACE / ENTER
            return KEY_DROP;  // hard drop / start
        case 'z': case 'Z': case 'x': case 'X':
            return KEY_ROT;  // rotar (alternativa)
        case '\t':
            return KEY_ROT;  // TAB = rotar
        case 'p': case 'P':
            return KEY_PAUSE;
        case 'q': case 'Q':
            return KEY_QUIT;
        case 'h': case 'H':
            return KEY_START;  // hold
        case 'r': case 'R':
            return KEY_START;  // restart / start
        case 's': case 'S':
            return KEY_DOWN;   // soft drop
    }
    return KEY_NONE;
}

// ============================================================
//  LECTURA DE ENTRADA UNIFICADA (GPIO + teclado)
// ============================================================
int input_init(void) {
    keyboard_init();
    return 0;
}

void input_close(void) {
    keyboard_restore();
}

int input_read(void) {
    // 1) GPIO buttons (activo bajo = 0 = presionado)
    if(gpio_btn_fd >= 0) {
        struct gpiohandle_data data;
        memset(&data, 0, sizeof(data));
        if(ioctl(gpio_btn_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) == 0) {
            if(data.values[0] == 0) return KEY_LEFT;    // BTN_LEFT
            if(data.values[1] == 0) return KEY_RIGHT;   // BTN_RIGHT
            if(data.values[2] == 0) return KEY_ROT;     // BTN_ROT
            if(data.values[3] == 0) return KEY_DOWN;    // BTN_DOWN
            if(data.values[4] == 0) return KEY_PAUSE;   // BTN_PAUSE
        }
    }

    // 2) Keyboard fallback
    return read_keyboard();
}

// ============================================================
//  MAIN - Con restauración garantizada al salir
// ============================================================
int main(void){
    fprintf(stderr, "=== TETRIS ARCADE — RPi Zero 2W (teclado consola) ===\n");
    fprintf(stderr, "TECLAS: ← → mover  ↑ rotar  ↓ bajar  SPACE drop\n");
    fprintf(stderr, "        P=pausa  Q=salir  R=hold/start\n");
    fprintf(stderr, "GPIO:  L=5  R=6  ROT=13  D=19  PAUSE=26\n");

    input_init();

    if(hw_init() < 0){
        fprintf(stderr, "ERROR hw_init.\n"
                        "  1) sudo ./bin/tetris\n"
                        "  2) dtparam=spi=on en /boot/firmware/config.txt\n");
        input_close();
        return 1;
    }

    init_display();
    delay_ms(50);
    BL_HIGH();
    fprintf(stderr, "Backlight ON\n");

    srand((unsigned)time(nullptr));
    tetris_sound_init();
    tetris_run();

    // Limpieza explícita antes de salir
    hw_close();
    input_close();
    
    fprintf(stderr, "=== TETRIS FINALIZADO CORRECTAMENTE ===\n");
    return 0;
}
