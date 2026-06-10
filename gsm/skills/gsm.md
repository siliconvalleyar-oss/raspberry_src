---
name: gsm
description: Control de módulo GSM SIM800/SIM800L con Raspberry Pi (Bash + C++)
---

# gsm — Skill de desarrollo

## Lenguajes y herramientas
- Bash scripting
- C++ (GCC)
- Serial port (UART)
- AT commands

## Estructura
```
gsm/
├── sim800Bash/
│   ├── sim800_init.sh      Inicialización del módulo
│   ├── sim800_call.sh      Realizar llamada
│   └── sim800_send_msj.sh  Enviar SMS
├── sim800Cpp/
│   ├── main.cc             Llamada + DTMF + hora
│   ├── single_call_sim800/ Envío de SMS
│   ├── dtmf_tone_01/       DTMF con respuesta horaria
│   ├── dtmf_02/            DTMF con timeout
│   ├── dtmf_singgle/       DTMF básico
│   ├── detect_tone_dtmf/   DTMF línea por línea
│   ├── dtmf_tone_file_03/  DTMF con depuración
│   └── olds_sim800/        Versiones antiguas
├── .gitignore
├── README.md
└── skills/gsm.md
```

## Compilación
```bash
g++ -o sim800_main sim800Cpp/main.cc
```

## Notas
- Usar `-DDEBUG` para ver trazas de los comandos AT
- El puerto serie es `/dev/serial0` a 9600 baudios
- Requiere UART habilitada en `config.txt` (`enable_uart=1`)
- Los scripts Bash y programas C++ necesitan `sudo` para acceder al puerto
- Números de teléfono hardcodeados — reemplazar antes de usar
