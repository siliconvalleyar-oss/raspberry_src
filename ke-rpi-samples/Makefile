CC = gcc
CFLAGS = -Wall
OLED_CFLAGS = $(CFLAGS) -Ispi-c-ioctl
BINDIR = bin

.PHONY: all clean

all: $(BINDIR) \
     $(BINDIR)/gpio_usage_sysfs \
     $(BINDIR)/i2c_ioctl_tmp006 \
     $(BINDIR)/spi_sysfs_loopback \
     $(BINDIR)/oled_demo \
     $(BINDIR)/uart_loopback

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/gpio_usage_sysfs: gpio-c-sysfs/gpio_usage_sysfs.c
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/i2c_ioctl_tmp006: i2c-c-ioctl/i2c_ioctl_tmp006.c
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/spi_sysfs_loopback: spi-c-ioctl/spi_sysfs_loopback.c
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/oled_demo: spi-c-ioctl/oled_demo.c spi-c-ioctl/oled_functions.c spi-c-ioctl/font.c
	$(CC) $(OLED_CFLAGS) -o $@ $^ -lbcm2835

$(BINDIR)/uart_loopback: uart-c-termios/uart_loopback.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BINDIR)
