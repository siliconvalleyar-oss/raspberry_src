# ADS1115 — ADC I2C de 16 bits

Driver para el convertidor analógico-digital **ADS1115** (16 bits, 4 canales) sobre I2C con Raspberry Pi.

## Cableado

| ADS1115 | Raspberry Pi |
|---------|-------------|
| VDD     | 3.3V (pin 1) |
| GND     | GND (pin 6)  |
| SCL     | GPIO 3 (pin 5) |
| SDA     | GPIO 2 (pin 3) |
| A0-A3   | Entradas analógicas |

## Dependencias

```bash
sudo apt install build-essential cmake wiringpi
```

Habilitar I2C en `/boot/firmware/config.txt`:
```
dtparam=i2c_arm=on
```

## Compilar

```bash
cd ads1115
mkdir build && cd build
cmake ..
make
```

## Ejecutar

```bash
sudo ./ads1115_app
```

## Notas

- Usa divisores de tensión R1=10kΩ, R2=1kΩ en cada entrada
- Ganancia configurada a GAIN_EIGHT (±0.512V)
- Muestra los 4 canales en tiempo real
