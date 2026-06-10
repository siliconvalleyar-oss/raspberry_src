#!/usr/bin/env python3
"""
OpenScope MZ - Generador 1 kHz + Osciloscopio
Comandos validados por el usuario
"""

import serial
import time
import struct
import json

PORT = '/dev/ttyUSB0'
BAUDRATE = 1250000

def send_cmd(ser, cmd_str, wait=0.3):
    ser.write(cmd_str.encode())
    time.sleep(wait)
    return ser.readline()

def main():
    print("🎵 OpenScope MZ - Generador 1 kHz + Osciloscopio")
    print("=" * 60)
    
    ser = serial.Serial(PORT, BAUDRATE, timeout=2)
    print("✅ Conectado")
    
    # 1. Modo JSON
    ser.reset_input_buffer()
    ser.write(b'{"mode":"JSON"}\r\n')
    time.sleep(0.5)
    print(f"Modo JSON: {ser.readline()}")
    
    # 2. Reset de instrumentos
    print("\n1. Reseteando instrumentos...")
    send_cmd(ser, '{"device":[{"command":"resetInstruments"}]}')
    print(f"Reset: {ser.readline()}")
    
    # 3. Configurar osciloscopio (canal 1)
    print("\n2. Configurando osciloscopio...")
    osc_cfg = '{"osc":{"1":[{"command":"setParameters","vOffset":0,"gain":0.25,"sampleFreq":1000000,"bufferSize":500,"triggerDelay":0}]}}'
    send_cmd(ser, osc_cfg)
    print(f"OSC config: {ser.readline()}")
    
    # 4. Configurar trigger para el canal 1 (flanco subida, nivel ~1.65V)
    print("\n3. Configurando trigger...")
    trig_cfg = '{"trigger":{"1":[{"command":"setParameters","source":{"instrument":"osc","channel":1,"type":"risingEdge","lowerThreshold":470,"upperThreshold":500},"targets":{"osc":[1]}}]}}'
    send_cmd(ser, trig_cfg)
    print(f"Trigger config: {ser.readline()}")
    
    # 5. Configurar generador (onda cuadrada, 1 kHz, 3Vpp, offset 0)
    print("\n4. Configurando generador...")
    awg_cfg = '{"awg":{"1":[{"command":"setRegularWaveform","signalFreq":1000,"signalType":"sine","vOffset":0,"vpp":3000}]}}'
    send_cmd(ser, awg_cfg)
    print(f"AWG config: {ser.readline()}")
    
    # 6. Encender generador
    print("\n5. Encendiendo generador...")
    send_cmd(ser, '{"awg":{"1":[{"command":"run"}]}}')
    print(f"AWG run: {ser.readline()}")
    
    # 7. Armar trigger (modo single)
    print("\n6. Armando trigger...")
    send_cmd(ser, '{"trigger":{"1":[{"command":"single"}]}}')
    print(f"Trigger single: {ser.readline()}")
    
    # 8. Leer osciloscopio (puede requerir varios intentos hasta que el trigger se dispare)
    print("\n7. Esperando datos del osciloscopio...")
    data_received = False
    for intento in range(20):
        time.sleep(0.1)
        ser.write(b'{"osc":{"1":[{"command":"read"}]}}')
        time.sleep(0.2)
        resp = ser.readline()
        
        if b'binaryLength' in resp:
            # Hay datos binarios
            print(f"\n✅ ¡Trigger disparado! Intento {intento+1}")
            # Leer el resto de los datos binarios
            time.sleep(0.05)
            binary = ser.read(ser.in_waiting)
            print(f"📦 Datos binarios: {len(binary)} bytes")
            
            # Decodificar
            muestras = []
            for i in range(0, len(binary), 2):
                if i+1 < len(binary):
                    valor = struct.unpack('<H', binary[i:i+2])[0]
                    voltaje = (valor / 1023.0) * 3.3
                    muestras.append(voltaje)
            
            print(f"\n📊 Estadísticas:")
            print(f"   Muestras: {len(muestras)}")
            if muestras:
                print(f"   Máximo: {max(muestras):.3f} V")
                print(f"   Mínimo: {min(muestras):.3f} V")
                print(f"   Promedio: {sum(muestras)/len(muestras):.3f} V")
                
                # Guardar CSV
                with open('generador_osc.csv', 'w') as f:
                    f.write("muestra,voltaje_v\n")
                    for i, v in enumerate(muestras):
                        f.write(f"{i},{v:.6f}\n")
                print(f"\n💾 Datos guardados en 'generador_osc.csv'")
            data_received = True
            break
        elif b'2684354571' in resp:
            print(f"   Intento {intento+1}: Trigger armado, esperando señal...")
        else:
            print(f"   Intento {intento+1}: {resp.strip()}")
    
    if not data_received:
        print("\n⚠️ No se capturaron datos. Verifique:")
        print("   - Conexión: Pin 21 (AWG) → Pin 29 (CH1+)")
        print("   - Conexión: Pin 25 (GND) → Pin 30 (CH1-)")
        print("   - El generador está encendido (LED parpadea)")
    
    # 9. Apagar generador y detener trigger
    print("\n8. Apagando generador...")
    send_cmd(ser, '{"awg":{"1":[{"command":"stop"}]}}')
    print(ser.readline())
    
    print("\n9. Deteniendo trigger...")
    send_cmd(ser, '{"trigger":{"1":[{"command":"stop"}]}}')
    print(ser.readline())
    
    ser.close()
    print("\n👋 Desconectado")

if __name__ == "__main__":
    main()
