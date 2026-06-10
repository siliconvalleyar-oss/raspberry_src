# PN532 — NFC Payment

Lector NFC para pagos sin contacto. Lee tarjetas ISO14443A usando libnfc y envía el UID a un servidor HTTP para procesar el pago.

## Hardware

- Raspberry Pi (cualquier modelo con SPI)
- Módulo NFC PN532 (conectado por SPI o I2C)
- Servidor de pagos (http://127.0.0.1:5000/pay)

### Conexión PN532 (SPI)

| PN532 | RPi GPIO |
|-------|----------|
| VCC   | 3.3V     |
| GND   | GND      |
| MOSI  | GPIO10   |
| MISO  | GPIO9    |
| SCK   | GPIO11   |
| CS    | GPIO8    |
| IRQ   | (opcional)|

## Dependencias

```bash
sudo apt install -y build-essential libnfc-dev libcurl4-openssl-dev
```

## Compilación

```bash
make
```

## Ejecución

```bash
# Iniciar el servidor de pagos primero, luego:
make run
# o
sudo ./bin/nfc_payment
```

## Funcionamiento

1. El programa inicializa el dispositivo NFC
2. Espera una tarjeta ISO14443A
3. Lee el UID de la tarjeta
4. Envía el UID como JSON al servidor: `POST {"uid":"...", "amount":10}`
5. Muestra el resultado del pago (aprobado/rechazado)

## Dependencias de librerías

- **libnfc**: lectura NFC de tarjetas Mifare/ISO14443A
- **libcurl**: envío HTTP POST al servidor de pagos
- **nlohmann/json**: parseo de respuesta JSON (incluida en `include/`)

## Estructura

```
pn532/
├── include/
│   ├── json.h       # nlohmann/json header
│   └── json.hpp
├── src/
│   └── payment.cpp  # Código principal
├── Makefile
└── README.md
```
