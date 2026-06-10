# Tetris Arcade — Raspberry Pi (ST7789 240x240)

Juego Tetris estilo arcade para Raspberry Pi Zero 2W con pantalla ST7789 de 240x240 píxeles. Sonido por GPIO (buzzer pasivo) y entrada por teclado USB o botones GPIO.

## 🎮 Características

- **SRS Rotation** — Super Rotation System con wall kicks completos (JLSTZ + I)
- **7-Bag Randomizer** — Sin repeticiones injustas
- **Ghost Piece** — Muestra dónde caerá la pieza
- **Hold** — Guarda una pieza para usar después
- **T-Spin Detection** — Regla de 3 esquinas para detección
- **Combo + B2B** — Bonificación por líneas consecutivas y Tetris/T-spin seguidos
- **15 Niveles** — Velocidad progresiva (Guideline oficial)
- **Lock Delay** — 500ms para maniobrar antes de fijar la pieza
- **Animaciones** — Flash de líneas, barrido de borrado, banners de nivel/Tetris/T-spin

## 📁 Estructura del proyecto

```
freebuff/tetris/
├── include/            # Archivos de cabecera (.h)
│   ├── Hardware.h      # Configuración pines, SPI, GPIO, colores
│   ├── TetrisEngine.h  # Constantes y API del motor
│   ├── TetrisGfx.h     # API gráfica (bloques 3D, HUD, paneles)
│   ├── TetrisSound.h   # API de sonido (bit-bang GPIO)
│   └── fonts.h         # Fuente bitmap 5×7
├── src/                # Código fuente (.cpp)
│   ├── main_arcade.cpp # Implementación hardware + entrada unificada + main()
│   ├── TetrisEngine.cpp# Motor completo con SRS, wall kicks, scoring
│   ├── TetrisGfx.cpp   # Gráficos ST7789 con bloques 3D, HUD, animaciones
│   └── TetrisSound.cpp # Sonido bit-bang con melodía Korobeiniki
├── obj/                # Objetos intermedios (.o)
├── bin/                # Binario final
├── Makefile
└── readme.md
```

## 🔧 Compilación

```bash
# En la Raspberry Pi:
cd /home/joy/src/freebuff/tetris
make

# O desde SSH:
ssh joy@raspberry.local "cd /home/joy/src/freebuff/tetris && make"
```

## ▶️ Ejecución

```bash
sudo ./bin/tetris
```

## 🎮 Controles

### Teclado (SSH/consola)

| Tecla | Acción |
|-------|--------|
| `←` / `→` | Mover izquierda / derecha |
| `↑` / `Z` / `X` | Rotar pieza |
| `↓` / `S` | Bajar rápido (soft drop) |
| `ESPACIO` / `ENTER` | Caída instantánea (hard drop) |
| `R` / `H` | Hold (guardar pieza) |
| `P` | Pausa |
| `Q` | Salir |

### GPIO (botones físicos)

| GPIO | Pin físico | Acción |
|------|------------|--------|
| 5 | 29 | Izquierda |
| 6 | 31 | Derecha |
| 13 | 33 | Rotar |
| 19 | 35 | Bajar rápido |
| 26 | 37 | Pausa |

## 🔌 Conexiones de pantalla (ST7789)

| Display | GPIO | Pin RPi |
|---------|------|---------|
| SCK | 11 | 23 |
| SDATA (MOSI) | 10 | 19 |
| DC | 25 | 22 |
| RST | 24 | 18 |
| BL | 23 | 16 |
| VCC | 3.3V | 17 |
| GND | GND | 20 |
| SOUND (buzzer) | 12 | 32 |

## 📊 Puntuación (Guideline oficial)

| Acción | Puntos (× nivel) |
|--------|-----------------|
| 1 línea (Single) | 100 |
| 2 líneas (Double) | 300 |
| 3 líneas (Triple) | 500 |
| 4 líneas (Tetris) | 800 |
| T-Spin (0 líneas) | 400 |
| T-Spin Single | 800 |
| T-Spin Double | 1200 |
| T-Spin Triple | 1600 |
| B2B bonus | +150% |
| Combo (por línea) | 50 × combo × nivel |
| Soft drop | +1 por fila |
| Hard drop | +2 por fila |

## ✅ Verificación

```bash
make check
```

## 🏗️ Requisitos

- Raspberry Pi Zero 2W / 3 / 4
- SPI habilitado (`dtparam=spi=on` en `/boot/firmware/config.txt`)
- Pantalla ST7789 240x240
- Buzzer pasivo en GPIO 12 (opcional)
- Botones GPIO (opcional)
