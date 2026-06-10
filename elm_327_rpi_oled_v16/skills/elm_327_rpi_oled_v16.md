# elm_327_rpi_oled_v16

## Descripción
Proyecto OBD-II con interfaz ELM327 y display OLED para Raspberry Pi. Es un wrapper que contiene el submódulo `obd2_rpi/`.

## Dependencias
- cmake
- libgpiod-dev, libbluetooth-dev
- nlohmann-json3-dev

## Compilación
```bash
ssh joy@raspberry.local "cd /home/joy/src/git/elm_327_rpi_oled_v16 && make"
```

## Estructura
```
├── bin/
├── include/
│   └── json/
├── libs/
├── obj/
├── scripts/
├── skills/
├── src/
├── obd2_rpi/          ← proyecto principal
├── Makefile
└── README.md
```

## Más información
Ver `obd2_rpi/README.md` para documentación detallada del proyecto OBD-II.
