#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <ctime> // Incluir para obtener la hora

using namespace std;

// Define serial port path
const char* serialPort = "/dev/serial0"; // For Raspberry Pi Zero 2 W

// Uncomment the line below to enable debugging
#define DEBUG

bool sendATCommand(int serial, const string& command, string& response) {
    #ifdef DEBUG 
    cout << "Debug: Sending AT command: " << command << endl;
    #endif

    // Write command to serial port
    if (write(serial, command.c_str(), command.size()) < 0) {
        cerr << "Error writing to serial port." << endl;
        return false;
    }
    #ifdef DEBUG 
    cout << "Debug: AT command sent, waiting for response..." << endl;
    #endif

    // Read response from serial port
    char buf[256];
    response.clear();
    ssize_t bytesRead;
    int timeout = 100; // 10 segundos de timeout (100 * VTIME)
    while (timeout > 0) {
        bytesRead = read(serial, buf, sizeof(buf) - 1);
        if (bytesRead > 0) {
            buf[bytesRead] = '\0'; // Null-terminate the buffer
            response += buf;

            // Debug output if DEBUG is defined
            #ifdef DEBUG
            cout << "Debug: Partial response: " << buf << endl;
            #endif

            // Check if the response contains "OK" or an error message
            if (response.find("OK\r\n") != string::npos ||
                response.find("ERROR") != string::npos) {
                return true;
            }
        } else if (bytesRead < 0) {
            cerr << "Error reading from serial port." << endl;
            return false;
        } else {
            // No data received, decrement timeout counter
            timeout--;
        }
    }

    cerr << "Timeout reached while waiting for AT command response." << endl;
    return false;
}

void printCurrentTime() {
    time_t now = time(0);
    tm *ltm = localtime(&now);

    // Imprimir la hora en formato HH:MM:SS
    cout << "Current time: " << 1 + ltm->tm_hour << ":"
         << 1 + ltm->tm_min << ":"
         << 1 + ltm->tm_sec << endl;
}

int main() {
    #ifdef DEBUG 
    cout << "Debug: Opening serial port..." << endl;
    #endif

    // Open serial port for reading and writing
    int serial = open(serialPort, O_RDWR | O_NOCTTY);
    if (serial == -1) {
        cerr << "Failed to open serial port." << endl;
        return 1;
    }
    #ifdef DEBUG 
    cout << "Debug: Serial port opened successfully." << endl;
    #endif

    // Configure serial port settings
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(serial, &tty) != 0) {
        cerr << "Error in tcgetattr" << endl;
        close(serial);
        return 1;
    }
    #ifdef DEBUG 
    cout << "Debug: Configuring serial port settings..." << endl;
    #endif

    // Set baud rate (replace B9600 with your baud rate if different)
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    // Configure serial port settings
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;     // 8 bits per byte
    tty.c_cflag &= ~CRTSCTS; // No hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Enable reading

    // Set input and output mode to non-canonical
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // Set minimum number of characters for non-canonical read
    tty.c_cc[VMIN] = 1; // Reducir VMIN a 1 para una respuesta más rápida

    // Set timeout for non-canonical read
    tty.c_cc[VTIME] = 1; // Reducir VTIME a 1 decisegundo

    // Apply settings to serial port
    if (tcsetattr(serial, TCSANOW, &tty) != 0) {
        cerr << "Error in tcsetattr" << endl;
        close(serial);
        return 1;
    }
    #ifdef DEBUG 
    cout << "Debug: Serial port settings configured." << endl;
    #endif

    string response;

    // Habilitar la detección de tonos DTMF
    if (!sendATCommand(serial, "AT+DDET=1\r", response)) {
        cerr << "Failed to enable DTMF detection." << endl;
        close(serial);
        return 1;
    }
    #ifdef DEBUG 
    cout << "Debug: DTMF detection enabled." << endl;
    #endif

    // Realizar la llamada
    string phoneNumber = "+541131123385"; // Reemplaza con el número al que deseas llamar
    string atCommand = "ATD" + phoneNumber + ";\r";
    if (!sendATCommand(serial, atCommand, response)) {
        cerr << "Failed to dial number." << endl;
        close(serial);
        return 1;
    }
    #ifdef DEBUG 
    cout << "Debug: Call initiated, waiting for DTMF tones..." << endl;
    #endif

    // Esperar y procesar los tonos DTMF durante la llamada
    while (true) {
        if (sendATCommand(serial, "", response)) {
            size_t dtmfPos = response.find("+DTMF:");
            if (dtmfPos != string::npos) {
                // Extraer el dígito DTMF
                char dtmfDigit = response[dtmfPos + 7]; // El dígito está en la posición +7
                cout << "DTMF Tone detected: " << dtmfDigit << endl;

                // Si se detecta el tono "5", imprimir la hora
                if (dtmfDigit == '5') {
                    cout << "Detected DTMF 5, printing time..." << endl;
                    printCurrentTime();
                }

                // Si deseas terminar la llamada al recibir un tono específico, por ejemplo, "3":
                if (dtmfDigit == '3') {
                    sendATCommand(serial, "ATH\r", response); // Terminar la llamada
                    break;
                }

                // Limpiar la respuesta para estar listo para el próximo tono
                response.clear();
            }
        }
        usleep(100000); // Agregar un pequeño retraso de 100 ms para reducir el uso de la CPU
    }

    // Cerrar puerto serie
    close(serial);
    #ifdef DEBUG 
    cout << "Debug: Serial port closed." << endl;
    #endif
    return 0;
}
