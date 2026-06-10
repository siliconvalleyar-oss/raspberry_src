#!/bin/bash

PROJECT_DIR="ds1994_project_fixed"

mkdir -p "${PROJECT_DIR}"/{src,include,obj,bin}

# ============================================================================
# include/ds1994.h (CORREGIDO)
# ============================================================================
cat > "${PROJECT_DIR}/include/ds1994.h" << 'HEADER_END'
#ifndef DS1994_H
#define DS1994_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>

constexpr uint8_t CMD_READ_ROM           = 0x33;
constexpr uint8_t CMD_WRITE_SCRATCHPAD   = 0x0F;
constexpr uint8_t CMD_READ_SCRATCHPAD    = 0xAA;
constexpr uint8_t CMD_COPY_SCRATCHPAD    = 0x55;
constexpr uint8_t CMD_READ_MEMORY        = 0xF0;

constexpr uint16_t ADDR_STATUS   = 0x0200;
constexpr uint16_t ADDR_CONTROL  = 0x0201;
constexpr uint16_t ADDR_RTC      = 0x0202;
constexpr uint16_t ADDR_INTERVAL = 0x0207;
constexpr uint16_t ADDR_CYCLE    = 0x020C;

constexpr int PAGE_SIZE     = 32;
constexpr int NUM_PAGES     = 16;
constexpr int TOTAL_SRAM    = 512;

constexpr uint8_t CR_WPR  = 0x01;
constexpr uint8_t CR_WPI  = 0x02;
constexpr uint8_t CR_WPC  = 0x04;
constexpr uint8_t CR_RO   = 0x08;
constexpr uint8_t CR_OSC  = 0x10;
constexpr uint8_t CR_AUTO = 0x20;
constexpr uint8_t CR_STOP = 0x40;
constexpr uint8_t CR_DSEL = 0x80;

constexpr uint8_t SR_RTF  = 0x01;
constexpr uint8_t SR_ITF  = 0x02;
constexpr uint8_t SR_CCF  = 0x04;
constexpr uint8_t SR_RTE  = 0x08;
constexpr uint8_t SR_ITE  = 0x10;
constexpr uint8_t SR_CCE  = 0x20;

struct DS1994Device {
    std::string id;
    std::string path;
    uint8_t family;
    std::string serial;
    bool active;
};

using EventCallback = std::function<void(const std::string&, bool, const std::string&)>;

class DS1994 {
public:
    explicit DS1994(EventCallback callback = nullptr, 
                    const std::string& bus_path = "/sys/devices/w1_bus_master1");
    ~DS1994();
    
    std::vector<DS1994Device> scanDevices();
    bool selectDevice(const std::string& device_id = "");
    std::string getCurrentDeviceId() const;
    
    std::vector<uint8_t> readMemory(uint16_t address, size_t length);
    bool writeMemory(uint16_t address, const std::vector<uint8_t>& data);
    std::vector<uint8_t> readPage(int page);
    bool writePage(int page, const std::vector<uint8_t>& data, int offset = 0);
    
    uint8_t readStatus();
    uint8_t readControl();
    time_t readRTC();
    bool writeRTC(time_t unix_time);
    double readIntervalTimer();
    uint32_t readCycleCounter();
    bool setOscillator(bool enable);
    
    void dumpMemory(bool include_timekeeping = true);
    void printInfo();
    bool isWriteProtected();
    bool testWrite();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif
HEADER_END

# ============================================================================
# src/ds1994.cpp (CORREGIDO)
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
#include <sstream>

namespace fs = std::filesystem;

using namespace std;

class DS1994::Impl {
private:
    string m_bus_path;
    string m_device_id;
    string m_device_path;
    EventCallback m_callback;
    bool m_initialized;
    
    bool write_raw(const vector<uint8_t>& data) {
        if (m_device_path.empty()) return false;
        
        ofstream dev(m_device_path, ios::binary);
        if (!dev.is_open()) return false;
        dev.write(reinterpret_cast<const char*>(data.data()), data.size());
        dev.flush();
        return dev.good();
    }
    
    vector<uint8_t> read_raw(size_t num_bytes) {
        ifstream dev(m_device_path, ios::binary);
        if (!dev.is_open()) return {};
        vector<uint8_t> buffer(num_bytes);
        dev.read(reinterpret_cast<char*>(buffer.data()), num_bytes);
        buffer.resize(dev.gcount());
        return buffer;
    }
    
public:
    Impl(EventCallback callback, const string& bus_path) 
        : m_bus_path(bus_path), m_callback(callback), m_initialized(false) {}
    
    vector<DS1994Device> scanDevices() {
        vector<DS1994Device> devices;
        
        if (!fs::exists(m_bus_path)) {
            if (m_callback) m_callback("SCAN", false, "Bus no encontrado: " + m_bus_path);
            return devices;
        }
        
        DIR* dir = opendir(m_bus_path.c_str());
        if (!dir) return devices;
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string name = entry->d_name;
            if (name.length() >= 15 && name[2] == '-') {
                string family = name.substr(0, 2);
                if (family == "04") {
                    DS1994Device dev;
                    dev.id = name;
                    dev.path = m_bus_path + "/" + name + "/rw";
                    dev.family = stoi(family, nullptr, 16);
                    dev.serial = name.substr(3);
                    dev.active = fs::exists(dev.path);
                    
                    if (dev.active) {
                        devices.push_back(dev);
                        if (m_callback) m_callback("SCAN", true, "Encontrado: " + dev.id);
                    }
                }
            }
        }
        closedir(dir);
        return devices;
    }
    
    bool selectDevice(const string& device_id) {
        if (!device_id.empty()) {
            m_device_id = device_id;
            m_device_path = m_bus_path + "/" + m_device_id + "/rw";
        } else {
            auto devices = scanDevices();
            if (devices.empty()) return false;
            m_device_id = devices[0].id;
            m_device_path = devices[0].path;
        }
        
        if (!fs::exists(m_device_path)) return false;
        
        m_initialized = true;
        return true;
    }
    
    string getCurrentDeviceId() const { return m_device_id; }
    
    vector<uint8_t> readMemory(uint16_t address, size_t length) {
        if (!m_initialized) return {};
        
        vector<uint8_t> cmd = {
            CMD_READ_MEMORY,
            static_cast<uint8_t>(address & 0xFF),
            static_cast<uint8_t>((address >> 8) & 0xFF)
        };
        
        if (!write_raw(cmd)) return {};
        
        this_thread::sleep_for(chrono::milliseconds(10));
        return read_raw(length);
    }
    
    bool writeMemory(uint16_t address, const vector<uint8_t>& data) {
        if (!m_initialized) return false;
        if (data.empty() || data.size() > 32) return false;
        
        uint8_t ta1 = address & 0xFF;
        uint8_t ta2 = (address >> 8) & 0xFF;
        
        // WRITE SCRATCHPAD
        vector<uint8_t> write_cmd = {CMD_WRITE_SCRATCHPAD, ta1, ta2};
        write_cmd.insert(write_cmd.end(), data.begin(), data.end());
        
        if (!write_raw(write_cmd)) return false;
        this_thread::sleep_for(chrono::milliseconds(10));
        
        // READ SCRATCHPAD
        if (!write_raw({CMD_READ_SCRATCHPAD})) return false;
        auto scratch = read_raw(3);
        
        // COPY SCRATCHPAD
        uint8_t ending_offset = (ta1 & 0x1F) + data.size() - 1;
        vector<uint8_t> copy_cmd = {
            CMD_COPY_SCRATCHPAD, ta1, ta2,
            static_cast<uint8_t>(ending_offset & 0x1F)
        };
        
        if (!write_raw(copy_cmd)) return false;
        
        this_thread::sleep_for(chrono::microseconds(30));
        return true;
    }
    
    vector<uint8_t> readPage(int page) {
        if (page < 0 || page >= NUM_PAGES) return {};
        return readMemory(page * PAGE_SIZE, PAGE_SIZE);
    }
    
    bool writePage(int page, const vector<uint8_t>& data, int offset) {
        if (page < 0 || page >= NUM_PAGES) return false;
        if (offset + data.size() > PAGE_SIZE) return false;
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
        uint32_t seconds = (data[4] << 24) | (data[3] << 16) | (data[2] << 8) | data[1];
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
        uint32_t seconds = (data[4] << 24) | (data[3] << 16) | (data[2] << 8) | data[1];
        return seconds + fraction;
    }
    
    uint32_t readCycleCounter() {
        auto data = readMemory(ADDR_CYCLE, 4);
        if (data.size() < 4) return 0;
        return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
    }
    
    bool setOscillator(bool enable) {
        uint8_t control = readControl();
        if (enable) control |= CR_OSC;
        else control &= ~CR_OSC;
        return writeMemory(ADDR_CONTROL, {control});
    }
    
    bool isWriteProtected() {
        uint8_t control = readControl();
        return (control & CR_RO) || (control & CR_WPR) || (control & CR_WPI) || (control & CR_WPC);
    }
    
    bool testWrite() {
        auto before = readMemory(0x0000, 1);
        uint8_t original = before.empty() ? 0xFF : before[0];
        cout << "Valor original: 0x" << hex << (int)original << dec << endl;
        
        if (!writeMemory(0x0000, {0xAA})) return false;
        
        auto after = readMemory(0x0000, 1);
        uint8_t nuevo = after.empty() ? 0xFF : after[0];
        cout << "Valor después: 0x" << hex << (int)nuevo << dec << endl;
        
        return nuevo == 0xAA;
    }
    
    void dumpMemory(bool include_timekeeping) {
        size_t total = include_timekeeping ? 542 : TOTAL_SRAM;
        auto mem = readMemory(0x0000, total);
        
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
        cout << " Bus Path:    " << m_bus_path << endl;
        cout << " Device Path: " << m_device_path << endl;
        cout << "========================================" << endl;
        
        uint8_t status = readStatus();
        uint8_t control = readControl();
        
        cout << "\n Status Register: 0x" << hex << setw(2) << setfill('0') << (int)status << dec;
        if (status == 0x00) cout << " (Sin alarmas)";
        cout << endl;
        
        cout << " Control Register: 0x" << hex << setw(2) << setfill('0') << (int)control << dec << endl;
        
        cout << "\n Bits de control:" << endl;
        cout << "   WPR=" << ((control >> 0) & 1) << "  WPI=" << ((control >> 1) & 1)
             << "  WPC=" << ((control >> 2) & 1) << "  RO=" << ((control >> 3) & 1)
             << "  OSC=" << ((control >> 4) & 1) << "  AUTO=" << ((control >> 5) & 1)
             << "  STP=" << ((control >> 6) & 1) << "  DSEL=" << ((control >> 7) & 1) << endl;
        
        time_t t = readRTC();
        cout << "\n RTC: " << ctime(&t);
        
        double interval = readIntervalTimer();
        cout << " Interval Timer: " << fixed << setprecision(3) << interval << " seg" << endl;
        cout << " Cycle Counter: " << readCycleCounter() << " ciclos" << endl;
        cout << "========================================" << endl;
    }
};

DS1994::DS1994(EventCallback callback, const string& bus_path) 
    : pImpl(make_unique<Impl>(callback, bus_path)) {}
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

# ============================================================================
# src/main.cpp (CORREGIDO)
# ============================================================================
cat > "${PROJECT_DIR}/src/main.cpp" << 'MAIN_END'
#include "ds1994.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

using namespace std;

void printBanner() {
    cout << "\n╔════════════════════════════════════════════════════════════════╗" << endl;
    cout << "║              DS1994-F5 iButton - Driver v2.1                    ║" << endl;
    cout << "║                   Detección Automática                          ║" << endl;
    cout << "╚════════════════════════════════════════════════════════════════╝" << endl;
}

void printHelp(const char* prog) {
    cout << "\nUso: " << prog << " [COMANDO]\n" << endl;
    cout << "Comandos: info, scan, status, dump, rtc, setrtc, interval, cycles" << endl;
    cout << "          read <pag>, write <pag> <hex>, osc <on|off>, test" << endl;
    cout << "\nEjemplo: sudo " << prog << " info" << endl;
}

int main(int argc, char* argv[]) {
    printBanner();
    
    // CORREGIDO: Pasar la ruta del bus explícitamente
    string bus_path = "/sys/devices/w1_bus_master1";
    DS1994 ds1994(nullptr, bus_path);
    
    if (!ds1994.selectDevice()) {
        cerr << "\n❌ No se encontró ningún dispositivo DS1994 en: " << bus_path << endl;
        cerr << "\nVerifique:" << endl;
        cerr << "  1. ls " << bus_path << "/" << endl;
        cerr << "  2. Conexión física del iButton" << endl;
        cerr << "  3. Resistencia pull-up de 4.7kΩ" << endl;
        return 1;
    }
    
    cout << "\n📟 Dispositivo: " << ds1994.getCurrentDeviceId() << endl;
    
    string cmd = (argc > 1) ? argv[1] : "info";
    
    if (cmd == "info") {
        ds1994.printInfo();
    }
    else if (cmd == "scan") {
        auto devices = ds1994.scanDevices();
        cout << "\n🔍 Dispositivos encontrados:" << endl;
        for (const auto& dev : devices) {
            cout << "   📟 " << dev.id << endl;
        }
    }
    else if (cmd == "status") {
        uint8_t s = ds1994.readStatus();
        uint8_t c = ds1994.readControl();
        cout << "\nStatus: 0x" << hex << setw(2) << setfill('0') << (int)s << dec << endl;
        cout << "Control: 0x" << hex << setw(2) << setfill('0') << (int)c << dec << endl;
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
        if (ds1994.writeRTC(now)) {
            cout << "\n✅ RTC actualizado: " << ctime(&now);
        }
    }
    else if (cmd == "interval") {
        double val = ds1994.readIntervalTimer();
        cout << "\nInterval Timer: " << fixed << setprecision(3) << val << " seg" << endl;
    }
    else if (cmd == "cycles") {
        cout << "\nCycle Counter: " << ds1994.readCycleCounter() << " ciclos" << endl;
    }
    else if (cmd == "read" && argc > 2) {
        int page = stoi(argv[2]);
        auto data = ds1994.readPage(page);
        cout << "\nPágina " << page << ":" << endl;
        for (size_t i = 0; i < data.size(); i++) {
            cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
            if ((i+1) % 16 == 0) cout << endl;
        }
        cout << dec << endl;
    }
    else if (cmd == "write" && argc > 3) {
        int page = stoi(argv[2]);
        string hex = argv[3];
        vector<uint8_t> data;
        for (size_t i = 0; i + 1 < hex.size(); i += 2) {
            data.push_back(stoi(hex.substr(i, 2), nullptr, 16));
        }
        if (ds1994.writePage(page, data)) {
            cout << "\n✅ Escritura exitosa" << endl;
        }
    }
    else if (cmd == "osc" && argc > 2) {
        bool on = (string(argv[2]) == "on");
        ds1994.setOscillator(on);
        cout << "\n✅ Oscilador " << (on ? "ON" : "OFF") << endl;
    }
    else if (cmd == "test") {
        if (ds1994.testWrite()) {
            cout << "\n✅ PRUEBA EXITOSA" << endl;
        } else {
            cout << "\n❌ PRUEBA FALLIDA" << endl;
        }
    }
    else {
        printHelp(argv[0]);
    }
    
    return 0;
}
MAIN_END

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

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "✅ Compilación exitosa"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

run: $(TARGET)
	sudo $(TARGET)

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "🧹 Limpieza completada"
MAKEFILE_END

echo ""
echo "✅ Proyecto regenerado con corrección en " $PROJECT_DIR
echo ""
echo "Para compilar y ejecutar:"
echo "  cd $PROJECT_DIR"
echo "  make"
echo "  sudo ./bin/ds1994_app info"

