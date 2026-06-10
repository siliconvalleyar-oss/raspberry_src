#!/bin/bash
# =============================================================
# Agrega LED con PWM en GPIO12 usando la librería bcm2835
# Corregido: usar número de GPIO directamente
# =============================================================

set -e

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

if [ ! -d "mrf24_rx" ]; then
    echo "Error: No se encuentra el directorio mrf24_rx"
    exit 1
fi

print_header "Agregando LED PWM en GPIO12 usando bcm2835"

# Instalar bcm2835 si no está
if [ ! -f "/usr/local/include/bcm2835.h" ]; then
    print_warning "bcm2835 no encontrado. Instalando..."
    wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.77.tar.gz
    tar zxvf bcm2835-1.77.tar.gz
    cd bcm2835-1.77
    ./configure
    make
    sudo make check
    sudo make install
    cd ..
    rm -rf bcm2835-1.77 bcm2835-1.77.tar.gz
    print_success "bcm2835 instalado"
else
    print_success "bcm2835 ya instalado"
fi

# Backup
cp mrf24_rx/src/main.cpp mrf24_rx/src/main.cpp.bak.bcm2835

# Crear nuevo main.cpp con número de GPIO directo
cat > mrf24_rx/src/main.cpp << 'RX_BCM2835_EOF'
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

// Configuración del PWM para el LED en GPIO12
#define LED_GPIO    12               // Número de GPIO (12)
#define PWM_CHANNEL 0                // Canal 0 para GPIO 12 y 18
#define PWM_RANGE   1024
#define BLINK_COUNT 3
#define BLINK_DELAY_MS 80

static volatile bool running = true;
static Mrf24j40 radio;
static OLED display;
static std::ofstream logfile;

void sig_handler(int) { running = false; }

void blink_led(int count, int delay_ms) {
    for (int i = 0; i < count; i++) {
        bcm2835_pwm_set_data(PWM_CHANNEL, PWM_RANGE);
        bcm2835_delay(delay_ms);
        bcm2835_pwm_set_data(PWM_CHANNEL, 0);
        if (i < count - 1) bcm2835_delay(delay_ms);
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

void update_oled_display(int packet_num, const char* payload, uint8_t lqi, int8_t rssi) {
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
    
    if (!bcm2835_init()) {
        fprintf(stderr, "Error: bcm2835_init falló. Ejecuta con sudo.\n");
        return 1;
    }
    
    // Configurar GPIO12 como salida PWM (ALT5)
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_ALT5);
    
    // Configurar PWM
    bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_16);
    bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);
    bcm2835_pwm_set_range(PWM_CHANNEL, PWM_RANGE);
    bcm2835_pwm_set_data(PWM_CHANNEL, 0);
    
    printf("[PWM] GPIO12 configurado como PWM\n");
    
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║    MRF24J40 RECEPTOR AVANZADO v2.0     ║\n");
    printf("║        + LED PWM (GPIO12 - bcm2835)    ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
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
            
            blink_led(BLINK_COUNT, BLINK_DELAY_MS);
            
            if (oled_ok) {
                update_oled_display(packet_count, (char*)buffer, radio.getLQI(), radio.getRSSI());
            }
            
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
    
    if (oled_ok) {
        display.clear();
        display.draw_string(0, 20, "Terminado", 1, true);
        display.update();
    }
    
    // Apagar PWM y liberar
    bcm2835_pwm_set_mode(PWM_CHANNEL, 0, 0);
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_INPT);
    bcm2835_close();
    
    printf("\n[FIN] Total paquetes: %d\n", packet_count);
    return 0;
}
RX_BCM2835_EOF

print_success "main.cpp actualizado (GPIO12 como número directo)"

# Actualizar Makefile
sed -i 's/-lpigpio//' mrf24_rx/Makefile
sed -i 's/LDFLAGS  = -pthread/LDFLAGS  = -pthread -lbcm2835/' mrf24_rx/Makefile
print_success "Makefile actualizado"

# Script de prueba
cat > test_led_pwm_bcm2835.c << 'TEST_EOF'
#include <bcm2835.h>
#include <stdio.h>
#define LED_GPIO 12
#define PWM_CHANNEL 0
#define PWM_RANGE 1024

int main() {
    if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_ALT5);
    bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_16);
    bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);
    bcm2835_pwm_set_range(PWM_CHANNEL, PWM_RANGE);
    for (int i = 0; i < 5; i++) {
        bcm2835_pwm_set_data(PWM_CHANNEL, PWM_RANGE);
        bcm2835_delay(100);
        bcm2835_pwm_set_data(PWM_CHANNEL, 0);
        bcm2835_delay(100);
    }
    bcm2835_pwm_set_mode(PWM_CHANNEL, 0, 0);
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_INPT);
    bcm2835_close();
    printf("Prueba completada.\n");
    return 0;
}
TEST_EOF

cat > test_led_pwm_bcm2835.sh << 'TEST_SCRIPT_EOF'
#!/bin/bash
gcc -o test_led_pwm_bcm2835 test_led_pwm_bcm2835.c -lbcm2835
sudo ./test_led_pwm_bcm2835
TEST_SCRIPT_EOF

chmod +x test_led_pwm_bcm2835.sh
print_success "Script de prueba creado"

print_header "LED PWM CON BCM2835 AGREGADO"
echo ""
echo "  GPIO12 configurado como PWM usando bcm2835."
echo "  Conecta un LED (ánodo a GPIO12, cátodo a GND con resistencia ~220Ω)."
echo ""
echo "  Recompila: cd mrf24_rx && make clean && make"
echo "  Ejecuta:   sudo ./bin/mrf24_receiver"
echo ""
echo "  Para probar el LED: sudo ./test_led_pwm_bcm2835.sh"
echo ""
print_success "¡Listo!"
