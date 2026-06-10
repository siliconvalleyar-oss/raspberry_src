



ARCH=$(uname -m)

if [[ $ARCH == "armv7l" || $ARCH == "arm" ]]; then
    PINS=$(seq 9 11)
else
    PINS=$(seq 521 523)
fi

# Setear los pines como salidas y en alto
for pin in $PINS; do
    sudo raspi-gpio set $pin a0
    sudo raspi-gpio get $pin 
    sleep 0.2
done

# Configurar GPIO 9 como SPI0_MISO (función alternativa 0)
   


