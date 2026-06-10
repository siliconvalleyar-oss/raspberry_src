// ============================================================
// OBD-II Monitor para Raspberry Pi
// ELM327 Bluetooth + SSD1306 OLED (I2C via bcm2835)
// ============================================================
//
// Compilación:
//   ssh joy@raspberry.local "cd /home/joy/src/freebuff/elm327_oled_v1 && make"
//
// Ejecución:
//   sudo ./bin/obd_monitor            (requiere root para GPIO/BT/I2C)
//
// Controles:
//   Botón MODE (GPIO 17): Cambiar página de visualización
//   Botón ACT  (GPIO 27): Acción (borrar DTCs en página DTC)
//   Ctrl+C: Salir
// ============================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <bcm2835.h>

#include "gpio.hpp"
#include "BluetoothManager.hpp"
#include "SSD1306_OLED.hpp"
#include "comunicacion.hpp"
#include "funciones.hpp"

// ============================================================
// Variables globales
// ============================================================

static volatile bool g_running = true;
static BluetoothManager g_bt;
static SSD1306 g_display(128, 64);   // SSD1306 128x64 pixels

// ============================================================
// Manejador de signal para salida limpia
// ============================================================

static void signal_handler(int sig) {
    (void)sig;
    g_running = false;
}

// ============================================================
// Inicialización del display SSD1306 vía I2C (bcm2835)
// ============================================================

static int init_display() {
    printf("[INIT] Inicializando bcm2835...\n");

    if (!bcm2835_init()) {
        fprintf(stderr, "[ERROR] bcm2835_init falló (requiere sudo)\n");
        return -1;
    }

    printf("[INIT] Inicializando display SSD1306 I2C...\n");

    // Iniciar I2C
    if (!g_display.OLED_I2C_ON()) {
        fprintf(stderr, "[ERROR] OLED_I2C_ON falló\n");
        return -1;
    }

    // Inicializar OLED (I2C speed 0 = 100kHz, address 0x3C, debug=false)
    g_display.OLEDbegin(0, OLED_I2C_ADDR, false);

    // Configurar contraste máximo
    g_display.OLEDContrast(0xCF);

    // Limpiar pantalla
    g_display.OLEDclearBuffer();
    g_display.OLEDupdate();

    printf("[INIT] Display SSD1306 inicializado correctamente\n");

    return 0;
}

// ============================================================
// Inicialización Bluetooth
// ============================================================

static int init_bluetooth() {
    printf("[INIT] Inicializando Bluetooth...\n");

    if (g_bt.init() < 0) {
        fprintf(stderr, "[ERROR] No se pudo iniciar Bluetooth\n");
        return -1;
    }

    printf("[INIT] Buscando dispositivo ELM327...\n");
    draw_splash(&g_display, "Buscando ELM327...");

    int ret = g_bt.connect_bt();
    if (ret < 0) {
        fprintf(stderr, "[ERROR] No se pudo conectar: %s\n",
                g_bt.getLastError());
        draw_splash(&g_display, g_bt.getLastError());
        return -1;
    }

    printf("[INIT] Conectado a: %s [%s]\n",
           g_bt.getDeviceInfo()->name,
           g_bt.getDeviceInfo()->address);

    return 0;
}

// ============================================================
// Inicialización ELM327
// ============================================================

static void init_elm327() {
    printf("[INIT] Inicializando ELM327...\n");

    comm_set_bt(&g_bt);
    comm_init();

    draw_splash(&g_display, "Configurando ELM327...");

    // Procesar máquina de estados de inicialización ELM327
    int init_timeout = 30;  // 30 segundos máximo
    while (init_timeout > 0 && g_running) {
        bool done = comm_process_init();
        ELMState state = comm_get_state();

        // Mostrar estado en display
        char status[32];
        snprintf(status, sizeof(status), "ELM: %s", comm_state_string(state));
        draw_splash(&g_display, status);

        printf("[INIT] ELM327: %s\n", comm_state_string(state));

        if (done || state == ELM_INIT_DONE || state == ELM_MONITORING) {
            printf("[INIT] ELM327 listo (protocolo %d)\n", comm_get_protocol());
            draw_splash(&g_display, "Conectado!");
            return;
        }

        if (state == ELM_ERROR && g_bt.getState() != BT_CONN_ACTIVE) {
            fprintf(stderr, "[ERROR] ELM327: %s\n",
                    comm_get_last_error());
            draw_splash(&g_display, comm_get_last_error());
            return;
        }

        init_timeout--;
        struct timespec ts;
        ts.tv_sec = 1;
        ts.tv_nsec = 0;
        nanosleep(&ts, NULL);
    }

    if (init_timeout <= 0) {
        fprintf(stderr, "[ERROR] Timeout inicializando ELM327\n");
        draw_splash(&g_display, "Timeout ELM327");
    }
}

// ============================================================
// Loop de monitoreo
// ============================================================

static void run() {
    printf("[MONITOR] Iniciando monitoreo OBD-II...\n");

    // Mostrar splash final antes del monitoreo
    draw_splash(&g_display, "Monitoreando...");
    struct timespec ts;
    ts.tv_sec = 1;
    ts.tv_nsec = 0;
    nanosleep(&ts, NULL);

    // Configurar monitor
    MonitorConfig config;
    config.brightness = 0xCF;
    config.auto_page_ms = 0;       // Cambio manual con botón
    config.refresh_rate_hz = 5;     // 5 fps
    config.show_gm_pids = true;
    config.show_graphs = false;
    config.save_energy = false;

    // Iniciar UI con configuración
    ui_init(&g_display, &config);

    // Iniciar primera lectura de PIDs
    OBD2Data first_data;
    comm_read_all_pids(&first_data);

    // Ejecutar loop principal de monitoreo
    run_monitor_loop(&g_display);
}

// ============================================================
// Main
// ============================================================

int main(int argc, char* argv[]) {
    printf("============================================\n");
    printf("  OBD-II Monitor v1.0\n");
    printf("  Raspberry Pi + ELM327 Bluetooth + OLED\n");
    printf("============================================\n\n");

    // Configurar signals
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    (void)argc;
    (void)argv;

    // 1. Inicializar display
    if (init_display() < 0) {
        fprintf(stderr, "[FATAL] No se pudo inicializar el display\n");
        return 1;
    }

    // Splash inicial
    draw_splash(&g_display, "Iniciando...");

    // 2. Inicializar Bluetooth y conectar
    if (init_bluetooth() < 0) {
        fprintf(stderr, "[FATAL] No se pudo conectar Bluetooth\n");
        struct timespec ts;
        ts.tv_sec = 5;
        ts.tv_nsec = 0;
        nanosleep(&ts, NULL);
        g_bt.disconnect();
        g_display.OLEDclearBuffer();
        g_display.OLEDupdate();
        bcm2835_close();
        return 1;
    }

    // 3. Inicializar ELM327
    init_elm327();
    if (comm_get_state() == ELM_ERROR) {
        fprintf(stderr, "[FATAL] Error en inicialización ELM327\n");
        g_bt.disconnect();
        bcm2835_close();
        return 1;
    }

    // 4. Iniciar monitoreo
    run();

    // 5. Limpieza (si run_monitor_loop retorna)
    printf("\n[SALIR] Limpiando recursos...\n");
    g_bt.disconnect();
    g_display.OLEDPowerDown();
    bcm2835_close();

    printf("[SALIR] Hasta luego!\n");
    return 0;
}
