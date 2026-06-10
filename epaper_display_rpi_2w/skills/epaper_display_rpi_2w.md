---
name: epaper_display_rpi_2w
description: Driver SPI para pantalla e-paper 2.13" en Raspberry Pi
---

# epaper_display_rpi_2w — Skill de desarrollo

## Lenguajes y herramientas
- C++14 (Clang)
- CMake
- SPI Linux (`spidev`)
- GPIO sysfs

## Estructura
```
epaper_display_rpi_2w/
├── main.cc           Punto de entrada
├── epaper.cc/h       Driver EPD
├── spi.cc/h          Comunicación SPI
├── config.h          Detección 32/64 bits
├── CMakeLists.txt    Build CMake
├── .gitignore
├── README.md
└── skills/epaper_display_rpi_2w.md
```

## Compilación
```bash
mkdir -p build && cd build && cmake .. && make
```

## Notas
- Usa Clang como compilador por defecto (configurado en CMakeLists.txt)
- SPI debe estar habilitado en la RPi
- Los pines se configuran al instanciar `EPaper_t(cs, dc, rst, busy)`
- Soporta pantallas de 2.13" (212×104) con `SCREEN 213`
