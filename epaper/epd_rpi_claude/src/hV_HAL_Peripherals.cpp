//
// hV_HAL_Peripherals.cpp
// Hardware Abstraction Layer – Raspberry Pi (bcm2835)
//
// Ported from Arduino/Energia to native Linux + bcm2835 library
//
// Compile: g++ -O2 -o epd_demo *.cpp -lbcm2835
//

#include "hV_HAL_Peripherals.h"
#include <time.h>

// ─── Global objects ────────────────────────────────────────────────────────────
SerialClass Serial;

// ─── SPI speed set at begin() ─────────────────────────────────────────────────
static uint32_t _spiSpeedHz = 8000000;
static bool     _spiActive  = false;

// ─── 3-wire SPI pins ──────────────────────────────────────────────────────────
static uint8_t _spi3Clock = 11; // SCK  = GPIO11
static uint8_t _spi3Data  = 10; // MOSI = GPIO10

// ─── Arduino-compat wrappers ──────────────────────────────────────────────────
void pinMode(uint8_t pin, uint8_t mode)
{
    if (mode == INPUT_PULLUP)
    {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(pin, BCM2835_GPIO_PUD_UP);
    }
    else
    {
        bcm2835_gpio_fsel(pin, (bcm2835_gpio_fsel_t)mode);
    }
}

void digitalWrite(uint8_t pin, uint8_t val)
{
    bcm2835_gpio_write(pin, val ? HIGH : LOW);
}

int digitalRead(uint8_t pin)
{
    return bcm2835_gpio_lev(pin) ? 1 : 0;
}

void delay(uint32_t ms)
{
    bcm2835_delay(ms);
}

void delayMicroseconds(uint32_t us)
{
    bcm2835_delayMicroseconds(us);
}

uint32_t millis()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000UL + ts.tv_nsec / 1000000UL);
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// ─── HAL general ─────────────────────────────────────────────────────────────
void hV_HAL_begin()
{
    if (!bcm2835_init())
    {
        fprintf(stderr, "hV_HAL_begin: bcm2835_init() failed. Run as root.\n");
        exit(1);
    }
}

void waitFor(uint8_t pin, uint8_t state)
{
    while (digitalRead(pin) != state)
    {
        delay(32);
    }
}

// ─── 4-wire SPI ───────────────────────────────────────────────────────────────
void hV_HAL_SPI_begin(uint32_t speed)
{
    if (_spiActive) return;

    _spiSpeedHz = speed;

    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

    // Find closest divider: bcm2835 core clock is 250 MHz
    uint16_t div = (uint16_t)(250000000UL / speed);
    // Round up to next power of 2 (bcm2835 requires power-of-2 dividers)
    if (div <=   2) div = BCM2835_SPI_CLOCK_DIVIDER_2;
    else if (div <=  4) div = BCM2835_SPI_CLOCK_DIVIDER_4;
    else if (div <=  8) div = BCM2835_SPI_CLOCK_DIVIDER_8;
    else if (div <= 16) div = BCM2835_SPI_CLOCK_DIVIDER_16;
    else if (div <= 32) div = BCM2835_SPI_CLOCK_DIVIDER_32;
    else                div = BCM2835_SPI_CLOCK_DIVIDER_64;

    bcm2835_spi_setClockDivider(div);
    // CS managed manually – disable automatic CS
    bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);

    _spiActive = true;
}

void hV_HAL_SPI_end()
{
    if (!_spiActive) return;
    bcm2835_spi_end();
    _spiActive = false;
}

uint8_t hV_HAL_SPI_transfer(uint8_t data)
{
    return bcm2835_spi_transfer(data);
}

// ─── 3-wire SPI (bit-bang) ────────────────────────────────────────────────────
void hV_HAL_SPI3_begin()
{
    // Default: SCK=GPIO11, MOSI=GPIO10 (same as hardware SPI)
    hV_HAL_SPI3_define(11, 10);
}

void hV_HAL_SPI3_define(uint8_t pinClock, uint8_t pinData)
{
    _spi3Clock = pinClock;
    _spi3Data  = pinData;
}

uint8_t hV_HAL_SPI3_read()
{
    uint8_t value = 0;

    bcm2835_gpio_fsel(_spi3Clock, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(_spi3Data,  BCM2835_GPIO_FSEL_INPT);

    for (uint8_t i = 0; i < 8; i++)
    {
        bcm2835_gpio_write(_spi3Clock, HIGH);
        delayMicroseconds(1);
        value |= (digitalRead(_spi3Data) << (7 - i));
        bcm2835_gpio_write(_spi3Clock, LOW);
        delayMicroseconds(1);
    }
    return value;
}

void hV_HAL_SPI3_write(uint8_t value)
{
    bcm2835_gpio_fsel(_spi3Clock, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(_spi3Data,  BCM2835_GPIO_FSEL_OUTP);

    for (uint8_t i = 0; i < 8; i++)
    {
        bcm2835_gpio_write(_spi3Data, (value & (1 << (7 - i))) ? HIGH : LOW);
        delayMicroseconds(1);
        bcm2835_gpio_write(_spi3Clock, HIGH);
        delayMicroseconds(1);
        bcm2835_gpio_write(_spi3Clock, LOW);
        delayMicroseconds(1);
    }
}

// ─── I2C (stub – not used for basic EPD) ─────────────────────────────────────
void hV_HAL_Wire_begin()
{
    bcm2835_i2c_begin();
    bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626); // ~400 kHz
}

void hV_HAL_Wire_end()
{
    bcm2835_i2c_end();
}

void hV_HAL_Wire_transfer(uint8_t address, uint8_t* dataWrite, size_t sizeWrite,
                           uint8_t* dataRead, size_t sizeRead)
{
    bcm2835_i2c_setSlaveAddress(address);

    if (sizeWrite > 0)
    {
        bcm2835_i2c_write((const char*)dataWrite, sizeWrite);
    }
    if (sizeRead > 0 && dataRead != nullptr)
    {
        memset(dataRead, 0x00, sizeRead);
        bcm2835_i2c_read((char*)dataRead, sizeRead);
    }
}

// ─── Debug LEDs and Buttons ────────────────────────────────────────────────────
void debug_leds_init()
{
    // LEDs as outputs, initially off
    bcm2835_gpio_fsel(GPIO_LED1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(GPIO_LED2, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(GPIO_LED1, LOW);
    bcm2835_gpio_write(GPIO_LED2, LOW);

    // Buttons as inputs with pull-up
    bcm2835_gpio_fsel(GPIO_BUTTON1, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(GPIO_BUTTON2, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(GPIO_BUTTON1, BCM2835_GPIO_PUD_UP);
    bcm2835_gpio_set_pud(GPIO_BUTTON2, BCM2835_GPIO_PUD_UP);
}

void debug_led1_set(bool on)
{
    bcm2835_gpio_write(GPIO_LED1, on ? HIGH : LOW);
}

void debug_led2_set(bool on)
{
    bcm2835_gpio_write(GPIO_LED2, on ? HIGH : LOW);
}

bool debug_button1_pressed()
{
    return (bcm2835_gpio_lev(GPIO_BUTTON1) == LOW); // active LOW
}

bool debug_button2_pressed()
{
    return (bcm2835_gpio_lev(GPIO_BUTTON2) == LOW); // active LOW
}
