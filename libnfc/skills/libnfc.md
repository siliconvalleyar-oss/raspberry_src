---
name: libnfc
description: >
  Librería NFC para espacio de usuario. Soporta PN532, ACR122, SCL3711
  y otros lectores NFC vía USB, UART, I2C o SPI.
---

# libnfc

## Descripción

libnfc provee una API unificada para acceder a dispositivos NFC desde
aplicaciones de usuario. Soporta múltiples drivers (pn53x_usb, acr122_usb,
acr122_pcsc, pcsc, pn53x_uart, pn53x_i2c, pn53x_spi).

## Desarrollo

- **Lenguaje:** C
- **Sistema de build:** Autotools (autoreconf + configure + make) y CMake
- **Versión actual:** 1.8.0
- **Estándar:** C99 con extensiones POSIX

### Compilar con autotools

```bash
autoreconf -vis
./configure --with-drivers=all
make
```

### Compilar con CMake

```bash
mkdir build && cd build
cmake ..
make
```

### Drivers soportados

- `pn53x_usb` — PN532 vía USB (libusb-0.1)
- `acr122_usb` — ACR122 vía USB (libusb-0.1)
- `acr122_pcsc` — ACR122 vía PCSC (pcsc-lite)
- `pcsc` — Lectores PCSC genéricos
- `pn53x_uart` — PN532 vía UART serie
- `pn53x_i2c` — PN532 vía I2C
- `pn53x_spi` — PN532 vía SPI

### Herramientas incluidas

- `nfc-list` — Listar dispositivos NFC
- `nfc-scan-device` — Escanear dispositivos
- `nfc-poll` — Polling pasivo
- `nfc-emulate-forum-tag2` — Emular tag Tipo 2
- `nfc-mfclassic` — Operaciones MIFARE Classic
- `nfc-relay-picc` — Relay NFC

### Notas

- En Linux >= 3.1, blacklistear módulos nfc/pn533/pn533_usb
- Usar udev rules de `contrib/udev/` para permisos de usuario
- Configuración en `/etc/nfc/libnfc.conf`
