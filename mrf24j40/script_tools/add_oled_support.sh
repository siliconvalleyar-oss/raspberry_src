#!/bin/bash
# =============================================================
# Agrega soporte OLED SSD1306 (I2C) al proyecto MRF24J40
# Ejecutar en el directorio raíz del proyecto (donde están mrf24_tx y mrf24_rx)
# =============================================================

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

print_header() {
    echo ""
    echo -e "${BLUE}=================================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}=================================================${NC}"
}

print_success() { echo -e "${GREEN}✓${NC} $1"; }
print_warning() { echo -e "${YELLOW}⚠${NC} $1"; }
print_error() { echo -e "${RED}✗${NC} $1"; }

# Verificar que estamos en el directorio correcto
if [ ! -d "mrf24_tx" ] || [ ! -d "mrf24_rx" ]; then
    print_error "No se encontraron las carpetas mrf24_tx y mrf24_rx"
    echo "Ejecuta este script desde el directorio raíz del proyecto MRF24J40"
    exit 1
fi

print_header "Agregando soporte OLED SSD1306"

# =============================================================
# 1. Crear archivos de la fuente de letras (separada)
# =============================================================
print_header "Creando fuente de letras"

# Crear directorio src si no existe (ya existe)
mkdir -p mrf24_rx/src
mkdir -p mrf24_tx/src

# font5x7.cpp - datos de la fuente
cat > mrf24_rx/src/font5x7.cpp << 'FONT_EOF'
#include "oled.hpp"

const uint8_t OLED::font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // 32 space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // 33 !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // 34 "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // 35 #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // 36 $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // 37 %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // 38 &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // 39 '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // 40 (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // 41 )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // 42 *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // 43 +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // 44 ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // 45 -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // 46 .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // 47 /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 48 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 49 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 50 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 51 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 52 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 53 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 54 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 55 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 56 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 57 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // 58 :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // 59 ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // 60 <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // 61 =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // 62 >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // 63 ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // 64 @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 65 A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 66 B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 67 C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 68 D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 69 E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 70 F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 71 G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 72 H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 73 I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 74 J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 75 K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 76 L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 77 M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 78 N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 79 O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 80 P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 81 Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 82 R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 83 S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 84 T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 85 U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 86 V
    {0x3F, 0x40, 0x30, 0x40, 0x3F}, // 87 W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 88 X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 89 Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 90 Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // 91 [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // 92 backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // 93 ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // 94 ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // 95 _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // 96 `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // 97 a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // 98 b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // 99 c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // 100 d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // 101 e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // 102 f
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // 103 g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // 104 h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // 105 i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // 106 j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // 107 k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // 108 l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // 109 m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // 110 n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // 111 o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // 112 p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // 113 q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // 114 r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // 115 s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // 116 t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 117 u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 118 v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 119 w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // 120 x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 121 y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // 122 z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // 123 {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // 124 |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // 125 }
    {0x08, 0x08, 0x2A, 0x1C, 0x08}, // 126 ~
};
FONT_EOF

# =============================================================
# 2. Crear oled.hpp y oled.cpp
# =============================================================
print_header "Creando librería OLED"

# oled.hpp
cat > mrf24_rx/src/oled.hpp << 'OLED_H_EOF'
#ifndef OLED_HPP
#define OLED_HPP

#include <string>
#include <cstdint>

#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_PAGES (OLED_HEIGHT / 8)

// Comandos SSD1306
#define OLED_CMD_DISPLAY_OFF 0xAE
#define OLED_CMD_DISPLAY_ON 0xAF
#define OLED_CMD_SET_DISPLAY_CLK_DIV 0xD5
#define OLED_CMD_SET_MULTIPLEX 0xA8
#define OLED_CMD_SET_DISPLAY_OFFSET 0xD3
#define OLED_CMD_SET_START_LINE 0x40
#define OLED_CMD_CHARGE_PUMP 0x8D
#define OLED_CMD_MEMORY_MODE 0x20
#define OLED_CMD_SET_COM_PINS 0xDA
#define OLED_CMD_SET_CONTRAST 0x81
#define OLED_CMD_SET_PRECHARGE 0xD9
#define OLED_CMD_SET_VCOM_DETECT 0xDB
#define OLED_CMD_DISPLAY_ALL_ON_RESUME 0xA4
#define OLED_CMD_NORMAL_DISPLAY 0xA6
#define OLED_CMD_INVERT_DISPLAY 0xA7
#define OLED_CMD_DEACTIVATE_SCROLL 0x2E
#define OLED_CMD_COLUMN_ADDR 0x21
#define OLED_CMD_PAGE_ADDR 0x22

class OLED {
private:
    int i2c_fd;
    uint8_t buffer[OLED_WIDTH * OLED_PAGES];
    static const uint8_t font5x7[][5];

    void write_command(uint8_t cmd);
    void write_command(uint8_t cmd, uint8_t data);
    void write_data(uint8_t data);

public:
    OLED();
    ~OLED();

    bool init();
    void clear();
    void update();
    
    void draw_pixel(uint8_t x, uint8_t y, bool color = true);
    void draw_char(uint8_t x, uint8_t y, char c, uint8_t size = 1, bool color = true);
    void draw_string(uint8_t x, uint8_t y, const char* str, uint8_t size = 1, bool color = true);
    void draw_string(uint8_t x, uint8_t y, const std::string& str, uint8_t size = 1, bool color = true);
    
    void set_contrast(uint8_t contrast);
    void set_power(bool on);
    void showInitScreen();
};

#endif
OLED_H_EOF

# oled.cpp
cat > mrf24_rx/src/oled.cpp << 'OLED_CPP_EOF'
#include "oled.hpp"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <iostream>
#include <cstring>

OLED::OLED() : i2c_fd(-1) {
    memset(buffer, 0, sizeof(buffer));
}

OLED::~OLED() {
    if (i2c_fd >= 0) close(i2c_fd);
}

void OLED::write_command(uint8_t cmd) {
    uint8_t data[2] = {0x00, cmd};
    if (write(i2c_fd, data, 2) != 2) {
        std::cerr << "Error escribiendo comando" << std::endl;
    }
}

void OLED::write_command(uint8_t cmd, uint8_t data) {
    write_command(cmd);
    write_command(data);
}

void OLED::write_data(uint8_t data) {
    uint8_t buf[2] = {0x40, data};
    if (write(i2c_fd, buf, 2) != 2) {
        std::cerr << "Error escribiendo datos" << std::endl;
    }
}

bool OLED::init() {
    const char* i2c_devices[] = {"/dev/i2c-1", "/dev/i2c-0"};
    bool opened = false;
    
    for (int i = 0; i < 2; i++) {
        if ((i2c_fd = open(i2c_devices[i], O_RDWR)) >= 0) {
            opened = true;
            break;
        }
    }
    
    if (!opened) {
        std::cerr << "Error: No se pudo abrir I2C" << std::endl;
        return false;
    }
    
    if (ioctl(i2c_fd, I2C_SLAVE, OLED_ADDR) < 0) {
        std::cerr << "Error: No se pudo configurar dirección I2C" << std::endl;
        close(i2c_fd);
        return false;
    }
    
    usleep(100000);
    
    write_command(OLED_CMD_DISPLAY_OFF);
    write_command(OLED_CMD_SET_DISPLAY_CLK_DIV, 0x80);
    write_command(OLED_CMD_SET_MULTIPLEX, 0x3F);
    write_command(OLED_CMD_SET_DISPLAY_OFFSET, 0x00);
    write_command(OLED_CMD_SET_START_LINE);
    write_command(OLED_CMD_CHARGE_PUMP);
    write_command(0x14);
    write_command(OLED_CMD_MEMORY_MODE);
    write_command(0x00);
    write_command(OLED_CMD_SET_COM_PINS, 0x12);
    write_command(OLED_CMD_SET_CONTRAST, 0x7F);
    write_command(OLED_CMD_SET_PRECHARGE, 0xF1);
    write_command(OLED_CMD_SET_VCOM_DETECT, 0x40);
    write_command(OLED_CMD_DISPLAY_ALL_ON_RESUME);
    write_command(OLED_CMD_NORMAL_DISPLAY);
    write_command(OLED_CMD_DEACTIVATE_SCROLL);
    
    clear();
    update();
    
    write_command(OLED_CMD_DISPLAY_ON);
    
    std::cout << "OLED inicializado correctamente" << std::endl;
    return true;
}

void OLED::clear() {
    memset(buffer, 0, sizeof(buffer));
}

void OLED::update() {
    write_command(OLED_CMD_COLUMN_ADDR);
    write_command(0);
    write_command(OLED_WIDTH - 1);
    write_command(OLED_CMD_PAGE_ADDR);
    write_command(0);
    write_command(OLED_PAGES - 1);
    
    uint8_t data[OLED_WIDTH * OLED_PAGES + 1];
    data[0] = 0x40;
    
    for (int i = 0; i < OLED_WIDTH * OLED_PAGES; i++) {
        data[i + 1] = buffer[i];
    }
    
    if (write(i2c_fd, data, OLED_WIDTH * OLED_PAGES + 1) != OLED_WIDTH * OLED_PAGES + 1) {
        std::cerr << "Error actualizando display" << std::endl;
    }
}

void OLED::draw_pixel(uint8_t x, uint8_t y, bool color) {
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    
    if (color) {
        buffer[x + page * OLED_WIDTH] |= (1 << bit);
    } else {
        buffer[x + page * OLED_WIDTH] &= ~(1 << bit);
    }
}

void OLED::draw_char(uint8_t x, uint8_t y, char c, uint8_t size, bool color) {
    if (c < 32 || c > 126) return;
    
    uint8_t idx = c - 32;
    
    for (int col = 0; col < 5; col++) {
        uint8_t line = font5x7[idx][col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                for (int sx = 0; sx < size; sx++)
                    for (int sy = 0; sy < size; sy++)
                        draw_pixel(x + col * size + sx, y + row * size + sy, color);
            }
        }
    }
}

void OLED::draw_string(uint8_t x, uint8_t y, const std::string& str, uint8_t size, bool color) {
    uint8_t current_x = x;
    for (char c : str) {
        draw_char(current_x, y, c, size, color);
        current_x += 6 * size;
        if (current_x + 6 * size > OLED_WIDTH) break;
    }
}

void OLED::draw_string(uint8_t x, uint8_t y, const char* str, uint8_t size, bool color) {
    uint8_t current_x = x;
    while (*str) {
        draw_char(current_x, y, *str, size, color);
        current_x += 6 * size;
        str++;
        if (current_x + 6 * size > OLED_WIDTH) break;
    }
}

void OLED::set_contrast(uint8_t contrast) {
    write_command(OLED_CMD_SET_CONTRAST, contrast);
}

void OLED::set_power(bool on) {
    if (on) write_command(OLED_CMD_DISPLAY_ON);
    else write_command(OLED_CMD_DISPLAY_OFF);
}

void OLED::showInitScreen() {
    clear();
    draw_string(0, 20, "MRF24J40", 1, true);
    draw_string(0, 35, "Iniciando...", 1, true);
    update();
}
OLED_CPP_EOF

# Copiar archivos al transmisor (opcional, si se quiere también mostrar estado)
cp mrf24_rx/src/oled.hpp mrf24_tx/src/
cp mrf24_rx/src/oled.cpp mrf24_tx/src/
cp mrf24_rx/src/font5x7.cpp mrf24_tx/src/

print_success "Archivos OLED creados"

# =============================================================
# 3. Modificar el main.cpp del receptor para mostrar mensajes en OLED
# =============================================================
print_header "Modificando receptor para usar OLED"

# Hacer backup del main.cpp original
cp mrf24_rx/src/main.cpp mrf24_rx/src/main.cpp.bak

# Crear nuevo main.cpp con soporte OLED
cat > mrf24_rx/src/main.cpp << 'RX_OLED_EOF'
#include <cstdio>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <ctime>
#include <chrono>
#include <fstream>
#include <sys/select.h>
#include <sys/time.h>
#include "mrf24j40.h"
#include "oled.hpp"

#define MY_ADDR     0x0002
#define PAN_ID      0xCAFE
#define CHANNEL     20

static volatile bool running = true;
static Mrf24j40 radio;
static OLED display;
static std::ofstream logfile;

void sig_handler(int) { running = false; }

void print_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    struct tm* tm_info = localtime(&time_t);
    printf("[%02d:%02d:%02d.%03d] ", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, (int)ms.count());
}

void print_stats() {
    RadioStats stats;
    radio.getStats(stats);
    
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║              ESTADÍSTICAS RX                     ║\n");
    printf("╠══════════════════════════════════════════════════╣\n");
    printf("║ Paquetes recibidos:    %-8d                      ║\n", stats.packets_received);
    printf("║ LQI promedio:          %-8.1f                    ║\n", 
           stats.rx_count > 0 ? (float)stats.rx_lqi_sum / stats.rx_count : 0);
    printf("║ RSSI promedio:         %-8.1f dBm                ║\n",
           stats.rx_count > 0 ? (float)stats.rx_rssi_sum / stats.rx_count : 0);
    printf("╚══════════════════════════════════════════════════╝\n");
}

void update_oled_display(int packet_num, const char* payload, uint8_t lqi, int8_t rssi) {
    display.clear();
    
    // Primera línea: título
    display.draw_string(0, 0, "MRF24J40 RX", 1, true);
    
    // Segunda línea: número de paquete
    char line2[32];
    snprintf(line2, sizeof(line2), "Paq #%d", packet_num);
    display.draw_string(0, 16, line2, 1, true);
    
    // Tercera línea: payload (truncado a 16 caracteres)
    char line3[20];
    strncpy(line3, payload, 16);
    line3[16] = '\0';
    display.draw_string(0, 32, line3, 1, true);
    
    // Cuarta línea: LQI y RSSI
    char line4[32];
    snprintf(line4, sizeof(line4), "LQI:%3d RSSI:%3ddBm", lqi, rssi);
    display.draw_string(0, 48, line4, 1, true);
    
    display.update();
}

void log_packet(int packet_num, const char* payload, uint8_t len, uint8_t lqi, int8_t rssi) {
    if (logfile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        logfile << time_t << "," << packet_num << ",\"" << payload << "\"," << (int)len << "," << (int)lqi << "," << (int)rssi << "\n";
        logfile.flush();
    }
}

int main() {
    signal(SIGINT, sig_handler);
    
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║    MRF24J40 RECEPTOR AVANZADO v2.0     ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    // Inicializar OLED (opcional, continúa si falla)
    bool oled_ok = display.init();
    if (oled_ok) {
        display.showInitScreen();
        usleep(500000);
        display.clear();
        display.draw_string(0, 20, "MRF24J40 RX", 1, true);
        display.draw_string(0, 35, "Esperando...", 1, true);
        display.update();
    } else {
        printf("[WARN] OLED no detectado. Continuando sin display.\n");
    }
    
    if (!radio.init(CHANNEL)) return 1;
    
    radio.setPan(PAN_ID);
    radio.setShortAddress(MY_ADDR);
    radio.selfTest();
    
    printf("\n[CONFIGURACION]\n");
    printf("  PAN ID: 0x%04X, Dirección: 0x%04X\n", radio.getPan(), radio.getShortAddress());
    printf("  Canal: %d, SPI: %u Hz\n\n", CHANNEL, SPI_SPEED_HZ);
    
    logfile.open("mrf24_receiver.log", std::ios::app);
    if (logfile.is_open()) {
        logfile << "#timestamp,packet_num,payload,len,lqi,rssi\n";
    }
    
    printf("[COMANDOS]\n");
    printf("  s - estadísticas | c - clear stats | q - salir\n\n");
    
    printf("[MAIN] Escuchando... Ctrl+C para salir.\n\n");
    
    int packet_count = 0;
    uint8_t buffer[MAX_PAYLOAD];
    
    while (running) {
        radio.poll();
        
        fd_set readfds;
        struct timeval tv = {0, 0};
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
            char cmd = getchar();
            if (cmd == 's') {
                print_stats();
            } else if (cmd == 'c') {
                radio.resetStats();
                printf("[CMD] Estadísticas reiniciadas\n");
            } else if (cmd == 'q') {
                running = false;
            }
        }
        
        if (radio.hasPacket()) {
            uint8_t len = radio.rxLen();
            radio.rxGet(buffer);
            buffer[len] = '\0';
            
            packet_count++;
            print_timestamp();
            printf("Paquete #%d:\n", packet_count);
            printf("  Payload (%d): \"%s\"\n", len, (char*)buffer);
            printf("  LQI: %d/255, RSSI: %d dBm\n\n", radio.getLQI(), radio.getRSSI());
            
            // Actualizar OLED
            if (oled_ok) {
                update_oled_display(packet_count, (char*)buffer, radio.getLQI(), radio.getRSSI());
            }
            
            log_packet(packet_count, (char*)buffer, len, radio.getLQI(), radio.getRSSI());
        }
        
        usleep(10000);
    }
    
    print_stats();
    radio.printRegisters();
    
    if (logfile.is_open()) {
        logfile.close();
        printf("\n[LOG] Guardado en mrf24_receiver.log\n");
    }
    
    if (oled_ok) {
        display.clear();
        display.draw_string(0, 20, "Terminado", 1, true);
        display.update();
    }
    
    printf("\n[FIN] Total paquetes: %d\n", packet_count);
    return 0;
}
RX_OLED_EOF

print_success "Receptor modificado con soporte OLED"

# =============================================================
# 4. (Opcional) Modificar transmisor para mostrar estado en OLED
# =============================================================
read -p "¿Deseas también agregar soporte OLED al transmisor? (s/n): " add_tx_oled
if [[ "$add_tx_oled" == "s" || "$add_tx_oled" == "S" ]]; then
    cp mrf24_rx/src/oled.hpp mrf24_tx/src/
    cp mrf24_rx/src/oled.cpp mrf24_tx/src/
    cp mrf24_rx/src/font5x7.cpp mrf24_tx/src/
    
    # Backup
    cp mrf24_tx/src/main.cpp mrf24_tx/src/main.cpp.bak
    
    # Modificar transmisor (versión simplificada)
    cat > mrf24_tx/src/main.cpp << 'TX_OLED_EOF'
#include <cstdio>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sys/select.h>
#include <sys/time.h>
#include "mrf24j40.h"
#include "oled.hpp"

#define MY_ADDR     0x0001
#define DEST_ADDR   0x0002
#define PAN_ID      0xCAFE
#define CHANNEL     20
#define TX_INTERVAL_MS 2000

enum Mode { MODE_NORMAL, MODE_BURST };
static Mode current_mode = MODE_NORMAL;
static volatile bool running = true;
static Mrf24j40 radio;
static OLED display;
static int burst_count = 0;

void sig_handler(int) { running = false; }

void print_stats() {
    RadioStats stats;
    radio.getStats(stats);
    
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║                 ESTADÍSTICAS                     ║\n");
    printf("╠══════════════════════════════════════════════════╣\n");
    printf("║ Paquetes enviados:     %-8d                      ║\n", stats.packets_sent);
    printf("║ TX exitosos:           %-8d                      ║\n", stats.tx_success);
    printf("║ TX fallidos:           %-8d                      ║\n", stats.tx_fail);
    printf("║ Retransmisiones total: %-8d                      ║\n", stats.tx_retries_total);
    printf("║ Tasa de éxito:         %-8.1f%%                  ║\n", 
           stats.packets_sent > 0 ? 100.0f * stats.tx_success / stats.packets_sent : 0);
    printf("╚══════════════════════════════════════════════════╝\n");
}

void update_oled_stats() {
    RadioStats stats;
    radio.getStats(stats);
    display.clear();
    display.draw_string(0, 0, "MRF24J40 TX", 1, true);
    char line2[32];
    snprintf(line2, sizeof(line2), "Env: %d", stats.packets_sent);
    display.draw_string(0, 16, line2, 1, true);
    snprintf(line2, sizeof(line2), "OK: %d", stats.tx_success);
    display.draw_string(0, 32, line2, 1, true);
    snprintf(line2, sizeof(line2), "Fail: %d", stats.tx_fail);
    display.draw_string(0, 48, line2, 1, true);
    display.update();
}

void burst_transmission(int packets, int delay_ms) {
    printf("\n[ BURST ] Enviando %d paquetes con delay %dms\n", packets, delay_ms);
    
    for (int i = 0; i < packets && running; i++) {
        char payload[64];
        snprintf(payload, sizeof(payload), "BURST:%d:%d", burst_count++, i);
        
        radio.sendString(DEST_ADDR, payload);
        
        int timeout = 100;
        while (timeout-- > 0 && radio.txDone() == false) {
            radio.poll();
            usleep(5000);
        }
        
        if (delay_ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        if (i % 5 == 0) update_oled_stats();
    }
    
    print_stats();
    update_oled_stats();
}

void normal_transmission() {
    static int msg_num = 0;
    char payload[64];
    snprintf(payload, sizeof(payload), "HELLO:%d", msg_num++);
    
    radio.sendString(DEST_ADDR, payload);
    
    int timeout = 500;
    while (timeout-- > 0 && radio.txDone() == false) {
        radio.poll();
        usleep(5000);
    }
    
    if (radio.txDone()) {
        if (radio.txSuccess())
            printf("✓ OK (retries=%d)\n", radio.txRetries());
        else
            printf("✗ FALLO (retries=%d)\n", radio.txRetries());
    } else {
        printf("⏱ TIMEOUT\n");
    }
    update_oled_stats();
}

int main() {
    signal(SIGINT, sig_handler);
    
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║   MRF24J40 TRANSMISOR AVANZADO v2.0    ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    bool oled_ok = display.init();
    if (oled_ok) {
        display.showInitScreen();
        usleep(500000);
        display.clear();
        display.draw_string(0, 20, "MRF24J40 TX", 1, true);
        display.update();
    } else {
        printf("[WARN] OLED no detectado. Continuando sin display.\n");
    }
    
    if (!radio.init(CHANNEL)) return 1;
    
    radio.setPan(PAN_ID);
    radio.setShortAddress(MY_ADDR);
    radio.selfTest();
    
    printf("\n[CONFIGURACION]\n");
    printf("  PAN ID: 0x%04X, Dirección: 0x%04X\n", radio.getPan(), radio.getShortAddress());
    printf("  Destino: 0x%04X, Canal: %d, SPI: %u Hz\n", DEST_ADDR, CHANNEL, SPI_SPEED_HZ);
    
    printf("\n[COMANDOS]\n");
    printf("  n - modo normal | b - burst (10 paq) | s - stats | q - salir\n\n");
    
    printf("[MAIN] Iniciando transmisión normal. Ctrl+C para salir.\n\n");
    
    auto last_tx = std::chrono::steady_clock::now();
    
    while (running) {
        radio.poll();
        
        fd_set readfds;
        struct timeval tv = {0, 0};
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
            char cmd = getchar();
            if (cmd == 'b') {
                current_mode = MODE_BURST;
                burst_transmission(10, 50);
                current_mode = MODE_NORMAL;
            } else if (cmd == 's') {
                print_stats();
            } else if (cmd == 'q') {
                running = false;
            }
        }
        
        auto now = std::chrono::steady_clock::now();
        if (current_mode == MODE_NORMAL && 
            std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tx).count() >= TX_INTERVAL_MS) {
            printf("[TX] ");
            normal_transmission();
            last_tx = now;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    print_stats();
    radio.printRegisters();
    if (oled_ok) {
        display.clear();
        display.draw_string(0, 20, "Terminado", 1, true);
        display.update();
    }
    printf("\n[FIN] Terminado.\n");
    return 0;
}
TX_OLED_EOF
    print_success "Transmisor modificado con soporte OLED"
else
    print_warning "Omitiendo OLED en transmisor"
fi

# =============================================================
# 5. Actualizar Makefiles para incluir los nuevos archivos
# =============================================================
print_header "Actualizando Makefiles"

# Función para actualizar Makefile
update_makefile() {
    local project_dir=$1
    local makefile="$project_dir/Makefile"
    
    # Verificar si ya incluye oled.cpp y font5x7.cpp
    if grep -q "oled.cpp" "$makefile"; then
        print_warning "$project_dir/Makefile ya incluye OLED"
    else
        # Reemplazar la línea SRCS
        sed -i 's/SRCS    = src\/main.cpp src\/mrf24j40.cpp/SRCS    = src\/main.cpp src\/mrf24j40.cpp src\/oled.cpp src\/font5x7.cpp/' "$makefile"
        print_success "$project_dir/Makefile actualizado"
    fi
}

update_makefile "mrf24_rx"
if [[ "$add_tx_oled" == "s" || "$add_tx_oled" == "S" ]]; then
    update_makefile "mrf24_tx"
fi

# =============================================================
# 6. Script para habilitar I2C en Raspberry Pi
# =============================================================
print_header "Verificando I2C"

cat > enable_i2c.sh << 'I2C_EOF'
#!/bin/bash
echo "Habilitando I2C en Raspberry Pi..."
if ! grep -q "dtparam=i2c_arm=on" /boot/config.txt; then
    echo "dtparam=i2c_arm=on" | sudo tee -a /boot/config.txt
fi
sudo raspi-config nonint do_i2c 0
sudo apt-get install -y i2c-tools
echo "I2C habilitado. Reinicia para aplicar cambios."
I2C_EOF

chmod +x enable_i2c.sh
print_success "Script enable_i2c.sh creado (ejecutar si no está habilitado I2C)"

# =============================================================
# RESUMEN FINAL
# =============================================================
print_header "SOPORTE OLED AGREGADO EXITOSAMENTE"
echo ""
echo "  Archivos añadidos:"
echo "    - src/oled.hpp"
echo "    - src/oled.cpp"
echo "    - src/font5x7.cpp"
echo ""
echo "  Receptor: mostrará en OLED:"
echo "    - Número de paquete"
echo "    - Payload (primeros 16 caracteres)"
echo "    - LQI y RSSI"
echo ""
if [[ "$add_tx_oled" == "s" || "$add_tx_oled" == "S" ]]; then
    echo "  Transmisor: mostrará en OLED:"
    echo "    - Paquetes enviados, éxito y fallos"
fi
echo ""
echo "  Próximos pasos:"
echo "    1. Habilitar I2C: sudo ./enable_i2c.sh && sudo reboot"
echo "    2. Conectar OLED SSD1306 a I2C (pines SDA=GPIO2, SCL=GPIO3, VCC=3.3V, GND)"
echo "    3. Recompilar: cd mrf24_rx && make clean && make"
echo "    4. Ejecutar: sudo ./bin/mrf24_receiver"
echo ""
print_success "¡Listo!"
