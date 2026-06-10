#!/usr/bin/env python3
"""
OpenScope MZ - Generador 100 kHz (COMANDO CORRECTO)
Basado en el tráfico capturado de WaveForms Live
"""

import serial
import time

PORT = '/dev/ttyUSB0'
BAUDRATE = 1250000

def main():
    print("🎵 OpenScope MZ - Generador 1 MHz (onda cuadrada)")
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
    
    # 3. CONFIGURAR GENERADOR - COMANDO CORRECTO
    print("\n2. Configurando generador (setRegularWaveform)...")
    cmd = b'{"awg":{"1":[{"command":"setRegularWaveform","signalFreq":1000000,"signalType":"square","vOffset":0,"vpp":3000}]}}'
    ser.write(cmd)
    time.sleep(0.3)
    print(f"Config: {ser.readline()}")
    
    # 4. ENCENDER GENERADOR - COMANDO "run"
    print("\n3. Encendiendo generador (run)...")
    ser.write(b'{"awg":{"1":[{"command":"run"}]}}')
    time.sleep(0.3)
    print(f"Run: {ser.readline()}")
    
    print("\n" + "=" * 50)
    print("✅ GENERADOR ACTIVADO")
    print(f"   Frecuencia: 1 MHz (1,000,000 Hz)")
    print(f"   Tipo: Onda cuadrada")
    print(f"   Voltaje: 0V a 3.0V (3000 mVpp, offset 0)")
    print(f"   Pin: 21 (AWG1/W1)")
    print("=" * 50)
    
    input("\n🔴 Presiona Enter para apagar...")
    
    # 5. APAGAR GENERADOR
    print("\n4. Apagando generador...")
    ser.write(b'{"awg":{"1":[{"command":"stop"}]}}')
    time.sleep(0.3)
    print(f"Stop: {ser.readline()}")
    
    ser.close()
    print("👋 Desconectado")

if __name__ == "__main__":
    main()
