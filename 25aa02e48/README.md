# 25AA02E48 — Memoria EEPROM SPI

Driver para la memoria EEPROM SPI **25AA02E48** (2 Kbit) con Raspberry Pi.

## Cableado

| Señal | GPIO | Pin físico |
|-------|------|------------|
| CS    | CE0  | 24         |
| SCLK  | 11   | 23         |
| MOSI  | 10   | 19         |
| MISO  | 9    | 21         |
| VCC   | 3.3V | 17         |
| GND   | GND  | 20         |

## Dependencias

```bash
sudo apt install build-essential cmake
```

Habilitar SPI en `/boot/firmware/config.txt`:
```
dtparam=spi=on
```

## Compilar

```bash
mkdir build && cd build
cmake ..
make
```

## Ejecutar

```bash
sudo ./build/app_25aa
```
