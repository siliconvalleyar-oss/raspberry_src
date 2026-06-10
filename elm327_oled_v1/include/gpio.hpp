#ifndef GPIO_HPP
#define GPIO_HPP

// ============================================================
// Configuración de pines para Raspberry Pi + SSD1306 OLED I2C
// ============================================================
//
// El SSD1306 se comunica por I2C (pines físicos 3 y 5).
// Los botones son opcionales para cambiar páginas manualmente.
//
// Pinout Raspberry Pi (BCM numbering):
//   I2C1 SDA  = GPIO 2  (pin físico 3)
//   I2C1 SCL  = GPIO 3  (pin físico 5)
//
// Opcional: botones para navegación
//   BTN_MODE = GPIO 17 (pin físico 11)  - Cambiar página
//   BTN_ACT  = GPIO 27 (pin físico 13)  - Acción / Reset
// ============================================================

#define I2C_DEVICE     "/dev/i2c-1"   // Bus I2C-1 en RPi
#define OLED_I2C_ADDR  0x3C           // Dirección I2C del SSD1306 (0x3C o 0x3D)

// Botones opcionales para navegación (comentar si no se usan)
#define BTN_MODE_PIN   17
#define BTN_ACT_PIN    27

// Buzzer opcional
#define BUZZER_PIN     18            // GPIO 18 (PCM) para sonido

#endif // GPIO_HPP
