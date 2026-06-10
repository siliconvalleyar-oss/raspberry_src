#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>

#define LED_GPIO RPI_GPIO_P1_32  // GPIO 12
#define PWM_CHANNEL 0
#define PWM_RANGE 1024

int main() {
    if (!bcm2835_init()) {
        printf("Error al inicializar bcm2835\n");
        return 1;
    }
    
    // Configurar GPIO12 como salida PWM
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_ALT5);
    
    // Configurar PWM
    bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_16);
    bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);
    bcm2835_pwm_set_range(PWM_CHANNEL, PWM_RANGE);
    
    printf("Probando LED: 5 parpadeos\n");
    for (int i = 0; i < 5; i++) {
        bcm2835_pwm_set_data(PWM_CHANNEL, PWM_RANGE);
        bcm2835_delay(100);
        bcm2835_pwm_set_data(PWM_CHANNEL, 0);
        bcm2835_delay(100);
    }
    
    // Limpiar
    bcm2835_pwm_set_mode(PWM_CHANNEL, 0, 0);
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_INPT);
    bcm2835_close();
    
    printf("Prueba completada.\n");
    return 0;
}
