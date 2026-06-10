# DS2482 - Puente I2C a 1-Wire para Raspberry Pi

Controlador y utilidades para el **DS2482**, un puente I2C a 1-Wire que permite
comunicarse con dispositivos 1-Wire como el **DS1994** (RTC iButton) desde una
Raspberry Pi a través del bus I2C.

## Descripción

El proyecto incluye:

- **ds2482Ds1994.cc** — Driver C++ que usa `libi2c` para inicializar el DS2482
  y leer un DS1994 conectado al bus 1-Wire.
- **ds1994.cc** — Lector alternativo que accede al DS1994 vía el subsistema
  `w1` del kernel (`/sys/bus/w1/devices/`).
- **Scripts Bash** (`commandsDs284.sh`, `funcCommandsDs1994.sh`) para enviar
  comandos AT al DS2482 manualmente (`i2cset`/`i2cget`).
- **onewired.sh** — Utilidad para inspeccionar dispositivos 1-Wire detectados
  por el kernel.

## Cableado

```
Raspberry Pi           DS2482 (I2C)
============           =============
GPIO 2 (Pin 3)  ────  SDA
GPIO 3 (Pin 5)  ────  SCL
3.3V (Pin 1)    ────  VCC
GND (Pin 6)     ────  GND

DS2482                Bus 1-Wire
─────                 ──────────
I/O        ────────  DS1994 (data)
GND        ────────  GND
```

Dirección I2C del DS2482: `0x18`

## Dependencias

- Raspberry Pi OS
- I2C habilitado (`raspi-config` → Interface Options → I2C)
- `libi2c-dev`

```bash
sudo apt-get update
sudo apt-get install -y build-essential libi2c-dev i2c-tools
```

## Compilación

```bash
# Driver vía I2C (libi2c)
g++ -o ds1994_reader ds2482Ds1994.cc -li2c

# Lector vía kernel w1
g++ -o ds1994_w1 ds1994.cc
```

## Ejecución

```bash
# Driver I2C (requiere sudo para acceso a /dev/i2c-1)
sudo ./ds1994_reader

# Lector kernel w1
./ds1994_w1

# Scripts de comandos manuales
sudo ./commandsDs284.sh
sudo ./funcCommandsDs1994.sh
```

## Controles

No aplica. Proyecto de línea de comandos / scripts de diagnóstico.
