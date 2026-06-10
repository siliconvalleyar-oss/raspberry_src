# tetris

## Descripción
Juego Tetris clásico para Raspberry Pi con salida en terminal usando caracteres ASCII.

## Dependencias
- libpthread
- libbcm2835 (opcional, para entrada por GPIO)

## Compilación
```bash
ssh joy@raspberry.local "cd /home/joy/src/git/tetris && make"
```

## Estructura
```
├── assets/
├── bin/
├── include/
├── libs/
├── obj/
├── scripts/
├── skills/
├── src/
├── Makefile
└── README.md
```

## Uso
```bash
sudo ./bin/tetris
```
