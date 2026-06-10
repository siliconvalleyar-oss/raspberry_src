#!/bin/bash

# Configuración del puerto serie y otros parámetros
SERIAL_PORT="/dev/serial0"
BAUD_RATE=9600  # La velocidad de baudios puede variar según la configuración del SIM800L

# Función para enviar comandos AT
send_at_command() {
    echo -e "AT$1\r" > $SERIAL_PORT
    sleep 1
}

# Iniciar comunicación con el módulo SIM800L
stty -F $SERIAL_PORT $BAUD_RATE raw -echo

# Configurar el módulo SIM800L en modo texto para SMS
send_at_command "+CMGF=1"

# Configurar el número de teléfono del destinatario
PHONE_NUMBER="+541131123385"  # Reemplaza con el número de teléfono al que deseas enviar el SMS
send_at_command "+CMGS=\"$PHONE_NUMBER\""

# Mensaje a enviar (reemplaza con tu mensaje)
SMS_MESSAGE="tu codigo es hora "

# Enviar el mensaje SMS
echo -e "$SMS_MESSAGE\x1A" > $SERIAL_PORT

# Leer la respuesta del módulo SIM800L
#RESPONSE=$(cat < $SERIAL_PORT)
#echo "Respuesta del SIM800L: $RESPONSE"

# Cerrar la conexión serial
stty -F $SERIAL_PORT 9600  # Restaurar la configuración de baudios por defecto
