#!/bin/bash

# Configurar el puerto serie a 9600 baudios (ajusta según sea necesario)
stty -F /dev/serial0 9600

# Reiniciar el módem GSM
echo -e "AT+CFUN=1,1\r" > /dev/serial0
sleep 10

# Verificar si la SIM está lista
echo -e "AT+CPIN?\r" > /dev/serial0
sleep 1
#cat /dev/serial0

# Configurar el módem en modo texto para SMS
echo -e "AT+CMGF=1\r" > /dev/serial0
sleep 1
#cat /dev/serial0
