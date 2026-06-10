#ifndef BLUETOOTH_MANAGER_HPP
#define BLUETOOTH_MANAGER_HPP

#include <stdint.h>
#include <string>

// Estados de la conexión Bluetooth
// NOTA: NO usar BT_CONNECTED como nombre - conflictúa con macro en bluetooth.h
enum BTState {
    BT_DISCONNECTED = 0,
    BT_SCANNING,         // Buscando dispositivo ELM327
    BT_CONNECTING,       // Conectando vía RFCOMM
    BT_CONN_ACTIVE,      // Conectado y listo
    BT_ERROR             // Error de conexión
};

// Información del dispositivo Bluetooth
struct BTDeviceInfo {
    char     address[18];    // Dirección MAC (ej: "00:1D:A5:00:00:00")
    char     name[64];       // Nombre del dispositivo
    uint8_t  channel;        // Canal RFCOMM
};

class BluetoothManager {
public:
    BluetoothManager();
    ~BluetoothManager();

    // Inicializar y escanear dispositivos Bluetooth
    // Retorna 0 en éxito, < 0 en error
    int init();

    // Escanear y conectarse al primer ELM327 encontrado
    // Retorna 0 en éxito, < 0 en error
    int connect_bt(const char* target_addr = nullptr);

    // Desconectar y liberar recursos
    void disconnect();

    // Enviar datos al ELM327
    // Retorna número de bytes escritos, < 0 en error
    int send(const char* data, int len);

    // Recibir datos del ELM327 con timeout en milisegundos
    // Retorna número de bytes leídos, 0 si timeout, < 0 en error
    int receive(char* buffer, int max_len, int timeout_ms);

    // Vaciar buffer de entrada
    void flush();

    // Obtener estado actual
    BTState getState() const { return state; }

    // Obtener información del dispositivo conectado
    const BTDeviceInfo* getDeviceInfo() const { return &device; }

    // Obtener último error como string
    const char* getLastError() const { return last_error; }

    // Número de reintentos de conexión realizados
    int getRetryCount() const { return retry_count; }

private:
    int  open_rfcomm_socket(const char* addr, uint8_t channel);
    int  scan_devices(BTDeviceInfo* devices, int max_devices);
    bool is_elm327_device(const char* name);

    BTState     state;
    BTDeviceInfo device;
    int         sock_fd;        // Socket RFCOMM
    int         retry_count;
    int         max_retries;
    char        last_error[128];

    static const int SCAN_TIMEOUT = 10;      // segundos para escaneo BT
    static const int CONNECT_TIMEOUT = 15;   // segundos para conexión
    static const int RECV_TIMEOUT = 3000;    // ms para recepción
};

#endif // BLUETOOTH_MANAGER_HPP
