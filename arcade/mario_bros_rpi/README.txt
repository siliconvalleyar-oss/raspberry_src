MARIO BROS ARCADE - Raspberry Pi Zero 2W + ST7789 240x240
===========================================================

CONTROLES (teclado):
  A / D   -> mover izquierda/derecha
  W       -> saltar
  Q       -> salir del juego

El juego incluye:
  - Física de plataformas (gravedad, colisiones AABB)
  - Bloques ? y ladrillos
  - Enemigos (Goomba) que pueden ser aplastados
  - Efectos de sonido (salto, moneda, muerte...)
  - HUD con vidas y monedas

CONEXIÓN (misma que Pac-Man):
  GMT130 -> RPi Zero 2W
  SCK    -> GPIO 11 (Pin 23)
  SDATA  -> GPIO 10 (Pin 19)
  DC     -> GPIO 25 (Pin 22)
  RST    -> GPIO 24 (Pin 18)
  BL     -> GPIO 23 (Pin 16)
  BUZZER -> GPIO 12 (Pin 32)

COMPILAR Y EJECUTAR:
  make
  sudo make run

¡Disfruta!
