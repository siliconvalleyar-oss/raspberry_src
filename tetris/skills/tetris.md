---
name: tetris
description: Tetris arcade game for Raspberry Pi with ST7789 display
---

# Tetris Development Skill

This skill helps you work with the Tetris Arcade game — a C++14 application for
Raspberry Pi with ST7789 240x240 display, SRS rotation, and GPIO input.

## Project Structure

```
tetris/
├── include/
│   ├── fonts.h              # 5x7 bitmap font
│   ├── Hardware.h           # Pin config, SPI, GPIO, colors
│   ├── TetrisEngine.h       # Game engine constants and API
│   ├── TetrisGfx.h          # Graphics API (3D blocks, HUD)
│   └── TetrisSound.h        # Sound API (bit-bang GPIO)
├── src/
│   ├── main_arcade.cpp      # Entry point, HW, input
│   ├── TetrisEngine.cpp     # SRS, wall kicks, scoring
│   ├── TetrisGfx.cpp        # ST7789 rendering, animations
│   └── TetrisSound.cpp      # Korobeiniki melody
├── Makefile
└── readme.md
```

## Build & Run

```bash
make              # Build
sudo ./bin/tetris # Run (requires sudo for GPIO/SPI)
make check        # Verify SPI configuration
```

## Features

- SRS rotation with full wall kicks (JLSTZ + I)
- 7-Bag randomizer
- Ghost piece, Hold, T-Spin detection
- Combo + B2B scoring
- 15 levels (Guideline speed curve)
- Lock delay (500ms)
- Animations (line flash, sweep, banners)

## Controls

| Key | Action |
|-----|--------|
| Arrow keys | Move/rotate |
| Space/Enter | Hard drop |
| R/H | Hold |
| P | Pause |
| Q | Quit |

## GPIO Buttons

GPIO 5, 6, 13, 19, 26 (see README for pin mapping)
