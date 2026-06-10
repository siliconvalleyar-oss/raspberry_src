# ELM327 OBD-II + OLED SSD1306 para Raspberry Pi

Generador de proyecto OBD-II para Raspberry Pi Zero 2W con pantalla OLED SSD1306 vía SPI y comunicación Bluetooth RFCOMM con escáner ELM327.

## Contenido

- **`elm_327_rpi_oled_v16.sh`** — Script bash que genera la estructura completa del proyecto C++ (src/, include/, CMakeLists.txt) con todos los archivos fuente embebidos. Útil para despliegue rápido.
- **`obd2_rpi/`** — Proyecto completo ya generado, listo para compilar con CMake. Incluye:
  - Lector OBD-II con PIDs estándar y comandos GM modo 22
  - Driver SPI para OLED SSD1306
  - 7 páginas de visualización con rotación automática
  - Sistema de logging CSV
  - Servicio systemd
  - Documentación en `docs/`

## Uso rápido

```bash
# Generar el proyecto desde cero
./elm_327_rpi_oled_v16.sh

# O usar el proyecto ya generado
cd obd2_rpi
mkdir -p build && cd build
cmake .. && make -j$(nproc)
./bin/obd2_rpi 00:1D:A5:07:23:6E
```

## Dependencias

```bash
sudo apt install -y build-essential cmake libbluetooth-dev libgpiod-dev bluez
```

## Hardware

Ver `obd2_rpi/README.md` para esquema de conexiones OLED y configuración Bluetooth.
