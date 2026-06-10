/**
 * ds1994_complete.cpp - Driver completo para DS1994-F5
 * Basado en datasheet oficial - SIN SKIP_ROM
 * 
 * Compilar: g++ -std=c++17 -o ds1994_app ds1994_complete.cpp -pthread
 * Ejecutar: sudo ./ds1994_app [comando]
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <iomanip>
#include <ctime>
#include <sstream>

using namespace std;

// ============================================================================
// COMANDOS DS1994 (datasheet página 8-9)
// ============================================================================
constexpr uint8_t CMD_READ_ROM           = 0x33;
constexpr uint8_t CMD_WRITE_SCRATCHPAD   = 0x0F;
constexpr uint8_t CMD_READ_SCRATCHPAD    = 0xAA;
constexpr uint8_t CMD_COPY_SCRATCHPAD    = 0x55;
constexpr uint8_t CMD_READ_MEMORY        = 0xF0;

// ============================================================================
// DIRECCIONES DE MEMORIA
// ============================================================================
constexpr uint16_t ADDR_STATUS   = 0x0200;
constexpr uint16_t ADDR_CONTROL  = 0x0201;
constexpr uint16_t ADDR_RTC      = 0x0202;
constexpr uint16_t ADDR_INTERVAL = 0x0207;
constexpr uint16_t ADDR_CYCLE    = 0x020C;

// ============================================================================
// CLASE DS1994
// ============================================================================
class DS1994 {
private:
    string device_path;
    
    bool write_raw(const vector<uint8_t>& data) {
        ofstream dev(device_path, ios::binary);
        if (!dev.is_open()) {
            cerr << "Error: No se puede abrir " << device_path << endl;
            return false;
        }
        dev.write(reinterpret_cast<const char*>(data.data()), data.size());
        dev.flush();
        return dev.good();
    }
    
    vector<uint8_t> read_raw(size_t num_bytes) {
        ifstream dev(device_path, ios::binary);
        if (!dev.is_open()) {
            cerr << "Error: No se puede leer " << device_path << endl;
            return {};
        }
        vector<uint8_t> buffer(num_bytes);
        dev.read(reinterpret_cast<char*>(buffer.data()), num_bytes);
        buffer.resize(dev.gcount());
        return buffer;
    }
    
public:
    DS1994(const string& device_id) {
        device_path = "/sys/devices/w1_bus_master1/" + device_id + "/rw";
    }
    
    // ========================================================================
    // LECTURA DE MEMORIA (datasheet página 9)
    // ========================================================================
    vector<uint8_t> read_memory(uint16_t address, size_t num_bytes) {
        // Comando: READ_MEMORY [0xF0] + TA1 + TA2
        vector<uint8_t> cmd = {
            CMD_READ_MEMORY,
            static_cast<uint8_t>(address & 0xFF),
            static_cast<uint8_t>((address >> 8) & 0xFF)
        };
        
        if (!write_raw(cmd)) {
            return {};
        }
        
        this_thread::sleep_for(chrono::milliseconds(10));
        return read_raw(num_bytes);
    }
    
    // ========================================================================
    // ESCRITURA DE MEMORIA (datasheet página 8-9)
    // ========================================================================
    bool write_memory(uint16_t address, const vector<uint8_t>& data) {
        if (data.empty() || data.size() > 32) {
            cerr << "Error: Datos deben ser 1-32 bytes" << endl;
            return false;
        }
        
        cout << "  Escribiendo " << data.size() << " bytes en 0x" 
             << hex << address << dec << endl;
        
        uint8_t ta1 = address & 0xFF;
        uint8_t ta2 = (address >> 8) & 0xFF;
        
        // PASO 1: WRITE SCRATCHPAD
        vector<uint8_t> write_cmd = {
            CMD_WRITE_SCRATCHPAD,
            ta1, ta2
        };
        write_cmd.insert(write_cmd.end(), data.begin(), data.end());
        
        if (!write_raw(write_cmd)) {
            cerr << "  Error: WRITE_SCRATCHPAD falló" << endl;
            return false;
        }
        
        this_thread::sleep_for(chrono::milliseconds(10));
        
        // PASO 2: READ SCRATCHPAD (verificar)
        if (!write_raw({CMD_READ_SCRATCHPAD})) {
            cerr << "  Error: READ_SCRATCHPAD falló" << endl;
            return false;
        }
        
        auto scratch = read_raw(3);
        if (scratch.size() >= 3) {
            cout << "  TA1: 0x" << hex << (int)scratch[0] << dec << endl;
            cout << "  TA2: 0x" << hex << (int)scratch[1] << dec << endl;
            cout << "  E/S: 0x" << hex << (int)scratch[2] << dec << endl;
        }
        
        // PASO 3: COPY SCRATCHPAD
        uint8_t ending_offset = (ta1 & 0x1F) + data.size() - 1;
        vector<uint8_t> copy_cmd = {
            CMD_COPY_SCRATCHPAD,
            ta1, ta2,
            static_cast<uint8_t>(ending_offset & 0x1F)
        };
        
        if (!write_raw(copy_cmd)) {
            cerr << "  Error: COPY_SCRATCHPAD falló" << endl;
            return false;
        }
        
        // Copy toma 30 MICROSEGUNDOS (datasheet página 9)
        this_thread::sleep_for(chrono::microseconds(30));
        
        return true;
    }
    
    // ========================================================================
    // LEER PÁGINA (32 bytes)
    // ========================================================================
    vector<uint8_t> read_page(int page) {
        if (page < 0 || page > 15) {
            throw out_of_range("Página debe ser 0-15");
        }
        return read_memory(page * 32, 32);
    }
    
    // ========================================================================
    // ESCRIBIR PÁGINA
    // ========================================================================
    bool write_page(int page, const vector<uint8_t>& data, int offset = 0) {
        if (page < 0 || page > 15) {
            throw out_of_range("Página debe ser 0-15");
        }
        if (offset + data.size() > 32) {
            throw out_of_range("Datos exceden página");
        }
        return write_memory(page * 32 + offset, data);
    }
    
    // ========================================================================
    // LEER STATUS REGISTER
    // ========================================================================
    uint8_t read_status() {
        auto data = read_memory(ADDR_STATUS, 1);
        return data.empty() ? 0xFF : data[0];
    }
    
    // ========================================================================
    // LEER CONTROL REGISTER
    // ========================================================================
    uint8_t read_control() {
        auto data = read_memory(ADDR_CONTROL, 1);
        return data.empty() ? 0xFF : data[0];
    }
    
    // ========================================================================
    // LEER RTC (Unix timestamp)
    // ========================================================================
    time_t read_rtc() {
        auto data = read_memory(ADDR_RTC, 5);
        if (data.size() < 5) return 0;
        
        // 5 bytes: byte0 = fracción, bytes1-4 = segundos
        uint32_t seconds = (data[4] << 24) | (data[3] << 16) | 
                           (data[2] << 8) | data[1];
        return static_cast<time_t>(seconds);
    }
    
    // ========================================================================
    // ESCRIBIR RTC
    // ========================================================================
    bool write_rtc(time_t unix_time) {
        vector<uint8_t> data(5, 0);
        data[1] = (unix_time >> 0) & 0xFF;
        data[2] = (unix_time >> 8) & 0xFF;
        data[3] = (unix_time >> 16) & 0xFF;
        data[4] = (unix_time >> 24) & 0xFF;
        // data[0] = fracción (0)
        
        return write_memory(ADDR_RTC, data);
    }
    
    // ========================================================================
    // LEER INTERVAL TIMER (segundos)
    // ========================================================================
    double read_interval_timer() {
        auto data = read_memory(ADDR_INTERVAL, 5);
        if (data.size() < 5) return 0;
        
        double fraction = data[0] / 256.0;
        uint32_t seconds = (data[4] << 24) | (data[3] << 16) | 
                           (data[2] << 8) | data[1];
        return seconds + fraction;
    }
    
    // ========================================================================
    // LEER CYCLE COUNTER
    // ========================================================================
    uint32_t read_cycle_counter() {
        auto data = read_memory(ADDR_CYCLE, 4);
        if (data.size() < 4) return 0;
        
        return (data[3] << 24) | (data[2] << 16) | 
               (data[1] << 8) | data[0];
    }
    
    // ========================================================================
    // CONTROL DEL OSCILADOR
    // ========================================================================
    bool set_oscillator(bool enable) {
        uint8_t control = read_control();
        if (enable) {
            control |= 0x10;  // CR_OSC
        } else {
            control &= ~0x10;
        }
        return write_memory(ADDR_CONTROL, {control});
    }
    
    // ========================================================================
    // DUMP DE MEMORIA COMPLETA
    // ========================================================================
    void dump_memory(bool include_timekeeping = true) {
        size_t total = include_timekeeping ? 542 : 512;
        auto mem = read_memory(0x0000, total);
        
        cout << "\n--- DUMP DE MEMORIA (" << mem.size() << " bytes) ---\n";
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
            cout << "|\n";
        }
        cout << dec;
    }
    
    // ========================================================================
    // INFO DEL DISPOSITIVO
    // ========================================================================
    void print_info() {
        cout << "\n========================================" << endl;
        cout << "   DS1994-F5 iButton" << endl;
        cout << "========================================" << endl;
        
        uint8_t status = read_status();
        uint8_t control = read_control();
        
        cout << " Status Register: 0x" << hex << setw(2) << setfill('0') 
             << (int)status << dec << endl;
        cout << " Control Register: 0x" << hex << setw(2) << setfill('0') 
             << (int)control << dec << endl;
        
        cout << "\n Control Register bits:" << endl;
        cout << "   WPR=" << ((control >> 0) & 1) 
             << "  WPI=" << ((control >> 1) & 1)
             << "  WPC=" << ((control >> 2) & 1)
             << "  RO="  << ((control >> 3) & 1)
             << "  OSC=" << ((control >> 4) & 1)
             << "  AUTO=" << ((control >> 5) & 1)
             << "  STP=" << ((control >> 6) & 1)
             << "  DSEL=" << ((control >> 7) & 1) << endl;
        
        time_t t = read_rtc();
        cout << "\n RTC: " << ctime(&t);
        
        double interval = read_interval_timer();
        cout << " Interval Timer: " << fixed << setprecision(3) << interval << " seg" << endl;
        
        uint32_t cycles = read_cycle_counter();
        cout << " Cycle Counter: " << cycles << " ciclos" << endl;
        
        cout << "========================================" << endl;
    }
    
    // ========================================================================
    // HEX DUMP
    // ========================================================================
    void hex_dump(const vector<uint8_t>& data, const string& title) {
        cout << "\n=== " << title << " ===" << endl;
        for (size_t i = 0; i < data.size(); i++) {
            if (i % 16 == 0) {
                if (i > 0) cout << endl;
                cout << "0x" << hex << setw(4) << setfill('0') << i << ": ";
            }
            cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
        }
        cout << dec << endl;
    }
};

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================
int main(int argc, char* argv[]) {
    // Dispositivo que FUNCIONA (totalmente desbloqueado)
    string device_id = "04-000000593dda";
    
    DS1994 ds1994(device_id);
    
    string cmd = (argc > 1) ? argv[1] : "info";
    
    cout << "\n╔══════════════════════════════════════════════════════════════╗" << endl;
    cout << "║     DS1994-F5 COMPLETE DRIVER - SIN SKIP_ROM (DATASHEET)      ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════════╝" << endl;
    
    try {
        if (cmd == "info") {
            ds1994.print_info();
        }
        else if (cmd == "dump") {
            ds1994.dump_memory(true);
        }
        else if (cmd == "status") {
            uint8_t status = ds1994.read_status();
            uint8_t control = ds1994.read_control();
            cout << "Status:  0x" << hex << setw(2) << setfill('0') << (int)status << dec << endl;
            cout << "Control: 0x" << hex << setw(2) << setfill('0') << (int)control << dec << endl;
        }
        else if (cmd == "rtc") {
            time_t t = ds1994.read_rtc();
            cout << "RTC: " << ctime(&t);
        }
        else if (cmd == "setrtc") {
            time_t now = time(nullptr);
            cout << "Escribiendo RTC: " << now << " (" << ctime(&now) << ")" << endl;
            if (ds1994.write_rtc(now)) {
                cout << "✅ RTC actualizado" << endl;
            } else {
                cout << "❌ Falló escritura RTC" << endl;
            }
        }
        else if (cmd == "interval") {
            double val = ds1994.read_interval_timer();
            cout << "Interval Timer: " << fixed << setprecision(3) << val << " seg" << endl;
        }
        else if (cmd == "cycles") {
            uint32_t val = ds1994.read_cycle_counter();
            cout << "Cycle Counter: " << val << " ciclos" << endl;
        }
        else if (cmd == "osc") {
            bool enable = (argc > 2 && string(argv[2]) == "on");
            if (ds1994.set_oscillator(enable)) {
                cout << "Oscilador " << (enable ? "ENCENDIDO" : "APAGADO") << endl;
            }
        }
        else if (cmd == "read") {
            if (argc < 3) {
                cout << "Uso: read <pagina 0-15>" << endl;
                return 1;
            }
            int page = stoi(argv[2]);
            auto data = ds1994.read_page(page);
            ds1994.hex_dump(data, "Página " + to_string(page));
        }
        else if (cmd == "write") {
            if (argc < 4) {
                cout << "Uso: write <pagina> <hex_data>" << endl;
                return 1;
            }
            int page = stoi(argv[2]);
            string hex = argv[3];
            vector<uint8_t> data;
            for (size_t i = 0; i + 1 < hex.size(); i += 2) {
                data.push_back(stoi(hex.substr(i, 2), nullptr, 16));
            }
            if (ds1994.write_page(page, data)) {
                cout << "✅ Escritura exitosa" << endl;
                auto verify = ds1994.read_page(page);
                ds1994.hex_dump(verify, "Verificación página " + to_string(page));
            }
        }
        else if (cmd == "test") {
            // PRUEBA DE ESCRITURA COMPLETA
            cout << "\n=== PRUEBA DE ESCRITURA ===" << endl;
            
            // Leer antes
            cout << "Leyendo valor original..." << endl;
            auto before = ds1994.read_memory(0x0000, 1);
            cout << "Original: 0x" << hex << (before.empty() ? 0 : (int)before[0]) << dec << endl;
            
            // Escribir 0xAA
            cout << "Escribiendo 0xAA..." << endl;
            if (ds1994.write_memory(0x0000, {0xAA})) {
                // Leer después
                auto after = ds1994.read_memory(0x0000, 1);
                cout << "Después: 0x" << hex << (after.empty() ? 0 : (int)after[0]) << dec << endl;
                
                if (!after.empty() && after[0] == 0xAA) {
                    cout << "✅ PRUEBA EXITOSA: El dispositivo escribe correctamente!" << endl;
                }
            } else {
                cout << "❌ Error en escritura" << endl;
            }
        }
        else {
            cout << "Comandos: info, dump, status, rtc, setrtc, interval, cycles, " << endl;
            cout << "          osc on/off, read <pag>, write <pag> <hex>, test" << endl;
        }
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
