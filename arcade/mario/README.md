# Mario Bros Arcade - RPi Zero 2W + ST7789 240x240

Videojuego estilo Mario Bros para Raspberry Pi con pantalla ST7789 de 240x240 píxeles. Incluye física de plataformas (gravedad, colisiones AABB), bloques ?, ladrillos, enemigos Goomba y efectos de sonido.

## Dependencias

- Raspberry Pi (probado en Zero 2W)
- `g++` con soporte C++11
- `libpthread`
- Buzzer pasivo en GPIO 12 (sonido)
- Pantalla ST7789 240x240 vía SPI

## Compilar

```bash
make
```

## Ejecutar

```bash
sudo make run
```

O directamente:

```bash
sudo bin/mario
```

## Controles

| Tecla | Acción                 |
|-------|------------------------|
| A     | Mover izquierda        |
| D     | Mover derecha          |
| W     | Saltar                 |
| Q     | Salir del juego        |

## Conexiones (pantalla ST7789)

| GMT130 / ST7789 | RPi Zero 2W         |
|-----------------|---------------------|
| SCK             | GPIO 11 (pin 23)    |
| SDATA           | GPIO 10 (pin 19)    |
| DC              | GPIO 25 (pin 22)    |
| RST             | GPIO 24 (pin 18)    |
| BL              | GPIO 23 (pin 16)    |
| BUZZER          | GPIO 12 (pin 32)    |
| VCC             | 3.3V                |
| GND             | GND                 |

## Características

- Física de plataformas con gravedad y colisiones AABB
- Bloques ? y ladrillos destructibles
- Enemigos Goomba aplastables
- Efectos de sonido (salto, moneda, muerte)
- HUD con vidas y monedas
