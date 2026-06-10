#include "BluetoothManager.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <time.h>

// Bluetooth headers
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>

// Definir SPP UUID manualmente para evitar problemas de API SDP
#define SPP_UUID_STR "00001101-0000-1000-8000-00805F9B34FB"

// ============================================================
// Constructor / Destructor
// ============================================================

BluetoothManager::BluetoothManager()
    : state(BT_DISCONNECTED)
    , sock_fd(-1)
    , retry_count(0)
    , max_retries(3)
{
    memset(&device, 0, sizeof(device));
    memset(last_error, 0, sizeof(last_error));
}

BluetoothManager::~BluetoothManager() {
    disconnect();
}

// ============================================================
// Inicialización
// ============================================================

int BluetoothManager::init() {
    state = BT_DISCONNECTED;
    retry_count = 0;
    return 0;
}

// ============================================================
// Escaneo de dispositivos Bluetooth
// ============================================================

bool BluetoothManager::is_elm327_device(const char* name) {
    if (!name) return false;

    // El ELM327 aparece con nombres como:
    //   "OBDII", "OBD2", "ELM327", "SCANTOOL", "V_LINK"
    // También puede tener nombres personalizados
    const char* keywords[] = {
        "OBD", "ELM", "SCAN", "V_LINK", "AUTOPHIX", "VEEPEAK",
        "BAFX", "KIWI", "PLX", "BLUE", "CAR",
        "MOST" // MostPlus, iCarSoft, etc
    };
    int n_keywords = sizeof(keywords) / sizeof(keywords[0]);

    // Convertir a mayúsculas para comparación
    char upper[64];
    int i;
    for (i = 0; name[i] && i < 63; i++) {
        char c = name[i];
        if (c >= 'a' && c <= 'z')
            upper[i] = c - 'a' + 'A';
        else
            upper[i] = c;
    }
    upper[i] = '\0';

    // Buscar cualquier keyword
    for (int k = 0; k < n_keywords; k++) {
        if (strstr(upper, keywords[k]))
            return true;
    }
    return false;
}

int BluetoothManager::scan_devices(BTDeviceInfo* devices, int max_devices) {
    state = BT_SCANNING;

    // Abrir HCI socket
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0) {
        snprintf(last_error, sizeof(last_error),
                 "hci_get_route: %s (¿Bluetooth activo?)", strerror(errno));
        return -1;
    }

    int hci_sock = hci_open_dev(dev_id);
    if (hci_sock < 0) {
        snprintf(last_error, sizeof(last_error),
                 "hci_open_dev: %s", strerror(errno));
        return -1;
    }

    // Configurar inquiry
    inquiry_info* ii = (inquiry_info*)malloc(max_devices * sizeof(inquiry_info));
    if (!ii) {
        close(hci_sock);
        snprintf(last_error, sizeof(last_error), "malloc inquiry_info failed");
        return -1;
    }

    int num_rsp = hci_inquiry(dev_id, SCAN_TIMEOUT,
                              max_devices, NULL, &ii, IREQ_CACHE_FLUSH);
    if (num_rsp < 0) {
        free(ii);
        close(hci_sock);
        snprintf(last_error, sizeof(last_error),
                 "hci_inquiry: %s (sin dispositivos encontrados)", strerror(errno));
        return -1;
    }

    // Procesar resultados
    int found = 0;
    for (int i = 0; i < num_rsp && found < max_devices; i++) {
        // Obtener nombre del dispositivo
        char name[64] = {0};
        if (hci_read_remote_name(hci_sock, &ii[i].bdaddr, sizeof(name)-1,
                                 name, 0) < 0) {
            continue;
        }
        name[sizeof(name)-1] = '\0';

        // Buscar canal RFCOMM:
        // En BlueZ moderno, el canal SPP suele ser 1 para la mayoría de ELM327
        // También intentar obtenerlo via hci (más simple que SDP)

        // Si el nombre sugiere OBD, lo agregamos con canal por defecto
        if (is_elm327_device(name)) {
            ba2str(&ii[i].bdaddr, devices[found].address);
            snprintf(devices[found].name, sizeof(devices[found].name), "%s", name);
            devices[found].channel = 1;  // La mayoría de ELM327 usan canal 1
            found++;
        }
    }

    free(ii);
    close(hci_sock);

    // Si no se encontraron dispositivos OBD, devolver TODOS los BT encontrados
    // como fallback (quizás el usuario los configuró con otro nombre)
    if (found == 0) {
        // Re-ejecutar inquiry pero aceptar cualquier dispositivo
        // En este caso no re-ejecutamos - simplemente retornamos 0
        snprintf(last_error, sizeof(last_error),
                 "No se encontraron dispositivos OBD/ELM327");
    }

    return found;
}

// ============================================================
// Conexión RFCOMM
// ============================================================

int BluetoothManager::open_rfcomm_socket(const char* addr, uint8_t channel) {
    struct sockaddr_rc rc_addr;
    bdaddr_t target;

    // Crear socket RFCOMM
    int fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (fd < 0) {
        snprintf(last_error, sizeof(last_error),
                 "socket RFCOMM: %s", strerror(errno));
        return -1;
    }

    // Configurar dirección Bluetooth
    str2ba(addr, &target);

    // Configurar socket
    memset(&rc_addr, 0, sizeof(rc_addr));
    rc_addr.rc_family = AF_BLUETOOTH;
    rc_addr.rc_bdaddr = target;
    rc_addr.rc_channel = channel;

    // Timeout de conexión
    struct timeval tv;
    tv.tv_sec = CONNECT_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    // Conectar - usar ::connect para evitar conflicto con el método connect_bt
    if (::connect(fd, (struct sockaddr*)&rc_addr, sizeof(rc_addr)) < 0) {
        snprintf(last_error, sizeof(last_error),
                 "connect RFCOMM %s chan %d: %s",
                 addr, channel, strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

int BluetoothManager::connect_bt(const char* target_addr) {
    disconnect();
    state = BT_CONNECTING;
    retry_count = 0;

    BTDeviceInfo target_dev;
    memset(&target_dev, 0, sizeof(target_dev));

    if (target_addr && strlen(target_addr) > 0) {
        // Conectar a dirección específica
        snprintf(target_dev.address, sizeof(target_dev.address), "%s", target_addr);
        snprintf(target_dev.name, sizeof(target_dev.name), "%s", "OBDII Device");
        target_dev.channel = 1;
    } else {
        // Escanear y encontrar ELM327
        BTDeviceInfo found_devices[8];
        int n_found = scan_devices(found_devices, 8);

        if (n_found <= 0) {
            snprintf(last_error, sizeof(last_error),
                     "No se encontraron dispositivos ELM327");
            state = BT_ERROR;
            return -1;
        }

        memcpy(&target_dev, &found_devices[0], sizeof(BTDeviceInfo));
    }

    // Intentar conexión con reintentos
    while (retry_count < max_retries) {
        if (retry_count > 0) {
            // Esperar antes de reintentar
            struct timespec ts;
            ts.tv_sec = 2;
            ts.tv_nsec = 0;
            nanosleep(&ts, NULL);
        }

        // Probar canales 1-5 para encontrar SPP
        for (int ch = target_dev.channel; ch <= 5; ch++) {
            sock_fd = open_rfcomm_socket(target_dev.address, ch);
            if (sock_fd >= 0) {
                // ¡Conectado!
                memcpy(&device, &target_dev, sizeof(BTDeviceInfo));
                device.channel = ch;

                // Configurar socket sin timeout de bloqueo
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 500000;  // 500ms timeout en recepción
                setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

                state = BT_CONN_ACTIVE;
                snprintf(last_error, sizeof(last_error), "Conectado a %s [%s] chan %d",
                         device.name, device.address, ch);
                return 0;
            }

            // Si el dispositivo tiene un canal específico conocido, solo probar ese
            if (ch == target_dev.channel && target_dev.channel != 1)
                break;
        }

        retry_count++;
    }

    snprintf(last_error, sizeof(last_error),
             "No se pudo conectar tras %d intentos", max_retries);
    state = BT_ERROR;
    return -1;
}

// ============================================================
// Desconexión
// ============================================================

void BluetoothManager::disconnect() {
    if (sock_fd >= 0) {
        close(sock_fd);
        sock_fd = -1;
    }
    state = BT_DISCONNECTED;
    memset(&device, 0, sizeof(device));
}

// ============================================================
// Comunicación ELM327
// ============================================================

int BluetoothManager::send(const char* data, int len) {
    if (state != BT_CONN_ACTIVE || sock_fd < 0) {
        snprintf(last_error, sizeof(last_error), "No conectado");
        return -1;
    }

    int written = write(sock_fd, data, len);
    if (written < 0) {
        snprintf(last_error, sizeof(last_error),
                 "send: %s", strerror(errno));
        // Desconectar si el socket está muerto
        if (errno == ECONNRESET || errno == EPIPE || errno == ENOTCONN) {
            disconnect();
        }
        return -1;
    }

    return written;
}

int BluetoothManager::receive(char* buffer, int max_len, int timeout_ms) {
    if (state != BT_CONN_ACTIVE || sock_fd < 0) {
        return -1;
    }

    // Configurar timeout
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Esperar datos con poll
    struct pollfd pfd;
    pfd.fd = sock_fd;
    pfd.events = POLLIN;
    int ret = poll(&pfd, 1, timeout_ms);

    if (ret < 0) {
        snprintf(last_error, sizeof(last_error),
                 "receive poll: %s", strerror(errno));
        if (errno == ECONNRESET || errno == ENOTCONN) {
            disconnect();
        }
        return -1;
    }

    if (ret == 0) {
        return 0;  // Timeout - sin datos
    }

    // Leer datos
    int n = read(sock_fd, buffer, max_len - 1);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;  // Timeout
        }
        snprintf(last_error, sizeof(last_error),
                 "receive read: %s", strerror(errno));
        if (errno == ECONNRESET || errno == EPIPE || errno == ENOTCONN) {
            disconnect();
        }
        return -1;
    }

    if (n == 0) {
        // Conexión cerrada por el peer
        disconnect();
        return -1;
    }

    buffer[n] = '\0';
    return n;
}

void BluetoothManager::flush() {
    if (sock_fd < 0) return;

    char temp[128];
    struct pollfd pfd;
    pfd.fd = sock_fd;
    pfd.events = POLLIN;

    while (poll(&pfd, 1, 10) > 0) {
        int n = read(sock_fd, temp, sizeof(temp) - 1);
        if (n <= 0) break;
    }
}
