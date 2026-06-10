#!/bin/bash
# =============================================================
# Generador de proyecto MRF24J40 AVANZADO (corregido)
# Velocidad SPI: 100000 Hz, con todas las mejoras
# =============================================================

set -e

PROJECT_ROOT="$(pwd)"
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_header() {
    echo ""
    echo -e "${BLUE}=================================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}=================================================${NC}"
}

print_success() { echo -e "${GREEN}✓${NC} $1"; }
print_warning() { echo -e "${YELLOW}⚠${NC} $1"; }

# Limpiar proyectos anteriores si existen
rm -rf mrf24_tx mrf24_rx

# Crear estructura
mkdir -p mrf24_tx/src mrf24_rx/src

# =============================================================
# DRIVER MEJORADO (mismo que antes, pero con velocidad 100000)
# =============================================================
print_header "Creando driver avanzado MRF24J40"

# mrf24j40.h (igual que antes, ya tiene #define SPI_SPEED_HZ 100000)
cat > mrf24_tx/src/mrf24j40.h << 'HEADER_EOF'
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <chrono>

// =============================================================
// CONFIGURACIÓN SPI - Velocidad unificada
// =============================================================
#define SPI_SPEED_HZ    100000      // 100 kHz (estable y confiable)
#define SPI_DEVICE      "/dev/spidev0.0"

// =============================================================
// MRF24J40 - Registros Short Address (6 bits)
// =============================================================
#define REG_RXMCR    0x00
#define REG_PANIDL   0x01
#define REG_PANIDH   0x02
#define REG_SADRL    0x03
#define REG_SADRH    0x04
#define REG_EADR0    0x05
#define REG_EADR1    0x06
#define REG_EADR2    0x07
#define REG_EADR3    0x08
#define REG_EADR4    0x09
#define REG_EADR5    0x0A
#define REG_EADR6    0x0B
#define REG_EADR7    0x0C
#define REG_RXFLUSH  0x0D
#define REG_ORDER    0x10
#define REG_TXMCR    0x11
#define REG_ACKTMOUT 0x12
#define REG_ESLOTG0  0x13
#define REG_SYMTICKL 0x14
#define REG_SYMTICKH 0x15
#define REG_PACON0   0x16
#define REG_PACON1   0x17
#define REG_PACON2   0x18
#define REG_RSSIVAL  0x19
#define REG_TXBCON0  0x1A
#define REG_TXNCON   0x1B
#define REG_TXG1CON  0x1C
#define REG_TXG2CON  0x1D
#define REG_ESLOTG23 0x1E
#define REG_ESLOTG45 0x1F
#define REG_ESLOTG67 0x20
#define REG_TXPEND   0x21
#define REG_WAKECON  0x22
#define REG_FRMOFFSET 0x23
#define REG_TXSTAT   0x24
#define REG_TXBCON1  0x25
#define REG_GATECLK  0x26
#define REG_TXTIME   0x27
#define REG_HSYMTMRL 0x28
#define REG_HSYMTMRH 0x29
#define REG_SOFTRST  0x2A
#define REG_SECCON0  0x2C
#define REG_SECCON1  0x2D
#define REG_TXSTBL   0x2E
#define REG_RXSR     0x30
#define REG_INTSTAT  0x31
#define REG_INTCON   0x32
#define REG_GPIO     0x33
#define REG_TRISGPIO 0x34
#define REG_SLPACK   0x35
#define REG_RFCTL    0x36
#define REG_SECCR2   0x37
#define REG_BBREG0   0x38
#define REG_BBREG1   0x39
#define REG_BBREG2   0x3A
#define REG_BBREG3   0x3B
#define REG_BBREG4   0x3C
#define REG_BBREG6   0x3E
#define REG_CCAEDTH  0x3F

// Registros Long Address
#define LREG_RFCON0  0x200
#define LREG_RFCON1  0x201
#define LREG_RFCON2  0x202
#define LREG_RFCON3  0x203
#define LREG_RFCON5  0x205
#define LREG_RFCON6  0x206
#define LREG_RFCON7  0x207
#define LREG_RFCON8  0x208
#define LREG_SLPCON0 0x211
#define LREG_SLPCON1 0x220
#define LREG_SLPCON2 0x221
#define LREG_WAKETIMEL 0x222
#define LREG_WAKETIMEH 0x223
#define LREG_REMCNTL 0x224
#define LREG_REMCNTH 0x225
#define LREG_MAINCNT0 0x226
#define LREG_MAINCNT1 0x227
#define LREG_MAINCNT2 0x228
#define LREG_MAINCNT3 0x229
#define LREG_ASSOEADR0 0x230
#define LREG_TESTMODE 0x22F

// FIFOs
#define TXNFIFO     0x000
#define TXBFIFO     0x080
#define TXG1FIFO    0x100
#define TXG2FIFO    0x180
#define RXFIFO      0x300

// Bits de interrupción
#define INT_SLPIF   0x80
#define INT_WAKEIF  0x40
#define INT_HSYMTMRIF 0x20
#define INT_SECIF   0x10
#define INT_RXIF    0x08
#define INT_TXG2IF  0x04
#define INT_TXG1IF  0x02
#define INT_TXNIF   0x01

// Bits TXNCON
#define TXNTRIG     0x01
#define TXNSECEN    0x02
#define TXNACKREQ   0x04

// Frame Control Field
#define FCF_LO      0x61
#define FCF_HI      0x88

#define MAX_PAYLOAD     100

// Estadísticas avanzadas
struct RadioStats {
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t tx_success;
    uint32_t tx_fail;
    uint32_t tx_retries_total;
    uint32_t rx_lqi_sum;
    int32_t  rx_rssi_sum;
    uint32_t rx_count;
    uint32_t crc_errors;
};

class Mrf24j40 {
public:
    Mrf24j40();
    ~Mrf24j40();

    bool init(uint8_t channel);
    void setPan(uint16_t pan);
    void setShortAddress(uint16_t addr);
    uint16_t getPan();
    uint16_t getShortAddress();
    bool setChannel(uint8_t channel);
    
    bool send(uint16_t dest_addr, uint16_t dest_pan, const uint8_t* data, uint8_t len);
    bool sendString(uint16_t dest_addr, const char* str);
    
    void poll();
    
    bool hasPacket() const { return rx_ready; }
    uint8_t rxLen() const { return rx_length; }
    void rxGet(uint8_t* buf);
    uint8_t getLQI() const { return rx_lqi; }
    int8_t getRSSI() const { return rx_rssi_dbm; }
    
    bool txDone() const { return !tx_pending; }
    bool txSuccess() const { return tx_ok; }
    uint8_t txRetries() const { return tx_retries; }
    
    void getStats(RadioStats& stats);
    void resetStats();
    void printRegisters();
    bool selfTest();

private:
    uint8_t readShort(uint8_t addr);
    void writeShort(uint8_t addr, uint8_t val);
    uint8_t readLong(uint16_t addr);
    void writeLong(uint16_t addr, uint8_t val);
    
    void flushRx();
    void handleRxIrq();
    void handleTxIrq();
    void rfReset();
    bool waitForReset();

    int spi_fd;
    bool initialized;
    
    bool tx_pending;
    bool tx_ok;
    uint8_t tx_retries;
    uint8_t seq;
    
    uint8_t rx_buf[MAX_PAYLOAD];
    uint8_t rx_length;
    uint8_t rx_lqi;
    int8_t rx_rssi_dbm;
    bool rx_ready;
    
    RadioStats stats;
};
HEADER_EOF

# mrf24j40.cpp (idéntico al anterior, pero con velocidad 100000)
cat > mrf24_tx/src/mrf24j40.cpp << 'CPP_EOF'
#include "mrf24j40.h"
#include <fcntl.h>
#include <errno.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <cstring>

Mrf24j40::Mrf24j40()
    : spi_fd(-1), initialized(false), tx_pending(false), tx_ok(false),
      tx_retries(0), seq(0), rx_length(0), rx_lqi(0), rx_rssi_dbm(0), rx_ready(false)
{
    memset(&stats, 0, sizeof(stats));
}

Mrf24j40::~Mrf24j40()
{
    if (spi_fd >= 0) close(spi_fd);
}

uint8_t Mrf24j40::readShort(uint8_t addr)
{
    uint8_t tx[2], rx[2];
    tx[0] = (addr & 0x3F) << 1;
    tx[1] = 0x00;
    
    struct spi_ioc_transfer tr = {0};
    tr.tx_buf = (unsigned long)tx;
    tr.rx_buf = (unsigned long)rx;
    tr.len = 2;
    tr.speed_hz = SPI_SPEED_HZ;
    tr.bits_per_word = 8;
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        perror("readShort");
    return rx[1];
}

void Mrf24j40::writeShort(uint8_t addr, uint8_t val)
{
    uint8_t tx[2];
    tx[0] = ((addr & 0x3F) << 1) | 0x01;
    tx[1] = val;
    
    struct spi_ioc_transfer tr = {0};
    tr.tx_buf = (unsigned long)tx;
    tr.len = 2;
    tr.speed_hz = SPI_SPEED_HZ;
    tr.bits_per_word = 8;
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        perror("writeShort");
}

uint8_t Mrf24j40::readLong(uint16_t addr)
{
    uint8_t tx[3], rx[3];
    tx[0] = 0x80 | ((addr >> 3) & 0x7F);
    tx[1] = (addr & 0x07) << 5;
    tx[2] = 0x00;
    
    struct spi_ioc_transfer tr = {0};
    tr.tx_buf = (unsigned long)tx;
    tr.rx_buf = (unsigned long)rx;
    tr.len = 3;
    tr.speed_hz = SPI_SPEED_HZ;
    tr.bits_per_word = 8;
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        perror("readLong");
    return rx[2];
}

void Mrf24j40::writeLong(uint16_t addr, uint8_t val)
{
    uint8_t tx[3];
    tx[0] = 0x80 | ((addr >> 3) & 0x7F);
    tx[1] = ((addr & 0x07) << 5) | 0x10;
    tx[2] = val;
    
    struct spi_ioc_transfer tr = {0};
    tr.tx_buf = (unsigned long)tx;
    tr.len = 3;
    tr.speed_hz = SPI_SPEED_HZ;
    tr.bits_per_word = 8;
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        perror("writeLong");
}

bool Mrf24j40::waitForReset()
{
    for (int i = 0; i < 200; i++) {
        if ((readShort(REG_SOFTRST) & 0x07) == 0x00)
            return true;
        usleep(1000);
    }
    return false;
}

bool Mrf24j40::init(uint8_t channel)
{
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        fprintf(stderr, "ERROR: No se pudo abrir %s\n", SPI_DEVICE);
        return false;
    }
    
    uint8_t mode = SPI_MODE_0;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    
    uint32_t speed = SPI_SPEED_HZ;
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    
    printf("[SPI] %s, velocidad: %u Hz\n", SPI_DEVICE, speed);
    
    writeShort(REG_SOFTRST, 0x07);
    usleep(2000);
    if (!waitForReset()) {
        fprintf(stderr, "ERROR: Timeout en soft reset\n");
        return false;
    }
    printf("[INIT] Soft reset OK\n");
    
    writeShort(REG_PACON2, 0x98);
    writeShort(REG_TXSTBL, 0x95);
    
    writeLong(LREG_RFCON1, 0x02);
    writeLong(LREG_RFCON2, 0x80);
    writeLong(LREG_RFCON3, 0x00);
    writeLong(LREG_RFCON6, 0x90);
    writeLong(LREG_RFCON7, 0x80);
    writeLong(LREG_RFCON8, 0x10);
    writeLong(LREG_SLPCON1, 0x21);
    
    setChannel(channel);
    printf("[INIT] Canal: %d\n", channel);
    
    writeShort(REG_BBREG2, 0x80);
    writeShort(REG_CCAEDTH, 0x60);
    writeShort(REG_BBREG6, 0x40);
    writeShort(REG_INTCON, 0xF6);
    
    flushRx();
    rfReset();
    
    initialized = true;
    printf("[INIT] MRF24J40 inicializado correctamente\n");
    return true;
}

void Mrf24j40::rfReset()
{
    writeShort(REG_RFCTL, 0x04);
    usleep(200);
    writeShort(REG_RFCTL, 0x00);
    usleep(1000);
}

void Mrf24j40::setPan(uint16_t pan)
{
    writeShort(REG_PANIDL, pan & 0xFF);
    writeShort(REG_PANIDH, (pan >> 8) & 0xFF);
}

void Mrf24j40::setShortAddress(uint16_t addr)
{
    writeShort(REG_SADRL, addr & 0xFF);
    writeShort(REG_SADRH, (addr >> 8) & 0xFF);
}

uint16_t Mrf24j40::getPan()
{
    return readShort(REG_PANIDL) | (readShort(REG_PANIDH) << 8);
}

uint16_t Mrf24j40::getShortAddress()
{
    return readShort(REG_SADRL) | (readShort(REG_SADRH) << 8);
}

bool Mrf24j40::setChannel(uint8_t ch)
{
    if (ch < 11 || ch > 26) return false;
    uint8_t val = ((ch - 11) << 4) | 0x03;
    writeLong(LREG_RFCON0, val);
    rfReset();
    return true;
}

void Mrf24j40::flushRx()
{
    writeShort(REG_BBREG1, 0x04);
    writeShort(REG_RXFLUSH, 0x01);
    usleep(100);
    writeShort(REG_BBREG1, 0x00);
}

bool Mrf24j40::send(uint16_t dest_addr, uint16_t dest_pan, const uint8_t* data, uint8_t len)
{
    if (!initialized || len > MAX_PAYLOAD) return false;
    
    int wait = 500;
    while (tx_pending && wait-- > 0) {
        poll();
        usleep(1000);
    }
    if (tx_pending) return false;
    
    uint16_t src_addr = getShortAddress();
    const uint8_t hdr_len = 9;
    const uint8_t frm_len = hdr_len + len;
    
    writeLong(TXNFIFO + 0, hdr_len);
    writeLong(TXNFIFO + 1, frm_len);
    writeLong(TXNFIFO + 2, FCF_LO);
    writeLong(TXNFIFO + 3, FCF_HI);
    writeLong(TXNFIFO + 4, seq++);
    writeLong(TXNFIFO + 5, dest_pan & 0xFF);
    writeLong(TXNFIFO + 6, (dest_pan >> 8) & 0xFF);
    writeLong(TXNFIFO + 7, dest_addr & 0xFF);
    writeLong(TXNFIFO + 8, (dest_addr >> 8) & 0xFF);
    writeLong(TXNFIFO + 9, src_addr & 0xFF);
    writeLong(TXNFIFO + 10, (src_addr >> 8) & 0xFF);
    
    for (uint8_t i = 0; i < len; i++)
        writeLong(TXNFIFO + 11 + i, data[i]);
    
    tx_pending = true;
    tx_ok = false;
    tx_retries = 0;
    stats.packets_sent++;
    
    writeShort(REG_TXNCON, TXNACKREQ | TXNTRIG);
    return true;
}

bool Mrf24j40::sendString(uint16_t dest_addr, const char* str)
{
    if (!str) return false;
    uint8_t len = strnlen(str, MAX_PAYLOAD);
    return send(dest_addr, getPan(), (const uint8_t*)str, len);
}

void Mrf24j40::handleTxIrq()
{
    uint8_t txstat = readShort(REG_TXSTAT);
    tx_ok = !(txstat & 0x01);
    tx_retries = (txstat >> 6) & 0x03;
    tx_pending = false;
    
    if (tx_ok) {
        stats.tx_success++;
    } else {
        stats.tx_fail++;
    }
    stats.tx_retries_total += tx_retries;
}

void Mrf24j40::handleRxIrq()
{
    writeShort(REG_BBREG1, 0x04);
    
    uint8_t frame_len = readLong(RXFIFO + 0);
    const uint8_t MIN_FRAME = 12;
    
    if (frame_len < MIN_FRAME || frame_len > 127) {
        flushRx();
        return;
    }
    
    const uint8_t HDR = 9;
    const uint8_t FCS = 2;
    int payload_len = frame_len - HDR - FCS;
    
    if (payload_len <= 0 || payload_len > MAX_PAYLOAD) {
        flushRx();
        return;
    }
    
    rx_length = payload_len;
    for (uint8_t i = 0; i < rx_length; i++)
        rx_buf[i] = readLong(RXFIFO + 1 + HDR + i);
    
    rx_lqi = readLong(RXFIFO + 1 + frame_len);
    uint8_t raw_rssi = readLong(RXFIFO + 1 + frame_len + 1);
    rx_rssi_dbm = -90 + raw_rssi / 3;
    
    rx_ready = true;
    
    stats.packets_received++;
    stats.rx_lqi_sum += rx_lqi;
    stats.rx_rssi_sum += rx_rssi_dbm;
    stats.rx_count++;
    
    flushRx();
    writeShort(REG_BBREG1, 0x00);
}

void Mrf24j40::poll()
{
    uint8_t irq = readShort(REG_INTSTAT);
    if (irq == 0) return;
    
    if (irq & INT_TXNIF) handleTxIrq();
    if (irq & INT_RXIF) handleRxIrq();
    writeShort(REG_INTSTAT, irq);
}

void Mrf24j40::rxGet(uint8_t* buf)
{
    if (buf && rx_ready) {
        memcpy(buf, rx_buf, rx_length);
    }
    rx_ready = false;
    rx_length = 0;
}

void Mrf24j40::getStats(RadioStats& s)
{
    s = stats;
}

void Mrf24j40::resetStats()
{
    memset(&stats, 0, sizeof(stats));
}

void Mrf24j40::printRegisters()
{
    printf("\n=== Registros MRF24J40 ===\n");
    printf("SOFTRST: 0x%02X\n", readShort(REG_SOFTRST));
    printf("INTSTAT: 0x%02X\n", readShort(REG_INTSTAT));
    printf("TXSTAT:  0x%02X\n", readShort(REG_TXSTAT));
    printf("PANID:   0x%04X\n", getPan());
    printf("SADDR:   0x%04X\n", getShortAddress());
    printf("RFCON2:  0x%02X\n", readLong(LREG_RFCON2));
    printf("========================\n");
}

bool Mrf24j40::selfTest()
{
    uint16_t original_pan = getPan();
    setPan(0x1234);
    uint16_t test_pan = getPan();
    setPan(original_pan);
    
    if (test_pan != 0x1234) {
        fprintf(stderr, "Self-test falló: escritura PAN\n");
        return false;
    }
    printf("Self-test OK\n");
    return true;
}
CPP_EOF

# Copiar driver al receptor
cp mrf24_tx/src/mrf24j40.h mrf24_rx/src/
cp mrf24_tx/src/mrf24j40.cpp mrf24_rx/src/
print_success "Driver avanzado creado"

# =============================================================
# TRANSMISOR AVANZADO (con corrección de includes)
# =============================================================
print_header "Creando transmisor avanzado"

cat > mrf24_tx/src/main.cpp << 'TX_EOF'
#include <cstdio>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sys/select.h>   // para fd_set, select
#include <sys/time.h>     // para struct timeval
#include "mrf24j40.h"

// Configuración
#define MY_ADDR     0x0001
#define DEST_ADDR   0x0002
#define PAN_ID      0xCAFE
#define CHANNEL     20
#define TX_INTERVAL_MS 2000

// Modos de operación
enum Mode { MODE_NORMAL, MODE_BURST, MODE_SCAN };
static Mode current_mode = MODE_NORMAL;
static volatile bool running = true;
static Mrf24j40 radio;
static int burst_count = 0;

void sig_handler(int) { running = false; }

void print_stats() {
    RadioStats stats;
    radio.getStats(stats);
    
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║                 ESTADÍSTICAS                     ║\n");
    printf("╠══════════════════════════════════════════════════╣\n");
    printf("║ Paquetes enviados:     %-8d                      ║\n", stats.packets_sent);
    printf("║ TX exitosos:           %-8d                      ║\n", stats.tx_success);
    printf("║ TX fallidos:           %-8d                      ║\n", stats.tx_fail);
    printf("║ Retransmisiones total: %-8d                      ║\n", stats.tx_retries_total);
    printf("║ Tasa de éxito:         %-8.1f%%                  ║\n", 
           stats.packets_sent > 0 ? 100.0f * stats.tx_success / stats.packets_sent : 0);
    printf("╚══════════════════════════════════════════════════╝\n");
}

void burst_transmission(int packets, int delay_ms) {
    printf("\n[ BURST ] Enviando %d paquetes con delay %dms\n", packets, delay_ms);
    
    for (int i = 0; i < packets && running; i++) {
        char payload[64];
        snprintf(payload, sizeof(payload), "BURST:%d:%d", burst_count++, i);
        
        radio.sendString(DEST_ADDR, payload);
        
        int timeout = 100;
        while (timeout-- > 0 && radio.txDone() == false) {
            radio.poll();
            usleep(5000);
        }
        
        if (delay_ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    
    print_stats();
}

void normal_transmission() {
    static int msg_num = 0;
    char payload[64];
    snprintf(payload, sizeof(payload), "HELLO:%d", msg_num++);
    
    radio.sendString(DEST_ADDR, payload);
    
    int timeout = 500;
    while (timeout-- > 0 && radio.txDone() == false) {
        radio.poll();
        usleep(5000);
    }
    
    if (radio.txDone()) {
        if (radio.txSuccess())
            printf("✓ OK (retries=%d)\n", radio.txRetries());
        else
            printf("✗ FALLO (retries=%d)\n", radio.txRetries());
    } else {
        printf("⏱ TIMEOUT\n");
    }
}

int main() {
    signal(SIGINT, sig_handler);
    
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║   MRF24J40 TRANSMISOR AVANZADO v2.0    ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    if (!radio.init(CHANNEL)) return 1;
    
    radio.setPan(PAN_ID);
    radio.setShortAddress(MY_ADDR);
    radio.selfTest();
    
    printf("\n[CONFIGURACION]\n");
    printf("  PAN ID: 0x%04X, Dirección: 0x%04X\n", radio.getPan(), radio.getShortAddress());
    printf("  Destino: 0x%04X, Canal: %d, SPI: %u Hz\n", DEST_ADDR, CHANNEL, SPI_SPEED_HZ);
    
    printf("\n[COMANDOS]\n");
    printf("  n - modo normal | b - burst (10 paq) | s - stats | q - salir\n\n");
    
    printf("[MAIN] Iniciando transmisión normal. Ctrl+C para salir.\n\n");
    
    auto last_tx = std::chrono::steady_clock::now();
    
    while (running) {
        radio.poll();
        
        // Comandos por consola (no bloqueante)
        fd_set readfds;
        struct timeval tv = {0, 0};
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
            char cmd = getchar();
            if (cmd == 'b') {
                current_mode = MODE_BURST;
                burst_transmission(10, 50);
                current_mode = MODE_NORMAL;
            } else if (cmd == 's') {
                print_stats();
            } else if (cmd == 'q') {
                running = false;
            }
        }
        
        // Transmisión periódica
        auto now = std::chrono::steady_clock::now();
        if (current_mode == MODE_NORMAL && 
            std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tx).count() >= TX_INTERVAL_MS) {
            printf("[TX] ");
            normal_transmission();
            last_tx = now;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    print_stats();
    radio.printRegisters();
    printf("\n[FIN] Terminado.\n");
    return 0;
}
TX_EOF

# =============================================================
# RECEPTOR AVANZADO (con corrección de includes)
# =============================================================
print_header "Creando receptor avanzado"

cat > mrf24_rx/src/main.cpp << 'RX_EOF'
#include <cstdio>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <ctime>
#include <chrono>
#include <fstream>
#include <sys/select.h>
#include <sys/time.h>
#include "mrf24j40.h"

#define MY_ADDR     0x0002
#define PAN_ID      0xCAFE
#define CHANNEL     20

static volatile bool running = true;
static Mrf24j40 radio;
static std::ofstream logfile;

void sig_handler(int) { running = false; }

void print_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    struct tm* tm_info = localtime(&time_t);
    printf("[%02d:%02d:%02d.%03d] ", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, (int)ms.count());
}

void print_stats() {
    RadioStats stats;
    radio.getStats(stats);
    
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║              ESTADÍSTICAS RX                     ║\n");
    printf("╠══════════════════════════════════════════════════╣\n");
    printf("║ Paquetes recibidos:    %-8d                      ║\n", stats.packets_received);
    printf("║ LQI promedio:          %-8.1f                    ║\n", 
           stats.rx_count > 0 ? (float)stats.rx_lqi_sum / stats.rx_count : 0);
    printf("║ RSSI promedio:         %-8.1f dBm                ║\n",
           stats.rx_count > 0 ? (float)stats.rx_rssi_sum / stats.rx_count : 0);
    printf("╚══════════════════════════════════════════════════╝\n");
}

void log_packet(int packet_num, const char* payload, uint8_t len, uint8_t lqi, int8_t rssi) {
    if (logfile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        logfile << time_t << "," << packet_num << ",\"" << payload << "\"," << (int)len << "," << (int)lqi << "," << (int)rssi << "\n";
        logfile.flush();
    }
}

int main() {
    signal(SIGINT, sig_handler);
    
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║    MRF24J40 RECEPTOR AVANZADO v2.0     ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    if (!radio.init(CHANNEL)) return 1;
    
    radio.setPan(PAN_ID);
    radio.setShortAddress(MY_ADDR);
    radio.selfTest();
    
    printf("\n[CONFIGURACION]\n");
    printf("  PAN ID: 0x%04X, Dirección: 0x%04X\n", radio.getPan(), radio.getShortAddress());
    printf("  Canal: %d, SPI: %u Hz\n\n", CHANNEL, SPI_SPEED_HZ);
    
    logfile.open("mrf24_receiver.log", std::ios::app);
    if (logfile.is_open()) {
        logfile << "#timestamp,packet_num,payload,len,lqi,rssi\n";
    }
    
    printf("[COMANDOS]\n");
    printf("  s - estadísticas | c - clear stats | q - salir\n\n");
    
    printf("[MAIN] Escuchando... Ctrl+C para salir.\n\n");
    
    int packet_count = 0;
    uint8_t buffer[MAX_PAYLOAD];
    
    while (running) {
        radio.poll();
        
        fd_set readfds;
        struct timeval tv = {0, 0};
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
            char cmd = getchar();
            if (cmd == 's') {
                print_stats();
            } else if (cmd == 'c') {
                radio.resetStats();
                printf("[CMD] Estadísticas reiniciadas\n");
            } else if (cmd == 'q') {
                running = false;
            }
        }
        
        if (radio.hasPacket()) {
            uint8_t len = radio.rxLen();
            radio.rxGet(buffer);
            buffer[len] = '\0';
            
            packet_count++;
            print_timestamp();
            printf("Paquete #%d:\n", packet_count);
            printf("  Payload (%d): \"%s\"\n", len, (char*)buffer);
            printf("  LQI: %d/255, RSSI: %d dBm\n\n", radio.getLQI(), radio.getRSSI());
            
            log_packet(packet_count, (char*)buffer, len, radio.getLQI(), radio.getRSSI());
        }
        
        usleep(10000);
    }
    
    print_stats();
    radio.printRegisters();
    
    if (logfile.is_open()) {
        logfile.close();
        printf("\n[LOG] Guardado en mrf24_receiver.log\n");
    }
    
    printf("\n[FIN] Total paquetes: %d\n", packet_count);
    return 0;
}
RX_EOF

# =============================================================
# MAKEFILES
# =============================================================
print_header "Creando Makefiles"

cat > mrf24_tx/Makefile << 'MK_EOF'
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I./src -pthread
LDFLAGS  = -pthread

SRCS    = src/main.cpp src/mrf24j40.cpp
OBJS    = $(patsubst src/%.cpp, obj/%.o, $(SRCS))
TARGET  = bin/mrf24_transmitter

.PHONY: all clean

all: obj bin $(TARGET)
	@echo ""
	@echo "  Compilación exitosa: $(TARGET)"
	@echo "  Ejecutar con: sudo ./$(TARGET)"

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.cpp src/mrf24j40.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p obj

bin:
	mkdir -p bin

clean:
	rm -rf obj bin
MK_EOF

cat > mrf24_rx/Makefile << 'MK_EOF'
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I./src -pthread
LDFLAGS  = -pthread

SRCS    = src/main.cpp src/mrf24j40.cpp
OBJS    = $(patsubst src/%.cpp, obj/%.o, $(SRCS))
TARGET  = bin/mrf24_receiver

.PHONY: all clean

all: obj bin $(TARGET)
	@echo ""
	@echo "  Compilación exitosa: $(TARGET)"
	@echo "  Ejecutar con: sudo ./$(TARGET)"

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.cpp src/mrf24j40.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p obj

bin:
	mkdir -p bin

clean:
	rm -rf obj bin
MK_EOF

# =============================================================
# DEPLOY SCRIPT
# =============================================================
print_header "Creando script de despliegue"

cat > deploy.sh << 'DEPLOY_EOF'
#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

print_header() { echo -e "\n=================================================\n  $1\n================================================="; }

check_spi() {
    print_header "Verificando SPI"
    if [ ! -e /dev/spidev0.0 ] && [ ! -e /dev/spidev0.1 ]; then
        echo "ERROR: SPI no habilitado. Ejecute: sudo raspi-config"
        exit 1
    fi
    echo "SPI disponible ✓"
}

check_compiler() {
    print_header "Verificando compilador"
    if ! command -v g++ &>/dev/null; then
        sudo apt-get update -q && sudo apt-get install -y -q g++ build-essential
    fi
    echo "g++: $(g++ --version | head -1)"
}

select_role() {
    print_header "Seleccionar rol"
    echo "  1) TRANSMISOR (Raspberry A)"
    echo "  2) RECEPTOR (Raspberry B)"
    read -p "Opción [1/2]: " role
    case "$role" in
        1) PROJECT="mrf24_tx"; BINARY="mrf24_transmitter" ;;
        2) PROJECT="mrf24_rx"; BINARY="mrf24_receiver" ;;
        *) exit 1 ;;
    esac
}

check_spi
check_compiler
select_role

cd "$SCRIPT_DIR/$PROJECT"
make clean && make
echo -e "\nBinario: $PROJECT/bin/$BINARY"
echo "Ejecutar: sudo ./$PROJECT/bin/$BINARY"
DEPLOY_EOF

chmod +x deploy.sh

# =============================================================
# RESUMEN FINAL
# =============================================================
print_header "PROYECTO GENERADO (CORREGIDO)"
echo ""
echo "  Velocidad SPI: 100000 Hz (unificada)"
echo ""
echo "  Correcciones aplicadas:"
echo "    ✓ Incluye <sys/select.h> y <sys/time.h>"
echo "    ✓ Eliminados proyectos previos antes de regenerar"
echo ""
echo "  Para compilar y ejecutar:"
echo "    sudo ./deploy.sh"
echo ""
echo "  O manual:"
echo "    cd mrf24_tx && make && sudo ./bin/mrf24_transmitter"
echo "    cd mrf24_rx && make && sudo ./bin/mrf24_receiver"
echo ""

print_success "¡Ahora debería compilar sin errores!"
