#!/bin/bash

# ============================================================================
# GENERADOR DE PROYECTO COMPLETO DS1994 - DETECCIÓN AUTOMÁTICA
# ============================================================================

set -e

PROJECT_DIR="ds1994_project"
DEVICE_FAMILY="04"  # Código de familia DS1994

echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║     GENERADOR DE PROYECTO DS1994 - DETECCIÓN AUTOMÁTICA            ║"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""

# Crear estructura de directorios
mkdir -p "${PROJECT_DIR}"/{src,include,obj,bin}

echo "✓ Directorios creados: src, include, obj, bin"

# ============================================================================
# Makefile
# ============================================================================
cat > "${PROJECT_DIR}/Makefile" << 'MAKEFILE_END'
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g -pthread
INCLUDES = -Iinclude
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/ds1994_app

.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "✅ Compilación exitosa: $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

debug: CXXFLAGS += -DDEBUG -g3
debug: clean $(TARGET)

run: $(TARGET)
	sudo $(TARGET)

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "🧹 Limpieza completada"

help:
	@echo "Comandos disponibles:"
	@echo "  make       - Compilar"
	@echo "  make debug - Compilar con debug"
	@echo "  make run   - Compilar y ejecutar"
	@echo "  make clean - Limpiar archivos"
MAKEFILE_END

echo "✓ Makefile creado"

# ============================================================================
# include/ds1994.h
# ============================================================================
cat > "${PROJECT_DIR}/include/ds1994.h" << 'HEADER_END'
#ifndef DS1994_H
#define DS1994_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Comandos DS1994 (datasheet Maxim)
constexpr uint8_t CMD_READ_ROM           = 0x33;
constexpr uint8_t CMD_WRITE_SCRATCHPAD   = 0x0F;
constexpr uint8_t CMD_READ_SCRATCHPAD    = 0xAA;
constexpr uint8_t CMD_COPY_SCRATCHPAD    = 0x55;
constexpr uint8_t CMD_READ_MEMORY        = 0xF0;

// Direcciones de memoria
constexpr uint16_t ADDR_STATUS   = 0x0200;
constexpr uint16_t ADDR_CONTROL  = 0x0201;
constexpr uint16_t ADDR_RTC      = 0x0202;
constexpr uint16_t ADDR_INTERVAL = 0x0207;
constexpr uint16_t ADDR_CYCLE    = 0x020C;

// Constantes
constexpr int PAGE_SIZE     = 32;
constexpr int NUM_PAGES     = 16;
constexpr int TOTAL_SRAM    = 512;

// Bits de Control Register
constexpr uint8_t CR_WPR  = 0x01;
constexpr uint8_t CR_WPI  = 0x02;
constexpr uint8_t CR_WPC  = 0x04;
constexpr uint8_t CR_RO   = 0x08;
constexpr uint8_t CR_OSC  = 0x10;
constexpr uint8_t CR_AUTO = 0x20;
constexpr uint8_t CR_STOP = 0x40;
constexpr uint8_t CR_DSEL = 0x80;

// Bits de Status Register
constexpr uint8_t SR_RTF  = 0x01;
constexpr uint8_t SR_ITF  = 0x02;
constexpr uint8_t SR_CCF  = 0x04;
constexpr uint8_t SR_RTE  = 0x08;
constexpr uint8_t SR_ITE  = 0x10;
constexpr uint8_t SR_CCE  = 0x20;

// Estructura para dispositivo detectado
struct DS1994Device {
    std::string id;
    std::string path;
    uint8_t family;
    std::string serial;
    bool active;
};

// Callback para eventos
using EventCallback = std::function<void(const std::string&, bool, const std::string&)>;

class DS1994 {
public:
    explicit DS1994(EventCallback callback = nullptr);
    ~DS1994();
    
    // Detección automática
    std::vector<DS1994Device> scanDevices();
    bool selectDevice(const std::string& device_id = "");
    std::string getCurrentDeviceId() const;
    
    // Operaciones de memoria
    std::vector<uint8_t> readMemory(uint16_t address, size_t length);
    bool writeMemory(uint16_t address, const std::vector<uint8_t>& data);
    std::vector<uint8_t> readPage(int page);
    bool writePage(int page, const std::vector<uint8_t>& data, int offset = 0);
    
    // Timekeeping
    uint8_t readStatus();
    uint8_t readControl();
    time_t readRTC();
    bool writeRTC(time_t unix_time);
    double readIntervalTimer();
    uint32_t readCycleCounter();
    bool setOscillator(bool enable);
    
    // Utilidades
    void dumpMemory(bool include_timekeeping = true);
    void printInfo();
    bool isWriteProtected();
    bool testWrite();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // DS1994_H
HEADER_END

echo "✓ include/ds1994.h creado"

# ============================================================================
# src/ds1994.cpp
# ============================================================================
cat > "${PROJECT_DIR}/src/ds1994.cpp" << 'CPP_END'
#include "ds1994.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <dirent.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cstring>
#include <ctime>
#include <algorithm>

namespace fs = std::filesystem;

using namespace std;

class DS1994::Impl {
private:
    string m_bus_path;
    string m_device_id;
    string m_device_path;
    EventCallback m_callback;
    bool m_initialized;
    
    const string W1_BASE = "/sys/devices/w1_bus_master1";
    
    bool write_raw(const vector<uint8_t>& data) {
        if (m_device_path.empty()) {
            if (m_callback) m_callback("ERROR", false, "No device selected");
            return false;
        }
        
        ofstream dev(m_device_path, ios::binary);
        if (!dev.is_open()) {
            cerr << "Error: No se puede abrir " << m_device_path << endl;
            return false;
        }
        dev.write(reinterpret_cast<const char*>(data.data()), data.size());
        dev.flush();
        return dev.good();
    }
    
    vector<uint8_t> read_raw(size_t num_bytes) {
        ifstream dev(m_device_path, ios::binary);
        if (!dev.is_open()) {
            return {};
        }
        vector<uint8_t> buffer(num_bytes);
        dev.read(reinterpret_cast<char*>(buffer.data()), num_bytes);
        buffer.resize(dev.gcount());
        return buffer;
    }
    
    void notify(const string& event, bool success, const string& msg = "") {
        if (m_callback) {
            m_callback(event, success, msg);
        }
        if (success) {
            cout << "[OK] " << event << ": " << msg << endl;
        } else if (!msg.empty()) {
            cerr << "[ERROR] " << event << ": " << msg << endl;
        }
    }
    
public:
    Impl(EventCallback callback) 
        : m_bus_path(W1_BASE), m_callback(callback), m_initialized(false) {}
    
    vector<DS1994Device> scanDevices() {
        vector<DS1994Device> devices;
        
        if (!fs::exists(m_bus_path)) {
            notify("SCAN", false, "Bus 1-Wire no encontrado: " + m_bus_path);
            return devices;
        }
        
        DIR* dir = opendir(m_bus_path.c_str());
        if (!dir) return devices;
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string name = entry->d_name;
            // Buscar dispositivos con formato XX-XXXXXXXXXXXX (familia 04 = DS1994)
            if (name.length() >= 15 && name[2] == '-') {
                string family = name.substr(0, 2);
                if (family == "04") {  // Solo DS1994
                    DS1994Device dev;
                    dev.id = name;
                    dev.path = m_bus_path + "/" + name + "/rw";
                    dev.family = stoi(family, nullptr, 16);
                    dev.serial = name.substr(3);
                    dev.active = fs::exists(dev.path);
                    
                    if (dev.active) {
                        devices.push_back(dev);
                        notify("SCAN", true, "Dispositivo encontrado: " + dev.id);
                    }
                }
            }
        }
        closedir(dir);
        
        return devices;
    }
    
    bool selectDevice(const string& device_id) {
        if (!device_id.empty()) {
            // Usar ID específico
            m_device_id = device_id;
            m_device_path = m_bus_path + "/" + m_device_id + "/rw";
        } else {
            // Detección automática
            auto devices = scanDevices();
            if (devices.empty()) {
                notify("SELECT", false, "No se encontraron dispositivos DS1994");
                return false;
            }
            
            if (devices.size() == 1) {
                m_device_id = devices[0].id;
                m_device_path = devices[0].path;
                notify("SELECT", true, "Dispositivo seleccionado automáticamente: " + m_device_id);
            } else {
                // Múltiples dispositivos - usar el primero
                cout << "\n⚠️  Múltiples dispositivos encontrados:" << endl;
                for (size_t i = 0; i < devices.size(); i++) {
                    cout << "   [" << i << "] " << devices[i].id << endl;
                }
                cout << "   Usando: " << devices[0].id << endl;
                m_device_id = devices[0].id;
                m_device_path = devices[0].path;
                notify("SELECT", true, "Dispositivo seleccionado: " + m_device_id);
            }
        }
        
        // Verificar que el archivo existe
        if (!fs::exists(m_device_path)) {
            notify("SELECT", false, "Dispositivo no encontrado: " + m_device_path);
            return false;
        }
        
        m_initialized = true;
        return true;
    }
    
    string getCurrentDeviceId() const {
        return m_device_id;
    }
    
    vector<uint8_t> readMemory(uint16_t address, size_t length) {
        if (!m_initialized) return {};
        
        // READ_MEMORY: 0xF0 + TA1 + TA2 (sin SKIP_ROM)
        vector<uint8_t> cmd = {
            CMD_READ_MEMORY,
            static_cast<uint8_t>(address & 0xFF),
            static_cast<uint8_t>((address >> 8) & 0xFF)
        };
        
        if (!write_raw(cmd)) {
            notify("READ", false, "Fallo al enviar comando");
            return {};
        }
        
        this_thread::sleep_for(chrono::milliseconds(10));
        auto data = read_raw(length);
        
        if (data.empty()) {
            notify("READ", false, "No se recibieron datos");
        }
        
        return data;
    }
    
    bool writeMemory(uint16_t address, const vector<uint8_t>& data) {
        if (!m_initialized) return false;
        
        if (data.empty() || data.size() > 32) {
            notify("WRITE", false, "Datos deben ser 1-32 bytes");
            return false;
        }
        
        uint8_t ta1 = address & 0xFF;
        uint8_t ta2 = (address >> 8) & 0xFF;
        
        // PASO 1: WRITE SCRATCHPAD (0x0F + TA1 + TA2 + data)
        vector<uint8_t> write_cmd = {
            CMD_WRITE_SCRATCHPAD,
            ta1, ta2
        };
        write_cmd.insert(write_cmd.end(), data.begin(), data.end());
        
        if (!write_raw(write_cmd)) {
            notify("WRITE", false, "WRITE_SCRATCHPAD falló");
            return false;
        }
        
        this_thread::sleep_for(chrono::milliseconds(10));
        
        // PASO 2: READ SCRATCHPAD (0xAA)
        if (!write_raw({CMD_READ_SCRATCHPAD})) {
            notify("WRITE", false, "READ_SCRATCHPAD falló");
            return false;
        }
        
        auto scratch = read_raw(3);
        if (scratch.size() >= 3) {
            cout << "  TA1: 0x" << hex << (int)scratch[0] << dec << endl;
            cout << "  TA2: 0x" << hex << (int)scratch[1] << dec << endl;
            cout << "  E/S: 0x" << hex << (int)scratch[2] << dec << endl;
        }
        
        // PASO 3: COPY SCRATCHPAD (0x55 + TA1 + TA2 + E/S)
        uint8_t ending_offset = (ta1 & 0x1F) + data.size() - 1;
        vector<uint8_t> copy_cmd = {
            CMD_COPY_SCRATCHPAD,
            ta1, ta2,
            static_cast<uint8_t>(ending_offset & 0x1F)
        };
        
        if (!write_raw(copy_cmd)) {
            notify("WRITE", false, "COPY_SCRATCHPAD falló");
            return false;
        }
        
        // Copy toma 30 MICROSEGUNDOS (datasheet página 9)
        this_thread::sleep_for(chrono::microseconds(30));
        
        notify("WRITE", true, to_string(data.size()) + " bytes escritos en 0x" + 
               [&](){ stringstream ss; ss << hex << address; return ss.str(); }());
        
        return true;
    }
    
    vector<uint8_t> readPage(int page) {
        if (page < 0 || page >= NUM_PAGES) {
            notify("READ_PAGE", false, "Página inválida: " + to_string(page));
            return {};
        }
        return readMemory(page * PAGE_SIZE, PAGE_SIZE);
    }
    
    bool writePage(int page, const vector<uint8_t>& data, int offset) {
        if (page < 0 || page >= NUM_PAGES) {
            notify("WRITE_PAGE", false, "Página inválida: " + to_string(page));
            return false;
        }
        if (offset + data.size() > PAGE_SIZE) {
            notify("WRITE_PAGE", false, "Datos exceden página");
            return false;
        }
        return writeMemory(page * PAGE_SIZE + offset, data);
    }
    
    uint8_t readStatus() {
        auto data = readMemory(ADDR_STATUS, 1);
        return data.empty() ? 0xFF : data[0];
    }
    
    uint8_t readControl() {
        auto data = readMemory(ADDR_CONTROL, 1);
        return data.empty() ? 0xFF : data[0];
    }
    
    time_t readRTC() {
        auto data = readMemory(ADDR_RTC, 5);
        if (data.size() < 5) return 0;
        
        uint32_t seconds = (data[4] << 24) | (data[3] << 16) | 
                           (data[2] << 8) | data[1];
        return static_cast<time_t>(seconds);
    }
    
    bool writeRTC(time_t unix_time) {
        vector<uint8_t> data(5, 0);
        data[1] = (unix_time >> 0) & 0xFF;
        data[2] = (unix_time >> 8) & 0xFF;
        data[3] = (unix_time >> 16) & 0xFF;
        data[4] = (unix_time >> 24) & 0xFF;
        return writeMemory(ADDR_RTC, data);
    }
    
    double readIntervalTimer() {
        auto data = readMemory(ADDR_INTERVAL, 5);
        if (data.size() < 5) return 0;
        
        double fraction = data[0] / 256.0;
        uint32_t seconds = (data[4] << 24) | (data[3] << 16) | 
                           (data[2] << 8) | data[1];
        return seconds + fraction;
    }
    
    uint32_t readCycleCounter() {
        auto data = readMemory(ADDR_CYCLE, 4);
        if (data.size() < 4) return 0;
        
        return (data[3] << 24) | (data[2] << 16) | 
               (data[1] << 8) | data[0];
    }
    
    bool setOscillator(bool enable) {
        uint8_t control = readControl();
        if (enable) {
            control |= CR_OSC;
        } else {
            control &= ~CR_OSC;
        }
        return writeMemory(ADDR_CONTROL, {control});
    }
    
    bool isWriteProtected() {
        uint8_t control = readControl();
        return (control & CR_RO) || (control & CR_WPR) || 
               (control & CR_WPI) || (control & CR_WPC);
    }
    
    bool testWrite() {
        cout << "\n=== PRUEBA DE ESCRITURA ===" << endl;
        
        // Leer valor original
        auto before = readMemory(0x0000, 1);
        uint8_t original = before.empty() ? 0xFF : before[0];
        cout << "Valor original en 0x0000: 0x" << hex << (int)original << dec << endl;
        
        // Escribir 0xAA
        if (!writeMemory(0x0000, {0xAA})) {
            cout << "❌ Error en escritura" << endl;
            return false;
        }
        
        // Leer después
        auto after = readMemory(0x0000, 1);
        uint8_t nuevo = after.empty() ? 0xFF : after[0];
        cout << "Valor después de escribir 0xAA: 0x" << hex << (int)nuevo << dec << endl;
        
        if (nuevo == 0xAA) {
            cout << "✅ PRUEBA EXITOSA: El dispositivo escribe correctamente!" << endl;
            return true;
        } else {
            cout << "❌ PRUEBA FALLIDA: No se pudo escribir" << endl;
            return false;
        }
    }
    
    void dumpMemory(bool include_timekeeping) {
        size_t total = include_timekeeping ? 542 : TOTAL_SRAM;
        auto mem = readMemory(0x0000, total);
        
        cout << "\n--- DUMP DE MEMORIA (" << mem.size() << " bytes) ---" << endl;
        for (size_t i = 0; i < mem.size(); i += 16) {
            cout << hex << setw(4) << setfill('0') << i << ": ";
            for (size_t j = i; j < min(i + 16, mem.size()); j++) {
                cout << setw(2) << setfill('0') << (int)mem[j] << " ";
            }
            cout << " |";
            for (size_t j = i; j < min(i + 16, mem.size()); j++) {
                char c = (mem[j] >= 32 && mem[j] <= 126) ? (char)mem[j] : '.';
                cout << c;
            }
            cout << "|" << endl;
        }
        cout << dec;
    }
    
    void printInfo() {
        cout << "\n========================================" << endl;
        cout << "   DS1994-F5 iButton" << endl;
        cout << "========================================" << endl;
        cout << " Device ID:   " << m_device_id << endl;
        cout << " Device Path: " << m_device_path << endl;
        cout << "========================================" << endl;
        
        uint8_t status = readStatus();
        uint8_t control = readControl();
        
        cout << "\n Status Register: 0x" << hex << setw(2) << setfill('0') 
             << (int)status << dec;
        if (status == 0x00) cout << " (Sin alarmas)";
        cout << endl;
        
        cout << " Control Register: 0x" << hex << setw(2) << setfill('0') 
             << (int)control << dec << endl;
        
        cout << "\n Bits de control:" << endl;
        cout << "   ┌─────────────────────────────────────────────────────────┐" << endl;
        cout << "   │ WPR=" << ((control >> 0) & 1) << "  WPI=" << ((control >> 1) & 1)
             << "  WPC=" << ((control >> 2) & 1) << "  RO=" << ((control >> 3) & 1)
             << "  OSC=" << ((control >> 4) & 1) << "  AUTO=" << ((control >> 5) & 1)
             << "  STP=" << ((control >> 6) & 1) << "  DSEL=" << ((control >> 7) & 1) << " │" << endl;
        cout << "   └─────────────────────────────────────────────────────────┘" << endl;
        
        if (control & CR_RO) cout << "   ⚠️  Modo SOLO LECTURA activado" << endl;
        if (control & CR_WPR) cout << "   ⚠️  RTC protegido contra escritura" << endl;
        if (control & CR_WPI) cout << "   ⚠️  Interval Timer protegido" << endl;
        if (control & CR_WPC) cout << "   ⚠️  Cycle Counter protegido" << endl;
        
        time_t t = readRTC();
        cout << "\n RTC: " << ctime(&t);
        
        double interval = readIntervalTimer();
        uint32_t hours = static_cast<uint32_t>(interval) / 3600;
        uint32_t mins = (static_cast<uint32_t>(interval) % 3600) / 60;
        double secs = interval - hours * 3600 - mins * 60;
        cout << " Interval Timer: " << fixed << setprecision(3) << interval 
             << " seg  (" << hours << "h " << mins << "m " << secs << "s)" << endl;
        
        cout << " Cycle Counter: " << readCycleCounter() << " ciclos" << endl;
        cout << "========================================" << endl;
    }
};

// ============================================================================
// IMPLEMENTACIÓN PÚBLICA
// ============================================================================

DS1994::DS1994(EventCallback callback) : pImpl(make_unique<Impl>(callback)) {}
DS1994::~DS1994() = default;

vector<DS1994Device> DS1994::scanDevices() { return pImpl->scanDevices(); }
bool DS1994::selectDevice(const string& device_id) { return pImpl->selectDevice(device_id); }
string DS1994::getCurrentDeviceId() const { return pImpl->getCurrentDeviceId(); }

vector<uint8_t> DS1994::readMemory(uint16_t address, size_t length) { return pImpl->readMemory(address, length); }
bool DS1994::writeMemory(uint16_t address, const vector<uint8_t>& data) { return pImpl->writeMemory(address, data); }
vector<uint8_t> DS1994::readPage(int page) { return pImpl->readPage(page); }
bool DS1994::writePage(int page, const vector<uint8_t>& data, int offset) { return pImpl->writePage(page, data, offset); }

uint8_t DS1994::readStatus() { return pImpl->readStatus(); }
uint8_t DS1994::readControl() { return pImpl->readControl(); }
time_t DS1994::readRTC() { return pImpl->readRTC(); }
bool DS1994::writeRTC(time_t unix_time) { return pImpl->writeRTC(unix_time); }
double DS1994::readIntervalTimer() { return pImpl->readIntervalTimer(); }
uint32_t DS1994::readCycleCounter() { return pImpl->readCycleCounter(); }
bool DS1994::setOscillator(bool enable) { return pImpl->setOscillator(enable); }

void DS1994::dumpMemory(bool include_timekeeping) { pImpl->dumpMemory(include_timekeeping); }
void DS1994::printInfo() { pImpl->printInfo(); }
bool DS1994::isWriteProtected() { return pImpl->isWriteProtected(); }
bool DS1994::testWrite() { return pImpl->testWrite(); }
CPP_END

echo "✓ src/ds1994.cpp creado"

# ============================================================================
# src/main.cpp
# ============================================================================
cat > "${PROJECT_DIR}/src/main.cpp" << 'MAIN_END'
#include "ds1994.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctime>
#include <sstream>

using namespace std;

void printBanner() {
    cout << "\n╔════════════════════════════════════════════════════════════════╗" << endl;
    cout << "║              DS1994-F5 iButton - Driver v2.0                    ║" << endl;
    cout << "║                   Detección Automática                          ║" << endl;
    cout << "╚════════════════════════════════════════════════════════════════╝" << endl;
}

void printHelp(const char* prog) {
    cout << "\nUso: " << prog << " [COMANDO] [OPCIONES]\n" << endl;
    cout << "Comandos:" << endl;
    cout << "  info              - Información del dispositivo" << endl;
    cout << "  scan              - Escanear dispositivos DS1994" << endl;
    cout << "  select <ID>       - Seleccionar dispositivo específico" << endl;
    cout << "  status            - Leer Status y Control registers" << endl;
    cout << "  dump              - Volcado hexadecimal de memoria" << endl;
    cout << "  rtc               - Leer RTC" << endl;
    cout << "  setrtc            - Sincronizar RTC con hora del sistema" << endl;
    cout << "  interval          - Leer Interval Timer" << endl;
    cout << "  cycles            - Leer Cycle Counter" << endl;
    cout << "  read <pag>        - Leer página (0-15)" << endl;
    cout << "  write <pag> <hex> - Escribir datos hex en página" << endl;
    cout << "  osc <on|off>      - Controlar oscilador" << endl;
    cout << "  test              - Prueba de escritura" << endl;
    cout << "  help              - Mostrar esta ayuda" << endl;
    cout << "\nEjemplos:" << endl;
    cout << "  sudo " << prog << " info" << endl;
    cout << "  sudo " << prog << " read 0" << endl;
    cout << "  sudo " << prog << " write 0 48656C6C6F" << endl;
    cout << "  sudo " << prog << " setrtc" << endl;
}

string hexToBytes(const string& hex) {
    string result;
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        result.push_back(static_cast<char>(stoi(hex.substr(i, 2), nullptr, 16)));
    }
    return result;
}

void hexDump(const vector<uint8_t>& data, const string& title) {
    cout << "\n=== " << title << " ===" << endl;
    for (size_t i = 0; i < data.size(); i++) {
        if (i % 16 == 0) {
            if (i > 0) cout << endl;
            cout << "0x" << hex << setw(4) << setfill('0') << i << ": ";
        }
        cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
    }
    cout << dec << endl << endl;
}

int main(int argc, char* argv[]) {
    printBanner();
    
    DS1994 ds1994;
    
    // Detección automática del dispositivo
    if (!ds1994.selectDevice()) {
        cerr << "\n❌ No se encontró ningún dispositivo DS1994" << endl;
        cerr << "\nVerifique:" << endl;
        cerr << "  1. Conexión física del iButton" << endl;
        cerr << "  2. Resistencia pull-up de 4.7kΩ" << endl;
        cerr << "  3. Módulos cargados: w1-gpio, w1-therm" << endl;
        return 1;
    }
    
    string device_id = ds1994.getCurrentDeviceId();
    cout << "\n📟 Dispositivo seleccionado: " << device_id << endl;
    
    string cmd = (argc > 1) ? argv[1] : "info";
    
    try {
        if (cmd == "info") {
            ds1994.printInfo();
        }
        else if (cmd == "scan") {
            auto devices = ds1994.scanDevices();
            cout << "\n🔍 Dispositivos DS1994 encontrados:" << endl;
            for (const auto& dev : devices) {
                cout << "  📟 " << dev.id << " - " << (dev.active ? "ACTIVO" : "INACTIVO") << endl;
            }
            if (devices.empty()) {
                cout << "  (ninguno)" << endl;
            }
        }
        else if (cmd == "select") {
            if (argc < 3) {
                cerr << "Uso: select <device_id>" << endl;
                return 1;
            }
            if (ds1994.selectDevice(argv[2])) {
                cout << "✅ Dispositivo seleccionado: " << ds1994.getCurrentDeviceId() << endl;
            } else {
                cerr << "❌ No se pudo seleccionar el dispositivo" << endl;
            }
        }
        else if (cmd == "status") {
            uint8_t status = ds1994.readStatus();
            uint8_t control = ds1994.readControl();
            cout << "\nStatus Register (0x0200): 0x" << hex << setw(2) << setfill('0') 
                 << (int)status << dec << endl;
            cout << "  RTF=" << ((status >> 0) & 1) << " ITF=" << ((status >> 1) & 1)
                 << " CCF=" << ((status >> 2) & 1) << " RTE=" << ((status >> 3) & 1)
                 << " ITE=" << ((status >> 4) & 1) << " CCE=" << ((status >> 5) & 1) << endl;
            
            cout << "\nControl Register (0x0201): 0x" << hex << setw(2) << setfill('0') 
                 << (int)control << dec << endl;
            cout << "  WPR=" << ((control >> 0) & 1) << " WPI=" << ((control >> 1) & 1)
                 << " WPC=" << ((control >> 2) & 1) << " RO=" << ((control >> 3) & 1)
                 << " OSC=" << ((control >> 4) & 1) << " AUTO=" << ((control >> 5) & 1)
                 << " STP=" << ((control >> 6) & 1) << " DSEL=" << ((control >> 7) & 1) << endl;
        }
        else if (cmd == "dump") {
            ds1994.dumpMemory(true);
        }
        else if (cmd == "rtc") {
            time_t t = ds1994.readRTC();
            cout << "\nRTC: " << ctime(&t);
        }
        else if (cmd == "setrtc") {
            time_t now = time(nullptr);
            cout << "\nEscribiendo RTC: " << now << " (" << ctime(&now) << ")" << endl;
            if (ds1994.writeRTC(now)) {
                cout << "✅ RTC actualizado correctamente" << endl;
            } else {
                cout << "❌ Falló la escritura del RTC" << endl;
            }
        }
        else if (cmd == "interval") {
            double val = ds1994.readIntervalTimer();
            uint32_t h = static_cast<uint32_t>(val) / 3600;
            uint32_t m = (static_cast<uint32_t>(val) % 3600) / 60;
            double s = val - h * 3600 - m * 60;
            cout << fixed << setprecision(3);
            cout << "\nInterval Timer: " << val << " segundos" << endl;
            cout << "               " << h << "h " << m << "m " << s << "s" << endl;
        }
        else if (cmd == "cycles") {
            uint32_t val = ds1994.readCycleCounter();
            cout << "\nCycle Counter: " << val << " ciclos" << endl;
        }
        else if (cmd == "read") {
            if (argc < 3) {
                cerr << "Uso: read <pagina 0-15>" << endl;
                return 1;
            }
            int page = stoi(argv[2]);
            auto data = ds1994.readPage(page);
            hexDump(data, "Página " + to_string(page));
        }
        else if (cmd == "write") {
            if (argc < 4) {
                cerr << "Uso: write <pagina> <hex_data>" << endl;
                cerr << "Ejemplo: write 0 48656C6C6F" << endl;
                return 1;
            }
            int page = stoi(argv[2]);
            string hex = argv[3];
            string bytes = hexToBytes(hex);
            vector<uint8_t> data(bytes.begin(), bytes.end());
            
            cout << "\nEscribiendo " << data.size() << " bytes en página " << page << endl;
            if (ds1994.writePage(page, data)) {
                cout << "✅ Escritura exitosa" << endl;
                auto verify = ds1994.readPage(page);
                hexDump(verify, "Verificación página " + to_string(page));
            }
        }
        else if (cmd == "osc") {
            if (argc < 3) {
                cerr << "Uso: osc <on|off>" << endl;
                return 1;
            }
            bool enable = (string(argv[2]) == "on");
            if (ds1994.setOscillator(enable)) {
                cout << "\n✅ Oscilador " << (enable ? "ENCENDIDO" : "APAGADO") << endl;
            }
        }
        else if (cmd == "test") {
            ds1994.testWrite();
        }
        else if (cmd == "help" || cmd == "-h" || cmd == "--help") {
            printHelp(argv[0]);
        }
        else {
            cerr << "❌ Comando desconocido: " << cmd << endl;
            printHelp(argv[0]);
            return 1;
        }
    }
    catch (const exception& e) {
        cerr << "❌ Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
MAIN_END

echo "✓ src/main.cpp creado"

# ============================================================================
# Script de configuración
# ============================================================================
cat > "${PROJECT_DIR}/setup.sh" << 'SETUP_END'
#!/bin/bash
echo "🔧 Configurando bus 1-Wire en GPIO4..."
sudo dtoverlay w1-gpio gpiopin=4
sudo modprobe w1-gpio
sudo modprobe w1-therm
sleep 2
echo "✅ Configuración completada"
echo ""
echo "Dispositivos detectados:"
ls /sys/devices/w1_bus_master1/ | grep -E "^[0-9a-f]{2}-" || echo "  (ninguno)"
SETUP_END

chmod +x "${PROJECT_DIR}/setup.sh"
echo "✓ setup.sh creado"

# ============================================================================
# Script de compilación rápida
# ============================================================================
cat > "${PROJECT_DIR}/build.sh" << 'BUILD_END'
#!/bin/bash
cd "$(dirname "$0")"
make clean
make
echo ""
echo "✅ Compilación completada"
echo "Ejecutar: sudo ./bin/ds1994_app info"
BUILD_END

chmod +x "${PROJECT_DIR}/build.sh"
echo "✓ build.sh creado"

# ============================================================================
# README
# ============================================================================
cat > "${PROJECT_DIR}/README.md" << 'README_END'
# DS1994-F5 iButton Driver

Driver C++ para DS1994-F5 con detección automática de dispositivos.

## Compilación

```bash
./build.sh

