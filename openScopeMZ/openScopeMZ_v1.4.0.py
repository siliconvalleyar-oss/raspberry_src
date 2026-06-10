#!/usr/bin/env python3
"""
OpenScope MZ - Osciloscopio (COMANDOS CORRECTOS)
Basado en el tráfico capturado de WaveForms Live
"""

import serial
import time
import base64
import struct
import json

PORT = '/dev/ttyUSB0'
BAUDRATE = 1250000

def send_cmd(ser, cmd, description=""):
    """Envía un comando y muestra la respuesta"""
    print(f"📤 {description}: {cmd}")
    ser.write(cmd.encode() if isinstance(cmd, str) else cmd)
    time.sleep(0.3)
    response = ser.readline()
    print(f"📥 Respuesta: {response}")
    return response

def main():
    print("🎵 OpenScope MZ - Osciloscopio")
    print("=" * 50)
    
    ser = serial.Serial(PORT, BAUDRATE, timeout=2)
    print("✅ Conectado")
    
    # 1. Entrar en modo JSON
    ser.reset_input_buffer()
    ser.write(b'{"mode":"JSON"}\r\n')
    time.sleep(0.5)
    print(f"Modo JSON: {ser.readline()}")
    
    # 2. Reiniciar instrumentos
    print("\n1. Reiniciando instrumentos...")
    ser.write(b'{"device":[{"command":"resetInstruments"}]}')
    time.sleep(0.5)
    print(f"Reset: {ser.readline()}")
    
    # 3. Configurar osciloscopio (parámetros básicos)
    print("\n2. Configurando osciloscopio...")
    osc_cmd = '{"osc":{"1":[{"command":"setParameters","vOffset":0,"gain":0.25,"sampleFreq":1000000,"bufferSize":500,"triggerDelay":0}]}}'
    send_cmd(ser, osc_cmd, "Config OSC")
    
    # 4. Configurar trigger (como en WaveForms Live)
    print("\n3. Configurando trigger...")
    trigger_cmd = '''{"trigger":{"1":[{"command":"setParameters","source":{"channel":1,"fallingEdge":1023,"instrument":"la","risingEdge":1023},"targets":{"osc":[1]}}]}}'''
    send_cmd(ser, trigger_cmd, "Config Trigger")
    
    # 5. Armar trigger en modo single
    print("\n4. Armando trigger (single)...")
    send_cmd(ser, '{"trigger":{"1":[{"command":"single"}]}}', "Armar Trigger")
    
    # 6. Esperar a que el trigger se dispare (o leer directamente)
    print("\n5. Esperando datos...")
    time.sleep(0.1)  # Pequeña pausa
    
    # 7. Leer datos del osciloscopio
    print("\n6. Leyendo osciloscopio...")
    read_cmd = '{"osc":{"1":[{"command":"read"}]}}'
    resp = send_cmd(ser, read_cmd, "Leer OSC")
    
    # 8. Decodificar datos si están presentes
    if b'data' in resp:
        try:
            # Extraer la parte JSON de la respuesta
            resp_str = resp.decode('utf-8')
            data = json.loads(resp_str)
            
            # Obtener datos en base64
            b64_data = data['osc']['1'][0]['data']
            raw = base64.b64decode(b64_data)
            
            # Convertir a voltajes
            muestras = []
            for i in range(0, len(raw), 2):
                if i + 1 < len(raw):
                    valor = struct.unpack('<H', raw[i:i+2])[0]
                    voltaje = (valor / 1023.0) * 3.3
                    muestras.append(voltaje)
            
            print(f"\n✅ Captura exitosa!")
            print(f"   Muestras: {len(muestras)}")
            if muestras:
                print(f"   Voltaje máximo: {max(muestras):.3f} V")
                print(f"   Voltaje mínimo: {min(muestras):.3f} V")
                print(f"   Voltaje promedio: {sum(muestras)/len(muestras):.3f} V")
                
                # Guardar a CSV
                with open('osciloscopio.csv', 'w') as f:
                    f.write("muestra,voltaje_v\n")
                    for i, v in enumerate(muestras):
                        f.write(f"{i},{v:.6f}\n")
                print("   Datos guardados en 'osciloscopio.csv'")
        except Exception as e:
            print(f"Error decodificando: {e}")
    else:
        print("\n⚠️ No hay datos. ¿Hay una señal conectada al Pin 29 (CH1+)?")
        print("   Conecta una señal (ej: Pin 21 AWG → Pin 29 CH1+)")
    
    ser.close()
    print("\n👋 Desconectado")

if __name__ == "__main__":
    main()
