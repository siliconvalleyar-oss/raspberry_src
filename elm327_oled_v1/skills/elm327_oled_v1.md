---
name: elm327_oled_v1
description: Monitor OBD-II con ELM327 Bluetooth y SSD1306 OLED para Raspberry Pi
---

# elm327_oled_v1 — Skill de desarrollo

## Lenguajes y herramientas
- C++11 (GCC)
- Makefile + CMake
- Bluetooth (BlueZ, RFCOMM)
- I2C (SSD1306 OLED)
- GPIO (bcm2835)

## Estructura
```
elm327_oled_v1/
├── include/           Headers públicos
├── src/               Fuentes C++
├── build/             Build CMake (legacy)
├── obj/               Objetos compilados
├── bin/               Binario final
├── CMakeLists.txt     Build alternativo
├── Makefile           Build principal
├── .gitignore
├── README.md
└── skills/elm327_oled_v1.md
```

## Compilación
```bash
make clean && make
# o
mkdir -p build && cd build && cmake .. && make
```

## Notas
- Requiere `libbluetooth-dev` y `libbcm2835`
- I2C debe estar habilitado en la RPi
- Ejecutar con `sudo` para acceso a GPIO y Bluetooth
- El display SSD1306 debe aparecer en `i2cdetect -y 1` como `0x3C` o `0x3D`
