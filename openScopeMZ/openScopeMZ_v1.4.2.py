#!/usr/bin/env python3
"""
OpenScope MZ - Osciloscopio (SECUENCIA REAL DE WAVEFORMS LIVE)
"""

import serial
import time
import base64
import struct
import json

PORT = '/dev/ttyUSB0'
BAUDRATE = 1250000

def send_cmd(ser, cmd_str):
    """Envía un comando y devuelve la respuesta"""
    ser.write(cmd_str.encode())
    time.sleep(0.2)
    return ser.readline().decode().strip()

def main():
    print("🎵 OpenScope MZ - Osciloscopio (SECUENCIA REAL)")
    print("=" * 50)
    
    ser = serial.Serial(PORT, BAUDRATE, timeout=2)
    print("✅ Conectado")
    
    # 1. Entrar en modo JSON
    ser.reset_input_buffer()
    ser.write(b'{"mode":"JSON"}\r\n')
    time.sleep(0.5)
    print(f"Modo JSON: {ser.readline()}")
    
    # 2. Configurar osciloscopio (parámetros básicos)
    print("\n1. Configurando osciloscopio...")
    osc_cmd = '{"osc":{"1":[{"command":"setParameters","vOffset":0,"gain":0.25,"sampleFreq":1000000,"bufferSize":500,"triggerDelay":0}]}}'
    send_cmd(ser, osc_cmd)
    print(f"OSC Config: {ser.readline()}")
    
    # 3. Configurar trigger
    print("\n2. Configurando trigger...")
    trig_cmd = '{"trigger":{"1":[{"command":"setParameters","source":{"channel":1,"fallingEdge":1023,"instrument":"la","risingEdge":1023},"targets":{"osc":[1]}}]}}'
    send_cmd(ser, trig_cmd)
    print(f"Trigger Config: {ser.readline()}")
    
    # 4. SECUENCIA REAL: Armar trigger con "single"
    print("\n3. Armando trigger (single)...")
    resp = send_cmd(ser, '{"trigger":{"1":[{"command":"single"}]}}')
    print(f"Trigger single: {resp}")
    
    # 5. Leer el osciloscopio (esto debería incrementar acqCount)
    print("\n4. Leyendo osciloscopio...")
    resp = send_cmd(ser, '{"osc":{"1":[{"command":"read"}]}}')
    print(f"Read OSC: {resp}")
    
    # 6. Si acqCount incrementó, hay datos disponibles
    if '"acqCount"' in resp:
        print("\n✅ ¡Adquisición completada! Datos disponibles")
        
        # Necesitamos obtener los datos reales
        # En WaveForms Live, los datos vienen por WebSocket o en otra petición
        # Probemos a leer nuevamente con un formato diferente
        print("\n5. Intentando obtener datos...")
        
        # Opción: leer con parámetro adicional
        ser.write(b'{"osc":{"1":[{"command":"read","format":"base64"}]}}')
        time.sleep(0.3)
        resp = ser.readline()
        print(f"Read con formato: {resp}")
        
        if b'data' in resp:
            try:
                data = json.loads(resp)
                b64_data = data['osc']['1'][0]['data']
                raw = base64.b64decode(b64_data)
                
                muestras = []
                for i in range(0, len(raw), 2):
                    if i+1 < len(raw):
                        valor = struct.unpack('<H', raw[i:i+2])[0]
                        voltaje = (valor / 1023.0) * 3.3
                        muestras.append(voltaje)
                
                print(f"\n📊 Estadísticas:")
                print(f"   Muestras: {len(muestras)}")
                print(f"   Máximo: {max(muestras):.3f} V")
                print(f"   Mínimo: {min(muestras):.3f} V")
                print(f"   Promedio: {sum(muestras)/len(muestras):.3f} V")
                
                with open('captura_osc.csv', 'w') as f:
                    f.write("muestra,voltaje_v\n")
                    for i, v in enumerate(muestras):
                        f.write(f"{i},{v:.6f}\n")
                print(f"\n💾 Datos guardados en 'captura_osc.csv'")
                
            except Exception as e:
                print(f"Error decodificando: {e}")
        else:
            print(f"Respuesta sin datos: {resp}")
    
    # 7. Detener trigger
    print("\n6. Deteniendo trigger...")
    send_cmd(ser, '{"trigger":{"1":[{"command":"stop"}]}}')
    
    ser.close()
    print("\n👋 Desconectado")

if __name__ == "__main__":
    main()

