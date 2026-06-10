#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <ctime>

using namespace std;

const char* serialPort = "/dev/serial0"; // Para Raspberry Pi Zero 2 W

bool sendATCommand(int serial, const string& command, string& response) {
    // Enviar el comando AT
    if (write(serial, command.c_str(), command.size()) < 0) {
        cerr << "Error al escribir en el puerto serie." << endl;
        return false;
    }

    // Leer la respuesta del puerto serie
    char buf[256];
    response.clear();
    ssize_t bytesRead;
    int timeout = 100; // 10 segundos de timeout (100 * VTIME)
    while (timeout > 0) {
        bytesRead = read(serial, buf, sizeof(buf) - 1);
        if (bytesRead > 0) {
            buf[bytesRead] = '\0'; // Terminar la cadena
            response += buf;

            // Imprimir la respuesta completa para depuración
            cout << "Respuesta recibida: " << buf << endl;

            // Verificar si la respuesta contiene "OK" o un mensaje de error
            if (response.find("OK\r\n") != string::npos ||
                response.find("ERROR") != string::npos) {
                return true;
            }
        } else if (bytesRead < 0) {
            cerr << "Error al leer del puerto serie." << endl;
            return false;
        } else {
            timeout--;
        }
    }

    cerr << "Se alcanzó el tiempo de espera esperando la respuesta del comando AT." << endl;
    return false;
}

void printCurrentTime() {
    time_t now = time(0);
    tm *ltm = localtime(&now);

    // Imprimir la hora en formato HH:MM:SS
    cout << "Hora actual: " << 1 + ltm->tm_hour << ":"
         << 1 + ltm->tm_min << ":"
         << 1 + ltm->tm_sec << endl;
}

int main() {
    // Abrir el puerto serie para lectura y escritura
    int serial = open(serialPort, O_RDWR | O_NOCTTY);
    if (serial == -1) {
        cerr << "Error al abrir el puerto serie." << endl;
        return 1;
    }

    // Configurar la configuración del puerto serie
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(serial, &tty) != 0) {
        cerr << "Error en tcgetattr" << endl;
        close(serial);
        return 1;
    }

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag &= ~PARENB; 
    tty.c_cflag &= ~CSTOPB; 
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;     
    tty.c_cflag &= ~CRTSCTS; 
    tty.c_cflag |= CREAD | CLOCAL; 

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_cc[VMIN] = 1; // Reducir VMIN a 1 para respuesta rápida
    tty.c_cc[VTIME] = 1; // Reducir VTIME a 1 decisegundo

    if (tcsetattr(serial, TCSANOW, &tty) != 0) {
        cerr << "Error en tcsetattr" << endl;
        close(serial);
        return 1;
    }

    string response;

    // Habilitar la detección de tonos DTMF
    if (!sendATCommand(serial, "AT+DDET=1\r", response)) {
        cerr << "Error al habilitar la detección de DTMF." << endl;
        close(serial);
        return 1;
    }

    // Realizar la llamada
    string phoneNumber = "+541131123385";
    string atCommand = "ATD" + phoneNumber + ";\r";
    if (!sendATCommand(serial, atCommand, response)) {
        cerr << "Error al marcar el número." << endl;
        close(serial);
        return 1;
    }

    // Procesar tonos DTMF durante la llamada
    while (true) {
        if (sendATCommand(serial, "", response)) {
            size_t dtmfPos = response.find("+DTMF:");
            if (dtmfPos != string::npos) {
                // Extraer el dígito DTMF
                char dtmfDigit = response[dtmfPos + 7];
                cout << "Tono DTMF detectado: " << dtmfDigit << endl;

                // Detectar el tono "5" y mostrar la hora
                if (dtmfDigit == '5') {
                    printCurrentTime();
                }

                // Terminar la llamada al recibir el tono "3"
                if (dtmfDigit == '3') {
                    sendATCommand(serial, "ATH\r", response);
                    break;
                }

                // Limpiar la respuesta para el próximo tono
                response.clear();
            } else {
                // Si no se detecta "+DTMF:", imprimir la respuesta completa para depuración
                cout << "Respuesta sin DTMF detectado: " << response << endl;
            }
        }
        usleep(100000); // Pequeño retraso de 100 ms para reducir el uso de CPU
    }

    // Cerrar puerto serie
    close(serial);
    return 0;
}
