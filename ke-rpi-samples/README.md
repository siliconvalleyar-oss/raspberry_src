# ke-rpi-samples

Ejemplos de código en C para Raspberry Pi que acompañan la serie Kickstart Embedded RPi.

Playlist en YouTube: https://youtube.com/playlist?list=PLFa3AkxUCNYTkmCQDSoQBUGKl3Qib3_yX

## Proyectos incluidos

| Directorio             | Descripción                              | Interface |
|------------------------|------------------------------------------|-----------|
| `gpio-c-sysfs/`        | GPIO entrada/salida vía sysfs            | GPIO      |
| `i2c-c-ioctl/`         | Lectura de sensor TMP006 vía ioctl       | I2C       |
| `spi-c-ioctl/`         | Loopback SPI y demo OLED SSD1306         | SPI       |
| `uart-c-termios/`      | Loopback UART con termios                | UART      |

## Dependencias

- Sistema Linux en Raspberry Pi (probado en RPi 4)
- Permisos de superusuario para acceso a GPIO/SPI/I2C/UART
- Interfaces SPI, I2C, UART habilitadas en `raspi-config`

## Compilar

Cada ejemplo se compila individualmente con `gcc`:

```bash
# GPIO
gcc gpio-c-sysfs/gpio_usage_sysfs.c -o gpio_usage_sysfs

# I2C
gcc i2c-c-ioctl/i2c_ioctl_tmp006.c -o i2c_ioctl_tmp006

# SPI loopback
gcc spi-c-ioctl/spi_sysfs_loopback.c -o spi_sysfs_loopback

# SPI OLED
gcc spi-c-ioctl/font.c -c
gcc spi-c-ioctl/oled_functions.c -c
gcc spi-c-ioctl/oled_demo.c -c
gcc oled_demo.o oled_functions.o font.o -o oled_demo

# UART
gcc uart-c-termios/uart_loopback.c -o uart_loopback
```

## Ejecutar

Todos requieren `sudo` por acceso a hardware:

```bash
sudo ./gpio_usage_sysfs <gpio-out> <gpio-in>
sudo ./i2c_ioctl_tmp006
sudo ./spi_sysfs_loopback
sudo ./oled_demo
sudo ./uart_loopback
```

## Conexiones

### GPIO (gpio_usage_sysfs)
Conectar un botón entre el GPIO de entrada y GND, y un LED (con resistencia) entre el GPIO de salida y GND.

### I2C (i2c_ioctl_tmp006)
Conectar TMP006 al bus I2C-1: SDA → pin 3, SCL → pin 5, VCC → 3.3V, GND → GND.

### SPI loopback (spi_sysfs_loopback)
Conectar MOSI (pin 19) a MISO (pin 21).

### SPI OLED (oled_demo)
| Display SSD1306 | RPi Pin      |
|-----------------|--------------|
| MOSI            | pin 19       |
| SCK             | pin 23       |
| CS              | pin 24       |
| DC              | pin 3        |
| RESET           | pin 5        |
| VCC             | 3.3V         |
| GND             | GND          |

### UART loopback (uart_loopback)
Conectar TX (pin 8) a RX (pin 10) en los pines GPIO 14/15 de UART0.
