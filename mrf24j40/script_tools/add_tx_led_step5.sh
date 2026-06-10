#!/bin/bash
# =============================================================
# Agrega LED en GPIO12 al TRANSMISOR (parpadea al enviar)
# Usa la misma librería bcm2835 que el receptor
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

# Verificar directorio del transmisor
if [ ! -d "mrf24_tx" ]; then
    echo "Error: No se encuentra el directorio mrf24_tx"
    exit 1
fi

print_header "Agregando LED de transmisión (GPIO12) al transmisor"

# Verificar que bcm2835 esté instalado (lo debería estar del receptor)
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

# Backup del main.cpp actual del transmisor
cp mrf24_tx/src/main.cpp mrf24_tx/src/main.cpp.bak.tx_led

# Modificar main.cpp del transmisor
cat > mrf24_tx/src/main.cpp << 'TX_LED_EOF'
#include <cstdio>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sys/select.h>
#include <sys/time.h>
#include <bcm2835.h>
#include "mrf24j40.h"

// Configuración de red
#define MY_ADDR     0x0001
#define DEST_ADDR   0x0002
#define PAN_ID      0xCAFE
#define CHANNEL     20
#define TX_INTERVAL_MS 2000

// LED en GPIO12 (pin físico 32)
#define LED_GPIO    12
#define BLINK_MS    50          // duración del parpadeo al enviar

static volatile bool running = true;
static Mrf24j40 radio;
static int burst_count = 0;

void sig_handler(int) { running = false; }

// Parpadeo rápido del LED
void blink_tx() {
    bcm2835_gpio_write(LED_GPIO, HIGH);
    bcm2835_delay(BLINK_MS);
    bcm2835_gpio_write(LED_GPIO, LOW);
}

void print_stats() {
    RadioStats stats;
    radio.getStats(stats);
    
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║              ESTADÍSTICAS TX                     ║\n");
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
        // Preparar payload de 100 bytes con datos incrementales
        uint8_t payload[100];
        for (int j = 0; j < 100; j++) {
            payload[j] = (burst_count + j) % 256;
        }
        burst_count++;
        
        radio.send(DEST_ADDR, radio.getPan(), payload, 100);
        
        int timeout = 100;
        while (timeout-- > 0 && radio.txDone() == false) {
            radio.poll();
            usleep(5000);
        }
        
        blink_tx();  // LED parpadea en cada envío
        
        if (delay_ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    
    print_stats();
}

void normal_transmission() {
    static int msg_num = 0;
    
    // Buffer de 100 bytes con datos incrementales
    uint8_t payload[100];
    for (int i = 0; i < 100; i++) {
        payload[i] = (msg_num + i) % 256;
    }
    
    radio.send(DEST_ADDR, radio.getPan(), payload, 100);
    
    int timeout = 500;
    while (timeout-- > 0 && radio.txDone() == false) {
        radio.poll();
        usleep(5000);
    }
    
    blink_tx();  // LED parpadea al enviar
    
    if (radio.txDone()) {
        if (radio.txSuccess())
            printf("✓ OK (retries=%d)\n", radio.txRetries());
        else
            printf("✗ FALLO (retries=%d)\n", radio.txRetries());
    } else {
        printf("⏱ TIMEOUT\n");
    }
    
    msg_num++;
}

int main() {
    signal(SIGINT, sig_handler);
    
    // Inicializar bcm2835
    if (!bcm2835_init()) {
        fprintf(stderr, "Error: bcm2835_init falló. Ejecuta con sudo.\n");
        return 1;
    }
    
    // Configurar GPIO12 como salida
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(LED_GPIO, LOW);
    printf("[LED] GPIO12 configurado como salida (parpadea al transmitir)\n");
    
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║    MRF24J40 TRANSMISOR v2.0            ║\n");
    printf("║         + LED TX (GPIO12)              ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    if (!radio.init(CHANNEL)) {
        bcm2835_close();
        return 1;
    }
    
    radio.setPan(PAN_ID);
    radio.setShortAddress(MY_ADDR);
    radio.selfTest();
    
    printf("\n[CONFIGURACION]\n");
    printf("  PAN ID: 0x%04X, Dirección: 0x%04X\n", radio.getPan(), radio.getShortAddress());
    printf("  Destino: 0x%04X, Canal: %d, SPI: %u Hz\n\n", DEST_ADDR, CHANNEL, SPI_SPEED_HZ);
    
    printf("[COMANDOS]\n");
    printf("  n - modo normal | b - burst (10 paq) | s - stats | q - salir\n\n");
    
    printf("[MAIN] Transmitiendo cada %d ms. Ctrl+C para salir.\n\n", TX_INTERVAL_MS);
    
    auto last_tx = std::chrono::steady_clock::now();
    enum Mode { MODE_NORMAL, MODE_BURST } current_mode = MODE_NORMAL;
    
    while (running) {
        radio.poll();
        
        // Comandos no bloqueantes
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
    
    // Apagar LED y liberar bcm2835
    bcm2835_gpio_write(LED_GPIO, LOW);
    bcm2835_close();
    
    printf("\n[FIN] Transmisor terminado.\n");
    return 0;
}
TX_LED_EOF

print_success "main.cpp del transmisor actualizado con LED"

# Actualizar Makefile del transmisor para linkear con bcm2835
sed -i 's/LDFLAGS  = -pthread/LDFLAGS  = -pthread -lbcm2835/' mrf24_tx/Makefile
print_success "Makefile del transmisor actualizado"

# Crear script de prueba para el LED del transmisor (opcional)
cat > test_tx_led.sh << 'TEST_TX_LED'
#!/bin/bash
echo "Probando LED del transmisor (GPIO12) - 5 parpadeos"
# Compilar programa de prueba simple
cat > test_tx_led.c << EOF
#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#define LED_GPIO 12
int main() {
    if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(LED_GPIO, BCM2835_GPIO_FSEL_OUTP);
    for (int i=0;i<5;i++) {
        bcm2835_gpio_write(LED_GPIO, HIGH);
        bcm2835_delay(100);
        bcm2835_gpio_write(LED_GPIO, LOW);
        bcm2835_delay(100);
    }
    bcm2835_close();
    return 0;
}
EOF
gcc -o test_tx_led test_tx_led.c -lbcm2835
sudo ./test_tx_led
rm -f test_tx_led test_tx_led.c
echo "Prueba completada. Si el LED no parpadeó, revisa conexión."
TEST_TX_LED

chmod +x test_tx_led.sh
print_success "Script test_tx_led.sh creado (para probar el LED independientemente)"

print_header "LED DE TRANSMISIÓN AGREGADO EXITOSAMENTE"
echo ""
echo "  El LED en GPIO12 parpadeará cada vez que el transmisor envíe un paquete."
echo ""
echo "  Pasos:"
echo "    1. Conecta un LED (ánodo a GPIO12, cátodo a GND con resistencia ~220Ω)."
echo "    2. Recompila el transmisor: cd mrf24_tx && make clean && make"
echo "    3. Ejecuta: sudo ./bin/mrf24_transmitter"
echo ""
echo "  Para probar el LED independientemente: sudo ./test_tx_led.sh"
echo ""
print_success "¡Listo!"

