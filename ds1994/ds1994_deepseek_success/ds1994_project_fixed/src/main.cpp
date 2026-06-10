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
