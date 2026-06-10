#!/bin/bash
echo "Probando LED del transmisor (GPIO12) - 5 parpadeos"
# Compilar programa de prueba simple
cat > test_tx_led.c << EOF
#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#define LED_GPIO 12
int main() {
    if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_OUTP);
    for (int i=0;i<5;i++) {
        bcm2835_gpio_write(LED_GPIO, HIGH);
        bcm2835_delay(100);
        bcm2835_gpio_write(LED_GPIO, LOW);
        bcm2835_delay(100);
    }
    bcm2835_close();
    return 0;
}
EOF
gcc -o test_tx_led test_tx_led.c -lbcm2835
sudo ./test_tx_led
rm -f test_tx_led test_tx_led.c
echo "Prueba completada. Si el LED no parpadeó, revisa conexión."
