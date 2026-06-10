# ALS31313_Pi — Sensor Hall 3D I2C

Driver para el sensor de efecto Hall 3D **ALS31313** de Allegro MicroSystems, leído mediante I2C en Raspberry Pi.

## Cableado

| ALS31313 | Raspberry Pi |
|----------|-------------|
| VDD      | 3.3V        |
| GND      | GND         |
| SCL      | GPIO 3 (pin 5) |
| SDA      | GPIO 2 (pin 3) |

## Dependencias

```bash
sudo apt install build-essential cmake
```

Habilitar I2C en `/boot/firmware/config.txt`:
```
dtparam=i2c_arm=on
```

## Compilar

```bash
mkdir build && cd build
cmake ..
make
```

## Ejecutar

```bash
./als31313 -a 0x60 -b 1 -d 200
```

## Opciones

| Opción | Descripción |
|--------|-------------|
| `-a <dir>` | Dirección I2C (ej. `0x65`) |
| `-b <bus>` | Bus I2C (ej. `1` para `/dev/i2c-1`) |
| `-d <ms>`  | Delay en ms (0 = una sola lectura) |
| `-f <val>` | Filtro interno (0-7) |
| `-r <val>` | Resolución ADC (0=18bit, 3=12bit) |
| `-q`       | Modo silencioso (retorna temperatura) |

Referencia: [ALS31313 Datasheet](https://www.allegromicro.com/~/media/Files/Datasheets/ALS31313-Datasheet.ashx)
