# SIM800 GSM Module — Raspberry Pi

Control a SIM800/SIM800L GSM module over serial (`/dev/serial0`) from a Raspberry Pi.

## Programs

| Binary | Source | Description |
|--------|--------|-------------|
| `sms_send` | `src/sms_send.cpp` | Send an SMS in text mode |
| `call` | `src/call.cpp` | Initiate a voice call, wait, hang up |
| `call_dtmf` | `src/call_dtmf.cpp` | Call with DTMF detection (5=time, 3=hangup) |

## Build

```bash
cd gsm
make -j4
```

## Run

```bash
sudo make run-sms    # Send SMS
sudo make run-call   # Simple call
sudo make run-dtmf   # Call with DTMF
```

## Bash Utilities

| Script | Description |
|--------|-------------|
| `scripts/sim800_init.sh` | Reset module, check SIM, set SMS text mode |
| `scripts/sim800_call.sh` | Place a voice call |
| `scripts/sim800_send_msj.sh` | Send an SMS |

## Hardware

- **UART**: `/dev/serial0` (GPIO14 TX → SIM RX, GPIO15 RX ← SIM TX)
- **Baud**: 9600 8N1
- **Power**: SIM800 needs 2A peak — use external supply, not RPi 3.3V

## Dependencies

```bash
sudo apt-get install minicom picocom
```
