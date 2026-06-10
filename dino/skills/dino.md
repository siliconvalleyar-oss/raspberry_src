---
name: dino
description: Juego Chrome Dino arcade en ST7789 con Raspberry Pi
---

# Dino — Chrome Dino Arcade

## Stack técnico

- **Lenguaje:** C++14
- **Build:** Makefile
- **Display:** ST7789 240×240, SPI 40 MHz
- **Entrada:** GPIO + teclado USB
- **Sonido:** Buzzer pasivo GPIO 12 (bit-bang)
- **Target:** Raspberry Pi Zero 2W (ARM32/64)

## Comandos útiles

```bash
make
sudo ./bin/dino
make clean
make check   # Verificar hardware (SPI, GPIO)
```

## Arquitectura

```
include/  →  DinoHardware.h, DinoGraphics.h, DinoEngine.h, DinoSound.h, fonts.h
src/      →  main_dino.cpp, DinoEngine.cpp, DinoGraphics.cpp, DinoSound.cpp
```

## Características del juego

- 10 niveles con velocidad progresiva (día/noche alternados)
- 7 tipos de obstáculos
- Power-ups: escudo (5s) y slow-mo (3s)
- 3 vidas, Hi-Score persistente en sesión
- Modo color RGB565 / escala de grises intercambiable
- Anti-flicker con bounding box exacto
