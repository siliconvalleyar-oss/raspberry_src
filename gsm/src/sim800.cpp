#include "sim800.h"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

int serial_open(const char* port, speed_t baud) {
    int fd = open(port, O_RDWR | O_NOCTTY);
    if (fd == -1) {
        cerr << "Error opening " << port << endl;
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(fd, &tty) != 0) {
        cerr << "tcgetattr failed" << endl;
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        cerr << "tcsetattr failed" << endl;
        close(fd);
        return -1;
    }

    return fd;
}

void serial_close(int fd) {
    if (fd >= 0) close(fd);
}

bool at_send(int fd, const string& cmd, string& resp, int timeout_ds) {
#ifdef DEBUG
    cout << "> " << cmd;
#endif

    if (write(fd, cmd.c_str(), cmd.size()) < 0) {
        cerr << "Write error" << endl;
        return false;
    }

    resp.clear();
    char buf[256];
    while (timeout_ds > 0) {
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            resp += buf;
#ifdef DEBUG
            cout << "< " << buf;
#endif
            if (resp.find("OK\r\n") != string::npos ||
                resp.find("ERROR") != string::npos ||
                resp.find(">") != string::npos) {
                return true;
            }
        } else if (n < 0) {
            cerr << "Read error" << endl;
            return false;
        }
        timeout_ds--;
    }
    return false;
}

bool at_wait_dtmf(int fd, string& resp, int timeout_ds) {
    resp.clear();
    char buf[256];
    while (timeout_ds > 0) {
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            resp += buf;
#ifdef DEBUG
            cout << buf;
#endif
            if (resp.find("+DTMF:") != string::npos) {
                return true;
            }
        } else if (n < 0) {
            return false;
        }
        usleep(100000);
        timeout_ds--;
    }
    return false;
}
