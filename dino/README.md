# 🦖 Chrome Dino Arcade — Raspberry Pi

Juego arcade del dinosaurio de Chrome para **Raspberry Pi Zero 2W** con pantalla **ST7789 240×240** y control por **GPIO + teclado USB**.

---

## 📋 Especificaciones

| Característica        | Detalle                                      |
|-----------------------|----------------------------------------------|
| **Plataforma**        | Raspberry Pi (ARM 32/64 bits)                |
| **Pantalla**          | ST7789 240×240 píxeles, SPI 40 MHz           |
| **Driver**            | `/dev/spidev0.0` (SPI modo 3)                |
| **GPIO**              | `/dev/gpiochip0`                             |
| **Sonido**            | Buzzer pasivo GPIO 12 (bit-bang)             |
| **Controles**         | Botón físico GPIO 5 + teclado USB (ESPACIO)  |
| **Idioma README**     | Español / English                             |

---

## 🔌 Conexiones

| Señal     | GPIO | Pin físico |
|-----------|------|------------|
| **SCLK**  | 11   | 23         |
| **MOSI**  | 10   | 19         |
| **DC**    | 25   | 22         |
| **RST**   | 24   | 18         |
| **BL**    | 23   | 16         |
| **VCC**   | 3.3V | 17         |
| **GND**   | GND  | 20         |
| **BUZZER**| 12   | 32         |
| **JUMP**  | 5    | 29         |
| **COLOR** | 6    | 31         |

---

## 🚀 Compilación

```bash
# En la Raspberry Pi
cd /home/joy/src/freebuff/dino
make

# Ejecutar (requiere sudo para GPIO/SPI)
sudo ./bin/dino

# Compilación remota desde PC
ssh joy@raspberry.local "cd /home/joy/src/freebuff/dino && make"
```

### Requisitos del sistema

```bash
# Habilitar SPI en /boot/firmware/config.txt
dtparam=spi=on

# Instalar dependencias (si hacen falta)
sudo apt-get install build-essential
```

---

## 🎮 Controles

| Acción          | Botón físico | Teclado USB     |
|-----------------|-------------|-----------------|
| **Saltar**      | GPIO 5      | `ESPACIO` / `W` / `↑` |
| **Color/B&N**   | GPIO 6      | `C`             |

---

## 🎯 Mecánicas del juego

### Obstáculos
- **7 tipos distintos**: Cactus pequeño, mediano, doble, triple; Pterodáctilo bajo y medio; Roca
- La variedad de obstáculos aumenta con cada nivel

### Power-ups
| Power-up     | Efecto                          | Duración |
|--------------|--------------------------------|----------|
| 🛡️ **Escudo** | Invencibilidad total (destruye obstáculos) | 5 segundos |
| 🐢 **Slow-mo** | Reduce velocidad del juego a la mitad | 3 segundos |

### Sistema de niveles
- **10 niveles** con velocidad progresiva
- Niveles pares: **modo noche** con estrellas y luna
- Niveles impares: **modo día** con nubes
- La distancia entre obstáculos se reduce en cada nivel

### Vidas y Game Over
- 3 vidas para completar la máxima puntuación
- Pantalla de Game Over con récord personal (Hi-Score)
- Hi-Score se mantiene durante la sesión

### Modo Color
- **Color** (RGB565) por defecto
- **Escala de grises** convertible en caliente con botón GPIO 6 o tecla `C`
- Conversión por luminancia BT.601 (aritmética entera)

---

## 📁 Estructura del proyecto

```
freebuff/dino/
├── include/          # Headers (.h)
│   ├── DinoHardware.h    # Configuración de pines y hardware
│   ├── DinoGraphics.h    # Primitivas gráficas y sprites
│   ├── DinoEngine.h      # Constantes y estructuras del juego
│   ├── DinoSound.h       # API de sonido
│   └── fonts.h           # Fuente bitmap 5×7
├── src/              # Código fuente (.cpp)
│   ├── main_dino.cpp     # Inicialización y loop de hardware
│   ├── DinoEngine.cpp    # Motor de juego completo
│   ├── DinoGraphics.cpp  # Primitivas y sprites pixel-art
│   └── DinoSound.cpp     # Sonido bit-bang por GPIO
├── obj/              # Objetos compilados (.o)
├── bin/              # Binario final
├── Makefile
└── README.md
```

---

## ⚙️ Características técnicas

- **Anti-flicker**: Borrado exacto del bounding box antes de redibujar cada sprite
- **Frame timing**: 25 FPS fijos con `nanosleep` de alta precisión
- **SPI 40 MHz**: Buffer de 512 píxeles para ráfagas rápidas
- **GPIO bit-bang**: Sonido polifónico simple sin hardware PWM
- **Dual input**: Botones GPIO + teclado USB simultáneos
- **ARM32/ARM64**: Binario compatible con todas las Raspberry Pi

---

## 📜 Licencia

Uso libre y educativo. Basado en el clásico juego del dinosaurio de Chrome.
