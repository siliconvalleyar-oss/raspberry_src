#include "funciones.hpp"
#include "gpio.hpp"
#include "BluetoothManager.hpp"
#include "comunicacion.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

// ============================================================
// Variables estáticas
// ============================================================

static SSD1306* g_display = NULL;
static MonitorConfig g_config;
static DisplayPage g_current_page = PAGE_DASHBOARD;
static HistoryData g_history;
static uint64_t g_last_page_change = 0;
static int g_frame_count = 0;
static bool g_bt_connected = false;
static bool g_elm_ready = false;
static uint64_t g_start_time = 0;

// ============================================================
// Millis helper
// ============================================================

static uint64_t millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

// ============================================================
// GPIO Button Reading (opcional)
// ============================================================

static int read_gpio_pin(int pin) {
    // Lectura opcional de GPIO
    // En caso de no tener botones, retorna 1 (no presionado)
    (void)pin;
    return 1;
}

// ============================================================
// Implementación de draw_splash
// ============================================================

void draw_splash(SSD1306* display, const char* status) {
    if (!display) return;

    display->OLEDclearBuffer();
    display->setTextSize(1);
    display->setTextColor(WHITE);

    // Logo
    display->setCursor(8, 4);
    display->print("OBD-II MONITOR");

    // Línea decorativa
    display->drawLine(0, 16, 127, 16, WHITE);

    // Estado
    display->setCursor(8, 24);
    display->print("Status:");
    display->setCursor(8, 34);
    display->print(status ? status : "Iniciando...");

    // Info de sistema
    display->setCursor(8, 52);
    display->print("Raspberry Pi");

    display->OLEDupdate();
}

// ============================================================
// Implementación de ui_init
// ============================================================

void ui_init(SSD1306* display, const MonitorConfig* config) {
    g_display = display;
    g_current_page = PAGE_DASHBOARD;
    g_frame_count = 0;
    g_start_time = millis();

    // Config por defecto
    g_config.brightness = 255;
    g_config.auto_page_ms = 0;
    g_config.refresh_rate_hz = 5;
    g_config.show_gm_pids = true;
    g_config.show_graphs = false;
    g_config.save_energy = false;

    if (config) {
        g_config = *config;
    }

    // Inicializar historial
    memset(&g_history, 0, sizeof(g_history));

    // GPIOs ya configurados en main (opcional, no crítico)

    // Splash
    draw_splash(display, "Inicializando...");
}

// ============================================================
// Funciones de dibujado de páginas
// ============================================================

// --- Dashboard (página principal) ---
void draw_page_dashboard(SSD1306* display, const OBD2Data* data) {
    if (!display || !data) return;

    display->OLEDclearBuffer();

    // RPM - Número grande
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", data->rpm);
    display->setTextSize(2);
    display->setTextColor(WHITE);
    display->setCursor(4, 4);
    display->print(buf);
    display->setTextSize(1);
    display->setCursor(4, 28);
    display->print("RPM");

    // Barra de carga para RPM
    int rpm_bar = (data->rpm > 7000) ? 128 : (data->rpm * 128) / 7000;
    display->drawRect(0, 38, 128, 5, WHITE);
    if (rpm_bar > 0) {
        display->fillRect(1, 39, rpm_bar - 2, 3, WHITE);
    }

    // Velocidad
    snprintf(buf, sizeof(buf), "%d", data->speed);
    display->setCursor(68, 10);
    display->setTextSize(1);
    display->print("KM/H:");
    display->setTextSize(2);
    display->setCursor(68, 20);
    display->print(buf);

    // Temperatura del refrigerante
    int temp = data->coolant_temp;
    display->setCursor(4, 46);
    display->setTextSize(1);
    display->print("TEMP:");
    if (temp >= 0) {
        snprintf(buf, sizeof(buf), "%d C", temp);
    } else {
        snprintf(buf, sizeof(buf), "---");
    }
    display->setCursor(48, 46);
    display->print(buf);

    // Temperatura en color inverso si está sobrecalentado
    if (temp > 100) {
        display->fillRect(48, 46, 40, 8, WHITE);
        display->setTextColor(BLACK);
        display->setCursor(48, 46);
        display->print(buf);
        display->setTextColor(WHITE);
    }

    // Throttle / Carga
    display->setCursor(4, 56);
    snprintf(buf, sizeof(buf), "THR:%d LOAD:%d",
             data->throttle_pos, data->engine_load);
    display->print(buf);

    display->OLEDupdate();
}

// --- Gauges tipo barra ---
void draw_page_gauges(SSD1306* display, const OBD2Data* data) {
    if (!display || !data) return;

    display->OLEDclearBuffer();

    display->setTextSize(1);
    display->setTextColor(WHITE);

    // RPM Gauge
    display->setCursor(0, 0);
    display->print("RPM");
    int r_val = (data->rpm > 8000) ? 8000 : data->rpm;
    draw_progress_bar(display, 24, 0, 104, 6, r_val, 8000, false);

    // Speed Gauge
    display->setCursor(0, 8);
    display->print("KMH");
    int s_val = (data->speed > 200) ? 200 : data->speed;
    draw_progress_bar(display, 24, 8, 104, 6, s_val, 200, false);

    // Coolant Temp
    display->setCursor(0, 16);
    display->print("TEMP");
    int t_val = data->coolant_temp;
    if (t_val < 0) t_val = 0;
    if (t_val > 140) t_val = 140;
    draw_progress_bar(display, 24, 16, 104, 6, t_val, 140, false);

    // Throttle
    display->setCursor(0, 24);
    display->print("THR");
    draw_progress_bar(display, 24, 24, 104, 6, data->throttle_pos, 100, false);

    // Engine Load
    display->setCursor(0, 32);
    display->print("LOAD");
    draw_progress_bar(display, 24, 32, 104, 6, data->engine_load, 100, false);

    // Fuel Level
    display->setCursor(0, 40);
    display->print("FUEL");
    int f_val = (data->fuel_level > 100) ? 100 : data->fuel_level;
    if (f_val <= 0) f_val = 0;
    draw_progress_bar(display, 24, 40, 104, 6, f_val, 100, false);

    // Intake Temp
    display->setCursor(0, 48);
    display->print("INTAKE");
    int i_val = data->intake_temp;
    if (i_val < -40) i_val = -40;
    if (i_val > 100) i_val = 100;
    draw_progress_bar(display, 24, 48, 104, 6, i_val + 40, 140, false);

    // TPS + LOAD numérico
    char buf[32];
    snprintf(buf, sizeof(buf), "TPS:%d%%  LOAD:%d%%",
             data->throttle_pos, data->engine_load);
    display->setCursor(0, 56);
    display->print(buf);

    display->OLEDupdate();
}

void draw_progress_bar(SSD1306* display, int x, int y, int w, int h,
                        int value, int max_val, bool vertical) {
    if (!display || max_val <= 0) return;

    // Limitar valor
    if (value < 0) value = 0;
    if (value > max_val) value = max_val;

    // Calcular tamaño
    int fill;
    if (vertical) {
        fill = (h * value) / max_val;
        display->drawRect(x, y, w, h, WHITE);
        if (fill > 0) {
            display->fillRect(x + 1, y + h - fill, w - 2, fill, WHITE);
        }
    } else {
        fill = (w * value) / max_val;
        display->drawRect(x, y, w, h, WHITE);
        if (fill > 0) {
            display->fillRect(x + 1, y + 1, fill - 2, h - 2, WHITE);
        }
    }
}

// --- Página de detalles ---
void draw_page_details(SSD1306* display, const OBD2Data* data) {
    if (!display || !data) return;

    display->OLEDclearBuffer();
    display->setTextSize(1);
    display->setTextColor(WHITE);

    char buf[48];
    int y = 0;

    snprintf(buf, sizeof(buf), "RPM:%d SPD:%d", data->rpm, data->speed);
    display->setCursor(0, y); display->print(buf); y += 8;

    snprintf(buf, sizeof(buf), "CLT:%d IAT:%d OIL:%d",
             data->coolant_temp, data->intake_temp, data->engine_oil_temp);
    display->setCursor(0, y); display->print(buf); y += 8;

    snprintf(buf, sizeof(buf), "THR:%d LOAD:%d FL:%d",
             data->throttle_pos, data->engine_load, data->fuel_level);
    display->setCursor(0, y); display->print(buf); y += 8;

    snprintf(buf, sizeof(buf), "MAF:%d FUEL:%d", data->maf_airflow, data->fuel_rate);
    display->setCursor(0, y); display->print(buf); y += 8;

    snprintf(buf, sizeof(buf), "TIMING:%d FPRES:%d",
             data->timing_advance, data->fuel_pressure);
    display->setCursor(0, y); display->print(buf); y += 8;

    snprintf(buf, sizeof(buf), "VOLT:%d.%d", data->control_module_voltage/10,
             data->control_module_voltage % 10);
    display->setCursor(0, y); display->print(buf); y += 8;

    snprintf(buf, sizeof(buf), "BARO:%d AMB:%dC",
             data->barometric_press, data->ambient_temp);
    display->setCursor(0, y); display->print(buf); y += 8;

    if (data->mil_on) {
        display->fillRect(0, y, 128, 8, WHITE);
        display->setTextColor(BLACK);
        snprintf(buf, sizeof(buf), "CHECK ENGINE! DTCs:%d", data->dtc_count);
        display->setCursor(0, y); display->print(buf);
        display->setTextColor(WHITE);
    } else {
        display->setCursor(0, y);
        display->print("NO DTCs");
    }

    display->OLEDupdate();
}

// --- Página de combustible ---
void draw_page_fuel(SSD1306* display, const OBD2Data* data) {
    if (!display || !data) return;
    display->OLEDclearBuffer();
    display->setTextSize(1);
    display->setTextColor(WHITE);
    
    char buf[32];

    display->setCursor(0, 0);
    display->print("FUEL ECONOMY");

    // Fuel Level bar
    display->setCursor(0, 10);
    display->print("FUEL:");
    int f_val = (data->fuel_level > 100) ? 100 : data->fuel_level;
    if (f_val < 0) f_val = 0;
    draw_progress_bar(display, 36, 10, 88, 6, f_val, 100, false);

    snprintf(buf, sizeof(buf), "%d%%", f_val);
    display->setCursor(36 + ((88 * f_val) / 100), 10);
    display->print(buf);

    // Fuel Rate
    display->setCursor(0, 20);
    snprintf(buf, sizeof(buf), "FUEL RATE: %d.%d L/h",
             data->fuel_rate / 100, (data->fuel_rate % 100) / 10);
    display->print(buf);

    // MAF Airflow
    display->setCursor(0, 28);
    snprintf(buf, sizeof(buf), "MAF AIR: %d.%d g/s",
             data->maf_airflow / 100, data->maf_airflow % 100);
    display->print(buf);

    // Fuel Pressure
    display->setCursor(0, 36);
    snprintf(buf, sizeof(buf), "FUEL PRESS: %d kPa", data->fuel_pressure);
    display->print(buf);

    // Engine Runtime
    display->setCursor(0, 44);
    int hrs = data->engine_runtime / 3600;
    int mins = (data->engine_runtime % 3600) / 60;
    int secs = data->engine_runtime % 60;
    snprintf(buf, sizeof(buf), "RUN TIME: %02d:%02d:%02d", hrs, mins, secs);
    display->print(buf);

    // Oxygen sensor info (general)
    display->setCursor(0, 52);
    snprintf(buf, sizeof(buf), "O2 SENSORS: SUPPORTED");
    display->print(buf);

    display->OLEDupdate();
}

// --- Página DTC ---
void draw_page_dtc(SSD1306* display) {
    if (!display) return;

    display->OLEDclearBuffer();
    display->setTextSize(1);
    display->setTextColor(WHITE);

    // Leer DTCs
    uint16_t dtc_buffer[8];
    int n_dtcs = comm_read_dtc(dtc_buffer, 8);

    display->setCursor(0, 0);
    if (n_dtcs <= 0) {
        display->print("NO DTCs FOUND");
        display->setCursor(0, 10);
        display->print("System OK");

        // Mostrar información adicional
        const OBD2Data* data = comm_get_data();
        if (data && data->mil_on) {
            display->setCursor(0, 24);
            display->print("MIL is ON but no");
            display->setCursor(0, 32);
            display->print("DTCs stored!");
        }
    } else {
        char buf[32];
        snprintf(buf, sizeof(buf), "DTCs: %d", n_dtcs);
        display->print(buf);

        for (int i = 0; i < n_dtcs && i < 6; i++) {
            uint16_t code = dtc_buffer[i];
            char letter;
            switch ((code >> 12) & 0x0F) {
                case 0: letter = 'P'; break; // Powertrain
                case 1: letter = 'C'; break; // Chassis
                case 2: letter = 'B'; break; // Body
                case 3: letter = 'U'; break; // Network
                default: letter = 'P';
            }
            int num = code & 0x0FFF;
            snprintf(buf, sizeof(buf), "%c%04d", letter, num);
            display->setCursor(0, (i + 1) * 8 + 2);
            display->print(buf);
        }

        // Opción para borrar
        display->setCursor(0, 56);
        display->print("[ACT] Clear DTCs");
    }

    display->OLEDupdate();
}

// --- Página de información ---
void draw_page_info(SSD1306* display) {
    if (!display) return;

    display->OLEDclearBuffer();
    display->setTextSize(1);
    display->setTextColor(WHITE);

    char buf[32];
    int y = 0;

    display->setCursor(0, y); display->print("OBD-II MONITOR v1.0"); y += 8;
    display->drawLine(0, y, 128, y, WHITE); y += 1;

    // Estado Bluetooth
    snprintf(buf, sizeof(buf), "BT: %s", g_bt_connected ? "OK" : "NO");
    display->setCursor(0, y); display->print(buf); y += 8;

    // Estado ELM327
    snprintf(buf, sizeof(buf), "ELM: %s", g_elm_ready ? "OK" : "NO");
    display->setCursor(0, y); display->print(buf); y += 8;

    // Protocolo
    int proto = comm_get_protocol();
    const char* proto_names[] = {
        "Auto", "SAE J1850PWM", "SAE J1850VPW", "ISO 9141-2",
        "ISO 14230KWP", "ISO 14230KWP", "ISO 15765 CAN",
        "ISO 15765 CAN", "ISO 15765 CAN", "ISO 15765 CAN",
        "SAE J1939 CAN", "USER1  CAN", "USER2  CAN"
    };
    if (proto >= 0 && proto <= 12) {
        snprintf(buf, sizeof(buf), "PROTO: %s", proto_names[proto]);
    } else {
        snprintf(buf, sizeof(buf), "PROTO: %d (?)", proto);
    }
    display->setCursor(0, y); display->print(buf); y += 8;

    // Estadísticas
    snprintf(buf, sizeof(buf), "OK: %d  ERR: %d  TO: %d",
             comm_get_success_count(), comm_get_error_count(),
             comm_get_timeout_count());
    display->setCursor(0, y); display->print(buf); y += 8;

    // Tiempo de ejecución
    uint64_t elapsed = (millis() - g_start_time) / 1000;
    int hrs = elapsed / 3600;
    int mins = (elapsed % 3600) / 60;
    int secs = elapsed % 60;
    snprintf(buf, sizeof(buf), "UPTIME: %02d:%02d:%02d", hrs, mins, secs);
    display->setCursor(0, y); display->print(buf); y += 8;

    // FPS
    display->setCursor(0, y);
    snprintf(buf, sizeof(buf), "FPS: %d", g_frame_count > 0 ? g_frame_count : 0);
    display->print(buf);

    display->OLEDupdate();
}

// ============================================================
// Barra de estado
// ============================================================

void draw_status_bar(SSD1306* display, const OBD2Data* data, int page) {
    if (!display) return;

    char buf[16];

    // Indicador Bluetooth
    snprintf(buf, sizeof(buf), "%s", g_bt_connected ? "BT" : "..");
    display->setCursor(0, 0);
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->print(buf);

    // Número de página
    snprintf(buf, sizeof(buf), "[%d/%d]", page + 1, PAGE_COUNT);
    display->setCursor(100, 0);
    display->print(buf);

    // RPM (si hay datos)
    if (data && data->supported) {
        snprintf(buf, sizeof(buf), "%d RPM", data->rpm);
        display->setCursor(48, 0);
        display->print(buf);
    }
}

// ============================================================
// Navegación de páginas
// ============================================================

void ui_next_page(void) {
    g_current_page = (DisplayPage)((g_current_page + 1) % PAGE_COUNT);
    g_last_page_change = millis();
}

void ui_prev_page(void) {
    g_current_page = (DisplayPage)((g_current_page - 1 + PAGE_COUNT) % PAGE_COUNT);
    g_last_page_change = millis();
}

int ui_get_current_page(void) {
    return (int)g_current_page;
}

// ============================================================
// Brillo
// ============================================================

void ui_set_brightness(uint8_t brightness) {
    g_config.brightness = brightness;
    if (g_display) {
        // SSD1306 brightness via contrast command
        g_display->OLEDContrast(brightness);
    }
}

// ============================================================
// Historial
// ============================================================

void history_update(int rpm, int speed, int coolant, int load, int throttle) {
    g_history.rpm[g_history.head] = rpm;
    g_history.speed[g_history.head] = speed;
    g_history.coolant[g_history.head] = coolant;
    g_history.engine_load[g_history.head] = load;
    g_history.throttle[g_history.head] = throttle;

    g_history.head = (g_history.head + 1) % HISTORY_SIZE;
    if (g_history.count < HISTORY_SIZE) {
        g_history.count++;
    }
}

const HistoryData* history_get(void) {
    return &g_history;
}

// ============================================================
// Analogue Gauge (semi-circular)
// ============================================================

void draw_analog_gauge(SSD1306* display, int cx, int cy, int radius,
                        int value, int min_val, int max_val,
                        const char* label, const char* unit) {
    if (!display || max_val <= min_val) return;

    (void)unit;

    // Arco semicircular (de -180 a 0 grados, en radianes es de -PI a 0)
    // En la pantalla: izquierda a derecha

    // Dibujar arco exterior
    for (int angle = 180; angle <= 360; angle += 3) {
        double rad = angle * 3.14159 / 180.0;
        int x = cx + (int)(radius * cos(rad));
        int y = cy + (int)(radius * sin(rad));
        display->drawPixel(x, y, WHITE);
    }

    // Calcular ángulo del valor
    int range = max_val - min_val;
    int val_clamped = value;
    if (val_clamped < min_val) val_clamped = min_val;
    if (val_clamped > max_val) val_clamped = max_val;

    int angle = 180 + ((val_clamped - min_val) * 180) / range;
    double rad = angle * 3.14159 / 180.0;
    int needle_x = cx + (int)(radius * cos(rad));
    int needle_y = cy + (int)(radius * sin(rad));

    // Dibujar aguja
    display->drawLine(cx, cy, needle_x, needle_y, WHITE);
    display->fillCircle(cx, cy, 2, WHITE);

    // Label
    if (label) {
        display->setCursor(cx - 12, cy + 4);
        display->setTextSize(1);
        display->print(label);
    }

    // Valor
    char val_str[8];
    snprintf(val_str, sizeof(val_str), "%d", val_clamped);
    display->setCursor(cx - 8, cy - 12);
    display->print(val_str);
}

void draw_big_number(SSD1306* display, int x, int y, int num, int digits) {
    if (!display) return;

    char fmt[8];
    char buf[16];
    snprintf(fmt, sizeof(fmt), "%%%dd", digits);
    snprintf(buf, sizeof(buf), fmt, num);

    display->setCursor(x, y);
    display->setTextSize(2);
    display->print(buf);
    display->setTextSize(1);
}

// ============================================================
// Loop principal de monitoreo
// ============================================================

void run_monitor_loop(SSD1306* display) {
    if (!display) return;

    const int TARGET_FRAME_MS = 1000 / g_config.refresh_rate_hz;
    uint64_t last_frame = millis();
    uint64_t last_pid_read = 0;
    OBD2Data current_data;
    memset(&current_data, 0, sizeof(current_data));
    bool data_valid = false;

    while (1) {
        uint64_t now = millis();

        // --- 1. Leer botones (cambio de página) ---
        static int prev_mode = 1, prev_act = 1;
        int mode = read_gpio_pin(BTN_MODE_PIN);
        int act = read_gpio_pin(BTN_ACT_PIN);

        if (prev_mode == 0 && mode == 1) {
            ui_next_page();
        }
        if (prev_act == 0 && act == 1) {
            // En página DTC: borrar códigos
            if (g_current_page == PAGE_DTC) {
                comm_clear_dtc();
            }
        }
        prev_mode = mode;
        prev_act = act;

        // --- 2. Auto-cambio de página ---
        if (g_config.auto_page_ms > 0 &&
            (now - g_last_page_change) > g_config.auto_page_ms * 1000) {
            ui_next_page();
        }

        // --- 3. Leer PIDs OBD2 cada ~500ms ---
        if (now - last_pid_read > 500) {
            comm_read_all_pids(&current_data);
            if (current_data.supported) {
                data_valid = true;
                history_update(current_data.rpm, current_data.speed,
                               current_data.coolant_temp,
                               current_data.engine_load,
                               current_data.throttle_pos);
            }
            last_pid_read = now;
        }

        // --- 4. Actualizar estados BT/ELM ---
        g_bt_connected = true; // Asumimos conectado si estamos en el loop
        g_elm_ready = (comm_get_state() == ELM_MONITORING ||
                       comm_get_state() == ELM_INIT_DONE);

        // --- 5. Dibujar página actual ---
        if ((now - last_frame) >= (uint64_t)TARGET_FRAME_MS) {
            const OBD2Data* draw_data = data_valid ? &current_data : NULL;

            switch (g_current_page) {
                case PAGE_DASHBOARD:
                    draw_page_dashboard(display, draw_data);
                    break;
                case PAGE_GAUGES:
                    draw_page_gauges(display, draw_data);
                    break;
                case PAGE_DETAILS:
                    draw_page_details(display, draw_data);
                    break;
                case PAGE_FUEL_ECONOMY:
                    draw_page_fuel(display, draw_data);
                    break;
                case PAGE_DTC:
                    draw_page_dtc(display);
                    break;
                case PAGE_INFO:
                    draw_page_info(display);
                    break;
                default:
                    draw_page_dashboard(display, draw_data);
                    break;
            }

            g_frame_count++;
            last_frame = now;
        }

        // --- 6. Pausa para no saturar CPU ---
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20000000;  // 20ms
        nanosleep(&ts, NULL);
    }
}
