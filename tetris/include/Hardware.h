#ifndef HARDWARE_H
#define HARDWARE_H

// ============================================================
//  Hardware.h — RPi Zero 2W + GMT130 ST7789 240x240
//  Tetris Arcade — misma capa hw que funciono en Pac-Man/Dino
//
//  ENTRADA: DOS modos simultaneos (cualquiera funciona):
//
//  [A] TECLADO SSH/consola (termios raw, sin Enter):
//      ← → = mover izquierda/derecha
//      ↑   = rotar
//      ↓   = bajar rapido (soft drop)
//      SPC = drop instantaneo (hard drop)
//      P   = pausa
//      Q   = salir
//
//  [B] GPIO botones (activo-bajo, pull-up externo o interno):
//      GPIO  5  Pin 29 → izquierda
//      GPIO  6  Pin 31 → derecha
//      GPIO 13  Pin 33 → rotar
//      GPIO 19  Pin 35 → bajar / hard-drop
//      GPIO 26  Pin 37 → pausa
//
//  Pantalla:
//      SCK   GPIO 11  Pin 23
//      SDATA GPIO 10  Pin 19
//      DC    GPIO 25  Pin 22
//      RST   GPIO 24  Pin 18
//      BL    GPIO 23  Pin 16
//      VCC   3.3V     Pin 17
//      GND   GND      Pin 20
//
//  Sonido: GPIO 12  Pin 32 (buzzer pasivo)
//
//  Compilar: make
//  Ejecutar: sudo ./bin/tetris
// ============================================================

#include <stdint.h>
#include <stdlib.h>

// ==================== PINES BCM ====================
//#define ADAFRUIT_BONNET

#if defined(ADAFRUIT_BONNET)
#define PIN_MOSI      10
#define PIN_SCLK      11
#define PIN_DC        25
#define PIN_RST       24
#define PIN_BL        23
#define PIN_SOUND     12
#else
// ==================== PINES BCM ====================
#define PIN_MOSI   10   // SPI0 MOSI  (pin físico 19)
#define PIN_SCLK   11   // SPI0 SCLK  (pin físico 23)
#define PIN_DC     21   // D/C        (pin físico 22)
#define PIN_RST    16   // RESET      (pin físico 18)
#define PIN_BL     20   // Backlight  (pin físico 16)
#define PIN_SOUND  13   // Buzzer     (pin físico 32)
#endif

// Botones GPIO (activo-bajo)
#define BTN_LEFT_PIN   5
#define BTN_RIGHT_PIN  6
#define BTN_ROT_PIN   13
#define BTN_DOWN_PIN  19
#define BTN_PAUSE_PIN 26

// ==================== SPI ====================
#define SPI_DEVICE    "/dev/spidev0.0"
#define SPI_SPEED_HZ  40000000

// ==================== GPIO ====================
#define GPIO_CHIP     "/dev/gpiochip0"

// ==================== PANTALLA ====================
#define TFT_W  240
#define TFT_H  240

// ==================== COLORES RGB565 ====================
#define BLACK        0x0000
#define WHITE        0xFFFF
#define RED          0xF800
#define GREEN        0x07E0
#define BLUE         0x001F
#define CYAN         0x07FF
#define YELLOW       0xFFE0
#define ORANGE       0xFC00
#define MAGENTA      0xF81F
#define GRAY         0x8410
#define DARK_GRAY    0x4208
#define LIGHT_GRAY   0xC618
#define PURPLE       COLOR565(128,0,128)
#define DARK_BLUE    COLOR565(0,0,80)
#define NEON_GREEN   COLOR565(57,255,20)
#define NEON_PINK    COLOR565(255,16,240)
#define NEON_CYAN    COLOR565(0,255,255)
#define GOLD         COLOR565(255,215,0)
#define SILVER       COLOR565(192,192,192)
#define COLOR565(r,g,b) ((uint16_t)((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|((b)>>3)))

// ==================== GPIO API ====================
void gpio_write(int pin, int val);
int  gpio_read (int pin);

#define DC_HIGH()  gpio_write(PIN_DC,  1)
#define DC_LOW()   gpio_write(PIN_DC,  0)
#define RST_HIGH() gpio_write(PIN_RST, 1)
#define RST_LOW()  gpio_write(PIN_RST, 0)
#define BL_HIGH()  gpio_write(PIN_BL,  1)
#define BL_LOW()   gpio_write(PIN_BL,  0)

// ==================== INPUT API ====================
// Teclas (se pueden recibir por teclado O por GPIO)
#define KEY_NONE   0
#define KEY_LEFT   1
#define KEY_RIGHT  2
#define KEY_ROT    3    // rotar
#define KEY_DOWN   4    // soft drop
#define KEY_DROP   5    // hard drop
#define KEY_PAUSE  6
#define KEY_QUIT   7
#define KEY_START  8

int  input_init(void);          // inicializa teclado raw + GPIO
void input_close(void);
int  input_read(void);          // devuelve KEY_* o KEY_NONE (no bloqueante)
bool gpio_btn(int pin);         // leer boton GPIO directamente

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
