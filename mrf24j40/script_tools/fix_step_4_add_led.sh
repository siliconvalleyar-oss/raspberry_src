#!/bin/bash
# =============================================================
# Fix LED y añadir envío de buffer grande (100 bytes)
# =============================================================

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_header() { echo -e "\n${BLUE}=================================================${NC}\n${BLUE}  $1${NC}\n${BLUE}=================================================${NC}"; }
print_success() { echo -e "${GREEN}✓${NC} $1"; }
print_warning() { echo -e "${YELLOW}⚠${NC} $1"; }

# Verificar directorios
if [ ! -d "mrf24_tx" ] || [ ! -d "mrf24_rx" ]; then
    echo "Error: No se encuentran mrf24_tx o mrf24_rx"
    exit 1
fi

print_header "Corrigiendo LED y añadiendo buffer grande"

# =============================================================
# 1. Modificar el transmisor para enviar payload de 100 bytes
# =============================================================
print_header "Modificando transmisor"

cat > mrf24_tx/src/main.cpp << 'TX_EOF'
#include <cstdio>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sys/select.h>
#include <sys/time.h>
#include "mrf24j40.h"

#define MY_ADDR     0x0001
#define DEST_ADDR   0x0002
#define PAN_ID      0xCAFE
#define CHANNEL     20
#define TX_INTERVAL_MS 2000

static volatile bool running = true;
static Mrf24j40 radio;
static int burst_count = 0;

void sig_handler(int) { running = false; }

void print_stats() {
    RadioStats stats;
    radio.getStats(stats);
    printf("\n=== ESTADÍSTICAS ===\n");
    printf("Enviados: %u, OK: %u, Fallos: %u, Retries: %u\n",
           stats.packets_sent, stats.tx_success, stats.tx_fail, stats.tx_retries_total);
}

// Enviar un buffer de tamaño fijo con datos de prueba
void send_buffer_test() {
    static uint8_t counter = 0;
    uint8_t buffer[100];  // Máximo payload definido en MAX_PAYLOAD (100)
    // Rellenar con datos de prueba: byte 0 = contador, luego bytes incrementales
    for (int i = 0; i < 100; i++) {
        buffer[i] = (counter + i) & 0xFF;
    }
    counter++;
    
    bool sent = radio.send(DEST_ADDR, PAN_ID, buffer, 100);
    if (!sent) {
        printf("[TX] Error al enviar\n");
        return;
    }
    
    int timeout = 500;
    while (timeout-- > 0 && !radio.txDone()) {
        radio.poll();
        usleep(5000);
    }
    
    if (radio.txDone()) {
        if (radio.txSuccess())
            printf("[TX] Buffer 100 bytes enviado OK (retries=%d)\n", radio.txRetries());
        else
            printf("[TX] Fallo (retries=%d)\n", radio.txRetries());
    } else {
        printf("[TX] Timeout\n");
    }
}

int main() {
    signal(SIGINT, sig_handler);
    printf("\n=== TRANSMISOR BUFFER GRANDE ===\n");
    if (!radio.init(CHANNEL)) return 1;
    radio.setPan(PAN_ID);
    radio.setShortAddress(MY_ADDR);
    radio.selfTest();
    printf("Enviando buffer de 100 bytes cada %d ms\n", TX_INTERVAL_MS);
    
    auto last_tx = std::chrono::steady_clock::now();
    while (running) {
        radio.poll();
        
        // Comandos
        fd_set readfds;
        struct timeval tv = {0,0};
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        if (select(STDIN_FILENO+1, &readfds, NULL, NULL, &tv) > 0) {
            char cmd = getchar();
            if (cmd == 's') print_stats();
            else if (cmd == 'q') running = false;
        }
        
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tx).count() >= TX_INTERVAL_MS) {
            send_buffer_test();
            last_tx = now;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    print_stats();
    return 0;
}
TX_EOF

print_success "Transmisor modificado: envía buffer de 100 bytes"

# =============================================================
# 2. Modificar receptor: mostrar buffer y usar LED simple (GPIO sin PWM)
# =============================================================
print_header "Modificando receptor: LED simple y buffer completo"

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
#include <bcm2835.h>
#include "mrf24j40.h"
#include "oled.hpp"

#define MY_ADDR     0x0002
#define PAN_ID      0xCAFE
#define CHANNEL     20

// LED en GPIO12 como salida digital (no PWM)
#define LED_GPIO    12
#define BLINK_COUNT 3
#define BLINK_DELAY_MS 80

static volatile bool running = true;
static Mrf24j40 radio;
static OLED display;
static std::ofstream logfile;

void sig_handler(int) { running = false; }

// Parpadeo con escritura digital
void blink_led(int count, int delay_ms) {
    for (int i = 0; i < count; i++) {
        bcm2835_gpio_write(LED_GPIO, HIGH);
        bcm2835_delay(delay_ms);
        bcm2835_gpio_write(LED_GPIO, LOW);
        if (i < count-1) bcm2835_delay(delay_ms);
    }
}

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
    printf("\n=== ESTADÍSTICAS RX ===\n");
    printf("Paquetes recibidos: %u\n", stats.packets_received);
    if (stats.rx_count > 0) {
        printf("LQI promedio: %.1f, RSSI promedio: %.1f dBm\n",
               (float)stats.rx_lqi_sum/stats.rx_count,
               (float)stats.rx_rssi_sum/stats.rx_count);
    }
}

void update_oled_display(int packet_num, const char* payload, int len, uint8_t lqi, int8_t rssi) {
    display.clear();
    display.draw_string(0, 0, "MRF24J40 RX", 1, true);
    char line2[32];
    snprintf(line2, sizeof(line2), "Paq #%d", packet_num);
    display.draw_string(0, 16, line2, 1, true);
    char line3[20];
    strncpy(line3, payload, 16);
    line3[16] = '\0';
    display.draw_string(0, 32, line3, 1, true);
    char line4[32];
    snprintf(line4, sizeof(line4), "LQI:%3d RSSI:%3ddBm", lqi, rssi);
    display.draw_string(0, 48, line4, 1, true);
    display.update();
}

void log_packet(int packet_num, const uint8_t* buffer, int len, uint8_t lqi, int8_t rssi) {
    if (!logfile.is_open()) return;
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    logfile << time_t << "," << packet_num << ",";
    for (int i = 0; i < len; i++) {
        logfile << std::hex << (int)buffer[i] << (i<len-1?":" : "");
    }
    logfile << std::dec << "," << len << "," << (int)lqi << "," << (int)rssi << "\n";
    logfile.flush();
}

int main() {
    signal(SIGINT, sig_handler);
    
    // Inicializar bcm2835
    if (!bcm2835_init()) {
        fprintf(stderr, "Error: bcm2835_init falló. ¿Ejecutaste con sudo?\n");
        return 1;
    }
    
    // Configurar GPIO12 como salida digital
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(LED_GPIO, LOW);
    printf("[LED] GPIO12 configurado como salida digital\n");
    
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║    MRF24J40 RECEPTOR BUFFER GRANDE     ║\n");
    printf("║         + LED (GPIO12)                 ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    // OLED opcional
    bool oled_ok = display.init();
    if (oled_ok) {
        display.showInitScreen();
        bcm2835_delay(500);
        display.clear();
        display.draw_string(0, 20, "MRF24J40 RX", 1, true);
        display.draw_string(0, 35, "Esperando...", 1, true);
        display.update();
    } else {
        printf("[WARN] OLED no detectado.\n");
    }
    
    if (!radio.init(CHANNEL)) {
        bcm2835_close();
        return 1;
    }
    
    radio.setPan(PAN_ID);
    radio.setShortAddress(MY_ADDR);
    radio.selfTest();
    
    printf("\n[CONFIG] PAN:0x%04X, Addr:0x%04X, Canal:%d, SPI:%u Hz\n",
           radio.getPan(), radio.getShortAddress(), CHANNEL, SPI_SPEED_HZ);
    
    logfile.open("mrf24_receiver.log", std::ios::app);
    if (logfile.is_open()) logfile << "#timestamp,packet_num,payload_hex,len,lqi,rssi\n";
    
    printf("\n[COMANDOS] s:stats, c:clear stats, q:salir\n\n");
    printf("[MAIN] Escuchando...\n");
    
    int packet_count = 0;
    uint8_t buffer[MAX_PAYLOAD];
    
    while (running) {
        radio.poll();
        
        // Comandos no bloqueantes
        fd_set readfds;
        struct timeval tv = {0,0};
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        if (select(STDIN_FILENO+1, &readfds, NULL, NULL, &tv) > 0) {
            char cmd = getchar();
            if (cmd == 's') print_stats();
            else if (cmd == 'c') radio.resetStats();
            else if (cmd == 'q') running = false;
        }
        
        if (radio.hasPacket()) {
            uint8_t len = radio.rxLen();
            radio.rxGet(buffer);
            packet_count++;
            
            print_timestamp();
            printf("Paquete #%d (%d bytes):\n", packet_count, len);
            // Mostrar primeros 20 bytes y últimos 20 si es grande
            printf("  Datos: ");
            for (int i = 0; i < (len < 40 ? len : 20); i++) printf("%02X ", buffer[i]);
            if (len > 40) printf("... ");
            for (int i = (len > 40 ? len-20 : 0); i < len; i++) printf("%02X ", buffer[i]);
            printf("\n");
            printf("  LQI: %d/255, RSSI: %d dBm\n", radio.getLQI(), radio.getRSSI());
            
            // Parpadeo LED
            blink_led(BLINK_COUNT, BLINK_DELAY_MS);
            
            if (oled_ok) {
                // Mostrar primeros caracteres como texto (si son imprimibles)
                char text[17] = {0};
                int txtlen = len > 16 ? 16 : len;
                for (int i = 0; i < txtlen; i++) text[i] = (buffer[i] >= 32 && buffer[i] <= 126) ? buffer[i] : '.';
                update_oled_display(packet_count, text, len, radio.getLQI(), radio.getRSSI());
            }
            
            log_packet(packet_count, buffer, len, radio.getLQI(), radio.getRSSI());
        }
        
        usleep(10000);
    }
    
    print_stats();
    radio.printRegisters();
    if (logfile.is_open()) logfile.close();
    if (oled_ok) {
        display.clear();
        display.draw_string(0, 20, "Terminado", 1, true);
        display.update();
    }
    bcm2835_close();
    printf("\n[FIN] Total paquetes: %d\n", packet_count);
    return 0;
}
RX_EOF

print_success "Receptor modificado: LED digital y muestra buffer completo"

# =============================================================
# 3. Actualizar Makefiles (ya incluyen bcm2835)
# =============================================================
print_header "Actualizando Makefiles"

for dir in mrf24_tx mrf24_rx; do
    sed -i 's/-lpigpio//' $dir/Makefile 2>/dev/null || true
    if ! grep -q "bcm2835" $dir/Makefile; then
        sed -i 's/LDFLAGS  = -pthread/LDFLAGS  = -pthread -lbcm2835/' $dir/Makefile
    fi
    # Asegurar que el transmisor también tenga bcm2835 (aunque no use LED, por si acaso)
    if [ "$dir" == "mrf24_tx" ] && ! grep -q "bcm2835" $dir/Makefile; then
        sed -i 's/LDFLAGS  = -pthread/LDFLAGS  = -pthread -lbcm2835/' $dir/Makefile
    fi
    print_success "$dir/Makefile actualizado"
done

# =============================================================
# 4. Script de prueba LED independiente
# =============================================================
cat > test_led_digital.c << 'TEST_EOF'
#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>

#define LED_GPIO 12

int main() {
    if (!bcm2835_init()) {
        printf("Error bcm2835_init\n");
        return 1;
    }
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_OUTP);
    printf("LED en GPIO12 parpadea 5 veces\n");
    for (int i = 0; i < 5; i++) {
        bcm2835_gpio_write(LED_GPIO, HIGH);
        usleep(200000);
        bcm2835_gpio_write(LED_GPIO, LOW);
        usleep(200000);
    }
    bcm2835_close();
    return 0;
}
TEST_EOF

cat > test_led_digital.sh << 'TEST_SCRIPT'
#!/bin/bash
gcc -o test_led_digital test_led_digital.c -lbcm2835
sudo ./test_led_digital
TEST_SCRIPT
chmod +x test_led_digital.sh

print_success "Script de prueba LED creado: ./test_led_digital.sh"

# =============================================================
# Resumen final
# =============================================================
print_header "CAMBIOS APLICADOS"
echo "1. Transmisor: envía buffer de 100 bytes con datos incrementales"
echo "2. Receptor:"
echo "   - LED en GPIO12 como salida digital (parpadeo simple)"
echo "   - Muestra el buffer completo en consola (hex)"
echo "   - Guarda log con datos hexadecimales"
echo "3. Se eliminó PWM problemático, se usa GPIO estándar"
echo ""
echo "Para probar:"
echo "  - Compila: cd mrf24_tx && make clean && make"
echo "            cd mrf24_rx && make clean && make"
echo "  - Prueba LED: sudo ./test_led_digital.sh"
echo "  - Ejecuta receptor: sudo ./mrf24_rx/bin/mrf24_receiver"
echo "  - Ejecuta transmisor: sudo ./mrf24_tx/bin/mrf24_transmitter"
echo ""
echo "El LED debería parpadear 3 veces por cada paquete recibido."
print_success "¡Listo!"
