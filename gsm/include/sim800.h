#ifndef SIM800_H
#define SIM800_H

#include <string>
#include <termios.h>

#define SERIAL_PORT "/dev/serial0"
#define SERIAL_BAUD B9600

int  serial_open(const char* port = SERIAL_PORT, speed_t baud = SERIAL_BAUD);
void serial_close(int fd);

bool at_send(int fd, const std::string& cmd, std::string& resp, int timeout_ds = 50);
bool at_wait_dtmf(int fd, std::string& resp, int timeout_ds = 200);

#endif
