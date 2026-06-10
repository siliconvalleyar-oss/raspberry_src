---
name: mario
description: >
  Juego Mario Bros arcade para Raspberry Pi Zero 2W con pantalla ST7789
  240x240, físicas de plataformas, enemigos y efectos de sonido.
---

# mario

## Descripción

Videojuego estilo Mario Bros que corre en una Raspberry Pi con pantalla
ST7789 de 240x240 píxeles. Implementa físicas de plataformas (gravedad y
colisiones AABB), enemigos Goomba, bloques ? y ladrillos, HUD con
vidas/monedas y efectos de sonido por buzzer.

## Desarrollo

- **Lenguaje:** C++11
- **Compilador:** g++ (arm-linux-gnueabihf o nativo)
- **Build system:** Makefile
- **Librerías:** pthread (para sonido asíncrono)
- **Sin librerías gráficas externas** — renderiza directo al framebuffer SPI

### Compilar

```bash
make
```

### Limpiar

```bash
make clean
```

### Depuración

```bash
make debug
```

### Estructura

```
src/          → Código fuente C++
include/      → Cabeceras
lib/          → Librerías externas (picoPNG)
assets/       → Sprites, imágenes, sonidos
obj/          → Objetos compilados
bin/          → Binario final
```

### Clases principales

- `Game` — Bucle principal del juego
- `Player` — Jugador (físicas, animaciones)
- `Enemy` — Enemigos Goomba
- `Level` — Niveles y tiles
- `Renderer` — Renderizado a ST7789
- `Graphics` — Carga de sprites PNG
- `Sound` — Reproducción de sonidos (buzzer PWM)
- `Physics` — Físicas y colisiones AABB
- `Collision` — Detección de colisiones
- `AnimatedSprite` — Sprites animados

### Notas

- Requiere ejecución con `sudo` para acceso a GPIO/memoria
- La pantalla ST7789 usa SPI con pines específicos (ver README)
- El buzzer en GPIO 12 genera tonos PWM
