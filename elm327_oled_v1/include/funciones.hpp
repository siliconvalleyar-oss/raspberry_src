#ifndef FUNCIONES_HPP
#define FUNCIONES_HPP

#include <stdint.h>
#include <stdbool.h>

#include "SSD1306_OLED.hpp"
#include "comunicacion.hpp"

// ============================================================
// Display / UI del monitor OBD2
// ============================================================

// Páginas de visualización disponibles
enum DisplayPage {
    PAGE_DASHBOARD = 0,     // Dashboard principal (RPM + Velocidad + Temp)
    PAGE_GAUGES,            // Gauges tipo barra
    PAGE_DETAILS,           // Datos detallados en texto
    PAGE_FUEL_ECONOMY,      // Consumo y eficiencia
    PAGE_DTC,               // Diagnóstico / DTCs
    PAGE_INFO,              // Información del sistema
    PAGE_COUNT              // Número total de páginas
};

// Configuración del monitor
typedef struct {
    uint8_t  brightness;     // Brillo OLED (0-255)
    uint8_t  auto_page_ms;   // Auto-cambio de página (0 = desactivado)
    uint8_t  refresh_rate_hz; // Velocidad de refresco (1-20)
    bool     show_gm_pids;   // Mostrar PIDs específicos GM
    bool     show_graphs;    // Mostrar gráficos históricos
    bool     save_energy;    // Modo ahorro de energía (apagar display)
} MonitorConfig;

// Datos históricos para gráficos
#define HISTORY_SIZE 64
typedef struct {
    int16_t rpm[HISTORY_SIZE];
    int16_t speed[HISTORY_SIZE];
    int16_t coolant[HISTORY_SIZE];
    int16_t engine_load[HISTORY_SIZE];
    int16_t throttle[HISTORY_SIZE];
    int     head;       // Índice de escritura
    int     count;      // Elementos escritos
} HistoryData;

// ============================================================
// API pública
// ============================================================

// Inicializar la UI
// display: puntero al objeto SSD1306 ya inicializado
// config: configuración opcional (nullptr = defaults)
void ui_init(SSD1306* display, const MonitorConfig* config);

// Ejecutar el loop principal de monitoreo
// No retorna a menos que ocurra un error grave
void run_monitor_loop(SSD1306* display);

// === Funciones de dibujado de páginas ===

// Dibujar página: Dashboard (RPM, Velocidad, Temperatura)
void draw_page_dashboard(SSD1306* display, const OBD2Data* data);

// Dibujar página: Gauges tipo barra
void draw_page_gauges(SSD1306* display, const OBD2Data* data);

// Dibujar página: Datos detallados
void draw_page_details(SSD1306* display, const OBD2Data* data);

// Dibujar página: Consumo de combustible
void draw_page_fuel(SSD1306* display, const OBD2Data* data);

// Dibujar página: DTCs / Diagnóstico
void draw_page_dtc(SSD1306* display);

// Dibujar página: Información del sistema
void draw_page_info(SSD1306* display);

// === Funciones auxiliares ===

// Dibujar splash / pantalla de inicio
void draw_splash(SSD1306* display, const char* status);

// Dibujar barra de estado (Bluetooth, RPM, página)
void draw_status_bar(SSD1306* display, const OBD2Data* data, int page);

// Dibujar gauge analógico (semi-circular)
void draw_analog_gauge(SSD1306* display, int cx, int cy, int radius,
                       int value, int min_val, int max_val,
                       const char* label, const char* unit);

// Dibujar barra de progreso horizontal
void draw_progress_bar(SSD1306* display, int x, int y, int w, int h,
                       int value, int max_val, bool vertical);

// Dibujar número grande (2x altura)
void draw_big_number(SSD1306* display, int x, int y, int num, int digits);

// Actualizar datos históricos
void history_update(int rpm, int speed, int coolant, int load, int throttle);

// Obtener datos históricos
const HistoryData* history_get(void);

// Cambiar página
void ui_next_page(void);
void ui_prev_page(void);
int  ui_get_current_page(void);

// Configurar brillo del display
void ui_set_brightness(uint8_t brightness);

#endif // FUNCIONES_HPP
