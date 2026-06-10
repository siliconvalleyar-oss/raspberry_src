#ifndef COMUNICACION_HPP
#define COMUNICACION_HPP

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// ELM327 OBD2 Communication Manager
// ============================================================
// Soporta protocolo estándar OBD2 + PIDs específicos Chevrolet GM
// ============================================================

// Máximos
#define MAX_PIDS          32
#define MAX_RESPONSE      128
#define AT_CMD_TIMEOUT    2000  // ms
#define PID_TIMEOUT       1000  // ms

// Estados del adaptador ELM327
enum ELMState {
    ELM_DISCONNECTED = 0,
    ELM_INIT_WAKE,          // Enviando ATZ
    ELM_INIT_RESET,         // Esperando respuesta ATZ
    ELM_INIT_SET_PROTO,     // Configurando protocolo
    ELM_INIT_HEADERS_OFF,   // ATH0
    ELM_INIT_ECHO_OFF,      // ATE0
    ELM_INIT_SPACES_OFF,    // ATS0
    ELM_INIT_LINEFEED_OFF,  // ATL0
    ELM_INIT_DONE,          // Inicializado y listo
    ELM_MONITORING,         // Monitoreando activamente
    ELM_ERROR               // Error irrecuperable
};

// Datos OBD2 estándar (modo 01)
typedef struct {
    bool    supported;      // PID soportado por el vehículo
    
    // PIDs estándar SAE J1979
    int16_t rpm;            // PID 0x0C  (RPM * 0.25)
    int16_t speed;          // PID 0x0D  (km/h)
    int16_t coolant_temp;   // PID 0x05  (°C)
    int16_t intake_temp;    // PID 0x0F  (°C)
    int16_t maf_airflow;    // PID 0x10  (g/s * 0.01)
    int16_t throttle_pos;   // PID 0x11  (%)
    int16_t engine_load;    // PID 0x04  (%)
    int16_t fuel_pressure;  // PID 0x0A  (kPa)
    int16_t timing_advance; // PID 0x0E  (° antes del TDC)
    int16_t fuel_level;     // PID 0x2F  (%)
    int16_t barometric_press; // PID 0x33 (kPa)
    int16_t control_module_voltage; // PID 0x42 (V * 10)
    int16_t ambient_temp;   // PID 0x46 (°C)
    int16_t engine_oil_temp; // PID 0x5C (°C)
    int16_t engine_runtime; // PID 0x1F (minutos)
    int16_t distance_mil;   // PID 0x21 (km)
    int16_t fuel_rate;      // PID 0x5E (L/h * 0.01)
    int16_t cat_temp_b1s1;  // PID 0x3C (°C)
    int16_t cat_temp_b1s2;  // PID 0x3D (°C)
    
    // GM-specific enhanced PIDs
    int16_t trans_temp;          // GM PID (AT 0xC1 o similar)
    int16_t battery_voltage;     // Calculado o PID específico
    int16_t instant_fuel_economy; // Calculado (km/L)
    int16_t average_fuel_economy; // Calculado
    int16_t overall_fuel_economy; // Calculado
    
    // Flags
    bool    mil_on;         // Check Engine encendido
    uint8_t dtc_count;      // Número de DTCs
    int16_t calculated_load; // PID 0x43 (%)
    int16_t absolute_load;  // PID 0x4F (%)
} OBD2Data;

// Resultado de un comando ELM327
typedef struct {
    char    raw[MAX_RESPONSE];
    int     length;
    bool    ok;             // Respuesta "OK" o con datos válidos
    bool    timeout;        // Timeout en la respuesta
} ELMResponse;

// ============================================================
// API pública
// ============================================================

// Inicializar comunicación (requiere BluetoothManager conectado)
void comm_init(void);

// Asignar BluetoothManager (llamar antes de comm_init)
void comm_set_bt(class BluetoothManager* bt);

// Obtener estado del ELM327
ELMState comm_get_state(void);

// Obtener string descriptivo del estado
const char* comm_state_string(ELMState s);

// Procesar máquina de estados de inicialización
// Llamar periódicamente desde el loop principal
// Retorna true cuando la inicialización está completa
bool comm_process_init(void);

// Leer todos los PIDs OBD2 soportados
// Almacena resultados en la estructura data
// Retorna número de PIDs leídos exitosamente
int comm_read_all_pids(OBD2Data* data);

// Leer PID específico
// mode = 0x01 (modo 01 - datos actuales)
// pid  = código del PID
// Retorna el valor A/D combinado, o < 0 en error
int comm_read_pid(uint8_t mode, uint8_t pid);

// Obtener datos actuales (última lectura)
const OBD2Data* comm_get_data(void);

// Verificar si un PID específico está soportado
bool comm_is_pid_supported(uint8_t pid);

// Leer códigos de error DTC
// Retorna número de DTCs encontrados (hasta max_dtcs)
// Cada DTC se almacena como uint16_t (ej: 0x0100 = P0100)
int comm_read_dtc(uint16_t* dtcs, int max_dtcs);

// Limpiar códigos de error (borrar DTCs / check engine)
bool comm_clear_dtc(void);

// Obtener protocolo OBD2 detectado
int comm_get_protocol(void);

// Forzar un protocolo específico (1-12, 0 = auto)
void comm_set_protocol(int protocol);

// Estadísticas de comunicación
int  comm_get_success_count(void);
int  comm_get_error_count(void);
int  comm_get_timeout_count(void);
void comm_reset_stats(void);

// Último error como string
const char* comm_get_last_error(void);

#endif // COMUNICACION_HPP
