# e-Paper + QR — Display EPD con generación de códigos QR

Controlador para pantallas **e-paper (EPD)** con soporte para generación de
códigos **QR** en la propia Raspberry Pi. Compatible con Raspberry Pi Zero 2W
y Pi 4.

## Descripción

- Driver EPD completo con inicialización COG, actualización global y apagado
- Soporte para múltiples tamaños: 1.54", 2.13", 2.66", 2.71", 2.87", 3.70",
  4.17", 4.37"
- Generación de códigos QR mediante `libqrencode`
- Imágenes precargadas en buffers monocromo (BW) y rojo (BWR)
- Utilidades de temporización (`Tyme`) y GPIO
- Scripts Bash para configuración de pines y depuración con GDB

## Cableado

Configuración para **Raspberry Pi Zero 2W** (pines por defecto en `boards.h`):

```
Pin   GPIO   Cable    Función
───   ────   ──────   ────────
13    27     Gris     CS
19    10     Azul     MOSI
21    9      Verde    MISO
23    11     Marrón   SCLK
15    22     Violeta  Flash CS
22    25     Amarillo RESET
24    8      Naranja  DC
26    7      Rojo     BUSY
```

Configuración alternativa para **Raspberry Pi 4** (mapeo gpiochip4):

| Función | GPIO | Pin |
|---------|------|-----|
| BUSY    | 537  | 22  |
| DC      | 536  | 18  |
| RESET   | 535  | 16  |
| CS      | 539  | 13  |
| Flash CS| 534  | 15  |

## Dependencias

```bash
sudo apt-get update
sudo apt-get install -y build-essential libqrencode-dev
```

SPI habilitado:
```bash
sudo raspi-config → Interface Options → SPI → Enable
```

## Compilación

```bash
make
# Para binarios específicos:
make TARGET=tx    # epaper_app_tx
make TARGET=rx    # epaper_app_rx
```

## Ejecución

```bash
sudo make run
# o directamente:
sudo ./bin/epaper_app
```

## Controles

No aplica. El programa muestra imágenes precargadas en secuencia al iniciar.
