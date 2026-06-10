---
name: ke-rpi-samples
description: >
  Ejemplos en C de GPIO, I2C, SPI y UART para Raspberry Pi, de la serie
  Kickstart Embedded RPi.
---

# ke-rpi-samples

## Descripción

Código de ejemplo en C puro para interfaces de bajo nivel en Raspberry Pi:
GPIO (sysfs), I2C (ioctl), SPI (ioctl) y UART (termios). Sin librerías
externas, usando solo las llamadas al sistema de Linux.

## Desarrollo

- **Lenguaje:** C (C99/C11)
- **Compilación:** `gcc` directo, sin Makefile
- **Estilo:** Funciones auxiliares estáticas, sysfs/ioctl/termios directo
- **Compilador:** gcc (arm-linux-gnueabihf o nativo en RPi)

### Compilar cada ejemplo

```bash
gcc gpio-c-sysfs/gpio_usage_sysfs.c -o gpio_usage_sysfs
gcc i2c-c-ioctl/i2c_ioctl_tmp006.c -o i2c_ioctl_tmp006
gcc spi-c-ioctl/spi_sysfs_loopback.c -o spi_sysfs_loopback
gcc uart-c-termios/uart_loopback.c -o uart_loopback
```

Para OLED: compilar font.o, oled_functions.o y oled_demo.o por separado.

### Notas

- No usa wiringPi ni bcm2835; todo es Linux API pura
- Ejecutar con `sudo` para acceso a /dev, /sys
- Habilitar SPI, I2C, UART en `raspi-config` antes de probar
