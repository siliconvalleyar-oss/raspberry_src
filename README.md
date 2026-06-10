# raspberry_src

Repositorio de proyectos para Raspberry Pi. Incluye controladores de hardware, juegos, herramientas de diagnóstico automotriz, sensores, pantallas y aplicaciones web.

## Índice de proyectos

### OBD-II / Automotriz

| Proyecto | Descripción |
|----------|-------------|
| **elm_327_rpi_oled_v16/obd2_rpi** | Lector OBD-II con OLED SSD1306 SPI. Conecta vía Bluetooth RFCOMM a escáner ELM327. Muestra RPM, velocidad, temperatura, fuel trim, sensores O2, DTCs y datos GM. C++17 + CMake. |
| **elm327_oled_v1** | Versión inicial del lector OBD-II con OLED. Antecesor de obd2_rpi. |

### Juegos para ST7789

| Proyecto | Descripción |
|----------|-------------|
| **space_invaders** | Space Shooter para pantalla ST7789 240x240. Nave, enemigos, power-ups, oleadas. Sonido por buzzer pasivo. C++11, Makefile. |
| **tetris** | Tetris arcade con SRS rotation, ghost piece, hold, T-spin detection, combo/B2B, 15 niveles. C++14, Makefile. |
| **mario** | Juego de plataformas estilo Mario para ST7789. |
| **dino** | Juego estilo Chrome Dino para ST7789. |

### NFC / Pagos

| Proyecto | Descripción |
|----------|-------------|
| **pn532** | Lector NFC para pagos. Usa libnfc para leer tarjetas ISO14443A y envía el UID a un servidor HTTP via libcurl. C++17, Makefile. |
| **libnfc** | Compilación local de libnfc para Raspberry Pi. |

### Sensores

| Proyecto | Descripción |
|----------|-------------|
| **ads1115** | Driver para ADC ADS1115 (I2C, 16-bit, 4 canales). |
| **ALS31313_Pi** | Driver para sensor de campo magnético 3D ALS31313 (I2C). |
| **ds1994** | Herramientas para DS1994 (iButton) — lectura/escritura via 1-Wire. |
| **ds2482** | Puente I2C a 1-Wire DS2482. Comunicación con DS1994 vía 1-Wire. Incluye scripts de compilación y comandos. |
| **25aa02e48** | Driver para EEPROM 25AA02E48 (SPI) con ID EUI-48. |

### Pantallas / Display

| Proyecto | Descripción |
|----------|-------------|
| **epaper** | Controlador para pantallas e-paper. Incluye Makefile, reglas, scripts bash. |
| **epaper_display_rpi_2w** | Driver para e-paper en RPi Zero 2W. Comunicación SPI directa. |
| **btc_oled** | Visualizador de cotización Bitcoin en OLED SSD1306. |

### Comunicación Inalámbrica

| Proyecto | Descripción |
|----------|-------------|
| **mrf24j40** | Driver para transceptor MRF24J40 (ZigBee/6LoWPAN, 2.4 GHz). Incluye soporte para pantalla OLED. |
| **gsm** | Comunicación GSM con SIM800 (bash y C++). |

### Web / App

| Proyecto | Descripción |
|----------|-------------|
| **www** | Aplicación web Waveforms Live (Ionic + Cordova). Visualización de formas de onda con jQuery Flot. |

### Herramientas / Utilidades

| Proyecto | Descripción |
|----------|-------------|
| **fflush** | Utilidad para forzar flush de buffers en Linux. |
| **ke-rpi-samples** | Ejemplos de KE para RPi: GPIO sysfs, I2C ioctl, SPI ioctl, UART termios. |
| **openScopeMZ** | Cliente Python para OpenScope MZ (osciloscopio). Varias versiones incluyendo servidor y captura. |

## Requisitos generales

- Raspberry Pi (Zero 2W, 3, 4 o superior)
- SPI habilitado (`dtparam=spi=on` en `/boot/firmware/config.txt`)
- I2C habilitado si aplica (`dtparam=i2c_arm=on`)
- Compilador GCC/G++ y Make o CMake
