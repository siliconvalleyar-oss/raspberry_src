# fflush

## Descripción
Utilidades de sistema para forzar el volcado (flush) de búferes de E/S en Linux usando `fflush()`. Contiene dos binarios independientes:

- **flush**: Programa simple que fuerza el flush de búferes
- **app**: Aplicación principal con funcionalidad extendida

## Dependencias
- libpthread

## Compilación
```bash
ssh joy@raspberry.local "cd /home/joy/src/git/fflush && make"
```

## Estructura
```
├── bin/
├── include/
├── libs/
├── obj/
├── scripts/
├── skills/
├── src/
│   ├── flush.cc
│   └── main.cc
├── Makefile
└── README.md
```

## Uso
```bash
sudo ./bin/flush
./bin/app
```
