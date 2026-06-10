---
name: btc_oled
description: Bitcoin price + Public IP display on SSD1306 OLED for Raspberry Pi
---

# BTC OLED Development Skill

C++17 project for Raspberry Pi that fetches Bitcoin price (CoinGecko API)
and public IP (ipify API) and displays them on a SSD1306 OLED via I2C.

## Project Structure

```
btc_oled/
├── Makefile
├── include/
│   ├── json/json.hpp       # nlohmann/json (bundled)
│   ├── btc.hpp             # BTC price client
│   ├── ip_global.hpp       # Public IP client
│   ├── SSD1306_OLED.hpp    # SSD1306 I2C driver
│   └── ...
├── src/
│   ├── OLED_BTC/
│   │   ├── main.cpp        # Entry point
│   │   ├── btc.cpp         # CoinGecko API consumer
│   │   └── ip.cpp          # ipify API consumer
│   ├── SSD1306_OLED.cpp    # OLED I2C driver
│   └── ...
└── scripts/
    └── install_deps.sh     # Dependency installer
```

## Wiring (SSD1306 I2C)

| OLED | RPi Pin | GPIO |
|------|---------|------|
| VCC  | 1 (3.3V) | - |
| GND  | 6 (GND) | - |
| SDA  | 3 | GPIO2 |
| SCL  | 5 | GPIO3 |

Address: 0x3C, Speed: 100kHz

## Commands

```bash
make              # Build
make run          # Run (sudo)
make clean        # Clean
make install      # Install deps
```

## Build via SSH

```bash
ssh joy@raspberry.local "cd /home/joy/src/git/btc_oled && make"
```

## Dependencies

- bcm2835 (GPIO/I2C library, v1.75)
- libcurl4-openssl-dev
- nlohmann-json (bundled in include/json/)

## APIs

- CoinGecko: `api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum&vs_currencies=usd,usdt`
- ipify: `api64.ipify.org`

## Architecture

```
main()
├── setup() → bcm2835_init() + OLEDbegin(I2C)
├── callDisplay()
│   ├── BTC::Btc_t::function() → libcurl → CoinGecko → JSON parse
│   ├── IP_GLOBAL::IpGlobal_t::global() → libcurl → ipify
│   └── OLED: clearBuffer → setFont → setCursor → print → update
└── EndTests() → OLEDPowerDown() + bcm2835_close()
```

## Troubleshooting

- **bcm2835 not found**: Run `make install` to download and compile
- **I2C not working**: Enable with `sudo raspi-config nonint do_i2c 0`
- **OLED blank**: Check wiring, verify with `sudo i2cdetect -y 1`
- **API timeout**: RPi needs internet access; check with `curl api64.ipify.org`
