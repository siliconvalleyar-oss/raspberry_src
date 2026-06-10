//#include <iostream>


#include <iostream>
#include <unistd.h> // For sleep and usleep
#include <fcntl.h>  // For open
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>  // For memset

#define DS2482_ADDRESS 0x18 // Dirección I2C del DS2482
#define DS1994_FAMILY_CODE 0x04 // Código de familia del DS1994

// Comandos del DS2482
#define CMD_DRST 0xF0  // Reset del dispositivo
#define CMD_WCFG 0xD2  // Configuración del dispositivo
#define CMD_1WRS 0xB4  // Restablecer el bus 1-Wire
#define CMD_1WWS 0xA5  // Comando Write 1-Wire Slot
#define CMD_1WRB 0x96  // Leer bit 1-Wire
/*
// Función para inicializar el DS2482
bool init_ds2482(int file) {
    // Reiniciar el DS2482
    char cmd[1] = {CMD_DRST};
    if (write(file, cmd, 1) != 1) {
        std::cerr << "Error al reiniciar DS2482" << std::endl;
        return false;
    }
    usleep(1000); // Esperar 1 ms
    return true;
}
*/

bool init_ds2482(int file) {
    char cmd[1] = {CMD_DRST};
    int result = write(file, cmd, 1);
    if (result != 1) {
        std::cerr << "Error al reiniciar DS2482. Resultado: " << result << " errno: " << strerror(errno) << std::endl;
        return false;
    }
    usleep(1000); // Esperar 1 ms
    return true;
}


// Función para realizar un reset en el bus 1-Wire
bool onewire_reset(int file) {
    char cmd[1] = {CMD_1WRS};
    if (write(file, cmd, 1) != 1) {
        std::cerr << "Error al enviar comando de reset 1-Wire" << std::endl;
        return false;
    }
    usleep(1000); // Esperar 1 ms
    return true;
}

// Función para leer el DS1994 (secuencialmente lee el ROM y la memoria)
bool read_ds1994(int file) {
    // Reiniciar el bus 1-Wire
    if (!onewire_reset(file)) {
        return false;
    }

    // Comando "Match ROM" (0x55) para seleccionar el DS1994
    char match_rom[9] = {0x55, DS1994_FAMILY_CODE, /* Dirección ROM del DS1994 */};
    
    // Aquí debes agregar el código para obtener el ROM ID completo del DS1994
    // que está conectado al bus 1-Wire.

    if (write(file, match_rom, 9) != 9) {
        std::cerr << "Error al enviar Match ROM" << std::endl;
        return false;
    }

    // Aquí puedes agregar el código para leer la memoria del DS1994
    // usando comandos específicos para acceder a su memoria RTC.

    return true;
}

int main() {
    const char *i2c_device = "/dev/i2c-1"; // Cambia esto según tu interfaz I2C
    int file;

    // Abrir el bus I2C
    if ((file = open(i2c_device, O_RDWR)) < 0) {
        std::cerr << "Error al abrir el bus I2C" << std::endl;
        return 1;
    }

    // Configurar la dirección del dispositivo I2C
    if (ioctl(file, I2C_SLAVE, DS2482_ADDRESS) < 0) {
        std::cerr << "Error al configurar la dirección I2C" << std::endl;
        return 1;
    }

    // Inicializar el DS2482
    if (!init_ds2482(file)) {
        std::cerr << "Error al inicializar el DS2482" << std::endl;
        return 1;
    }

    // Leer el DS1994 conectado al bus 1-Wire
    if (!read_ds1994(file)) {
        std::cerr << "Error al leer DS1994" << std::endl;
        return 1;
    }

    std::cout << "Lectura del DS1994 completada exitosamente" << std::endl;

    close(file); // Cerrar el bus I2C
    return 0;
}


