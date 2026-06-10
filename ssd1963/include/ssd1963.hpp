#pragma once 

#include <cstdint>

namespace SSD1963_DRV{

    struct Ssd1963_t{
        
            Ssd1963_t(){
                ssd1963_init();
                setup_gpio();// setup_gpio();
                ssd1963_init();            
            }
            ~Ssd1963_t()=default;            
        
        public:       

            void clear_screen(uint16_t color) ;

            void draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) ;
        
            void draw_pixel(uint16_t x, uint16_t y, uint16_t color) ;
        
            void draw_image_rgb565(const char* filepath) ;
        
            void draw_sprite_frame(const char* filepath, uint16_t frame_width, uint16_t frame_height, uint16_t frame_index, uint16_t x, uint16_t y) ;                    
                        
            void draw_rounded_rect( int x, int y, int w, int h, int radius, uint16_t fill_color, uint16_t border_color, int border_thickness) ;

            void delay_ms(uint32_t ms) ;

            void set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) ;            

        private: 
            
            void setup_gpio() ;            
        
            void ssd1963_init() ;

            void write_data_bus(uint16_t data) ;
        
            void write_command(uint8_t cmd) ;
        
            void write_data(uint16_t data);                
        
            
    };


}