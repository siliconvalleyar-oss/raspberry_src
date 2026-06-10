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
