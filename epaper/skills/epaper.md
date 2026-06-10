---
name: epaper
description: Driver e-paper con generación de QR para Raspberry Pi (C++20)
---

# epaper — Skill de desarrollo

## Lenguajes y herramientas
- C++20 (GCC)
- Makefile
- SPI Linux (`spidev`)
- GPIO sysfs
- `libqrencode` (generación QR)
- GDB (debug)

## Estructura
```
epaper/
├── src/main.cpp          Punto de entrada
├── libs/
│   ├── epaper/           Driver EPD (COG, boards, refresh)
│   ├── gpio/             Control GPIO
│   ├── spi/              Comunicación SPI
│   ├── qr/               Generación de códigos QR
│   ├── graphics/         Imágenes precargadas (BW, BWR)
│   ├── tyme/             Temporización
│   ├── app/config.h      Configuración 32/64 bits
│   └── work/work.h       Utilidades
├── bash/                 Scripts de configuración y debug
├── rules/rulesEpaper.gdb Script GDB
├── Makefile
├── .gitignore
├── README.md
└── skills/epaper.md
```

## Compilación
```bash
make
make TARGET=tx   # binario con sufijo _tx
make TARGET=rx   # binario con sufijo _rx
```

## Notas
- Requiere `libqrencode-dev`
- Soporta múltiples tamaños de pantalla EPD (1.54" a 4.37")
- Selecciona placa automáticamente según CPU 32/64 bits
- Usa `boardRaspberryPiZero2W` para 32 bits, `boardRaspberryPi` para 64 bits
- Los scripts Bash en `bash/` facilitan configuración de pines y debug
