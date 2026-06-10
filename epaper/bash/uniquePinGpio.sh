#!/bin/bash

PIN=$1

# Verificar si PIN está vacío
if [[ -z $PIN ]]; then
    echo "Error: El PIN no puede estar vacío."
    exit 1
fi


ARCH=$(uname -m)
if [[ $ARCH == "armv7l" || $ARCH == "arm" ]]; then
	GPIO=$PIN
else
	GPIO=$((512+$PIN))
fi


echo "in" | sudo tee /sys/class/gpio/gpio$GPIO/direction
echo "out" | sudo tee /sys/class/gpio/gpio$GPIO/direction
#cat /sys/class/gpio/gpio$GPIO/value

RESULT=$(echo 1 | sudo tee /sys/class/gpio/gpio$GPIO/value)
echo "gpio : $GPIO result : $RESULT"

#cat /sys/class/gpio/gpio$GPIO/value

RESULT=$(echo 0 | sudo tee /sys/class/gpio/gpio$GPIO/value)

echo "gpio : $GPIO result : $RESULT"

#cat /sys/class/gpio/gpio$GPIO/value



echo "..............."



# Configurar el GPIO como salida
echo "out" | sudo tee /sys/class/gpio/gpio$GPIO/direction

# Configurar el GPIO como entrada
echo "in" | sudo tee /sys/class/gpio/gpio$GPIO/direction

# Leer el valor actual del GPIO
#cat /sys/class/gpio/gpio$GPIO/value

# Establecer el valor a 1 y verificar
RESULT=$(echo "rising" | sudo tee /sys/class/gpio/gpio$GPIO/edge)
echo "1 RESULT: $RESULT"
#cat /sys/class/gpio/gpio$GPIO/value

# Establecer el valor a 0 y verificar
RESULT=$(echo "falling" | sudo tee /sys/class/gpio/gpio$GPIO/edge)
echo "0 RESULT: $RESULT"
#cat /sys/class/gpio/gpio$GPIO/value
