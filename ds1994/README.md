# DS1994-F5 — iButton RTC + SRAM (1-Wire)

Driver en C++17 y scripts Bash para el **DS1994-F5**, un iButton con RTC, temporizador de intervalos, contador de ciclos y 512 bytes de SRAM, comunicado por bus 1-Wire.

## Estructura

```
ds1994/
├── src_deepseek/              # Driver C++17 (main.cpp funcional)
├── ds1994_deepseek_success/   # Proyecto generado con Makefile y detección automática
│   └── ds1994_project_fixed/  # Versión corregida del driver
└── scrpits_ds1994/            # Scripts Bash de diagnóstico
    ├── busqueda_ds1994.sh     # Búsqueda intensiva en bus 1-Wire
    ├── ds2482_detect.sh       # Detección via DS2482 (I2C)
    ├── reset_kernel_w1_ds1994.sh  # Reinicio del bus 1-Wire
    └── verif_ds1994.sh        # Verificación completa del dispositivo
```

## Cableado

| DS1994 (iButton) | Raspberry Pi |
|------------------|-------------|
| Data (centro)    | GPIO 4 (pin 7) con pull-up 4.7kΩ a 3.3V |
| GND (cápsula)    | GND |

## Dependencias

```bash
sudo apt install build-essential
```

Cargar módulos del kernel:
```bash
sudo modprobe w1-gpio
sudo modprobe w1-therm
```

## Compilar y ejecutar

```bash
cd src_deepseek
g++ -std=c++17 -o ds1994_app main.cpp -pthread
sudo ./ds1994_app info
```

## Comandos

| Comando | Descripción |
|---------|-------------|
| `info`      | Información completa del dispositivo |
| `dump`      | Volcado hexadecimal de memoria |
| `status`    | Registros de status y control |
| `rtc`       | Leer RTC |
| `setrtc`    | Sincronizar RTC con hora del sistema |
| `interval`  | Leer temporizador de intervalo |
| `cycles`    | Leer contador de ciclos |
| `osc on/off`| Controlar oscilador |
| `read <pag>`| Leer página (0-15) |
| `write <pag> <hex>` | Escribir datos hex en página |
| `test`      | Prueba de escritura |

Referencia: [DS1994 Datasheet (Maxim)](https://www.analog.com/en/products/ds1994.html)
