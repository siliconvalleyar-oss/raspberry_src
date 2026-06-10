#!/usr/bin/env python3
# capture_oscope.py - Captura una forma de onda del OpenScope MZ y la imprime como JSON

import serial
import time
import struct
import json
import sys
import argparse

def capture_waveform(port, baudrate, sample_freq, buffer_size, timeout_wait=2.0):
    ser = serial.Serial(port, baudrate, timeout=2)
    ser.reset_input_buffer()

    # Modo JSON
    ser.write(b'{"mode":"JSON"}\r\n')
    time.sleep(0.5)
    resp = ser.readline()
    if b'statusCode' not in resp:
        raise Exception("No se pudo activar modo JSON")

    # Reset instrumentos
    ser.write(b'{"device":[{"command":"resetInstruments"}]}')
    time.sleep(0.3)
    ser.readline()

    # Configurar osciloscopio canal 1
    cmd_osc = {
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
    ser.write(json.dumps(cmd_osc).encode())
    time.sleep(0.3)
    ser.readline()

    # Configurar trigger (flanco subida, ~1.65V)
    trig_cmd = {
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
    ser.write(json.dumps(trig_cmd).encode())
    time.sleep(0.3)
    ser.readline()

    # Armar trigger single
    ser.write(b'{"trigger":{"1":[{"command":"single"}]}}')
    time.sleep(0.3)
    ser.readline()

    # Esperar datos
    start = time.time()
    data_received = False
    muestras_mV = []

    while time.time() - start < timeout_wait:
        ser.write(b'{"osc":{"1":[{"command":"read"}]}}')
        time.sleep(0.1)
        resp = ser.readline()

        if b'binaryLength' in resp:
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
            # Trigger armado, esperando
            pass
        else:
            # Otra respuesta, ignorar
            pass

    ser.close()
    if not data_received or len(muestras_mV) == 0:
        raise Exception("No se capturó ninguna señal")

    return muestras_mV

def main():
    parser = argparse.ArgumentParser(description='Captura de OpenScope MZ a JSON')
    parser.add_argument('--port', default='/dev/ttyUSB0', help='Puerto serial')
    parser.add_argument('--baudrate', type=int, default=1250000, help='Baudrate')
    parser.add_argument('--freq', type=int, default=1000000, help='Frecuencia de muestreo (Hz)')
    parser.add_argument('--buffer', type=int, default=500, help='Tamaño del buffer (muestras)')
    parser.add_argument('--timeout', type=float, default=2.0, help='Timeout en segundos')
    args = parser.parse_args()

    try:
        datos = capture_waveform(args.port, args.baudrate, args.freq, args.buffer, args.timeout)
        print(json.dumps({"voltajes_mV": datos}))
    except Exception as e:
        print(json.dumps({"error": str(e)}))
        sys.exit(1)

if __name__ == "__main__":
    main()
