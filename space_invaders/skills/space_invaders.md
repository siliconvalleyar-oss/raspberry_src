---
name: space_invaders
description: Space Shooter game for Raspberry Pi with ST7789 display
---

# Space Shooter Development Skill

This skill helps you work with the Space Shooter game — a C++11 application for
Raspberry Pi Zero 2W with ST7789 240x240 display and GPIO/PWM sound.

## Project Structure

```
space_invaders/
├── include/
│   ├── fonts.h              # 5x7 bitmap font
│   ├── GameEngine.h         # Game state and logic API
│   ├── Graphics.h           # Graphics primitives and sprites
│   ├── HardwareProfile.h    # Pin definitions and config
│   └── Sound.h              # Sound API
├── src/
│   ├── main.cpp             # HW init, SPI, GPIO, display init
│   ├── GameEngine.cpp       # Game loop, enemies, collisions
│   ├── Graphics.cpp         # Rendering, sprites, text
│   └── Sound.cpp            # PWM bit-bang sound
├── Makefile
└── README.md
```

## Build & Run

```bash
make              # Build
make run          # Build and run (with sudo)
sudo ./bin/pacman # Direct execution
make debug        # Debug build
make check        # Verify SPI configuration
```

## Controls

- **Demo mode**: automatic (default, no buttons needed)
- **GPIO buttons**: enable `USE_BUTTONS` in HardwareProfile.h

## Hardware

- ST7789 240x240 via SPI (mode 3, 40 MHz)
- Buzzer pasivo on GPIO 12
- Uses `/dev/spidev0.0` and `/dev/gpiochip0`

## Notes

- The binary is named `pacman` but the game is a Space Shooter
- No external graphics libraries needed
- Requires SPI enabled in `/boot/firmware/config.txt`
