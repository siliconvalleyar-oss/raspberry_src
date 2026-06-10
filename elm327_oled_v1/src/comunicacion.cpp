#include "comunicacion.hpp"
#include "BluetoothManager.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

// ============================================================
// Variables globales del módulo
// ============================================================

static BluetoothManager* g_bt = NULL;
static ELMState g_state = ELM_DISCONNECTED;
static OBD2Data g_data;
static int g_init_step = 0;
static uint8_t g_pid_supported[4] = {0};  // PIDs soportados (4 bloques de 32)
static int g_protocol = 0;  // 0 = auto detectado
static int g_success = 0;
static int g_errors = 0;
static int g_timeouts = 0;
static char g_last_error[64] = {0};
static char g_rx_buffer[MAX_RESPONSE];
static uint64_t g_last_cmd_ms = 0;

// ============================================================
// Funciones auxiliares
// ============================================================

static uint64_t millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static bool has_elapsed(uint32_t ms) {
    return (millis() - g_last_cmd_ms) >= ms;
}

static void timer_start() {
    g_last_cmd_ms = millis();
}

static char* trim(char* s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    if (*s == '\0') return s;
    char* end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
        end--;
    *(end + 1) = '\0';
    return s;
}

// ============================================================
// Envío y recepción ELM327
// ============================================================

static int send_at(const char* cmd) {
    if (!g_bt || g_bt->getState() != BT_CONN_ACTIVE) return -1;

    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%s\r", cmd);
    return g_bt->send(buf, len);
}

static int recv_response(char* buffer, int max_len, int timeout_ms) {
    if (!g_bt) return -1;

    memset(buffer, 0, max_len);
    int total = 0;
    uint64_t start = millis();

    while ((millis() - start) < (uint64_t)timeout_ms) {
        char chunk[32];
        int n = g_bt->receive(chunk, sizeof(chunk) - 1, 50);
        if (n > 0) {
            chunk[n] = '\0';
            // Acumular
            int remaining = max_len - total - 1;
            if (remaining > n) remaining = n;
            if (remaining > 0) {
                memcpy(buffer + total, chunk, remaining);
                total += remaining;
                buffer[total] = '\0';

                // Verificar si la respuesta está completa (termina con ">")
                if (strchr(buffer, '>')) {
                    break;
                }
            }
        } else if (n < 0) {
            return -1;  // Error
        }
        // Pequeña pausa para no saturar CPU
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 5000000;
        nanosleep(&ts, NULL);
    }

    if (total == 0) {
        g_timeouts++;
        return 0;  // Timeout
    }

    // Limpiar respuesta: quitar eco, ">" y espacios extra
    // El buffer contiene la respuesta cruda
    // Buscar el prompt ">" para saber dónde termina
    char* prompt = strchr(buffer, '>');
    if (prompt) *prompt = '\0';

    return total;
}

static bool send_and_wait(const char* cmd, char* response, int max_len,
                          int timeout_ms) {
    if (send_at(cmd) < 0) return false;

    int n = recv_response(response, max_len, timeout_ms);
    if (n <= 0) return false;

    // Saltar líneas que contienen el comando enviado (eco del AT cmd)
    char* line = strtok(response, "\r\n");
    while (line) {
        char* trimmed = trim(line);
        if (strcasecmp(trimmed, cmd) != 0 && strlen(trimmed) > 0) {
            // Primera línea que no es el eco del comando
            strncpy(g_rx_buffer, trimmed, sizeof(g_rx_buffer)-1);
            return true;
        }
        line = strtok(NULL, "\r\n");
    }

    // Si todo estaba vacío o solo eco
    return false;
}

// ============================================================
// API pública
// ============================================================

void comm_set_bt(BluetoothManager* bt) {
    g_bt = bt;
}

void comm_init(void) {
    g_state = ELM_DISCONNECTED;
    g_init_step = 0;
    memset(&g_data, 0, sizeof(g_data));
    memset(g_pid_supported, 0, sizeof(g_pid_supported));
    g_success = 0;
    g_errors = 0;
    g_timeouts = 0;
    g_protocol = 0;
    g_last_error[0] = '\0';
}

ELMState comm_get_state(void) {
    return g_state;
}

const char* comm_state_string(ELMState s) {
    switch (s) {
        case ELM_DISCONNECTED:   return "Desconectado";
        case ELM_INIT_WAKE:      return "Iniciando...";
        case ELM_INIT_RESET:     return "Reset...";
        case ELM_INIT_SET_PROTO: return "Protocolo...";
        case ELM_INIT_HEADERS_OFF: return "Configurando...";
        case ELM_INIT_ECHO_OFF:  return "Eco OFF...";
        case ELM_INIT_SPACES_OFF: return "Spaces OFF...";
        case ELM_INIT_LINEFEED_OFF: return "Linefeed OFF...";
        case ELM_INIT_DONE:      return "Listo";
        case ELM_MONITORING:     return "Monitoreando";
        case ELM_ERROR:          return "Error";
        default:                 return "???";
    }
}

#define INIT_STEPS 7

bool comm_process_init(void) {
    if (!g_bt || g_bt->getState() != BT_CONN_ACTIVE) {
        g_state = ELM_DISCONNECTED;
        return false;
    }

    static char resp[MAX_RESPONSE];

    switch (g_init_step) {
        case 0: // ATZ - Reset ELM327
            g_state = ELM_INIT_WAKE;
            g_bt->flush();
            if (send_at("ATZ") < 0) { g_errors++; return false; }
            timer_start();
            g_init_step = 1;
            return false;

        case 1: // Esperar respuesta ATZ (puede tomar hasta 2s)
            g_state = ELM_INIT_RESET;
            if (!has_elapsed(2500)) return false;
            g_bt->flush(); // Limpiar cualquier respuesta residual
            g_init_step = 2;
            return false;

        case 2: // ATE0 - Eco OFF
            g_state = ELM_INIT_ECHO_OFF;
            if (!send_and_wait("ATE0", resp, sizeof(resp), AT_CMD_TIMEOUT)) {
                // Reintentar
                if (!send_and_wait("ATE0", resp, sizeof(resp), AT_CMD_TIMEOUT)) {
                    g_errors++;
                    snprintf(g_last_error, sizeof(g_last_error), "ATE0 falló");
                    g_init_step = 0;
                    return false;
                }
            }
            g_success++;
            g_init_step = 3;
            return false;

        case 3: // ATL0 - Linefeed OFF
            g_state = ELM_INIT_LINEFEED_OFF;
            send_and_wait("ATL0", resp, sizeof(resp), AT_CMD_TIMEOUT);
            g_success++;
            g_init_step = 4;
            return false;

        case 4: // ATS0 - Spaces OFF (respuesta sin espacios)
            g_state = ELM_INIT_SPACES_OFF;
            send_and_wait("ATS0", resp, sizeof(resp), AT_CMD_TIMEOUT);
            g_success++;
            g_init_step = 5;
            return false;

        case 5: // ATH0 - Headers OFF
            g_state = ELM_INIT_HEADERS_OFF;
            send_and_wait("ATH0", resp, sizeof(resp), AT_CMD_TIMEOUT);
            g_success++;
            g_init_step = 6;
            return false;

        case 6: // ATSP0 - Protocolo automático
            g_state = ELM_INIT_SET_PROTO;
            // Intentar primero ATSP0 (auto) para GM que usa VPW/CAN
            send_and_wait("ATSP0", resp, sizeof(resp), AT_CMD_TIMEOUT);
            g_success++;

            // Leer protocolo detectado
            send_and_wait("ATDPN", resp, sizeof(resp), AT_CMD_TIMEOUT);
            if (resp[0]) {
                g_protocol = strtol(resp, NULL, 10);
            } else {
                g_protocol = 0;
            }

            g_init_step = INIT_STEPS;
            g_state = ELM_INIT_DONE;
            return true;

        default:
            g_state = ELM_MONITORING;
            return true;
    }
}

// ============================================================
// Lectura de PIDs
// ============================================================

static uint8_t hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

static bool parse_pid_response(const char* resp, int* value) {
    if (!resp || !*resp) return false;

    // Respuesta esperada: "41 XX YY [ZZ ...]"
    // o "41XXYYZZ" (sin espacios si ATS0 activo)
    // o "XX YY ZZ" (con ATH0)  (respuesta cruda)
    // o "XXYYZZ"

    // Limpiar caracteres no hexadecimales
    char hex[32];
    int h = 0;
    for (int i = 0; resp[i] && h < 30; i++) {
        char c = resp[i];
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
            (c >= 'a' && c <= 'f')) {
            hex[h++] = c;
        }
    }
    hex[h] = '\0';

    if (h < 2) return false;

    // La respuesta debe tener al menos 2 hex digits
    // Formato típico: 41XXYYZZ (ATH0=off)
    // o XXYYZZ (ATH0=on)
    // La primera parte es el modo (41 = respuesta modo 01)
    // La segunda parte es el PID
    // El resto son los datos

    if (h >= 4) {
        // Si empieza con 41, saltar modo y PID (4 chars)
        int offset = 0;
        if (hex[0] == '4' && hex[1] == '1') {
            offset = 4; // saltar "41" + PID (2 chars)
        }

        if (h > offset) {
            *value = 0;
            for (int i = offset; i < h; i++) {
                *value = (*value << 4) | hex_val(hex[i]);
            }
            return true;
        }
    }

    return false;
}

int comm_read_pid(uint8_t mode, uint8_t pid) {
    if (g_state != ELM_INIT_DONE && g_state != ELM_MONITORING) return -1;
    if (!g_bt || g_bt->getState() != BT_CONN_ACTIVE) return -1;

    char cmd[16];
    snprintf(cmd, sizeof(cmd), "%02X%02X", mode, pid);

    char resp[MAX_RESPONSE];
    if (!send_and_wait(cmd, resp, sizeof(resp), PID_TIMEOUT)) {
        g_errors++;
        return -1;
    }

    g_success++;

    int value = 0;
    if (parse_pid_response(resp, &value)) {
        return value;
    }

    // Si falló parsing, intentar leer más datos
    char raw[MAX_RESPONSE];
    send_and_wait(cmd, raw, sizeof(raw), PID_TIMEOUT);
    if (parse_pid_response(raw, &value)) {
        return value;
    }

    return -1;
}

// ============================================================
// Lectura de todos los PIDs soportados
// ============================================================

int comm_read_all_pids(OBD2Data* data) {
    if (!data) return 0;
    memset(data, 0, sizeof(OBD2Data));

    // Leer PIDs soportados (modo 01, PID 00, 20, 40, 60)
    // Primero bloque: PIDs 01-20
    int pids = comm_read_pid(0x01, 0x00);
    if (pids >= 0) {
        // Los bits indican qué PIDs 01-20 están soportados
        for (int i = 0; i < 32 && i < 4; i++) {
            g_pid_supported[i] = (pids >> ((3-i)*8)) & 0xFF;
        }
    }

    // Leer PID 0x0C - RPM
    if (comm_is_pid_supported(0x0C)) {
        int val = comm_read_pid(0x01, 0x0C);
        if (val >= 0) {
            data->rpm = val / 4;  // ((A*256)+B)/4
        }
    }

    // Leer PID 0x0D - Speed
    if (comm_is_pid_supported(0x0D)) {
        int val = comm_read_pid(0x01, 0x0D);
        if (val >= 0) data->speed = val;
    }

    // Leer PID 0x05 - Coolant Temp
    if (comm_is_pid_supported(0x05)) {
        int val = comm_read_pid(0x01, 0x05);
        if (val >= 0) data->coolant_temp = val - 40;
    }

    // Leer PID 0x0F - Intake Temp
    if (comm_is_pid_supported(0x0F)) {
        int val = comm_read_pid(0x01, 0x0F);
        if (val >= 0) data->intake_temp = val - 40;
    }

    // Leer PID 0x10 - MAF Airflow
    if (comm_is_pid_supported(0x10)) {
        int val = comm_read_pid(0x01, 0x10);
        if (val >= 0) data->maf_airflow = val;  // ((A*256)+B)/100
    }

    // Leer PID 0x11 - Throttle Position
    if (comm_is_pid_supported(0x11)) {
        int val = comm_read_pid(0x01, 0x11);
        if (val >= 0) data->throttle_pos = val * 100 / 255;
    }

    // Leer PID 0x04 - Engine Load
    if (comm_is_pid_supported(0x04)) {
        int val = comm_read_pid(0x01, 0x04);
        if (val >= 0) data->engine_load = val * 100 / 255;
    }

    // Leer PID 0x0A - Fuel Pressure
    if (comm_is_pid_supported(0x0A)) {
        int val = comm_read_pid(0x01, 0x0A);
        if (val >= 0) data->fuel_pressure = val * 3;
    }

    // Leer PID 0x0E - Timing Advance
    if (comm_is_pid_supported(0x0E)) {
        int val = comm_read_pid(0x01, 0x0E);
        if (val >= 0) data->timing_advance = (val / 2) - 64;
    }

    // Leer PID 0x2F - Fuel Level
    if (comm_is_pid_supported(0x2F)) {
        int val = comm_read_pid(0x01, 0x2F);
        if (val >= 0) data->fuel_level = val * 100 / 255;
    }

    // Leer PID 0x33 - Barometric Pressure
    if (comm_is_pid_supported(0x33)) {
        int val = comm_read_pid(0x01, 0x33);
        if (val >= 0) data->barometric_press = val;
    }

    // Leer PID 0x42 - Control Module Voltage
    if (comm_is_pid_supported(0x42)) {
        int val = comm_read_pid(0x01, 0x42);
        if (val >= 0) data->control_module_voltage = val / 10; // Guardamos * 10
    }

    // Leer PID 0x46 - Ambient Temp
    if (comm_is_pid_supported(0x46)) {
        int val = comm_read_pid(0x01, 0x46);
        if (val >= 0) data->ambient_temp = val - 40;
    }

    // Leer PID 0x5C - Engine Oil Temperature
    if (comm_is_pid_supported(0x5C)) {
        int val = comm_read_pid(0x01, 0x5C);
        if (val >= 0) data->engine_oil_temp = val - 40;
    }

    // Leer PID 0x1F - Run Time
    if (comm_is_pid_supported(0x1F)) {
        int val = comm_read_pid(0x01, 0x1F);
        if (val >= 0) data->engine_runtime = val; // segundos
    }

    // Leer PID 0x21 - Distance with MIL
    if (comm_is_pid_supported(0x21)) {
        int val = comm_read_pid(0x01, 0x21);
        if (val >= 0) data->distance_mil = val;
    }

    // Leer PID 0x5E - Fuel Rate
    if (comm_is_pid_supported(0x5E)) {
        int val = comm_read_pid(0x01, 0x5E);
        if (val >= 0) data->fuel_rate = val; // ((A*256)+B)*0.01 L/h
    }

    // Leer PID 0x3C - Catalyst Temperature Bank 1 Sensor 1
    if (comm_is_pid_supported(0x3C)) {
        int val = comm_read_pid(0x01, 0x3C);
        if (val >= 0) data->cat_temp_b1s1 = (val / 10) - 40;
    }

    // Leer PID 0x3D - Catalyst Temperature Bank 1 Sensor 2
    if (comm_is_pid_supported(0x3D)) {
        int val = comm_read_pid(0x01, 0x3D);
        if (val >= 0) data->cat_temp_b1s2 = (val / 10) - 40;
    }

    // Leer PID 0x01 - MIL status
    if (comm_is_pid_supported(0x01)) {
        int val = comm_read_pid(0x01, 0x01);
        if (val >= 0) {
            data->mil_on = (val & 0x80) != 0;
            data->dtc_count = val & 0x7F;
        }
    }

    // Leer PID 0x43 - Calculated Load Value
    if (comm_is_pid_supported(0x43)) {
        int val = comm_read_pid(0x01, 0x43);
        if (val >= 0) data->calculated_load = val * 100 / 255;
    }

    // Leer PID 0x4F - Absolute Load
    if (comm_is_pid_supported(0x4F)) {
        int val = comm_read_pid(0x01, 0x4F);
        if (val >= 0) data->absolute_load = val; // ((A*256)+B)/2.55 %
    }

    // Chequear soporte general
    data->supported = true;

    // Actualizar datos globales
    memcpy(&g_data, data, sizeof(OBD2Data));

    return 1;
}

const OBD2Data* comm_get_data(void) {
    return &g_data;
}

bool comm_is_pid_supported(uint8_t pid) {
    if (pid >= 1 && pid <= 0x20) {
        return (g_pid_supported[0] >> (0x20 - pid)) & 1;
    }
    if (pid >= 0x21 && pid <= 0x40) {
        return (g_pid_supported[1] >> (0x40 - pid)) & 1;
    }
    if (pid >= 0x41 && pid <= 0x60) {
        return (g_pid_supported[2] >> (0x60 - pid)) & 1;
    }
    if (pid >= 0x61 && pid <= 0x80) {
        return (g_pid_supported[3] >> (0x80 - pid)) & 1;
    }
    return false;
}

// ============================================================
// DTCs
// ============================================================

int comm_read_dtc(uint16_t* dtcs, int max_dtcs) {
    if (g_state != ELM_MONITORING) return 0;

    char resp[MAX_RESPONSE];
    if (!send_and_wait("03", resp, sizeof(resp), AT_CMD_TIMEOUT)) {
        return 0;
    }

    // Parsear respuesta DTC
    // Formato: "43 XX YY ZZ WW ..."
    // Primero viene el conteo de DTCs, luego pares (XX YY) por cada DTC
    // El código DTC = ((XX-0x80)<<8) + YY  (pero usualmente XX e YY directos)
    // O simplemente XX*256 + YY donde XX es el byte alto y YY el byte bajo

    // Extraer hex
    char hex[64];
    int h = 0;
    for (int i = 0; resp[i] && h < 62; i++) {
        char c = resp[i];
        if (isxdigit(c)) hex[h++] = c;
    }
    hex[h] = '\0';

    if (h < 4) return 0;

    // Saltar "43" y leer pares
    int offset = 2; // saltar "43"
    int count = 0;

    while (offset + 4 <= h && count < max_dtcs) {
        uint8_t hi = hex_val(hex[offset]) * 16 + hex_val(hex[offset+1]);
        uint8_t lo = hex_val(hex[offset+2]) * 16 + hex_val(hex[offset+3]);
        dtcs[count++] = (hi << 8) | lo;
        offset += 4;
    }

    return count;
}

bool comm_clear_dtc(void) {
    if (g_state != ELM_MONITORING) return false;

    char resp[MAX_RESPONSE];
    // "04" = Clear DTCs
    bool ok = send_and_wait("04", resp, sizeof(resp), AT_CMD_TIMEOUT);
    if (ok) {
        g_success++;
    } else {
        g_errors++;
    }
    return ok;
}

// ============================================================
// Protocolo y estadísticas
// ============================================================

int comm_get_protocol(void) {
    return g_protocol;
}

void comm_set_protocol(int protocol) {
    if (protocol >= 0 && protocol <= 12) {
        char cmd[16];
        snprintf(cmd, sizeof(cmd), "ATSP%d", protocol);
        char resp[MAX_RESPONSE];
        send_and_wait(cmd, resp, sizeof(resp), AT_CMD_TIMEOUT);
        g_protocol = protocol;
        // Reinicializar máquina de estados
        g_init_step = INIT_STEPS;
        g_state = ELM_INIT_DONE;
    }
}

int comm_get_success_count(void) {
    return g_success;
}

int comm_get_error_count(void) {
    return g_errors;
}

int comm_get_timeout_count(void) {
    return g_timeouts;
}

void comm_reset_stats(void) {
    g_success = 0;
    g_errors = 0;
    g_timeouts = 0;
}

const char* comm_get_last_error(void) {
    return g_last_error;
}
