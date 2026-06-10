# BTC OLED — Bitcoin Price + Public IP en SSD1306

> Muestra el precio de Bitcoin y la IP pública en una pantalla OLED SSD1306
> de 128x64 píxeles via I2C en Raspberry Pi.

![Lenguaje](https://img.shields.io/badge/lang-C%2B%2B17-blue)
![Display](https://img.shields.io/badge/display-SSD1306%20I2C-green)
![API](https://img.shields.io/badge/API-CoinGecko%20%7C%20ipify-orange)

## Hardware

### Conexiones OLED SSD1306 I2C

| SSD1306 | RPi Pin | GPIO |
|---------|---------|------|
| VCC | Pin 1 (3.3V) | — |
| GND | Pin 6 (GND) | — |
| SDA | Pin 3 | GPIO2 (SDA) |
| SCL | Pin 5 | GPIO3 (SCL) |

**Dirección I2C:** `0x3C` (default)
**Velocidad:** 100 kHz (BCM2835_I2C_CLOCK_DIVIDER_626)

### Dependencias

| Paquete | Versión | Propósito |
|---------|---------|-----------|
| [bcm2835](http://www.airspayce.com/mikem/bcm2835/) | 1.75+ | GPIO/I2C Raspberry Pi |
| libcurl4-openssl-dev | — | HTTP requests (APIs) |
| nlohmann-json | 3.11.3 | Parseo JSON (incluido) |

## Compilación

### 1. Instalar dependencias

```bash
# Opcion 1: via make
make install

# Opcion 2: manual
sudo apt update
sudo apt install -y libcurl4-openssl-dev

# bcm2835 (si no esta instalado)
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.75.tar.gz
tar xzf bcm2835-1.75.tar.gz
cd bcm2835-1.75 && ./configure && make && sudo make install
```

### 2. Compilar

```bash
# Local (en la RPi)
make

# Via SSH
ssh joy@raspberry.local "cd /home/joy/src/git/btc_oled && make"
```

### 3. Ejecutar

```bash
# Requiere sudo por bcm2835
make run
# o
sudo ./bin/app_oled
```

## APIs Utilizadas

| API | URL | Propósito |
|-----|-----|-----------|
| [CoinGecko](https://www.coingecko.com/en/api) | `api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum&vs_currencies=usd,usdt` | Precio BTC y ETH |
| [ipify](https://www.ipify.org/) | `api64.ipify.org` | IP pública |

### Formato de respuesta CoinGecko

```json
{
  "bitcoin": { "usd": 65000, "usdt": 65000 },
  "ethereum": { "usd": 3500 }
}
```

## Estructura del Proyecto

```
btc_oled/
├── Makefile                    # Build system
├── README.md                   # Documentation
├── .gitignore
├── bin/                        # Binary output
├── obj/                        # Object files
├── include/                    # Headers
│   ├── json/
│   │   └── json.hpp            # nlohmann/json (bundled)
│   ├── btc.hpp                 # BTC price client
│   ├── ip_global.hpp           # Public IP client
│   ├── Bitmap_test_data.hpp    # Test bitmaps
│   ├── SSD1306_OLED.hpp        # SSD1306 driver
│   ├── SSD1306_OLED_graphics.hpp
│   ├── SSD1306_OLED_Print.hpp
│   └── SSD1306_OLED_font.hpp
├── src/
│   ├── OLED_BTC/
│   │   ├── main.cpp            # Entry point
│   │   ├── btc.cpp             # CoinGecko API
│   │   └── ip.cpp              # ipify API
│   ├── SSD1306_OLED.cpp        # SSD1306 I2C driver
│   ├── SSD1306_OLED_graphics.cpp
│   ├── SSD1306_OLED_Print.cpp
│   └── SSD1306_OLED_font.cpp
└── scripts/
    └── install_deps.sh         # Dependency installer
```

## Arquitectura

```
main.cpp
  ├── BTC::Btc_t::function()  ──→ CoinGecko API ──→ JSON parse ──→ "BTC 65000"
  ├── IP_GLOBAL::IpGlobal_t::global() ──→ ipify API ──→ "123.45.67.89"
  └── SSD1306 OLED
        ├── OLEDbegin(I2C)     ──→ init I2C 0x3C
        ├── OLEDclearBuffer()
        ├── setFontNum(Wide)
        ├── setCursor(12, 32)
        ├── print(texto)
        └── OLEDupdate()       ──→ vuelca framebuffer por I2C
```

**Flujo:**
1. `setup()` → init bcm2835 + OLED
2. `callDisplay()` → fetch BTC price + IP
3. Render en OLED con fuente `OLEDFontType_Wide`
4. Espera 5 segundos
5. `EndTests()` → power down OLED + cierra bcm2835

## Objetos Binarios (`.o`) Existentes

Los archivos `obj/btc.o` y `obj/ip.o` son compilaciones previas
del RPi. Se regeneran automáticamente al compilar.

## Fuente del Driver SSD1306

El driver OLED proviene de
[gavinlyonsrepo/SSD1306_OLED_RPI](https://github.com/gavinlyonsrepo/SSD1306_OLED_RPI)
(v1.61). Incluido directamente en `src/` y `include/` para compilación
estática sin dependencias externas adicionales.

## Licencia

MIT
