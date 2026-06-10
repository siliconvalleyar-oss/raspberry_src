#include <iostream>
#include <iomanip>
#include <unistd.h>  // Para la función sleep
#include "Adafruit_ADS1015.h"

// Crear una instancia del ADS1115 (16 bits).
Adafruit_ADS1115 adc(0x48);  // Establece la dirección I2C a 0x49

// Elegir una ganancia de 2/3 para leer tensiones en un rango de ±6.144V.
//const adsGain_t GAIN = GAIN_TWOTHIRDS;
const adsGain_t GAIN =GAIN_EIGHT;
// Definir los rangos de tensión para cada configuración de ganancia
const float GAIN_VOLTAGE_RANGES[] = {
    6.144,  // 2/3
    4.096,  // 1
    2.048,  // 2
    1.024,  // 4
    0.512,  // 8
    0.256   // 16
};

// Definir los valores de las resistencias para el divisor de tensión
const float R1 = 10e3;  // 10 kΩ
const float R2 = 1e3;   // 1 kΩ

float adc_to_voltage(int16_t adc_value, adsGain_t gain) {
    // Obtener el rango de tensión para la ganancia especificada
    float voltage_range = 6.144;  // Valor predeterminado

    switch (gain) {
        case GAIN_TWOTHIRDS:
            voltage_range = GAIN_VOLTAGE_RANGES[0];
            break;
        case GAIN_ONE:
            voltage_range = GAIN_VOLTAGE_RANGES[1];
            break;
        case GAIN_TWO:
            voltage_range = GAIN_VOLTAGE_RANGES[2];
            break;
        case GAIN_FOUR:
            voltage_range = GAIN_VOLTAGE_RANGES[3];
            break;
        case GAIN_EIGHT:
            voltage_range = GAIN_VOLTAGE_RANGES[4];
            break;
        case GAIN_SIXTEEN:
            voltage_range = GAIN_VOLTAGE_RANGES[5];
            break;
        default:
            std::cerr << "Valor de ganancia inválido" << std::endl;
            return 0.0;
    }

    // Convertir el valor ADC a voltaje
    return adc_value * (voltage_range / 32767.0);  // 32767 para un ADC de 16 bits firmado
}

float voltage_to_input_voltage(float voltage, float R1, float R2) {
    // Calcular la tensión de entrada utilizando la fórmula del divisor de tensión
    return voltage * (R1 + R2) / R2;
}

int main() {
    // Inicializar el ADC
    adc.begin();
    adc.setGain(GAIN);  // Asegúrate de configurar la ganancia aquí

    std::cout << "Leyendo valores de ADS1x15, presiona Ctrl-C para salir..." << std::endl;
    // Imprimir encabezados de columna
    std::cout << "| " << std::setw(6) << "0" << " | "
              << std::setw(6) << "1" << " | "
              << std::setw(6) << "2" << " | "
              << std::setw(6) << "3" << " |" << std::endl;
    std::cout << std::string(37, '-') << std::endl;

    while (true) {
        // Leer todos los valores de los canales ADC en una lista
        int16_t values[4];
        for (int i = 0; i < 4; ++i) {
            // Leer el canal ADC especificado utilizando el valor de ganancia previamente configurado
            values[i] = adc.readADC_SingleEnded(i);
        }

        // Convertir los valores ADC a voltajes y calcular las tensiones de entrada
        float voltages[4];
        float input_voltages[4];
        for (int i = 0; i < 4; ++i) {
            voltages[i] = adc_to_voltage(values[i], GAIN);
            input_voltages[i] = voltage_to_input_voltage(voltages[i], R1, R2);
        }

        // Imprimir los valores ADC y las tensiones de entrada calculadas
        std::cout << "| " << std::setw(6) << values[0] << " | "
                  << std::setw(6) << values[1] << " | "
                  << std::setw(6) << values[2] << " | "
                  << std::setw(6) << values[3] << " |" << std::endl;
        std::cout << "| " << std::setw(8) << std::fixed << std::setprecision(5) << input_voltages[0] << " | "
                  << std::setw(8) << std::fixed << std::setprecision(5) << input_voltages[1] << " | "
                  << std::setw(8) << std::fixed << std::setprecision(5) << input_voltages[2] << " | "
                  << std::setw(8) << std::fixed << std::setprecision(5) << input_voltages[3] << " |" << std::endl;

        // Pausa de medio segundo
        usleep(500000);  // Dormir por 500 milisegundos
    }

    return 0;
}
