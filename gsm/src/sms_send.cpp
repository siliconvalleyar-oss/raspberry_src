#include "sim800.h"
#include <iostream>
#include <unistd.h>

using namespace std;

int main() {
    int fd = serial_open();
    if (fd < 0) return 1;

    string resp;

    cout << "Setting SMS text mode..." << endl;
    if (!at_send(fd, "AT+CMGF=1\r", resp)) {
        cerr << "Failed: " << resp << endl;
        serial_close(fd);
        return 1;
    }

    string number = "+541131123385";
    string msg    = "code reserve 8943753";
    string cmd    = "AT+CMGS=\"" + number + "\"\r";

    cout << "Sending to " << number << endl;
    if (!at_send(fd, cmd, resp)) {
        cerr << "Failed: " << resp << endl;
        serial_close(fd);
        return 1;
    }

    if (!at_send(fd, msg + "\x1A", resp)) {
        cerr << "Send failed: " << resp << endl;
        serial_close(fd);
        return 1;
    }

    cout << "SMS sent. Response: " << resp << endl;
    serial_close(fd);
    return 0;
}
