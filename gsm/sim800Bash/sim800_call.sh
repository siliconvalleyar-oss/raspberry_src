#!/bin/bash

# Puerto serie (ajusta esto según tu configuración)
SERIAL_PORT="/dev/serial0"
BAUD_RATE="9600"

# Número de teléfono al que quieres llamar (ajusta esto según tu caso)
PHONE_NUMBER="+541131123385"

# Función para enviar un comando AT al SIM800L
send_at_command() {
    echo -e "$1" > $SERIAL_PORT
    sleep 1
    cat $SERIAL_PORT
}

# Configurar el puerto serie
stty -F $SERIAL_PORT $BAUD_RATE

# Hacer la llamada
send_at_command "ATD$PHONE_NUMBER;"
