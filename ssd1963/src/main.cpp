#include <bcm2835.h>
#include <iostream>
#include <ssd1963.hpp>
#include <color_rgb565.hpp>


int main() {
    
    if (!bcm2835_init()) {
        std::cerr << "Error al inicializar bcm2835" << std::endl;
        return 1;
    }

   SSD1963_DRV::Ssd1963_t lcd;

    //lcd.setup_gpio();// setup_gpio();
    //lcd.ssd1963_init();
    // Limpiar toda la pantalla a negro primero
    lcd.clear_screen(BLACK);
    lcd.delay_ms(500);
    /*
    struct ColorBlock {
        uint16_t x;
        uint16_t y;
        uint16_t color;
        const char* name;
    };
    // Bloques grandes 60x60, distribuidos horizontalmente
    ColorBlock blocks[] = {
        {10, 10, RED, "RED"},
        {80, 10, GREEN, "GREEN"},
        {150, 10, BLUE, "BLUE"},
        {220, 10, YELLOW, "YELLOW"},
        {290, 10, CYAN, "CYAN"},
        {360, 10, MAGENTA, "MAGENTA"},
        {430, 10, WHITE, "WHITE"},
        // Aquí podrías agregar más o cambiar posición si tu pantalla es más ancha
    };
    const uint16_t block_width = 45;
    const uint16_t block_height = 45;
    for (const auto& block : blocks) {
        std::cout << "Dibujando bloque color: " << block.name << " en (" << block.x << ", " << block.y << ")" << std::endl;
        lcd.draw_block(block.x, block.y, block_width, block_height, block.color);
        // Sin delay para que quede fijo rápido
    }
    std::cout << "Fin del test de colores." << std::endl;
*/
    lcd.draw_rounded_rect(20, 20, 96, 96, 12, 0xFFFF, 0x0000, 2); // x, y, w, h, radio, color fondo, color borde, grosor

    lcd.delay_ms(5000);

    //lcd.draw_image_rgb565("assets/capibaras.rgb565");
    bcm2835_close();
    
    return 0;
}

