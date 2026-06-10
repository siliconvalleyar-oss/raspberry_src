# MRF24J40 - Radio ZigBee/802.15.4 + OLED + MQTT para Raspberry Pi

Aplicación en C++17 para Raspberry Pi que integra:

- Módulo radio **MRF24J40MA** (ZigBee / 802.15.4)
- Pantalla **OLED SSD1306** vía SPI
- Comunicación **MQTT** con broker Mosquitto
- Generación de códigos **QR**
- Conexión a base de datos **MySQL**
- Interfaz **SPI** y **GPIO** con librería **bcm2835**

## Dependencias

```bash
# Instalación rápida (ver scripts/ para automatizado)
sudo apt install -y libbcm2835-dev libmosquitto-dev libmysqlcppconn-dev \
  libpng-dev libqrencode-dev qrencode zlib1g-dev

# SSD1306 OLED library
git clone https://github.com/gavinlyonsrepo/SSD1306_OLED_RPI.git
cd SSD1306_OLED_RPI
make && sudo make install
```

## Compilar

```bash
make
```

El binario se genera en `bin/` con nombre según la arquitectura:

| Arquitectura | Binario               |
|-------------|-----------------------|
| aarch64     | `bin/mrf24_tx_app`    |
| armv7l      | `bin/mrf24_rx_app`    |
| x86_64      | `bin/mrf24_app`       |

## Ejecutar

```bash
sudo make run
```

## Conexiones MRF24J40MA

| MRF24J40MA | RPi GPIO            |
|------------|---------------------|
| MOSI       | GPIO 10 (pin 19)    |
| MISO       | GPIO 9  (pin 21)    |
| SCK        | GPIO 11 (pin 23)    |
| CS         | GPIO 8  (pin 24)    |
| WAKE       | GPIO asignable      |
| INT        | GPIO asignable      |
| RESET      | GPIO asignable      |
| VCC        | 3.3V                |
| GND        | GND                 |

## Scripts útiles

En `scripts/`:

| Script                    | Propósito                     |
|---------------------------|-------------------------------|
| `dependeciesLibs.sh`      | Instala todas las dependencias|
| `mosquitto.sh`            | Gestiona servicio Mosquitto   |
| `config_radio.sh`         | Configura la interfaz radio   |
| `git_commit.sh`           | Commit automático con fecha   |
| `installBCM2835.sh`       | Instala bcm2835 desde fuente  |

## Comandos MQTT de ejemplo

```bash
# Publicar
mosquitto_pub -h raspberry.local -p 1883 -t "house/room" \
  -m "{ temp:40 }" -u Username -P "psw"

# Suscribir
mosquitto_sub -h raspberry.local -p 1883 -t "house/room" -v \
  -u Username -P "psw"
```

## Estructura del proyecto

```
mrf24_prj/       → Código fuente principal
  app/           → Punto de entrada
  display/       → Controlador OLED
  epaper/        → Soporte e-paper
  gpio/          → Manejo de GPIO
  mrf24/         → Driver MRF24J40
  network/       → Conexiones MQTT/red
  qr/            → Generación QR
  security/      → Cifrado/seguridad
  spi/           → Comunicación SPI
oled/            → Librería OLED
install/         → Scripts de instalación
scripts/         → Utilidades
```
