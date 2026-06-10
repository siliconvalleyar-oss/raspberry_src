---
name: openScopeMZ
description: >
  Scripts en Python 3 para controlar el OpenScope MZ (Digilent) como
  osciloscopio digital y generador de onda cuadrada vía puerto serie.
---

# openScopeMZ

## Descripción

Scripts Python que comunican con el OpenScope MZ de Digilent (instrumento
USB basado en FPGA) para actuar como osciloscopio digital. Los scripts
configuran el dispositivo en modo JSON, ajustan parámetros de muestreo y
trigger, y devuelven los voltajes en mV.

## Desarrollo

- **Lenguaje:** Python 3
- **Dependencias:** `pyserial`, `requests`, `numpy`
- **Entorno virtual recomendado:** `python3 -m venv openscope_env`
- **Comunicación:** Puerto serie a 1250000 baud

### Scripts y su evolución

| Script | Propósito |
|--------|-----------|
| `openScopeMZ_v1.4.0.py` | Primera versión funcional |
| `openScopeMZ_v1.4.1.py` a `v1.4.4.py` | Iteraciones de mejora |
| `openScopeMZ_Flutter_v1.0.1.py` | Adaptación para app Flutter |
| `openScopeMZ_Flutter_v1.0.2.py` | Versión Flutter mejorada |
| `openScopeMZ.py` | Versión estable actual (con --debug) |
| `capture_scope.py` | Captura simple a JSON |
| `oscope_server_v1.py` | Clase OScope para integración |

### Ejecutar

```bash
python3 openScopeMZ.py --port /dev/ttyUSB0 --freq 1000000 --buffer 500
```

### Formato de salida

```json
{"voltajes_mV": [1650.0, 1648.5, ...]}
```

### Flujo de trabajo típico

1. Conectar OpenScope MZ vía USB
2. Identificar puerto (`dmesg | grep ttyUSB`)
3. Ejecutar script con argumentos deseados
4. Recibir JSON con las muestras de voltaje

### Notas

- El OpenScope MZ debe estar en modo JSON (`{"mode":"JSON"}`)
- Usar `--debug` para ver trazas de la comunicación serie
- El generador produce onda cuadrada 1 kHz, 3 Vpp por defecto
- Trigger configurado en rising edge (~1.65V)
