#ifndef OLED_FUNCTIONS_H
#define OLED_FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>

extern uint8_t *font_table[];

int gpio_export(int gpio_num);
int gpio_unexport(int gpio_num);
int gpio_set_direction(int gpio_num, const char *dir);
int gpio_set_value(int gpio_num, const char *value);
void gpio_configure_for_display(void);
void spi_configure_for_display(void);
void spi_write_1_byte(uint8_t val);
void display_write_command(uint8_t cmd);
void display_write_data(uint8_t data);
void display_do_reset(void);
void display_turn_on(void);
void display_turn_off(void);
void display_set_contrast(uint8_t contrast);
void display_invert_disable(void);
void display_set_page_address(uint8_t address);
void display_set_column_address(uint8_t address);
void display_clear(void);
void display_do_init(void);
void display_write_string(const char *string);

#endif
