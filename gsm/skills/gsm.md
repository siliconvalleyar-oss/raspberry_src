# SIM800 GSM Module for Raspberry Pi

Control a SIM800/SIM800L GSM module from a Raspberry Pi over serial UART.

## Hardware

- Raspberry Pi (any model with UART)
- SIM800 / SIM800L GSM module
- External 5V/2A power supply for the module
- Level shifter (3.3V ↔ 5V) recommended
- UART connection: GPIO14 TX → SIM RX, GPIO15 RX ← SIM TX

## Build

```bash
cd /home/joy/src/git/gsm
make -j4
```

## Programs

- `sms_send` — Send an SMS via AT commands
- `call` — Place a voice call
- `call_dtmf` — Call with DTMF tone detection (press 5 for time, 3 to hang up)

## Run

```bash
sudo ./bin/sms_send
sudo ./bin/call
sudo ./bin/call_dtmf
```

## Bash Scripts

- `scripts/sim800_init.sh` — Initialize module, check SIM, set SMS mode
- `scripts/sim800_call.sh` — Quick voice call
- `scripts/sim800_send_msj.sh` — Quick SMS

## Port

`/dev/serial0` at 9600 baud, 8N1, no flow control.

## Install Dependencies

```bash
sudo apt-get install minicom picocom
```
Puedes usar `picocom /dev/serial0 -b 9600` para probar comandos AT manualmente.
