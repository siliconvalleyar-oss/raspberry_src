#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

// Define serial port path
const char* serialPort = "/dev/serial0"; // For Raspberry Pi Zero 2 W

// Uncomment the line below to enable debugging
#define DEBUG

bool sendATCommand(int serial, const string& command, string& response) {
#ifndef DEBUG 
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
    while (true) {
        bytesRead = read(serial, buf, sizeof(buf) - 1);
        if (bytesRead > 0) {
            buf[bytesRead] = '\0'; // Null-terminate the buffer
            response += buf;

            // Debug output if DEBUG is defined
            #ifdef DEBUG
            cout << "Debug: Partial response: " << buf << endl;
            #endif

            // Check if "OK\r\n" or "ERROR" or ">" is in the response
            if (response.find("OK\r\n") != string::npos ||
                response.find("ERROR") != string::npos ||
                response.find(">") != string::npos) {
                return true;
            }
        } else if (bytesRead < 0) {
            cerr << "Error reading from serial port." << endl;
            return false;
        }
    }
    return false;
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
    tty.c_cc[VMIN] = 0;

    // Set timeout for non-canonical read
    tty.c_cc[VTIME] = 10; // 1 second timeout (10 deciseconds)

    // Apply settings to serial port
    if (tcsetattr(serial, TCSANOW, &tty) != 0) {
        cerr << "Error in tcsetattr" << endl;
        close(serial);
        return 1;
    }
#ifdef DEBUG 
    cout << "Debug: Serial port settings configured." << endl;
#endif
    // Send AT command to set SMS text mode
string response;

#ifdef DEBUG 
    cout << "Debug: Sending AT+CMGF=1 command..." << endl;
#endif

    if (!sendATCommand(serial, "AT+CMGF=1\r", response)) {
        cerr << "Failed to set SMS text mode." << endl;
        close(serial);
        return 1;
    }
#ifdef DEBUG 
    cout << "Debug: AT+CMGF=1 command sent successfully." << endl;
#endif
    // Set recipient's phone number
    string phoneNumber = "+541131123385"; // Replace with recipient's phone number
    string message = "code reserve 8943753"; // Replace with your message
    string atCommand = "AT+CMGS=\"" + phoneNumber + "\"\r";
#ifdef DEBUG 
    cout << "Debug: Sending AT+CMGS command..." << endl;
#endif    
    if (!sendATCommand(serial, atCommand, response)) {
        cerr << "Failed to set recipient's phone number." << endl;
        close(serial);
        return 1;
    }
#ifdef DEBUG 
    cout << "Debug: AT+CMGS command sent successfully." << endl;
#endif
    // Send message and CTRL+Z
    atCommand = message + "\x1A";
#ifdef DEBUG 
    cout << "Debug: Sending message..." << endl;
#endif    
    if (!sendATCommand(serial, atCommand, response)) {
        cerr << "Failed to send message." << endl;
        close(serial);
        return 1;
    }
#ifdef DEBUG 
    cout << "Debug: Message sent successfully." << endl;
#endif
    // Print the complete response
#ifdef DEBUG 
    cout << "Response: " << response << endl;
#endif
    // Close serial port
    close(serial);
#ifdef DEBUG 
    cout << "Debug: Serial port closed." << endl;
#endif
    return 0;
}
