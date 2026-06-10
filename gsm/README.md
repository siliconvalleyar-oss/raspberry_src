# GSM SIM800 — Control de módulo GSM para Raspberry Pi

Control y comunicación con el **módulo GSM SIM800/SIM800L** desde una
Raspberry Pi mediante comandos AT por puerto serie.

## Descripción

El proyecto incluye dos enfoques:

### Bash (`sim800Bash/`)
Scripts shell para operaciones básicas:

- **sim800_init.sh** — Inicialización del módulo (AT+CFUN, AT+CPIN, AT+CMGF)
- **sim800_call.sh** — Realizar una llamada de voz
- **sim800_send_msj.sh** — Enviar un SMS en modo texto

### C++ (`sim800Cpp/`)
Programas de demostración para funcionalidades avanzadas:

- **main.cc** — Llamada con detección de tonos DTMF y respuesta horaria
- **single_call_sim800/sim800.cc** — Envío de SMS
- **dtmf_tone_01/** — Llamada + DTMF con respuesta horaria
- **dtmf_02/** — Llamada + DTMF con timeout
- **dtmf_singgle/** — Detección DTMF básica
- **detect_tone_dtmf/** — Detección DTMF línea por línea
- **dtmf_tone_file_03/** — Llamada + DTMF con depuración
- **olds_sim800/** — Versiones anteriores

## Cableado

```
Raspberry Pi              SIM800L
============              =======
GPIO 14 (TXD)  ────────  RXD
GPIO 15 (RXD)  ────────  TXD
GND            ────────  GND
```

Puerto serie: `/dev/serial0` (9600 baudios, 8N1)

## Dependencias

```bash
sudo apt-get update
sudo apt-get install -y build-essential
```

UART debe estar habilitada en la RPi:
```bash
# En config.txt añadir:
# enable_uart=1
# dtoverlay=disable-bt
```

## Compilación

```bash
# main.cc (llamada + DTMF)
g++ -o sim800_main sim800Cpp/main.cc

# Envío de SMS
g++ -o sim800_sms sim800Cpp/single_call_sim800/sim800.cc

# DTMF tone
g++ -o sim800_dtmf sim800Cpp/dtmf_tone_01/sim800_dtmf_tone_date.cc
```

## Ejecución

```bash
# Scripts Bash
sudo ./sim800Bash/sim800_init.sh
sudo ./sim800Bash/sim800_call.sh
sudo ./sim800Bash/sim800_send_msj.sh

# Programas C++
sudo ./sim800_main
```

## Controles

- Llamada: el programa marca el número configurado
- DTMF: tono `5` → muestra la hora; tono `3` → finaliza la llamada
