# e-Paper Display RPi 2W — Driver SPI para pantalla EPD

Controlador para pantallas **e-paper (EPD)** conectadas por **SPI** a una
Raspberry Pi (Zero 2W, Pi 3, Pi 4). Orientado a pantallas de **2.13"**
(212×104 píxeles).

## Descripción

- Driver EPD con control de pines CS, DC, RST y BUSY vía sysfs GPIO
- Comunicación SPI mediante `/dev/spidev0.0` o `/dev/spidev0.1`
- Soporte para modos de 32 y 64 bits
- API simple: `initialize()`, `reset()`, `sendCommand()`, `sendData()`,
  `displayFrame()`

## Cableado

```
Raspberry Pi                Pantalla EPD
============                ============
GPIO 8 (Pin 24)  ────────  DC
GPIO 25 (Pin 22) ────────  RST
GPIO 17 (Pin 11) ────────  CS
GPIO 24 (Pin 18) ────────  BUSY
GPIO 10 (Pin 19) ────────  MOSI
GPIO 9  (Pin 21) ────────  MISO
GPIO 11 (Pin 23) ────────  SCLK
3.3V              ────────  VCC
GND               ────────  GND
```

Dispositivo SPI: `/dev/spidev0.1`

## Dependencias

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake clang
```

SPI debe estar habilitado:
```bash
sudo raspi-config  → Interface Options → SPI → Enable
```

## Compilación

```bash
mkdir -p build && cd build
cmake ..
make
```

## Ejecución

```bash
./app_epaper
```

## Controles

No aplica. Proyecto de demostración/driver.
