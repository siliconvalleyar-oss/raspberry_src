# OBD-II Monitor para Raspberry Pi

Monitor de diagnóstico automotriz en tiempo real para Raspberry Pi, compatible
con adaptadores **ELM327 Bluetooth** y display **SSD1306 OLED** (128×64 I2C).

## Características

- Conexión Bluetooth automática con ELM327 (auto-descubrimiento RFCOMM)
- Inicialización robusta con reintentos y manejo de errores
- PIDs OBD2 estándar (RPM, velocidad, temperaturas, carga, etc.)
- Compatible Chevrolet GM (VPW y CAN)
- 6 páginas de visualización: Dashboard, Gauges, Detalles, Fuel, DTC, Info
- Historial de datos para gráficos (64 muestras por sensor)
- Detección de MIL (Check Engine) con alerta visual
- Diseño modular y escalable

## Cableado

```
Raspberry Pi               SSD1306 OLED
============               ============
GPIO 2 (Pin 3)  <──────>  SDA
GPIO 3 (Pin 5)  <──────>  SCL
3.3V (Pin 1)    <──────>  VCC
GND (Pin 6)     <──────>  GND

Botón MODE: GPIO 17 (Pin 11) → GND (pull-up interno)
Botón ACT:  GPIO 27 (Pin 13) → GND (pull-up interno)
```

## Dependencias

```bash
sudo apt-get update
sudo apt-get install -y build-essential git bluetooth bluez libbluetooth-dev
sudo apt-get install -y i2c-tools libi2c-dev
```

Requiere librería **bcm2835** (para GPIO/I2C/SPI):
http://www.airspayce.com/mikem/bcm2835/

## Compilación

```bash
make clean && make
# o vía CMake:
mkdir -p build && cd build && cmake .. && make
```

## Ejecución

```bash
# Requiere sudo para GPIO y Bluetooth
sudo ./bin/obd_monitor
```

## Controles

| Botón  | GPIO | Acción                          |
|--------|------|---------------------------------|
| MODE   | 17   | Cambiar página de visualización |
| ACT    | 27   | Borrar DTCs (en página DTC)     |
| Ctrl+C | -    | Salir del programa              |
