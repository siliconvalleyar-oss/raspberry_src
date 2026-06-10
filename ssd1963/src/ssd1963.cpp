
#include <bcm2835.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <tft_ssd1963.hpp>
#include <tft_cmd_ssd1963.hpp>
#include <tft_ssd1963.hpp>
#include <ssd1963.hpp>

namespace SSD1963_DRV{
        
    void Ssd1963_t::delay_ms(uint32_t ms) {
        usleep(ms * 1000);
    }

    void Ssd1963_t::write_data_bus(uint16_t data) {
        // Limpia todos los pines de datos
        for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
            bcm2835_gpio_write(pin, LOW);
        }
        // Escribe los bits correspondientes a HIGH
        for (uint8_t i = 0; i < 16; i++) {
            if (data & (1 << i)) {
                bcm2835_gpio_write(SSD1963_LCD_D0 + i, HIGH);
            }
        }
    }

    void Ssd1963_t::write_command(uint8_t cmd) {
        RS_LOW();
        CS_LOW();
        write_data_bus(cmd);
        WR_LOW();
    //  usleep(10); // o eliminarlo
        WR_HIGH();
        CS_HIGH();
    }

    void Ssd1963_t::write_data(uint16_t data) {
        RS_HIGH();
        CS_LOW();
        write_data_bus(data);
        WR_LOW();
        //usleep(10); // o eliminarlo
        WR_HIGH();
        CS_HIGH();
    }

    void Ssd1963_t::set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
        write_command(SSD1963_SET_COLUMN_ADDRESS);
        write_data(x1 >> 8);
        write_data(x1 & 0xFF);
        write_data(x2 >> 8);
        write_data(x2 & 0xFF);
        write_command(SSD1963_SET_PAGE_ADDRESS);
        write_data(y1 >> 8);
        write_data(y1 & 0xFF);
        write_data(y2 >> 8);
        write_data(y2 & 0xFF);
    }

    void Ssd1963_t::clear_screen(uint16_t color) {
        set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
        write_command(SSD1963_WRITE_MEMORY_START);
        for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
            write_data(color);
            write_data(color>>8);
        }
    }

    void Ssd1963_t::ssd1963_init() {
        // 1. Reset físico (mismo estilo que STM32)
        RESET_LOW();
        delay_ms(10); // equivalente a un pequeño ciclo de espera
        RESET_HIGH();
        delay_ms(10);
        // 2. Soft Reset (ANTES del PLL)
        write_command(SSD1963_SOFT_RESET);
        delay_ms(10);
        // 3. Configurar PLL (para REFclk = 10 MHz → PLLclk = 500MHz, SYSclk = 100MHz)
        write_command(SSD1963_SET_PLL_MN);
        write_data(49);     // PLLclk = REFclk * 50
        write_data(4);      // SYSclk = PLLclk / 5
        write_data(4);      // Dummy (según código STM32)
        // Activar PLL
        write_command(SSD1963_SET_PLL);
        write_data(0x01);
        delay_ms(10);  // largo retardo
        write_command(SSD1963_SET_PLL);
        write_data(0x03);
        delay_ms(10);
        // 4. Configurar modo LCD
        write_command(SSD1963_SET_LCD_MODE);
        write_data(0x0C);    // DE mode
        write_data(0x00);    // RGB565
        write_data((LCD_WIDTH - 1) >> 8);
        write_data((LCD_WIDTH - 1) & 0xFF);
        write_data((LCD_HEIGHT - 1) >> 8);
        write_data((LCD_HEIGHT - 1) & 0xFF);
        write_data(0x00);    // RGB
        // 5. Interfaz de datos (RGB565)
        write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
        write_data(SSD1963_PDI_16BIT565);  // Interfaz 16-bit 565
        // 6. Frecuencia LSHIFT
        uint32_t LCD_FPR = 0x01E848; // ejemplo: 2.0MHz para SYSCLK=100MHz
        write_command(SSD1963_SET_LSHIFT_FREQ);
        write_data((LCD_FPR >> 16) & 0xFF);
        write_data((LCD_FPR >> 8) & 0xFF);
        write_data(LCD_FPR & 0xFF);
        // 7. Tiempos de sincronización horizontales
        write_command(SSD1963_SET_HOR_PERIOD);
        write_data((TFT_HSYNC_PERIOD >> 8) & 0xFF);
        write_data(TFT_HSYNC_PERIOD & 0xFF);
        write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) >> 8);
        write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) & 0xFF);
        write_data(TFT_HSYNC_PULSE);
        write_data(0x00); // LPS = 0
        write_data(0x00); // Opcional
        write_data(0x00); // Opcional
        // 8. Tiempos de sincronización verticales
        write_command(SSD1963_SET_VER_PERIOD);
        write_data((TFT_VSYNC_PERIOD >> 8) & 0xFF);
        write_data(TFT_VSYNC_PERIOD & 0xFF);
        write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) >> 8);
        write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) & 0xFF);
        write_data(TFT_VSYNC_PULSE);
        write_data(0x00);
        write_data(0x00);
        // 9. Encender display
        write_command(SSD1963_ON_DISPLAY);
        delay_ms(50);
        // 12. Encender luz de fondo (GPIO)
        BACKLIGHT_ON();
    }

    void Ssd1963_t::setup_gpio() {
        // Configurar pines de datos como salida
        for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
            bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
        }
        // Configurar pines de control como salida
        bcm2835_gpio_fsel(SSD1963_LCD_RS, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(SSD1963_LCD_WR, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(SSD1963_LCD_CS, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(SSD1963_LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(SSD1963_LCD_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);
        // Estados iniciales
        CS_HIGH();
        WR_HIGH();
        RS_HIGH();
        RESET_HIGH();
        BACKLIGHT_OFF();
    }

    void Ssd1963_t::draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
        set_area(x, y, x + width - 1, y + height - 1);
        write_command(SSD1963_WRITE_MEMORY_START);
        for (uint32_t i = 0; i < width * height; i++) {
            write_data(color);
        }
    }

    void Ssd1963_t::draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
        set_area(x, y, x, y);  // Define el área como 1x1 píxel
        write_command(SSD1963_WRITE_MEMORY_START);
        write_data(color);     // Envía el color (pixel)
    }

    void Ssd1963_t::draw_image_rgb565(const char* filepath) {
        FILE* file = fopen(filepath, "rb");
        if (!file) {
            std::cerr << "No se pudo abrir la imagen: " << filepath << std::endl;
            return;
        }

        set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
        write_command(SSD1963_WRITE_MEMORY_START);

        for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
            uint8_t high, low;
            if (fread(&high, 1, 1, file) != 1) break;
            if (fread(&low, 1, 1, file) != 1) break;

            uint16_t color = (high << 8) | low;
            write_data(color);
        }

        fclose(file);
    }

    void Ssd1963_t::draw_sprite_frame(const char* filepath, uint16_t frame_width, uint16_t frame_height, uint16_t frame_index, uint16_t x, uint16_t y) {
                FILE* file = fopen(filepath, "rb");
                if (!file) {
                    std::cerr << "No se pudo abrir: " << filepath << std::endl;
                    return;
                }
            
                // Asumimos que los frames están uno al lado del otro horizontalmente
                uint32_t bytes_per_frame = frame_width * frame_height * 2; // 2 bytes por píxel
                uint32_t offset = bytes_per_frame * frame_index;
            
                // Saltar al inicio del frame deseado
                fseek(file, offset, SEEK_SET);
            
                set_area(x, y, x + frame_width - 1, y + frame_height - 1);
                write_command(SSD1963_WRITE_MEMORY_START);
            
                for (uint32_t i = 0; i < frame_width * frame_height; i++) {
                    uint8_t high, low;
                    if (fread(&high, 1, 1, file) != 1) break;
                    if (fread(&low, 1, 1, file) != 1) break;
                    uint16_t color = (high << 8) | low;
                    write_data(color);
                }
            
                fclose(file);
            }
     
    void Ssd1963_t::draw_rounded_rect( int x, int y, int w, int h, int radius, uint16_t fill_color, uint16_t border_color, int border_thickness) {
    // Relleno del centro
        for (int i = y + radius; i < y + h - radius; i++) {
            set_area(x + radius, i, x + w - radius - 1, i);
            write_command(SSD1963_WRITE_MEMORY_START);
            for (int j = x + radius; j < x + w - radius; j++) {
                write_data(fill_color);
            }
        }

        // Rellenar lados rectos (verticales y horizontales)
        for (int i = 0; i < radius; i++) {
            // Línea superior
            set_area(x + radius, y + i, x + w - radius - 1, y + i);
            write_command(SSD1963_WRITE_MEMORY_START);
            for (int j = x + radius; j < x + w - radius; j++) {
                write_data(fill_color);
            }

            // Línea inferior
            set_area(x + radius, y + h - 1 - i, x + w - radius - 1, y + h - 1 - i);
            write_command(SSD1963_WRITE_MEMORY_START);
            for (int j = x + radius; j < x + w - radius; j++) {
                write_data(fill_color);
            }
        }

        // Laterales
        for (int i = y + radius; i < y + h - radius; i++) {
            for (int t = 0; t < radius; t++) {
                draw_pixel(x + t, i, fill_color);                     // Izquierda
                draw_pixel(x + w - 1 - t, i, fill_color);             // Derecha
            }
        }

        // Esquinas redondeadas (cuartos de círculo)
        for (int i = 0; i < radius; i++) {
            for (int j = 0; j < radius; j++) {
                if ((i * i + j * j) <= (radius * radius)) {
                    // Superior izquierda
                    draw_pixel(x + i, y + j, fill_color);
                    // Superior derecha
                    draw_pixel(x + w - 1 - i, y + j, fill_color);
                    // Inferior izquierda
                    draw_pixel(x + i, y + h - 1 - j, fill_color);
                    // Inferior derecha
                    draw_pixel(x + w - 1 - i, y + h - 1 - j, fill_color);
                }
            }
        }

        // Bordes (si se quiere grosor mayor a 0)
        if (border_thickness > 0) {
            for (int t = 0; t < border_thickness; t++) {
                // Borde superior
                draw_block(x + t, y + t, w - 2 * t, 1, border_color);
                // Borde inferior
                draw_block(x + t, y + h - 1 - t, w - 2 * t, 1, border_color);
                // Borde izquierdo
                draw_block(x + t, y + t, 1, h - 2 * t, border_color);
                // Borde derecho
                draw_block(x + w - 1 - t, y + t, 1, h - 2 * t, border_color);
            }
        }
    }

}

    
