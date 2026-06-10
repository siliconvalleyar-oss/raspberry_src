#!/usr/bin/env python3
import serial, time, struct, json, sys

PORT = '/dev/ttyUSB0'
BAUDRATE = 1250000

class OScope:
    def __init__(self):
        self.ser = serial.Serial(PORT, BAUDRATE, timeout=2)
        time.sleep(1)
        self.ser.write(b'{"mode":"JSON"}\r\n')
        time.sleep(0.5)
        if b'statusCode' not in self.ser.readline():
            raise Exception("Modo JSON falló")
        self.ser.write(b'{"device":[{"command":"resetInstruments"}]}\r\n')
        time.sleep(0.3); self.ser.readline()
        self._config_osc(1000000, 500)
        self._config_trigger()
        self._start_generator()

    def _config_osc(self, freq, buf):
        cmd = {"osc":{"1":[{"command":"setParameters","vOffset":0,"gain":0.25,
               "sampleFreq":freq,"bufferSize":buf,"triggerDelay":0}]}}
        self.ser.write(json.dumps(cmd).encode()+b'\r\n')
        time.sleep(0.3); self.ser.readline()

    def _config_trigger(self):
        cmd = {"trigger":{"1":[{"command":"setParameters","source":{"instrument":"osc","channel":1,
               "type":"risingEdge","lowerThreshold":470,"upperThreshold":500},
               "targets":{"osc":[1]}}]}}
        self.ser.write(json.dumps(cmd).encode()+b'\r\n')
        time.sleep(0.3); self.ser.readline()

    def _start_generator(self):
        cmd = {"awg":{"1":[{"command":"setRegularWaveform","signalFreq":1000,
               "signalType":"square","vOffset":0,"vpp":3000}]}}
        self.ser.write(json.dumps(cmd).encode()+b'\r\n')
        time.sleep(0.3); self.ser.readline()
        self.ser.write(b'{"awg":{"1":[{"command":"run"}]}}\r\n')
        time.sleep(0.3); self.ser.readline()

    def capture(self, freq=None, buf=None):
        if freq and buf:
            self._config_osc(freq, buf)
        self.ser.write(b'{"trigger":{"1":[{"command":"single"}]}}\r\n')
        time.sleep(0.3); self.ser.readline()
        start = time.time()
        while time.time()-start < 1.0:
            self.ser.write(b'{"osc":{"1":[{"command":"read"}]}}\r\n')
            time.sleep(0.1)
            resp = self.ser.readline()
            if b'binaryLength' in resp:
                time.sleep(0.05)
                data = self.ser.read(self.ser.in_waiting)
                muestras = []
                for i in range(0, len(data), 2):
                    if i+1 < len(data):
                        raw = struct.unpack('<H', data[i:i+2])[0]
                        muestras.append((raw/1023.0)*3300)
                return muestras
        return []

if __name__ == "__main__":
    osc = OScope()
    for line in sys.stdin:
        if line.strip().lower() == "capture":
            datos = osc.capture()
            print(json.dumps({"voltajes_mV": datos}))
            sys.stdout.flush()
        elif line.strip().lower() == "exit":
            break
