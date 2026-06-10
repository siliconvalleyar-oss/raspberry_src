#include "sim800.h"
#include <iostream>
#include <ctime>
#include <unistd.h>

using namespace std;

static void print_time() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    cout << "Time: " << ltm->tm_hour << ":"
         << ltm->tm_min << ":"
         << ltm->tm_sec << endl;
}

int main() {
    int fd = serial_open();
    if (fd < 0) return 1;

    string resp;

    cout << "Enabling DTMF detection..." << endl;
    if (!at_send(fd, "AT+DDET=1\r", resp)) {
        cerr << "DTMF enable failed: " << resp << endl;
        serial_close(fd);
        return 1;
    }

    string number = "+541131123385";
    cout << "Calling " << number << "..." << endl;
    if (!at_send(fd, "ATD" + number + ";\r", resp, 100)) {
        cerr << "Dial failed: " << resp << endl;
        serial_close(fd);
        return 1;
    }

    cout << "Waiting for DTMF tones (5=time, 3=hangup)..." << endl;
    while (true) {
        string dtmf;
        if (at_wait_dtmf(fd, dtmf)) {
            size_t pos = dtmf.find("+DTMF:");
            if (pos != string::npos) {
                char digit = dtmf[pos + 7];
                cout << "DTMF: " << digit << endl;
                if (digit == '5') print_time();
                if (digit == '3') {
                    at_send(fd, "ATH\r", resp);
                    break;
                }
            }
        }
    }

    serial_close(fd);
    return 0;
}
