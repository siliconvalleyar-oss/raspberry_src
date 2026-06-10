# Space Shooter — Raspberry Pi + ST7789

Juego Space Shooter para Raspberry Pi Zero 2W con pantalla ST7789 de 240x240 píxeles. Control automático con demo mode o joystick GPIO.

## Hardware

### Conexión ST7789 (GMT130 v1.0)

| GMT130 | GPIO BCM | Pin RPi |
|--------|----------|---------|
| VCC    | 3.3V     | 17      |
| GND    | GND      | 20      |
| SCK    | 11       | 23      |
| SDATA  | 10       | 19      |
| DC     | 25       | 22      |
| RST    | 24       | 18      |
| BL     | 23       | 16      |

### Buzzer (opcional)

| Buzzer | GPIO | Pin |
|--------|------|-----|
| +      | 12   | 32  |
| -      | GND  | -   |

> El buzzer debe ser **pasivo** (no activo). Usar resistencia de 100Ω en serie.

## Dependencias

```bash
# Habilitar SPI en /boot/firmware/config.txt:
#   dtparam=spi=on
sudo reboot
```

## Compilación

```bash
make
```

## Ejecución

```bash
sudo ./bin/pacman
```

## Controles

El juego incluye un **demo mode** automático (movimiento horizontal + disparo automático). Para usar botones GPIO, descomentar `#define USE_BUTTONS` en `include/HardwareProfile.h` y conectar:

| Acción   | GPIO | Pin |
|----------|------|-----|
| Arriba   | 27   | 13  |
| Abajo    | 27   | 16  |
| Izquierda| 22   | 15  |
| Derecha  | 17   | 11  |

## Mecánica del juego

- **Jugador**: nave espacial que se mueve y dispara automáticamente
- **Enemigos**: 3 tipos (Basic, Fast, Tank) que aparecen en oleadas
- **Power-up**: escudo temporal (destello dorado) que absorbe golpes
- **Puntuación**: 10pts (Basic), 15pts (Fast), 25pts (Tank)
- **Vidas**: 3 por partida
- **Oleadas**: incrementan dificultad y cantidad de enemigos

## Solución de problemas

- `open /dev/spidev0.0: No such file or directory` → SPI no habilitado
- `open /dev/gpiochip0: Permission denied` → ejecutar con `sudo`
- Pantalla en blanco → verificar VCC (3.3V) y conexión BL

## Estructura

```
space_invaders/
├── include/
│   ├── fonts.h
│   ├── GameEngine.h
│   ├── Graphics.h
│   ├── HardwareProfile.h
│   └── Sound.h
├── src/
│   ├── main.cpp
│   ├── GameEngine.cpp
│   ├── Graphics.cpp
│   └── Sound.cpp
├── Makefile
└── README.md
```
