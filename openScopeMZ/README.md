# OpenScope MZ - Osciloscopio digital + Generador de onda

Scripts en Python 3 para controlar el **OpenScope MZ** (Digilent) como osciloscopio digital y generador de onda cuadrada. Comunicación vía puerto serie/UART.

## Scripts

| Archivo                              | Descripción                                  |
|--------------------------------------|----------------------------------------------|
| `openScopeMZ.py`                     | Osciloscopio + generador, salida JSON        |
| `openScopeMZ_Flutter_v1.0.1.py`      | Versión compatible con app Flutter           |
| `openScopeMZ_Flutter_v1.0.2.py`      | Versión Flutter mejorada                     |
| `openScopeMZ_v1.4.0.py`              | Versión inicial con comandos correctos       |
| `openScopeMZ_v1.4.1.py`              |                                              |
| `openScopeMZ_v1.4.2.py`              |                                              |
| `openScopeMZ_v1.4.3.py`              |                                              |
| `openScopeMZ_v1.4.4.py`              |                                              |
| `capture_scope.py`                   | Captura forma de onda simple a JSON          |
| `oscope_server_v1.py`                | Servidor clase OScope                        |

## Dependencias

```bash
python3 -m venv openscope_env
source openscope_env/bin/activate
pip install pyserial requests numpy
```

O vía el script incluido:

```bash
bash config_pyhton_for_rpi_ospensopeMS.sh
```

## Hardware

- **OpenScope MZ** (Digilent)
- Cable USB-UART (el OpenScope MZ se presenta como `/dev/ttyUSB0`)
- Baudrate predeterminado: **1250000**

## Uso

### openScopeMZ.py (recomendado)

```bash
python3 openScopeMZ.py --port /dev/ttyUSB0 --freq 1000000 --buffer 500
```

Argumentos:

| Argumento    | Default        | Descripción                         |
|-------------|----------------|-------------------------------------|
| `--port`    | `/dev/ttyUSB0` | Puerto serie del OpenScope MZ       |
| `--baudrate`| `1250000`      | Velocidad de comunicación           |
| `--freq`    | `1000000`      | Frecuencia de muestreo (Hz)         |
| `--buffer`  | `500`          | Tamaño del buffer de muestras       |
| `--timeout` | `5.0`          | Timeout total en segundos           |
| `--debug`   | `false`        | Muestra mensajes de depuración      |

Salida: JSON con array de voltajes en mV.

### capture_scope.py

```bash
python3 capture_scope.py --port /dev/ttyUSB0 --freq 1000000 --buffer 500
```

## Funcionamiento

1. Activa modo JSON en el OpenScope MZ
2. Configura el osciloscopio (canal 1, ganancia 0.25, trigger rising edge)
3. Configura el generador de onda cuadrada (1 kHz, 3 Vpp)
4. Arma el trigger en modo single
5. Lee los datos binarios y los convierte a mV
6. Devuelve JSON con `{"voltajes_mV": [...]}`
