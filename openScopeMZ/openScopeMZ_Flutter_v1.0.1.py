#!/usr/bin/env python3
"""
Script para OpenScope MZ que actúa como osciloscopio + generador de onda cuadrada.
Devuelve los voltajes en mV en formato JSON.
Compatible con la app Flutter.
"""

import serial
import time
import struct
import json
import sys
import argparse

def send_cmd(ser, cmd_str, wait=0.3):
    """Envía un comando y devuelve la línea de respuesta."""
    ser.write(cmd_str.encode() + b'\r\n')
    time.sleep(wait)
    return ser.readline()

def capture_waveform(port, baudrate, sample_freq, buffer_size, timeout_total=5.0, debug=False):
    """
    Configura el OpenScope MZ como generador (1 kHz, 3 Vpp) y osciloscopio,
    espera el trigger y devuelve lista de voltajes en mV.
    """
    ser = serial.Serial(port, baudrate, timeout=2)
    time.sleep(1)  # Estabilización
    ser.reset_input_buffer()

    # 1. Modo JSON
    if debug:
        print("DEBUG: Activando modo JSON...", file=sys.stderr)
    ser.write(b'{"mode":"JSON"}\r\n')
    time.sleep(0.5)
    resp = ser.readline()
    if debug:
        print(f"DEBUG: Modo JSON respuesta: {resp}", file=sys.stderr)
    if b'statusCode' not in resp:
        raise Exception("No se pudo activar modo JSON")

    # 2. Reset de instrumentos
    send_cmd(ser, '{"device":[{"command":"resetInstruments"}]}')
    # Descartar respuesta
    ser.readline()

    # 3. Configurar osciloscopio (canal 1)
    osc_cfg = {
        "osc": {
            "1": [{
                "command": "setParameters",
                "vOffset": 0,
                "gain": 0.25,
                "sampleFreq": sample_freq,
                "bufferSize": buffer_size,
                "triggerDelay": 0
            }]
        }
    }
    send_cmd(ser, json.dumps(osc_cfg))
    ser.readline()

    # 4. Configurar trigger (flanco subida, nivel ~1.65V)
    trig_cfg = {
        "trigger": {
            "1": [{
                "command": "setParameters",
                "source": {
                    "instrument": "osc",
                    "channel": 1,
                    "type": "risingEdge",
                    "lowerThreshold": 470,
                    "upperThreshold": 500
                },
                "targets": {"osc": [1]}
            }]
        }
    }
    send_cmd(ser, json.dumps(trig_cfg))
    ser.readline()

    # 5. Configurar generador (onda cuadrada, 1 kHz, 3 Vpp, offset 0)
    awg_cfg = {
        "awg": {
            "1": [{
                "command": "setRegularWaveform",
                "signalFreq": 1000,
                "signalType": "square",
                "vOffset": 0,
                "vpp": 3000
            }]
        }
    }
    send_cmd(ser, json.dumps(awg_cfg))
    ser.readline()

    # 6. Encender generador
    send_cmd(ser, '{"awg":{"1":[{"command":"run"}]}}')
    ser.readline()

    # 7. Armar trigger (modo single)
    send_cmd(ser, '{"trigger":{"1":[{"command":"single"}]}}')
    ser.readline()

    # 8. Bucle de lectura con reintentos (similar al script funcional)
    max_intentos = max(20, int(timeout_total / 0.2))  # al menos 20 intentos
    muestras_mV = []
    data_received = False

    for intento in range(max_intentos):
        time.sleep(0.1)
        ser.write(b'{"osc":{"1":[{"command":"read"}]}}\r\n')
        time.sleep(0.2)
        resp = ser.readline()

        if b'binaryLength' in resp:
            if debug:
                print(f"DEBUG: Trigger disparado en intento {intento+1}", file=sys.stderr)
            time.sleep(0.05)
            binary_data = ser.read(ser.in_waiting)
            for i in range(0, len(binary_data), 2):
                if i+1 < len(binary_data):
                    raw = struct.unpack('<H', binary_data[i:i+2])[0]
                    voltaje_mV = (raw / 1023.0) * 3300
                    muestras_mV.append(voltaje_mV)
            data_received = True
            break
        elif b'2684354571' in resp:
            # Trigger armado, esperando señal
            if debug:
                print(f"DEBUG: Intento {intento+1}: Trigger armado, esperando...", file=sys.stderr)
        else:
            if debug:
                print(f"DEBUG: Intento {intento+1}: Respuesta inesperada: {resp}", file=sys.stderr)

    # 9. Apagar generador y detener trigger (limpieza)
    send_cmd(ser, '{"awg":{"1":[{"command":"stop"}]}}')
    ser.readline()
    send_cmd(ser, '{"trigger":{"1":[{"command":"stop"}]}}')
    ser.readline()
    ser.close()

    if not data_received or len(muestras_mV) == 0:
        raise Exception("No se capturaron muestras (trigger no disparado o sin datos)")

    return muestras_mV

def main():
    parser = argparse.ArgumentParser(description='Captura de OpenScope MZ a JSON')
    parser.add_argument('--port', default='/dev/ttyUSB0', help='Puerto serial')
    parser.add_argument('--baudrate', type=int, default=1250000, help='Baudrate')
    parser.add_argument('--freq', type=int, default=1000000, help='Frecuencia de muestreo (Hz)')
    parser.add_argument('--buffer', type=int, default=500, help='Tamaño del buffer (muestras)')
    parser.add_argument('--timeout', type=float, default=5.0, help='Timeout total en segundos')
    parser.add_argument('--debug', action='store_true', help='Muestra mensajes de depuración en stderr')
    args = parser.parse_args()

    try:
        datos = capture_waveform(args.port, args.baudrate, args.freq, args.buffer, args.timeout, args.debug)
        # Salida JSON en stdout
        print(json.dumps({"voltajes_mV": datos}))
    except Exception as e:
        print(json.dumps({"error": str(e)}))
        sys.exit(1)

if __name__ == "__main__":
    main()
