#ifndef DINO_HARDWARE_H
#define DINO_HARDWARE_H

// ============================================================
//  DinoHardware.h — RPi Zero 2W + GMT130 ST7789 240x240
//  Chrome Dino arcade — capa de hardware identica al Pac-Man
//  que ya funciono (mismo SPI, GPIO, Sound).
//
//  Conexion fisica (igual que Pac-Man):
//    SCK   → GPIO 11  Pin 23
//    SDATA → GPIO 10  Pin 19
//    DC    → GPIO 25  Pin 22
//    RST   → GPIO 24  Pin 18
//    BL    → GPIO 23  Pin 16
//    VCC   → 3.3V     Pin 17
//    GND   → GND      Pin 20
//    SOUND → GPIO 12  Pin 32  (buzzer pasivo)
//
//  Boton SALTAR → GPIO 5   Pin 29  (definir USE_BUTTONS)
//  Boton COLOR  → GPIO 6   Pin 31  (toggle BN/color)
//
//  Compilar:
//    g++ -O2 -std=c++11 -o dino main_dino.cpp DinoGraphics.cpp \
//        DinoEngine.cpp DinoSound.cpp -lpthread
//  Ejecutar:
//    sudo ./dino
// ============================================================

#include <stdint.h>
#include <stdlib.h>

// ==================== PINES ====================
#define PIN_MOSI    10
#define PIN_SCLK    11
#define PIN_DC      25
#define PIN_RST     24
#define PIN_BL      23
#define PIN_SOUND   12

#define BTN_JUMP_PIN  5    // GPIO 5, Pin 29 — saltar / iniciar
#define BTN_COLOR_PIN 6    // GPIO 6, Pin 31 — toggle color/BN

// ==================== SPI ====================
#define SPI_DEVICE    "/dev/spidev0.0"
#define SPI_SPEED_HZ  40000000   // 40 MHz

// ==================== GPIO ====================
#define GPIO_CHIP     "/dev/gpiochip0"

// ==================== PANTALLA ====================
#define TFT_W   240
#define TFT_H   240

// ==================== COLORES RGB565 ====================
#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define YELLOW      0xFFE0
#define ORANGE      0xFC00
#define MAGENTA     0xF81F
#define GRAY        0x8410
#define DARK_GRAY   0x4208
#define LIGHT_GRAY  0xC618
#define BROWN       0x8200
#define DARK_GREEN  0x03E0
#define SKY_BLUE    COLOR565(135,206,235)
#define CLOUD_WHITE COLOR565(240,248,255)
#define SAND_COLOR  COLOR565(210,180,140)
#define CACTUS_GR   COLOR565(34,139,34)
#define DINO_GREEN  COLOR565(83,141,78)
#define DINO_DARK   COLOR565(50,90,47)
#define PTERO_GRAY  COLOR565(120,120,140)
#define MOON_YELLOW COLOR565(255,255,180)
#define STAR_WHITE  WHITE
#define ROCK_BROWN  COLOR565(139,115,85)

#define COLOR565(r,g,b) ((uint16_t)((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|((b)>>3)))

// ==================== GPIO API ====================
void gpio_write(int pin, int val);
int  gpio_read (int pin);

#define DC_HIGH()   gpio_write(PIN_DC,  1)
#define DC_LOW()    gpio_write(PIN_DC,  0)
#define RST_HIGH()  gpio_write(PIN_RST, 1)
#define RST_LOW()   gpio_write(PIN_RST, 0)
#define BL_HIGH()   gpio_write(PIN_BL,  1)
#define BL_LOW()    gpio_write(PIN_BL,  0)

#define BTN_JUMP    (!gpio_read(BTN_JUMP_PIN))
#define BTN_COLOR   (!gpio_read(BTN_COLOR_PIN))






// ... al final del archivo, reemplazar las definiciones de BTN_JUMP y BTN_COLOR

// Estas funciones se implementan en main_dino.cpp
extern int keyboard_jump(void);
extern int keyboard_color(void);

#define BTN_JUMP   keyboard_jump()
#define BTN_COLOR  keyboard_color()

// ==================== HARDWARE API ====================
int  hw_init(void);
void hw_close(void);

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

void spi_write_byte(uint8_t d);
void spi_write_buf (const uint8_t *buf, uint32_t len);

void write_cmd   (uint8_t c);
void write_data  (uint8_t d);
void push_color  (uint16_t color);
void push_color_n(uint16_t color, uint32_t n);

void reset_display(void);
void init_display (void);
void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#endif
