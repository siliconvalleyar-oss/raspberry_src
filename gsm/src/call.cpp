#include "sim800.h"
#include <iostream>
#include <unistd.h>

using namespace std;

int main() {
    int fd = serial_open();
    if (fd < 0) return 1;

    string number = "+541131123385";
    string resp;

    cout << "Calling " << number << "..." << endl;
    if (!at_send(fd, "ATD" + number + ";\r", resp, 100)) {
        cerr << "Dial failed: " << resp << endl;
        serial_close(fd);
        return 1;
    }

    cout << "Call initiated. Press Ctrl+C to hang up." << endl;
    cout << "Response: " << resp << endl;

    sleep(30);

    cout << "Hanging up..." << endl;
    at_send(fd, "ATH\r", resp);
    cout << "Done: " << resp << endl;

    serial_close(fd);
    return 0;
}
