#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>

#define LED_GPIO 12

int main() {
    if (!bcm2835_init()) {
        printf("Error bcm2835_init\n");
        return 1;
    }
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_OUTP);
    printf("LED en GPIO12 parpadea 5 veces\n");
    for (int i = 0; i < 5; i++) {
        bcm2835_gpio_write(LED_GPIO, HIGH);
        usleep(200000);
        bcm2835_gpio_write(LED_GPIO, LOW);
        usleep(200000);
    }
    bcm2835_close();
    return 0;
}
