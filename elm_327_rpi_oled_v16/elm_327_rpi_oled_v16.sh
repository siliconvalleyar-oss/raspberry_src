#!/bin/bash
# =============================================================================
# build_rpi_obd2.sh
# Genera el proyecto C++ completo OBD2 + OLED SSD1306 SPI para Raspberry Pi 2W
#
# Hardware:
#   - Raspberry Pi Zero 2W
#   - Pantalla OLED SSD1306 128x64 via SPI
#   - ELM327 via Bluetooth RFCOMM (igual que versión PC)
#
# Conexión SSD1306 SPI → RPi 2W GPIO:
#   VCC  → Pin 1  (3.3V)
#   GND  → Pin 6  (GND)
#   DIN  → Pin 19 (GPIO10, SPI0 MOSI)
#   CLK  → Pin 23 (GPIO11, SPI0 SCLK)
#   CS   → Pin 24 (GPIO8,  SPI0 CE0)
#   DC   → Pin 22 (GPIO25)
#   RES  → Pin 11 (GPIO17)
#
# Dependencias del sistema:
#   sudo apt install -y libbluetooth-dev cmake build-essential
#   sudo apt install -y libgpiod-dev
#   sudo raspi-config → Interface Options → SPI → Enable
#
# =============================================================================

set -e

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; NC='\033[0m'

log()  { echo -e "${GREEN}[OK]${NC} $1"; }
info() { echo -e "${CYAN}[INFO]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
fail() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

PROJECT="obd2_rpi"

info "Creando proyecto: $PROJECT"
mkdir -p "$PROJECT"/{src,include,build}
cd "$PROJECT"

# =============================================================================
# ESTRUCTURA:
#   include/elm327.hpp
#   include/gm_commands.hpp
#   include/logger.hpp
#   include/ssd1306.hpp       ← NUEVO: driver OLED SPI
#   include/oled_display.hpp  ← NUEVO: lógica de pantallas OLED
#   src/elm327.cpp
#   src/gm_commands.cpp
#   src/logger.cpp
#   src/ssd1306.cpp           ← NUEVO: driver SSD1306 SPI bajo nivel
#   src/oled_display.cpp      ← NUEVO: pages/widgets para OLED
#   src/main.cpp
#   CMakeLists.txt
# =============================================================================

# ─────────────────────────────────────────────────────────────────────────────
# INCLUDE: elm327.hpp
# ─────────────────────────────────────────────────────────────────────────────
cat > include/elm327.hpp << 'HPP'
#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>

struct OxygenSensor {
    int bank;
    int sensor;
    double voltage;
    double shortTermTrim;
};

struct DTCCode {
    std::string code;
    std::string description;
};

struct FuelTrim {
    double shortTermBank1;
    double shortTermBank2;
    double longTermBank1;
    double longTermBank2;
    bool available;
};

class ELM327
{
public:
    ELM327(const std::string& mac, int channel = 1);
    ~ELM327();

    bool connectBT();
    void disconnect();

    bool isConnected() { return sock >= 0; }
    bool isOnline()    { return sock >= 0; }

    std::string send(const std::string& cmd, int delayMs = 200);

    // Parámetros básicos OBD-II
    int    getRPM();
    int    getSpeed();
    int    getTemp();
    int    getCoolantTemp();
    int    getEngineLoad();
    double getThrottlePosition();
    double getIntakePressure();
    int    getIntakeTemp();
    double getTimingAdvance();
    double getFuelPressure();
    double getMAF();
    double getFuelLevel();
    double getAmbientTemp();
    double getOilTemp();
    double getCommandedEGR();
    double getEGRError();
    double getEVAPPressure();
    double getBarometricPressure();

    // Fuel Trim — PIDs corregidos (0106/0108 STFT, 0107/0109 LTFT)
    double getShortTermTrimBank1();
    double getShortTermTrimBank2();
    double getLongTermTrimBank1();
    double getLongTermTrimBank2();
    FuelTrim getAllFuelTrims();

    // Sensores O2 — 1 sensor por PID (0x14-0x1B)
    std::vector<OxygenSensor> getOxygenSensors();
    OxygenSensor getO2Sensor(int bank, int sensor);

    // DTCs
    std::vector<DTCCode> getDTCs();
    bool clearDTCs();

    // Diagnóstico
    std::string getProtocol();
    std::string getVehicleInfo();
    std::string getVIN();
    bool checkMIL();
    bool setProtocol(int protocol);
    bool resetELM();

    // Modo 22 GM
    std::string sendCommand(const std::string& pidHex);

    // Logging
    void logAllSensorsRaw(const std::string& filename);
    void logP0171Diagnostic(const std::string& filename, int durationSec = 60);

private:
    int sock;
    std::string mac;
    int channel;

    std::string readRaw();
    std::string parseResponse(const std::string& response, const std::string& expected);
    std::string decodeDTCCode(const std::string& code);
    std::vector<std::string> splitResponse(const std::string& response);
};
HPP

# ─────────────────────────────────────────────────────────────────────────────
# INCLUDE: gm_commands.hpp
# ─────────────────────────────────────────────────────────────────────────────
cat > include/gm_commands.hpp << 'HPP'
#pragma once
#include "elm327.hpp"
#include <string>
#include <cstdint>

class GMCommands {
public:
    explicit GMCommands(ELM327* elm);
    ~GMCommands();

    std::string getKilometers();
    std::string getCatalystTemp();
    std::string getFuelPressure();
    std::string getEngineTorque();
    std::string getECUVoltage();
    std::string getGMHistory();
    std::string clearGMHistory();
    std::string resetAdaptations();
    void        scanPIDs(uint16_t start = 0xB000, uint16_t end = 0xB1FF);

    std::string sendCommand(const std::string& pidHex, bool useGMHeader = true);

    std::string decodeKilometers(const std::string& hexResponse);
    std::string decodeCatalystTemp(const std::string& hexResponse);
    std::string decodeFuelPressure(const std::string& hexResponse);
    std::string decodeEngineTorque(const std::string& hexResponse);
    std::string decodeECUVoltage(const std::string& hexResponse);
    std::string decodeGMHistory(const std::string& hexResponse);

private:
    ELM327* elm;
    bool setupGMHeader();
    bool restoreHeader();
    std::string processResponse(const std::string& response, const std::string& expectedPrefix = "62");
    void showError(const std::string& cmd, const std::string& response);
};
HPP

# ─────────────────────────────────────────────────────────────────────────────
# INCLUDE: logger.hpp
# ─────────────────────────────────────────────────────────────────────────────
cat > include/logger.hpp << 'HPP'
#pragma once
#include <fstream>
#include <memory>
#include <string>

class Logger {
private:
    std::unique_ptr<std::ofstream> file;
    std::string filename;
public:
    Logger();
    ~Logger();
    bool open();
    void log(const std::string& key, const std::string& value);
    void log(const std::string& key, int value);
    void log(const std::string& key, float value);
    void log(const std::string& key, double value);
    void close();
};
HPP

# ─────────────────────────────────────────────────────────────────────────────
# INCLUDE: ssd1306.hpp  — Driver SPI bajo nivel para SSD1306 128x64
# ─────────────────────────────────────────────────────────────────────────────
cat > include/ssd1306.hpp << 'HPP'
#pragma once
// =============================================================================
// SSD1306 SPI Driver — Raspberry Pi (Linux spidev + gpiod)
//
// Conexión GPIO (configurable):
//   DIN  (MOSI) → SPI0_MOSI  GPIO10  Pin19
//   CLK  (SCLK) → SPI0_SCLK  GPIO11  Pin23
//   CS          → SPI0_CE0   GPIO8   Pin24
//   DC          → GPIO25     Pin22
//   RES (Reset) → GPIO17     Pin11
//
// El driver usa /dev/spidev0.0 con SPI mode 0, 8 MHz.
// Para mayor compatibilidad con RPi Zero 2W se usa ioctl directo.
// =============================================================================

#include <cstdint>
#include <string>
#include <array>
#include <vector>

// Dimensiones de la pantalla
constexpr int OLED_WIDTH  = 128;
constexpr int OLED_HEIGHT = 64;
constexpr int OLED_PAGES  = OLED_HEIGHT / 8;  // 8 páginas de 8 bits

class SSD1306 {
public:
    // Pines GPIO (BCM numbering)
    struct Config {
        std::string spiDevice = "/dev/spidev0.0";
        int gpioChip  = 0;     // /dev/gpiochip0
        int pinDC     = 25;    // Data/Command
        int pinReset  = 17;    // Reset
        uint32_t spiSpeedHz = 8000000;  // 8 MHz
    };

    explicit SSD1306(const Config& cfg = Config{});
    ~SSD1306();

    bool begin();
    void end();

    // Control
    void clear();
    void display();          // Vuelca el framebuffer a la pantalla
    void setContrast(uint8_t contrast);
    void invertDisplay(bool invert);
    void setBrightness(uint8_t b) { setContrast(b); }

    // Framebuffer (1 bit por píxel, organizado en páginas de 8 filas)
    // pixel(x,y) donde x=0..127, y=0..63
    void drawPixel(int x, int y, bool on = true);
    bool getPixel(int x, int y) const;

    // Primitivas de dibujo
    void drawLine(int x0, int y0, int x1, int y1, bool on = true);
    void drawRect(int x, int y, int w, int h, bool on = true);
    void fillRect(int x, int y, int w, int h, bool on = true);
    void drawCircle(int cx, int cy, int r, bool on = true);

    // Texto — fuente 5x7 integrada
    void drawChar(int x, int y, char c, int scale = 1, bool on = true);
    void drawString(int x, int y, const std::string& s, int scale = 1, bool on = true);
    int  charWidth(int scale = 1)  const { return 6 * scale; }
    int  charHeight(int scale = 1) const { return 8 * scale; }

    // Helpers de layout
    void drawStringCentered(int y, const std::string& s, int scale = 1);
    void drawStringRight(int y, const std::string& s, int scale = 1);

    // Barra de progreso horizontal
    void drawProgressBar(int x, int y, int w, int h, float percent, bool on = true);

    // Icono de señal (3 barras)
    void drawSignalIcon(int x, int y, int strength); // strength 0-3

    bool isReady() const { return ready_; }

private:
    Config cfg_;
    int    spiFd_   = -1;
    int    gpioFd_  = -1;   // fd del chip GPIO
    void*  dcLine_  = nullptr;
    void*  rstLine_ = nullptr;
    bool   ready_   = false;

    // Framebuffer: 128 * 8 páginas = 1024 bytes
    std::array<uint8_t, OLED_WIDTH * OLED_PAGES> fb_{};

    // Comandos SSD1306
    void sendCmd(uint8_t cmd);
    void sendCmds(std::initializer_list<uint8_t> cmds);
    void sendData(const uint8_t* data, size_t len);
    void spiWrite(const uint8_t* data, size_t len);

    // GPIO helpers
    bool  gpioInit();
    void  gpioFree();
    void  setDC(bool high);
    void  setReset(bool high);

    // Fuente 5x7
    static const uint8_t FONT5x7[][5];
};
HPP

# ─────────────────────────────────────────────────────────────────────────────
# INCLUDE: oled_display.hpp  — Páginas de visualización OBD2 en OLED
# ─────────────────────────────────────────────────────────────────────────────
cat > include/oled_display.hpp << 'HPP'
#pragma once
// =============================================================================
// OLEDDisplay — Gestiona las páginas de información OBD2 en la pantalla SSD1306
//
// Páginas disponibles (rotación automática o manual):
//   PAGE_MAIN    : RPM grande + Velocidad + Temp
//   PAGE_ENGINE  : Carga motor, MAF, Avance, Presión admisión
//   PAGE_FUEL    : STFT B1/B2, LTFT B1/B2, Nivel combustible
//   PAGE_O2      : Voltaje O2 B1S1 / B2S1 + trim
//   PAGE_GM      : Odómetro GM, Voltaje batería, Torque
//   PAGE_DTC     : Códigos de error activos
//   PAGE_DEBUG   : Última línea TX/RX BT (debug raw)
//   PAGE_SPLASH  : Pantalla de inicio / sin conexión
// =============================================================================

#include "ssd1306.hpp"
#include "elm327.hpp"
#include "gm_commands.hpp"
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>

enum class OLEDPage {
    SPLASH = 0,
    MAIN,
    ENGINE,
    FUEL,
    O2,
    GM,
    DTC,
    DEBUG,
    COUNT
};

// Datos compartidos entre el hilo OBD y el hilo de display
struct VehicleData {
    std::mutex mtx;

    // Estado conexión
    bool connected   = false;
    std::string protocol;
    std::string lastTX;
    std::string lastRX;

    // OBD-II estándar
    int    rpm          = 0;
    int    speed        = 0;
    int    coolantTemp  = 0;
    int    engineLoad   = 0;
    double throttle     = 0.0;
    double intakePressure = 0.0;
    int    intakeTemp   = 0;
    double timingAdvance  = 0.0;
    double maf          = 0.0;
    double fuelLevel    = 0.0;
    double baroPressure = 0.0;

    // Fuel Trim
    double stftBank1   = 0.0;
    double stftBank2   = 0.0;
    double ltftBank1   = 0.0;
    double ltftBank2   = 0.0;

    // O2
    double o2B1S1V     = 0.0;
    double o2B2S1V     = 0.0;
    double o2B1S1T     = 0.0;
    double o2B2S1T     = 0.0;

    // GM específico
    std::string gmKm        = "--";
    std::string gmVoltage   = "--";
    std::string gmTorque    = "--";
    std::string gmCatTemp   = "--";
    std::string gmFuelPress = "--";

    // DTCs
    std::vector<std::string> dtcCodes;
    bool milActive = false;

    // Timestamp última actualización
    std::chrono::steady_clock::time_point lastUpdate;
};

class OLEDDisplay {
public:
    OLEDDisplay(SSD1306& oled, VehicleData& data);
    ~OLEDDisplay();

    // Arrancar/detener el hilo de refresco
    void start(int refreshMs = 500);
    void stop();

    // Cambiar página manualmente
    void nextPage();
    void prevPage();
    void setPage(OLEDPage page);
    OLEDPage currentPage() const { return page_; }

    // Activar/desactivar rotación automática de páginas
    void setAutoRotate(bool enabled, int intervalSec = 5);

    // Dibuja la página actual al framebuffer y hace display()
    void render();

private:
    SSD1306&      oled_;
    VehicleData&  data_;
    OLEDPage      page_     = OLEDPage::SPLASH;
    bool          running_  = false;
    bool          autoRotate_ = true;
    int           autoIntervalSec_ = 5;
    int           refreshMs_  = 500;

    std::thread   thread_;
    std::chrono::steady_clock::time_point lastPageChange_;

    void threadLoop();

    // Renders por página
    void renderSplash();
    void renderMain();
    void renderEngine();
    void renderFuel();
    void renderO2();
    void renderGM();
    void renderDTC();
    void renderDebug();

    // Helpers comunes
    void drawHeader(const std::string& title, int pageNum);
    void drawFooter();
    void drawConnectionStatus();
    std::string fmtDouble(double v, int decimals = 1);
    std::string fmtInt(int v);
    bool dataFresh() const;  // true si última actualización < 5s
};
HPP

# ─────────────────────────────────────────────────────────────────────────────
# SRC: elm327.cpp — Código completo corregido
# ─────────────────────────────────────────────────────────────────────────────
cat > src/elm327.cpp << 'CPP'
// elm327.cpp — Comunicación BT RFCOMM con ELM327
// Código corregido: PIDs fuel trim, O2 sensor, fórmulas
#include "elm327.hpp"

#include <fstream>
#include <functional>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

ELM327::ELM327(const std::string& macAddr, int ch)
    : sock(-1), mac(macAddr), channel(ch) {}

ELM327::~ELM327() { disconnect(); }

bool ELM327::connectBT()
{
    struct sockaddr_rc addr{};
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock < 0) { perror("[ERROR] socket"); return false; }

    addr.rc_family  = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t)channel;
    str2ba(mac.c_str(), &addr.rc_bdaddr);

    std::cout << "[INFO] Conectando a " << mac << " canal " << channel << "...\n";

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[ERROR] connect");
        close(sock); sock = -1;
        return false;
    }
    std::cout << "[OK] Conectado BT!\n";

    // Secuencia de inicialización ELM327
    send("ATZ",   1000);  // reset completo
    send("ATE0",   300);  // echo OFF
    send("ATL0",   200);  // linefeeds OFF
    send("ATS0",   200);  // spaces OFF
    send("ATSP0",  500);  // protocolo automático
    send("ATAT1",  200);  // adaptive timing
    send("ATST20", 200);  // timeout 200ms

    return true;
}

void ELM327::disconnect()
{
    if (sock >= 0) {
        std::cout << "[INFO] Cerrando socket BT\n";
        close(sock);
        sock = -1;
    }
}

std::string ELM327::readRaw()
{
    if (!isConnected()) return "";

    // Leer con timeout vía select
    fd_set fds;
    struct timeval tv;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    tv.tv_sec  = 0;
    tv.tv_usec = 600000; // 600ms timeout lectura

    if (select(sock + 1, &fds, nullptr, nullptr, &tv) <= 0) return "";

    char buffer[4096];
    int n = read(sock, buffer, sizeof(buffer) - 1);
    if (n <= 0) return "";

    buffer[n] = '\0';
    std::string result(buffer);
    // Limpiar: quitar CR, LF, espacios, prompt '>'
    result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    result.erase(std::remove(result.begin(), result.end(), ' '),  result.end());
    result.erase(std::remove(result.begin(), result.end(), '>'),  result.end());
    // Convertir a mayúsculas
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string ELM327::send(const std::string& cmd, int delayMs)
{
    if (!isConnected()) return "";
    std::string full = cmd + "\r";
    std::cout << "[TX] " << cmd << std::endl;
    write(sock, full.c_str(), full.size());
    usleep(delayMs * 1000);
    std::string resp = readRaw();
    if (!resp.empty())
        std::cout << "[RX] " << resp.substr(0, 80) << std::endl;
    return resp;
}

std::vector<std::string> ELM327::splitResponse(const std::string& response)
{
    std::vector<std::string> results;
    // La respuesta ya viene limpia (sin espacios, en mayúsculas)
    std::string data = response;
    // Recortar hasta '>' si quedara alguno
    size_t arrowPos = data.find('>');
    if (arrowPos != std::string::npos) data = data.substr(0, arrowPos);

    for (size_t i = 0; i + 2 <= data.length(); i += 2)
        results.push_back(data.substr(i, 2));

    return results;
}

// ─── PARÁMETROS OBD-II ───────────────────────────────────────────────────────

int ELM327::getRPM() {
    auto b = splitResponse(send("010C"));
    if (b.size() >= 4 && b[0]=="41" && b[1]=="0C")
        return (std::stoi(b[2],nullptr,16)*256 + std::stoi(b[3],nullptr,16)) / 4;
    return -1;
}
int ELM327::getSpeed() {
    auto b = splitResponse(send("010D"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="0D")
        return std::stoi(b[2],nullptr,16);
    return -1;
}
int ELM327::getCoolantTemp() {
    auto b = splitResponse(send("0105"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="05")
        return std::stoi(b[2],nullptr,16) - 40;
    return -99;
}
int ELM327::getTemp() { return getCoolantTemp(); }
int ELM327::getEngineLoad() {
    auto b = splitResponse(send("0104"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="04")
        return (std::stoi(b[2],nullptr,16)*100)/255;
    return -1;
}
double ELM327::getThrottlePosition() {
    auto b = splitResponse(send("0111"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="11")
        return std::stoi(b[2],nullptr,16)*100.0/255.0;
    return -1.0;
}
double ELM327::getIntakePressure() {
    auto b = splitResponse(send("010B"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="0B")
        return std::stoi(b[2],nullptr,16);
    return -1.0;
}
int ELM327::getIntakeTemp() {
    auto b = splitResponse(send("010F"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="0F")
        return std::stoi(b[2],nullptr,16) - 40;
    return -99;
}
double ELM327::getTimingAdvance() {
    auto b = splitResponse(send("010E"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="0E")
        return std::stoi(b[2],nullptr,16)/2.0 - 64.0;
    return -99.0;
}
double ELM327::getFuelPressure() {
    auto b = splitResponse(send("010A"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="0A")
        return std::stoi(b[2],nullptr,16)*3.0;
    return -1.0;
}
double ELM327::getMAF() {
    auto b = splitResponse(send("0110"));
    if (b.size() >= 4 && b[0]=="41" && b[1]=="10")
        return (std::stoi(b[2],nullptr,16)*256 + std::stoi(b[3],nullptr,16))/100.0;
    return -1.0;
}
double ELM327::getFuelLevel() {
    auto b = splitResponse(send("012F"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="2F")
        return std::stoi(b[2],nullptr,16)*100.0/255.0;
    return -1.0;
}
double ELM327::getAmbientTemp() {
    auto b = splitResponse(send("0146"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="46")
        return std::stoi(b[2],nullptr,16) - 40.0;
    return -99.0;
}
double ELM327::getOilTemp() {
    auto b = splitResponse(send("015C"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="5C")
        return std::stoi(b[2],nullptr,16) - 40.0;
    return -99.0;
}
double ELM327::getCommandedEGR() {
    auto b = splitResponse(send("012C"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="2C")
        return std::stoi(b[2],nullptr,16)*100.0/255.0;
    return -1.0;
}
double ELM327::getEGRError() {
    auto b = splitResponse(send("012D"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="2D")
        return (std::stoi(b[2],nullptr,16)*100.0/128.0) - 100.0;
    return -1.0;
}
double ELM327::getEVAPPressure() {
    auto b = splitResponse(send("0132"));
    if (b.size() >= 4 && b[0]=="41" && b[1]=="32")
        return (std::stoi(b[2],nullptr,16)*256 + std::stoi(b[3],nullptr,16))/4.0;
    return -1.0;
}
double ELM327::getBarometricPressure() {
    auto b = splitResponse(send("0133"));
    if (b.size() >= 3 && b[0]=="41" && b[1]=="33")
        return std::stoi(b[2],nullptr,16);
    return -1.0;
}

// ─── FUEL TRIM — PIDs corregidos ─────────────────────────────────────────────
// Fórmula correcta OBD-II: ((A-128)*100)/128

double ELM327::getShortTermTrimBank1() {
    auto b = splitResponse(send("0106"));
    if (b.size()>=3 && b[0]=="41" && b[1]=="06")
        return ((std::stoi(b[2],nullptr,16)-128)*100.0)/128.0;
    return -999.0;
}
double ELM327::getShortTermTrimBank2() {
    auto b = splitResponse(send("0108"));
    if (b.size()>=3 && b[0]=="41" && b[1]=="08")
        return ((std::stoi(b[2],nullptr,16)-128)*100.0)/128.0;
    return -999.0;
}
double ELM327::getLongTermTrimBank1() {
    auto b = splitResponse(send("0107"));
    if (b.size()>=3 && b[0]=="41" && b[1]=="07")
        return ((std::stoi(b[2],nullptr,16)-128)*100.0)/128.0;
    return -999.0;
}
double ELM327::getLongTermTrimBank2() {
    auto b = splitResponse(send("0109"));
    if (b.size()>=3 && b[0]=="41" && b[1]=="09")
        return ((std::stoi(b[2],nullptr,16)-128)*100.0)/128.0;
    return -999.0;
}

FuelTrim ELM327::getAllFuelTrims() {
    FuelTrim ft;
    ft.shortTermBank1 = getShortTermTrimBank1();
    ft.shortTermBank2 = getShortTermTrimBank2();
    ft.longTermBank1  = getLongTermTrimBank1();
    ft.longTermBank2  = getLongTermTrimBank2();
    ft.available = (ft.shortTermBank1 != -999.0 || ft.longTermBank1 != -999.0);
    return ft;
}

// ─── SENSORES O2 — 1 sensor por PID (0x14-0x1B) ──────────────────────────────
// Voltaje: A*0.005 V    Trim: ((B-128)*100)/128 %

OxygenSensor ELM327::getO2Sensor(int bank, int sensor)
{
    OxygenSensor r;
    r.bank = bank; r.sensor = sensor;
    r.voltage = -1.0; r.shortTermTrim = -999.0;

    int pidBase;
    if (bank == 1 && sensor >= 1 && sensor <= 4) pidBase = 0x14 + (sensor-1);
    else if (bank == 2 && sensor >= 1 && sensor <= 4) pidBase = 0x18 + (sensor-1);
    else return r;

    std::stringstream ss;
    ss << "01" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << pidBase;
    auto bytes = splitResponse(send(ss.str()));

    std::stringstream pb;
    pb << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << pidBase;

    if (bytes.size()>=4 && bytes[0]=="41" && bytes[1]==pb.str()) {
        int vB = std::stoi(bytes[2],nullptr,16);
        int tB = std::stoi(bytes[3],nullptr,16);
        if (vB==0xFF && tB==0xFF) return r;
        r.voltage       = vB * 0.005;
        r.shortTermTrim = ((tB-128)*100.0)/128.0;
    }
    return r;
}

std::vector<OxygenSensor> ELM327::getOxygenSensors()
{
    std::vector<OxygenSensor> sensors;
    struct Map { int pid; int bank; int s; };
    std::vector<Map> map = {
        {0x14,1,1},{0x15,1,2},{0x16,1,3},{0x17,1,4},
        {0x18,2,1},{0x19,2,2},{0x1A,2,3},{0x1B,2,4}
    };
    for (auto& m : map) {
        auto s = getO2Sensor(m.bank, m.s);
        if (s.voltage >= 0) sensors.push_back(s);
    }
    return sensors;
}

// ─── DTCs ────────────────────────────────────────────────────────────────────

std::string ELM327::decodeDTCCode(const std::string& rawCode) {
    if (rawCode.length() < 4) return rawCode;
    // rawCode es 4 hex chars ej "0171"
    int hi = std::stoi(rawCode.substr(0,2),nullptr,16);
    int lo = std::stoi(rawCode.substr(2,2),nullptr,16);
    const char* types[] = {"P","C","B","U"};
    int typeIdx = (hi >> 6) & 0x03;
    int num = ((hi & 0x3F) << 8) | lo;
    std::stringstream ss;
    ss << types[typeIdx] << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << num;
    return ss.str();
}

std::vector<DTCCode> ELM327::getDTCs()
{
    std::vector<DTCCode> dtcs;
    std::string r = send("03", 700);
    auto bytes = splitResponse(r);

    if (!bytes.empty() && bytes[0]=="43") {
        for (size_t i=1; i+1<bytes.size(); i+=2) {
            std::string code = bytes[i]+bytes[i+1];
            if (code!="0000" && code!="FFFF") {
                DTCCode dtc;
                dtc.code = decodeDTCCode(code);
                dtc.description = dtc.code;
                dtcs.push_back(dtc);
            }
        }
    }
    if (dtcs.empty()) { DTCCode n; n.code="NONE"; n.description="Sin codigos"; dtcs.push_back(n); }
    return dtcs;
}

bool ELM327::clearDTCs()
{
    send("ATZ",2000); send("ATE0",300); send("ATL0",200); send("ATS0",200); send("ATSP0",500);
    for (int i=0; i<3; i++) {
        std::string r = send("04",1500);
        if (r.find("44")!=std::string::npos || r.find("OK")!=std::string::npos) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return false;
}

// ─── DIAGNÓSTICO ─────────────────────────────────────────────────────────────

std::string ELM327::getProtocol() {
    std::string r = send("ATDP",200);
    return r;
}

bool ELM327::checkMIL() {
    auto b = splitResponse(send("0101"));
    if (b.size()>=3) return (std::stoi(b[2],nullptr,16) & 0x80) != 0;
    return false;
}

bool ELM327::setProtocol(int p) {
    return send("ATSP"+std::to_string(p),500).find("OK")!=std::string::npos;
}
bool ELM327::resetELM() {
    return send("ATZ",2000).find("ELM")!=std::string::npos;
}

std::string ELM327::getVIN()
{
    std::string fullCmd = "0902\r";
    write(sock, fullCmd.c_str(), fullCmd.size());
    usleep(700000);
    char buf[1024]; int n = read(sock,buf,sizeof(buf)-1);
    if (n<=0) return "N/A";
    buf[n]='\0';
    std::string resp(buf);
    resp.erase(std::remove(resp.begin(),resp.end(),'\r'),resp.end());
    resp.erase(std::remove(resp.begin(),resp.end(),' '),resp.end());
    std::transform(resp.begin(),resp.end(),resp.begin(),::toupper);

    std::string vin;
    size_t pos = resp.find("4902");
    if (pos!=std::string::npos) {
        std::string data = resp.substr(pos+4);
        for (size_t i=2; i+2<=data.length(); i+=2) {
            try {
                int v = std::stoi(data.substr(i,2),nullptr,16);
                if (v>=32 && v<=126) vin += (char)v;
            } catch(...) { break; }
        }
    }
    return vin.empty() ? "N/A" : vin;
}

std::string ELM327::getVehicleInfo() {
    std::string info = "Protocolo: " + getProtocol() + "\n";
    info += "VIN: " + getVIN() + "\n";
    info += checkMIL() ? "MIL: ACTIVA\n" : "MIL: OK\n";
    return info;
}

// ─── MODO 22 GM — sendCommand ────────────────────────────────────────────────

std::string ELM327::sendCommand(const std::string& pidHex)
{
    if (!isConnected()) return "";
    // Configurar header GM
    send("AT SH 7E0", 100);
    send("AT CRA 7E8", 100);
    send("AT FC SH 7E0", 60);
    send("AT FC SD 30 00 00", 60);
    send("AT FC SM 1", 60);
    usleep(100000);

    std::string cmd = "22 " + pidHex;
    std::string full = cmd + "\r";
    std::cout << "[TX GM] " << cmd << std::endl;
    write(sock, full.c_str(), full.size());

    // Leer con timeout 600ms
    char buffer[512]; std::string response;
    fd_set fds; struct timeval tv;
    FD_ZERO(&fds); FD_SET(sock,&fds);
    tv.tv_sec=0; tv.tv_usec=600000;
    while (true) {
        if (select(sock+1,&fds,nullptr,nullptr,&tv) <= 0) break;
        int n = recv(sock,buffer,sizeof(buffer)-1,0);
        if (n<=0) break;
        buffer[n]='\0'; response += buffer;
        if (response.find(">")!=std::string::npos) break;
    }

    // Restaurar
    send("AT SH 7DF",100);
    send("AT CRA",100);
    send("AT FC SM 0",60);

    // Limpiar
    response.erase(std::remove(response.begin(),response.end(),'\r'),response.end());
    response.erase(std::remove(response.begin(),response.end(),'\n'),response.end());
    response.erase(std::remove(response.begin(),response.end(),' '),response.end());
    response.erase(std::remove(response.begin(),response.end(),'>'),response.end());
    std::transform(response.begin(),response.end(),response.begin(),::toupper);

    std::cout << "[RX GM] " << response << std::endl;
    if (response.find("7F")!=std::string::npos) return "";
    return response;
}

// ─── LOGGING ─────────────────────────────────────────────────────────────────

void ELM327::logAllSensorsRaw(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    file << "timestamp,sensor,value\n";

    struct SI { std::string name; std::function<std::string()> fn; };
    std::vector<SI> sensors = {
        {"RPM",         [this](){ return std::to_string(getRPM()); }},
        {"Speed",       [this](){ return std::to_string(getSpeed()); }},
        {"CoolantTemp", [this](){ return std::to_string(getCoolantTemp()); }},
        {"EngineLoad",  [this](){ return std::to_string(getEngineLoad()); }},
        {"Throttle",    [this](){ return std::to_string(getThrottlePosition()); }},
        {"MAP",         [this](){ return std::to_string(getIntakePressure()); }},
        {"IntakeTemp",  [this](){ return std::to_string(getIntakeTemp()); }},
        {"TimingAdv",   [this](){ return std::to_string(getTimingAdvance()); }},
        {"MAF",         [this](){ return std::to_string(getMAF()); }},
        {"FuelLevel",   [this](){ return std::to_string(getFuelLevel()); }},
        {"BaroPress",   [this](){ return std::to_string(getBarometricPressure()); }},
        {"STFT_B1",     [this](){ return std::to_string(getShortTermTrimBank1()); }},
        {"STFT_B2",     [this](){ return std::to_string(getShortTermTrimBank2()); }},
        {"LTFT_B1",     [this](){ return std::to_string(getLongTermTrimBank1()); }},
        {"LTFT_B2",     [this](){ return std::to_string(getLongTermTrimBank2()); }},
        {"O2_B1S1_V",   [this](){ return std::to_string(getO2Sensor(1,1).voltage); }},
        {"O2_B2S1_V",   [this](){ return std::to_string(getO2Sensor(2,1).voltage); }},
    };
    time_t now = time(nullptr);
    for (auto& s : sensors) {
        file << now << ",\"" << s.name << "\",\"" << s.fn() << "\"\n";
        file.flush();
    }
    file.close();
}

void ELM327::logP0171Diagnostic(const std::string& filename, int durationSec) {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    file << "timestamp,rpm,speed,stft_b1(%),stft_b2(%),ltft_b1(%),ltft_b2(%),o2_b1s1_V,o2_b2s1_V\n";

    time_t start = time(nullptr); int count=0;
    while (time(nullptr)-start < durationSec) {
        file << time(nullptr) << ","
             << getRPM() << ","
             << getSpeed() << ","
             << getShortTermTrimBank1() << ","
             << getShortTermTrimBank2() << ","
             << getLongTermTrimBank1() << ","
             << getLongTermTrimBank2() << ","
             << getO2Sensor(1,1).voltage << ","
             << getO2Sensor(2,1).voltage << "\n";
        file.flush();
        if (++count % 10 == 0)
            std::cout << "Registros: " << count << "\r" << std::flush;
        sleep(1);
    }
    file.close();
    std::cout << "\n[OK] Log P0171: " << count << " registros en " << filename << "\n";
}
CPP

# ─────────────────────────────────────────────────────────────────────────────
# SRC: gm_commands.cpp — Código completo corregido
# ─────────────────────────────────────────────────────────────────────────────
cat > src/gm_commands.cpp << 'CPP'
// gm_commands.cpp — Comandos GM modo 22 UDS con PIDs corregidos
#include "gm_commands.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdint>
#include <unistd.h>

GMCommands::GMCommands(ELM327* elm327) : elm(elm327) {}
GMCommands::~GMCommands() { restoreHeader(); }

bool GMCommands::setupGMHeader() {
    elm->send("AT SH 7E0", 100);
    elm->send("AT CRA 7E8", 100);
    elm->send("AT FC SH 7E0", 60);
    elm->send("AT FC SD 30 00 00", 60);
    elm->send("AT FC SM 1", 60);
    usleep(100000);
    return true;
}
bool GMCommands::restoreHeader() {
    elm->send("AT SH 7DF", 100);
    elm->send("AT CRA", 100);
    elm->send("AT FC SM 0", 60);
    return true;
}

std::string GMCommands::sendCommand(const std::string& pidHex, bool useGMHeader) {
    if (!elm->isOnline()) return "Error: Sin conexion";
    if (useGMHeader) setupGMHeader();
    std::string response = elm->send("22 " + pidHex, 600);
    if (useGMHeader) restoreHeader();
    if (response.empty()) return "Sin respuesta";
    if (response.find("7F") != std::string::npos) {
        size_t pos = response.find("7F");
        if (pos!=std::string::npos && response.length()>=pos+6) {
            std::string nrc = response.substr(pos+4,2);
            if (nrc=="11") return "Error: Servicio no soportado";
            if (nrc=="22") return "Error: Condiciones incorrectas";
            if (nrc=="31") return "Error: PID fuera de rango";
            return "Error UDS NRC=" + nrc;
        }
        return "Error UDS: " + response;
    }
    return response;
}

// Helper: extraer bytes de datos tras prefijo
static std::string extractData(const std::string& resp, const std::string& pfx) {
    std::string u=resp, p=pfx;
    std::transform(u.begin(),u.end(),u.begin(),::toupper);
    std::transform(p.begin(),p.end(),p.begin(),::toupper);
    u.erase(std::remove(u.begin(),u.end(),' '),u.end());
    p.erase(std::remove(p.begin(),p.end(),' '),p.end());
    size_t pos=u.find(p);
    return pos==std::string::npos ? "" : u.substr(pos+p.length());
}
static uint32_t toU32(const std::string& h, size_t off, int n) {
    uint32_t r=0;
    for(int i=0;i<n;i++){size_t idx=off+i*2;if(idx+2>h.length())break;r=(r<<8)|(uint32_t)std::stoi(h.substr(idx,2),nullptr,16);}
    return r;
}

// ODÓMETRO — PID B100, 4 bytes BE, unidad 0.1 km
std::string GMCommands::decodeKilometers(const std::string& r) {
    std::string d = extractData(r,"62B100");
    if (d.length()<8) return "Formato invalido: "+r;
    double km = toU32(d,0,4)/10.0;
    std::stringstream ss; ss<<std::fixed<<std::setprecision(1)<<km<<" km";
    return ss.str();
}
std::string GMCommands::getKilometers() {
    std::string r = sendCommand("B1 00",true);
    if (r.find("Error")!=std::string::npos||r.find("Sin")!=std::string::npos) return r;
    return decodeKilometers(r);
}

// TEMP CATALIZADOR — PID 01B4, (raw16*0.1)-40 °C
std::string GMCommands::decodeCatalystTemp(const std::string& r) {
    std::string d=extractData(r,"6201B4"); bool b2=false;
    if(d.empty()){d=extractData(r,"6201B5");b2=true;}
    if(d.length()<4) return "Formato invalido: "+r;
    double t=toU32(d,0,2)*0.1-40.0;
    std::stringstream ss; ss<<std::fixed<<std::setprecision(1)<<t<<" C (B"<<(b2?2:1)<<")";
    return ss.str();
}
std::string GMCommands::getCatalystTemp() {
    std::string r=sendCommand("01 B4",true);
    if(r.find("Error")!=std::string::npos||r.find("Sin")!=std::string::npos) r=sendCommand("01 B5",true);
    if(r.find("Error")!=std::string::npos||r.find("Sin")!=std::string::npos) return r;
    return decodeCatalystTemp(r);
}

// PRESIÓN COMBUSTIBLE — PID 1180, raw16*4 kPa
std::string GMCommands::decodeFuelPressure(const std::string& r) {
    std::string d=extractData(r,"621180");
    if(d.empty()) d=extractData(r,"621181");
    if(d.length()<4) return "Formato invalido: "+r;
    double kPa=toU32(d,0,2)*4.0;
    std::stringstream ss; ss<<std::fixed<<std::setprecision(1)<<kPa<<" kPa ("<<kPa/100.0<<"bar)";
    return ss.str();
}
std::string GMCommands::getFuelPressure() {
    std::string r=sendCommand("11 80",true);
    if(r.find("Error")!=std::string::npos||r.find("Sin")!=std::string::npos) r=sendCommand("11 81",true);
    if(r.find("Error")!=std::string::npos||r.find("Sin")!=std::string::npos) return r;
    return decodeFuelPressure(r);
}

// TORQUE — PID 01A9, (raw16*0.5)-848 Nm
std::string GMCommands::decodeEngineTorque(const std::string& r) {
    std::string d=extractData(r,"6201A9");
    if(d.length()<4) return "Formato invalido: "+r;
    double nm=toU32(d,0,2)*0.5-848.0;
    std::stringstream ss; ss<<std::fixed<<std::setprecision(1)<<nm<<" Nm";
    return ss.str();
}
std::string GMCommands::getEngineTorque() {
    std::string r=sendCommand("01 A9",true);
    if(r.find("Error")!=std::string::npos||r.find("Sin")!=std::string::npos) return r;
    return decodeEngineTorque(r);
}

// VOLTAJE ECU — PID 01A1, raw16*0.001 V
std::string GMCommands::decodeECUVoltage(const std::string& r) {
    std::string d=extractData(r,"6201A1"); bool alt=false;
    if(d.empty()){d=extractData(r,"620280");alt=true;}
    if(d.length()<4) return "Formato invalido: "+r;
    double v=toU32(d,0,2)*(alt?0.1:0.001);
    std::stringstream ss; ss<<std::fixed<<std::setprecision(3)<<v<<" V";
    if(v<11.5) ss<<" BAJA!";
    return ss.str();
}
std::string GMCommands::getECUVoltage() {
    std::string r=sendCommand("01 A1",true);
    if(r.find("Error")!=std::string::npos||r.find("Sin")!=std::string::npos) r=sendCommand("02 80",true);
    if(r.find("Error")!=std::string::npos||r.find("Sin")!=std::string::npos) return r;
    return decodeECUVoltage(r);
}

// HISTORIAL — Servicio 19 02 FF
std::string GMCommands::decodeGMHistory(const std::string& r) {
    if(r.empty()) return "Sin datos";
    std::string d=extractData(r,"5902");
    if(d.empty()) return "Formato invalido";
    if(d.length()>2) d=d.substr(2); // saltar availability mask
    if(d.empty()) return "Sin codigos";
    const char* types[]={"P","C","B","U"};
    std::string result; int cnt=0;
    for(size_t i=0; i+8<=d.length(); i+=8) {
        uint8_t b1=(uint8_t)std::stoi(d.substr(i,2),nullptr,16);
        uint8_t b2=(uint8_t)std::stoi(d.substr(i+2,2),nullptr,16);
        uint8_t b3=(uint8_t)std::stoi(d.substr(i+4,2),nullptr,16);
        uint8_t st=(uint8_t)std::stoi(d.substr(i+6,2),nullptr,16);
        if(b1==0&&b2==0&&b3==0) continue;
        int ti=(b1>>6)&3; uint32_t n=((b1&0x3F)<<16)|(b2<<8)|b3;
        std::stringstream ss;
        ss<<types[ti]<<std::setw(4)<<std::setfill('0')<<std::hex<<std::uppercase<<n;
        if(st&0x08) ss<<" CONF"; if(st&0x01) ss<<" ACT";
        if(!result.empty()) result+=", ";
        result+=ss.str(); cnt++;
    }
    if(result.empty()) return "Sin codigos";
    return std::to_string(cnt)+" DTC: "+result;
}
std::string GMCommands::getGMHistory() {
    std::string r=sendCommand("19 02 FF",true);
    if(r.find("Error")!=std::string::npos||r.find("Sin")!=std::string::npos) return r;
    return decodeGMHistory(r);
}
std::string GMCommands::clearGMHistory() {
    std::string r=sendCommand("14 FF FF FF",true);
    if(r.find("54")!=std::string::npos) return "Historial borrado OK";
    return "Error: "+r;
}
std::string GMCommands::resetAdaptations() {
    std::string r=sendCommand("31 01 C1 0F",true);
    if(r.find("71")!=std::string::npos) return "Adaptativos reseteados OK";
    return "Error: "+r;
}

void GMCommands::scanPIDs(uint16_t start, uint16_t end) {
    std::cout<<"\n=== SCAN PIDs GM modo 22 ===\n";
    setupGMHeader();
    int found=0;
    for(uint32_t pid=start; pid<=end; pid++) {
        std::stringstream ss;
        ss<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<((pid>>8)&0xFF)
          <<" "<<std::setw(2)<<std::setfill('0')<<(pid&0xFF);
        std::string r=elm->send("22 "+ss.str(),400);
        if(!r.empty()&&r.find("7F")==std::string::npos&&r.find("62")!=std::string::npos) {
            found++;
            std::cout<<"✓ 0x"<<std::hex<<pid<<" -> "<<r<<"\n";
        }
        usleep(60000);
    }
    restoreHeader();
    std::cout<<"Encontrados: "<<std::dec<<found<<"\n";
}

std::string GMCommands::processResponse(const std::string& r, const std::string& pfx) {
    if(r.empty()||r.find("7F")!=std::string::npos) return "";
    if(!pfx.empty()&&r.find(pfx)==std::string::npos) return "";
    return r;
}
void GMCommands::showError(const std::string& cmd, const std::string& r) {
    std::cerr<<"[GM ERR] "<<cmd<<": "<<(r.empty()?"timeout":r)<<"\n";
}
CPP

# ─────────────────────────────────────────────────────────────────────────────
# SRC: logger.cpp
# ─────────────────────────────────────────────────────────────────────────────
cat > src/logger.cpp << 'CPP'
#include "logger.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

Logger::Logger() {}
Logger::~Logger() { close(); }

bool Logger::open() {
    time_t now=time(nullptr); tm* t=localtime(&now);
    std::stringstream ss;
    ss<<"log_"<<(t->tm_year+1900)
      <<std::setw(2)<<std::setfill('0')<<(t->tm_mon+1)
      <<std::setw(2)<<std::setfill('0')<<t->tm_mday
      <<"_"<<std::setw(2)<<std::setfill('0')<<t->tm_hour
      <<std::setw(2)<<std::setfill('0')<<t->tm_min<<".csv";
    filename=ss.str();
    file=std::make_unique<std::ofstream>(filename);
    if(!file->is_open()) return false;
    *file<<"timestamp,key,value\n";
    return true;
}
void Logger::log(const std::string& k,const std::string& v){
    if(!file||!file->is_open())return;
    *file<<time(nullptr)<<","<<k<<","<<v<<"\n"; file->flush();
}
void Logger::log(const std::string& k,int v)    { log(k,std::to_string(v)); }
void Logger::log(const std::string& k,float v)  { log(k,std::to_string(v)); }
void Logger::log(const std::string& k,double v) { log(k,std::to_string(v)); }
void Logger::close(){
    if(file&&file->is_open()){file->flush();file->close();}
}
CPP

# ─────────────────────────────────────────────────────────────────────────────
# SRC: ssd1306.cpp — Driver SPI completo para SSD1306
# ─────────────────────────────────────────────────────────────────────────────
cat > src/ssd1306.cpp << 'CPP'
// ssd1306.cpp — Driver SPI para SSD1306 128x64 en Raspberry Pi
// Usa Linux spidev (/dev/spidev0.0) + libgpiod para DC y RES
//
// Referencia: SSD1306 Datasheet Rev 1.1, Solomon Systech, 2008

#include "ssd1306.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <cmath>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <gpiod.h>

// ─── Fuente 5x7 (ASCII 32-127) ───────────────────────────────────────────────
// Cada carácter ocupa 5 bytes (columnas de 8 bits)
const uint8_t SSD1306::FONT5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // 32 espacio
    {0x00,0x00,0x5F,0x00,0x00}, // 33 !
    {0x00,0x07,0x00,0x07,0x00}, // 34 "
    {0x14,0x7F,0x14,0x7F,0x14}, // 35 #
    {0x24,0x2A,0x7F,0x2A,0x12}, // 36 $
    {0x23,0x13,0x08,0x64,0x62}, // 37 %
    {0x36,0x49,0x55,0x22,0x50}, // 38 &
    {0x00,0x05,0x03,0x00,0x00}, // 39 '
    {0x00,0x1C,0x22,0x41,0x00}, // 40 (
    {0x00,0x41,0x22,0x1C,0x00}, // 41 )
    {0x14,0x08,0x3E,0x08,0x14}, // 42 *
    {0x08,0x08,0x3E,0x08,0x08}, // 43 +
    {0x00,0x50,0x30,0x00,0x00}, // 44 ,
    {0x08,0x08,0x08,0x08,0x08}, // 45 -
    {0x00,0x60,0x60,0x00,0x00}, // 46 .
    {0x20,0x10,0x08,0x04,0x02}, // 47 /
    {0x3E,0x51,0x49,0x45,0x3E}, // 48 0
    {0x00,0x42,0x7F,0x40,0x00}, // 49 1
    {0x42,0x61,0x51,0x49,0x46}, // 50 2
    {0x21,0x41,0x45,0x4B,0x31}, // 51 3
    {0x18,0x14,0x12,0x7F,0x10}, // 52 4
    {0x27,0x45,0x45,0x45,0x39}, // 53 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 54 6
    {0x01,0x71,0x09,0x05,0x03}, // 55 7
    {0x36,0x49,0x49,0x49,0x36}, // 56 8
    {0x06,0x49,0x49,0x29,0x1E}, // 57 9
    {0x00,0x36,0x36,0x00,0x00}, // 58 :
    {0x00,0x56,0x36,0x00,0x00}, // 59 ;
    {0x08,0x14,0x22,0x41,0x00}, // 60 <
    {0x14,0x14,0x14,0x14,0x14}, // 61 =
    {0x00,0x41,0x22,0x14,0x08}, // 62 >
    {0x02,0x01,0x51,0x09,0x06}, // 63 ?
    {0x32,0x49,0x79,0x41,0x3E}, // 64 @
    {0x7E,0x11,0x11,0x11,0x7E}, // 65 A
    {0x7F,0x49,0x49,0x49,0x36}, // 66 B
    {0x3E,0x41,0x41,0x41,0x22}, // 67 C
    {0x7F,0x41,0x41,0x22,0x1C}, // 68 D
    {0x7F,0x49,0x49,0x49,0x41}, // 69 E
    {0x7F,0x09,0x09,0x09,0x01}, // 70 F
    {0x3E,0x41,0x49,0x49,0x7A}, // 71 G
    {0x7F,0x08,0x08,0x08,0x7F}, // 72 H
    {0x00,0x41,0x7F,0x41,0x00}, // 73 I
    {0x20,0x40,0x41,0x3F,0x01}, // 74 J
    {0x7F,0x08,0x14,0x22,0x41}, // 75 K
    {0x7F,0x40,0x40,0x40,0x40}, // 76 L
    {0x7F,0x02,0x0C,0x02,0x7F}, // 77 M
    {0x7F,0x04,0x08,0x10,0x7F}, // 78 N
    {0x3E,0x41,0x41,0x41,0x3E}, // 79 O
    {0x7F,0x09,0x09,0x09,0x06}, // 80 P
    {0x3E,0x41,0x51,0x21,0x5E}, // 81 Q
    {0x7F,0x09,0x19,0x29,0x46}, // 82 R
    {0x46,0x49,0x49,0x49,0x31}, // 83 S
    {0x01,0x01,0x7F,0x01,0x01}, // 84 T
    {0x3F,0x40,0x40,0x40,0x3F}, // 85 U
    {0x1F,0x20,0x40,0x20,0x1F}, // 86 V
    {0x3F,0x40,0x38,0x40,0x3F}, // 87 W
    {0x63,0x14,0x08,0x14,0x63}, // 88 X
    {0x07,0x08,0x70,0x08,0x07}, // 89 Y
    {0x61,0x51,0x49,0x45,0x43}, // 90 Z
    {0x00,0x7F,0x41,0x41,0x00}, // 91 [
    {0x02,0x04,0x08,0x10,0x20}, // 92 backslash
    {0x00,0x41,0x41,0x7F,0x00}, // 93 ]
    {0x04,0x02,0x01,0x02,0x04}, // 94 ^
    {0x40,0x40,0x40,0x40,0x40}, // 95 _
    {0x00,0x01,0x02,0x04,0x00}, // 96 `
    {0x20,0x54,0x54,0x54,0x78}, // 97 a
    {0x7F,0x48,0x44,0x44,0x38}, // 98 b
    {0x38,0x44,0x44,0x44,0x20}, // 99 c
    {0x38,0x44,0x44,0x48,0x7F}, // 100 d
    {0x38,0x54,0x54,0x54,0x18}, // 101 e
    {0x08,0x7E,0x09,0x01,0x02}, // 102 f
    {0x0C,0x52,0x52,0x52,0x3E}, // 103 g
    {0x7F,0x08,0x04,0x04,0x78}, // 104 h
    {0x00,0x44,0x7D,0x40,0x00}, // 105 i
    {0x20,0x40,0x44,0x3D,0x00}, // 106 j
    {0x7F,0x10,0x28,0x44,0x00}, // 107 k
    {0x00,0x41,0x7F,0x40,0x00}, // 108 l
    {0x7C,0x04,0x18,0x04,0x78}, // 109 m
    {0x7C,0x08,0x04,0x04,0x78}, // 110 n
    {0x38,0x44,0x44,0x44,0x38}, // 111 o
    {0x7C,0x14,0x14,0x14,0x08}, // 112 p
    {0x08,0x14,0x14,0x18,0x7C}, // 113 q
    {0x7C,0x08,0x04,0x04,0x08}, // 114 r
    {0x48,0x54,0x54,0x54,0x20}, // 115 s
    {0x04,0x3F,0x44,0x40,0x20}, // 116 t
    {0x3C,0x40,0x40,0x40,0x3C}, // 117 u
    {0x1C,0x20,0x40,0x20,0x1C}, // 118 v
    {0x3C,0x40,0x30,0x40,0x3C}, // 119 w
    {0x44,0x28,0x10,0x28,0x44}, // 120 x
    {0x0C,0x50,0x50,0x50,0x3C}, // 121 y
    {0x44,0x64,0x54,0x4C,0x44}, // 122 z
    {0x00,0x08,0x36,0x41,0x00}, // 123 {
    {0x00,0x00,0x7F,0x00,0x00}, // 124 |
    {0x00,0x41,0x36,0x08,0x00}, // 125 }
    {0x10,0x08,0x08,0x10,0x08}, // 126 ~
    {0x00,0x00,0x00,0x00,0x00}, // 127 DEL
};

// ─── Comandos de inicialización SSD1306 ──────────────────────────────────────
static const uint8_t INIT_CMDS[] = {
    0xAE,       // display OFF
    0xD5, 0x80, // Set Display Clock Divide Ratio / Oscillator Frequency
    0xA8, 0x3F, // Set Multiplex Ratio (63 = 64 líneas)
    0xD3, 0x00, // Set Display Offset = 0
    0x40,       // Set Display Start Line = 0
    0x8D, 0x14, // Charge Pump: enable (0x14=ON, 0x10=OFF)
    0x20, 0x00, // Memory Addressing Mode: Horizontal
    0xA1,       // Segment Re-map: col 127→SEG0 (espejo horizontal)
    0xC8,       // COM Output Scan Direction: remapped (espejo vertical)
    0xDA, 0x12, // Set COM Pins Hardware Configuration
    0x81, 0xCF, // Set Contrast Control
    0xD9, 0xF1, // Set Pre-charge Period
    0xDB, 0x40, // Set VCOMH Deselect Level
    0xA4,       // Entire Display ON (follow RAM)
    0xA6,       // Set Normal Display (no invertir)
    0x2E,       // Deactivate scroll
    0xAF        // display ON
};

// ─── Constructor / Destructor ────────────────────────────────────────────────

SSD1306::SSD1306(const Config& cfg) : cfg_(cfg) {}

SSD1306::~SSD1306() { end(); }

// ─── Inicialización ──────────────────────────────────────────────────────────

bool SSD1306::begin()
{
    // Abrir SPI
    spiFd_ = open(cfg_.spiDevice.c_str(), O_RDWR);
    if (spiFd_ < 0) {
        std::cerr << "[SSD1306] No se pudo abrir " << cfg_.spiDevice
                  << " — ¿SPI habilitado con raspi-config?\n";
        return false;
    }

    // Configurar SPI: Mode 0, 8 bits, velocidad
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = cfg_.spiSpeedHz;
    ioctl(spiFd_, SPI_IOC_WR_MODE, &mode);
    ioctl(spiFd_, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spiFd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    // GPIO
    if (!gpioInit()) return false;

    // Reset hardware: RES=LOW 10ms → HIGH
    setReset(false); usleep(10000);
    setReset(true);  usleep(10000);

    // Enviar secuencia de inicialización
    for (uint8_t cmd : INIT_CMDS) sendCmd(cmd);

    clear();
    display();

    ready_ = true;
    std::cout << "[SSD1306] OLED inicializado OK\n";
    return true;
}

void SSD1306::end()
{
    if (ready_) {
        // Apagar display antes de cerrar
        sendCmd(0xAE);
        ready_ = false;
    }
    gpioFree();
    if (spiFd_ >= 0) { close(spiFd_); spiFd_ = -1; }
}

// ─── GPIO con libgpiod ───────────────────────────────────────────────────────

bool SSD1306::gpioInit()
{
    std::string chipPath = "/dev/gpiochip" + std::to_string(cfg_.gpioChip);
    gpioFd_ = open(chipPath.c_str(), O_RDWR);
    if (gpioFd_ < 0) {
        std::cerr << "[SSD1306] No se pudo abrir " << chipPath << "\n";
        return false;
    }

    struct gpiochip_info info;
    ioctl(gpioFd_, GPIO_GET_CHIPINFO_IOCTL, &info);

    // Configurar DC y RES como salida via ioctl
    // Usamos gpiod_chip / gpiod_line para compatibilidad
    struct gpiod_chip* chip = gpiod_chip_open(chipPath.c_str());
    if (!chip) {
        std::cerr << "[SSD1306] gpiod_chip_open falló\n";
        return false;
    }

    struct gpiod_line* dc  = gpiod_chip_get_line(chip, cfg_.pinDC);
    struct gpiod_line* rst = gpiod_chip_get_line(chip, cfg_.pinReset);

    if (!dc || !rst) {
        std::cerr << "[SSD1306] No se pudieron obtener líneas GPIO DC=" 
                  << cfg_.pinDC << " RST=" << cfg_.pinReset << "\n";
        gpiod_chip_close(chip);
        return false;
    }

    gpiod_line_request_output(dc,  "ssd1306_dc",  0);
    gpiod_line_request_output(rst, "ssd1306_rst", 1);

    dcLine_  = dc;
    rstLine_ = rst;

    // Guardar chip handle en gpioFd_ area — usamos puntero a chip
    close(gpioFd_);
    gpioFd_ = -1;

    // Guardamos el chip como void* usando un miembro adicional
    // (reutilizamos gpioFd_ como flag, chip se guarda en dcLine_ parent)
    return true;
}

void SSD1306::gpioFree()
{
    if (dcLine_)  { gpiod_line_release((struct gpiod_line*)dcLine_);  dcLine_=nullptr;  }
    if (rstLine_) { gpiod_line_release((struct gpiod_line*)rstLine_); rstLine_=nullptr; }
}

void SSD1306::setDC(bool high) {
    if (dcLine_) gpiod_line_set_value((struct gpiod_line*)dcLine_, high ? 1 : 0);
}
void SSD1306::setReset(bool high) {
    if (rstLine_) gpiod_line_set_value((struct gpiod_line*)rstLine_, high ? 1 : 0);
}

// ─── SPI ─────────────────────────────────────────────────────────────────────

void SSD1306::spiWrite(const uint8_t* data, size_t len)
{
    if (spiFd_ < 0 || !data || len == 0) return;
    struct spi_ioc_transfer tr{};
    tr.tx_buf       = (unsigned long)data;
    tr.rx_buf       = 0;
    tr.len          = len;
    tr.speed_hz     = cfg_.spiSpeedHz;
    tr.bits_per_word= 8;
    tr.delay_usecs  = 0;
    ioctl(spiFd_, SPI_IOC_MESSAGE(1), &tr);
}

void SSD1306::sendCmd(uint8_t cmd) {
    setDC(false);  // DC=0 → comando
    spiWrite(&cmd, 1);
}

void SSD1306::sendCmds(std::initializer_list<uint8_t> cmds) {
    setDC(false);
    for (uint8_t c : cmds) spiWrite(&c, 1);
}

void SSD1306::sendData(const uint8_t* data, size_t len) {
    setDC(true);   // DC=1 → datos
    spiWrite(data, len);
}

// ─── Framebuffer ─────────────────────────────────────────────────────────────

void SSD1306::clear() {
    fb_.fill(0x00);
}

void SSD1306::display()
{
    if (!ready_) return;
    // Configurar ventana completa
    sendCmds({0x21, 0, 127}); // column start=0, end=127
    sendCmds({0x22, 0, 7});   // page start=0, end=7
    sendData(fb_.data(), fb_.size());
}

void SSD1306::setContrast(uint8_t contrast) {
    sendCmds({0x81, contrast});
}
void SSD1306::invertDisplay(bool invert) {
    sendCmd(invert ? 0xA7 : 0xA6);
}

// ─── Primitivas ──────────────────────────────────────────────────────────────

void SSD1306::drawPixel(int x, int y, bool on)
{
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) return;
    int page = y / 8;
    int bit  = y % 8;
    int idx  = page * OLED_WIDTH + x;
    if (on) fb_[idx] |=  (1 << bit);
    else    fb_[idx] &= ~(1 << bit);
}

bool SSD1306::getPixel(int x, int y) const {
    if (x<0||x>=OLED_WIDTH||y<0||y>=OLED_HEIGHT) return false;
    return (fb_[(y/8)*OLED_WIDTH+x] >> (y%8)) & 1;
}

void SSD1306::drawLine(int x0, int y0, int x1, int y1, bool on)
{
    int dx = std::abs(x1-x0), dy = std::abs(y1-y0);
    int sx = x0<x1?1:-1, sy = y0<y1?1:-1;
    int err = dx-dy;
    while (true) {
        drawPixel(x0,y0,on);
        if (x0==x1 && y0==y1) break;
        int e2=2*err;
        if (e2>-dy){err-=dy; x0+=sx;}
        if (e2< dx){err+=dx; y0+=sy;}
    }
}

void SSD1306::drawRect(int x, int y, int w, int h, bool on) {
    drawLine(x,   y,   x+w-1, y,     on);
    drawLine(x,   y+h-1,x+w-1,y+h-1, on);
    drawLine(x,   y,   x,     y+h-1, on);
    drawLine(x+w-1,y,  x+w-1, y+h-1, on);
}

void SSD1306::fillRect(int x, int y, int w, int h, bool on) {
    for (int j=y; j<y+h; j++)
        for (int i=x; i<x+w; i++)
            drawPixel(i,j,on);
}

void SSD1306::drawCircle(int cx, int cy, int r, bool on) {
    int x=r, y=0, err=0;
    while (x>=y) {
        drawPixel(cx+x,cy+y,on); drawPixel(cx+y,cy+x,on);
        drawPixel(cx-y,cy+x,on); drawPixel(cx-x,cy+y,on);
        drawPixel(cx-x,cy-y,on); drawPixel(cx-y,cy-x,on);
        drawPixel(cx+y,cy-x,on); drawPixel(cx+x,cy-y,on);
        y++;
        if (err<=0) err+=2*y+1;
        else { x--; err+=2*(y-x)+1; }
    }
}

// ─── Texto ───────────────────────────────────────────────────────────────────

void SSD1306::drawChar(int x, int y, char c, int scale, bool on)
{
    int idx = (int)c - 32;
    if (idx < 0 || idx >= 96) idx = 0;
    const uint8_t* glyph = FONT5x7[idx];

    for (int col=0; col<5; col++) {
        uint8_t column = glyph[col];
        for (int row=0; row<8; row++) {
            if ((column >> row) & 1) {
                if (scale == 1) {
                    drawPixel(x+col, y+row, on);
                } else {
                    fillRect(x+col*scale, y+row*scale, scale, scale, on);
                }
            }
        }
    }
}

void SSD1306::drawString(int x, int y, const std::string& s, int scale, bool on)
{
    int cx = x;
    for (char c : s) {
        if (c == '\n') { cx=x; y += 8*scale+1; continue; }
        drawChar(cx, y, c, scale, on);
        cx += 6*scale;
        if (cx >= OLED_WIDTH) break;
    }
}

void SSD1306::drawStringCentered(int y, const std::string& s, int scale)
{
    int len = (int)s.length() * 6 * scale - scale; // último char sin gap
    int x = (OLED_WIDTH - len) / 2;
    if (x < 0) x = 0;
    drawString(x, y, s, scale);
}

void SSD1306::drawStringRight(int y, const std::string& s, int scale)
{
    int len = (int)s.length() * 6 * scale - scale;
    int x = OLED_WIDTH - len - 1;
    if (x < 0) x = 0;
    drawString(x, y, s, scale);
}

void SSD1306::drawProgressBar(int x, int y, int w, int h, float percent, bool on)
{
    drawRect(x, y, w, h, on);
    int fill = (int)((w-2) * std::max(0.0f, std::min(1.0f, percent/100.0f)));
    if (fill > 0) fillRect(x+1, y+1, fill, h-2, on);
}

void SSD1306::drawSignalIcon(int x, int y, int strength)
{
    // 3 barras verticales de altura creciente
    for (int i=0; i<3; i++) {
        int bh = 3 + i*3;
        int bx = x + i*4;
        int by = y + (9-bh);
        if (i < strength) fillRect(bx, by, 3, bh, true);
        else              drawRect(bx, by, 3, bh, true);
    }
}
CPP

# ─────────────────────────────────────────────────────────────────────────────
# SRC: oled_display.cpp — Páginas de visualización OBD2
# ─────────────────────────────────────────────────────────────────────────────
cat > src/oled_display.cpp << 'CPP'
// oled_display.cpp — Renderizado de páginas OBD2 en SSD1306 128x64
#include "oled_display.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

OLEDDisplay::OLEDDisplay(SSD1306& oled, VehicleData& data)
    : oled_(oled), data_(data)
{
    lastPageChange_ = std::chrono::steady_clock::now();
}

OLEDDisplay::~OLEDDisplay() { stop(); }

// ─── Control del hilo ────────────────────────────────────────────────────────

void OLEDDisplay::start(int refreshMs)
{
    if (running_) return;
    refreshMs_ = refreshMs;
    running_ = true;
    thread_ = std::thread(&OLEDDisplay::threadLoop, this);
}

void OLEDDisplay::stop()
{
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

void OLEDDisplay::threadLoop()
{
    while (running_) {
        auto t0 = std::chrono::steady_clock::now();

        // Auto-rotación de páginas
        if (autoRotate_ && data_.connected) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                t0 - lastPageChange_).count();
            if (elapsed >= autoIntervalSec_) {
                int next = (static_cast<int>(page_) + 1) % static_cast<int>(OLEDPage::COUNT);
                if (next == static_cast<int>(OLEDPage::SPLASH)) next = 1; // skip splash
                page_ = static_cast<OLEDPage>(next);
                lastPageChange_ = t0;
            }
        }

        render();

        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
        int wait = refreshMs_ - (int)dt;
        if (wait > 0) std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    }
}

void OLEDDisplay::nextPage() {
    int next = (static_cast<int>(page_) + 1) % static_cast<int>(OLEDPage::COUNT);
    if (next == 0) next = 1;
    page_ = static_cast<OLEDPage>(next);
    lastPageChange_ = std::chrono::steady_clock::now();
}
void OLEDDisplay::prevPage() {
    int prev = static_cast<int>(page_) - 1;
    if (prev <= 0) prev = static_cast<int>(OLEDPage::COUNT) - 1;
    page_ = static_cast<OLEDPage>(prev);
    lastPageChange_ = std::chrono::steady_clock::now();
}
void OLEDDisplay::setPage(OLEDPage page) {
    page_ = page;
    lastPageChange_ = std::chrono::steady_clock::now();
}
void OLEDDisplay::setAutoRotate(bool enabled, int intervalSec) {
    autoRotate_ = enabled;
    autoIntervalSec_ = intervalSec;
}

// ─── Render principal ────────────────────────────────────────────────────────

void OLEDDisplay::render()
{
    oled_.clear();

    if (!data_.connected) {
        renderSplash();
        oled_.display();
        return;
    }

    switch (page_) {
        case OLEDPage::SPLASH:  renderSplash();  break;
        case OLEDPage::MAIN:    renderMain();    break;
        case OLEDPage::ENGINE:  renderEngine();  break;
        case OLEDPage::FUEL:    renderFuel();    break;
        case OLEDPage::O2:      renderO2();      break;
        case OLEDPage::GM:      renderGM();      break;
        case OLEDPage::DTC:     renderDTC();     break;
        case OLEDPage::DEBUG:   renderDebug();   break;
        default:                renderMain();    break;
    }

    oled_.display();
}

// ─── Helpers ────────────────────────────────────────────────────────────────

std::string OLEDDisplay::fmtDouble(double v, int decimals)
{
    if (v <= -999.0 || v < -90.0) return "--";
    std::stringstream ss;
    ss << std::fixed << std::setprecision(decimals) << v;
    return ss.str();
}

std::string OLEDDisplay::fmtInt(int v)
{
    if (v < -90) return "--";
    return std::to_string(v);
}

bool OLEDDisplay::dataFresh() const {
    auto age = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - data_.lastUpdate).count();
    return age < 5;
}

// Cabecera: título en línea 0 + indicador de página
void OLEDDisplay::drawHeader(const std::string& title, int pageNum)
{
    // Barra superior sólida
    oled_.fillRect(0, 0, OLED_WIDTH, 9, true);
    // Título en negro (invertido)
    int tx = (OLED_WIDTH - (int)title.length()*6) / 2;
    if (tx < 1) tx = 1;
    oled_.drawString(tx, 1, title, 1, false);
    // Indicador de página
    std::string pg = std::to_string(pageNum)+"/7";
    oled_.drawString(OLED_WIDTH - (int)pg.length()*6 - 1, 1, pg, 1, false);
}

// Footer: línea separadora + número de página como puntos
void OLEDDisplay::drawFooter()
{
    oled_.drawLine(0, 62, OLED_WIDTH-1, 62, true);
    // Puntos indicadores de página
    int total = static_cast<int>(OLEDPage::COUNT) - 1; // sin SPLASH
    int cur   = static_cast<int>(page_) - 1;
    int startX = (OLED_WIDTH - total*5) / 2;
    for (int i=0; i<total; i++) {
        int px = startX + i*5;
        if (i == cur) oled_.fillRect(px, 63, 3, 1, true);
        else          oled_.drawPixel(px+1, 63, true);
    }
}

// ─── PÁGINA SPLASH ───────────────────────────────────────────────────────────
// Sin conexión: logo ASCII + estado BT
void OLEDDisplay::renderSplash()
{
    // Marco exterior
    oled_.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, true);

    // Título grande
    oled_.drawStringCentered(4,  "OBD2", 2);
    oled_.drawStringCentered(22, "DIAGNOSTIC", 1);

    // Línea separadora
    oled_.drawLine(10, 32, 118, 32, true);

    if (data_.connected) {
        oled_.drawStringCentered(36, "CONECTADO", 1);
        // Protocolo (truncar a 20 chars)
        std::string proto = data_.protocol;
        if (proto.length() > 20) proto = proto.substr(0,20);
        oled_.drawStringCentered(48, proto, 1);
    } else {
        oled_.drawStringCentered(36, "Buscando BT...", 1);
        // Animación: puntos parpadeantes
        static int dots = 0;
        std::string anim(dots % 4, '.');
        oled_.drawStringCentered(50, anim, 1);
        dots++;
    }
}

// ─── PÁGINA MAIN ─────────────────────────────────────────────────────────────
// RPM grande (escala, barra) + velocidad + temperatura
void OLEDDisplay::renderMain()
{
    std::lock_guard<std::mutex> lock(data_.mtx);

    drawHeader("MOTOR", 1);

    // RPM — número grande
    std::string rpmStr = std::to_string(data_.rpm);
    int rpmX = 2;
    oled_.drawString(rpmX, 11, rpmStr, 2);
    oled_.drawString(rpmX + (int)rpmStr.length()*12 + 2, 17, "rpm", 1);

    // Barra de RPM (0-6000)
    float rpmPct = std::min(100.0f, data_.rpm / 60.0f);
    oled_.drawProgressBar(2, 27, 80, 6, rpmPct);

    // Zona roja (>80%)
    if (rpmPct > 80.0f) {
        // Parpadeo
        static bool blink = false; blink = !blink;
        if (blink) oled_.drawRect(2, 27, 80, 6, true);
    }

    // Velocidad
    oled_.drawString(85, 11, "VEL", 1);
    oled_.drawString(84, 20, std::to_string(data_.speed), 2);
    oled_.drawString(84, 34, "km/h", 1);

    // Línea vertical separadora
    oled_.drawLine(80, 10, 80, 41, true);

    // Temperatura y carga en fila inferior
    oled_.drawLine(0, 43, OLED_WIDTH-1, 43, true);

    // Temperatura
    std::string tStr = fmtInt(data_.coolantTemp) + "C";
    oled_.drawString(2, 46, "TEMP:", 1);
    oled_.drawString(34, 46, tStr, 1);
    // Alerta temperatura alta
    if (data_.coolantTemp > 100) oled_.drawString(75, 46, "!HOT!", 1);

    // Carga
    oled_.drawString(2, 55, "CARGA:", 1);
    oled_.drawString(40, 55, std::to_string(data_.engineLoad)+"%", 1);
    // Barra carga
    oled_.drawProgressBar(85, 47, 40, 5, (float)data_.engineLoad);
    oled_.drawString(85, 55, "MAF:", 1);
    oled_.drawString(108, 55, fmtDouble(data_.maf,1), 1);
}

// ─── PÁGINA ENGINE ───────────────────────────────────────────────────────────
// Admisión: MAP, Acelerador, Timing, Temp admisión
void OLEDDisplay::renderEngine()
{
    std::lock_guard<std::mutex> lock(data_.mtx);
    drawHeader("ADMISION", 2);

    // 4 campos en 2 columnas
    struct Field { const char* label; std::string value; const char* unit; };
    std::vector<Field> fields = {
        {"MAP",  fmtDouble(data_.intakePressure,0), "kPa"},
        {"THRT", fmtDouble(data_.throttle,1),       "%"  },
        {"TIMG", fmtDouble(data_.timingAdvance,1),  "deg"},
        {"ITAK", fmtInt(data_.intakeTemp),          "C"  },
        {"MAF",  fmtDouble(data_.maf,2),            "g/s"},
        {"BARO", fmtDouble(data_.baroPressure,0),   "kPa"},
    };

    int row=0, col=0;
    for (auto& f : fields) {
        int x = col==0 ? 2 : 66;
        int y = 11 + row*17;
        // Label
        oled_.drawString(x, y,   f.label, 1);
        // Valor + unidad
        std::string val = f.value + f.unit;
        oled_.drawString(x, y+8, val, 1);
        // Separador vertical
        if (col==0) oled_.drawLine(63, 10, 63, 62, true);
        col = 1-col;
        if (col==0) row++;
    }
    drawFooter();
}

// ─── PÁGINA FUEL TRIM ────────────────────────────────────────────────────────
// STFT B1/B2, LTFT B1/B2, Nivel combustible
void OLEDDisplay::renderFuel()
{
    std::lock_guard<std::mutex> lock(data_.mtx);
    drawHeader("FUEL TRIM", 3);

    // STFT
    oled_.drawString(2, 11, "STFT B1:", 1);
    std::string s1 = fmtDouble(data_.stftBank1,2)+"%";
    oled_.drawStringRight(11, s1);
    // Color de advertencia (valor > 10%)
    if (std::abs(data_.stftBank1) > 10.0)
        oled_.drawRect(OLED_WIDTH-(int)s1.length()*6-3, 10, (int)s1.length()*6+2, 9, true);

    oled_.drawString(2, 21, "STFT B2:", 1);
    std::string s2 = fmtDouble(data_.stftBank2,2)+"%";
    oled_.drawStringRight(21, s2);

    oled_.drawLine(0, 30, OLED_WIDTH-1, 30, true);

    // LTFT
    oled_.drawString(2, 32, "LTFT B1:", 1);
    oled_.drawStringRight(32, fmtDouble(data_.ltftBank1,2)+"%");

    oled_.drawString(2, 42, "LTFT B2:", 1);
    oled_.drawStringRight(42, fmtDouble(data_.ltftBank2,2)+"%");

    oled_.drawLine(0, 51, OLED_WIDTH-1, 51, true);

    // Nivel combustible con barra
    oled_.drawString(2, 53, "COMB:", 1);
    std::string fuelStr = fmtDouble(data_.fuelLevel,1)+"%";
    oled_.drawString(36, 53, fuelStr, 1);
    oled_.drawProgressBar(80, 54, 46, 7, (float)data_.fuelLevel);
    drawFooter();
}

// ─── PÁGINA O2 ───────────────────────────────────────────────────────────────
// Voltaje y trim de sensor O2 B1S1 y B2S1
void OLEDDisplay::renderO2()
{
    std::lock_guard<std::mutex> lock(data_.mtx);
    drawHeader("SENSOR O2", 4);

    // B1S1
    oled_.drawString(2, 11, "BANCO 1  S1", 1);
    oled_.drawString(2, 21, "Volt:", 1);
    oled_.drawString(34, 21, fmtDouble(data_.o2B1S1V,3)+"V", 1);
    // Barra voltaje 0-1.275V
    oled_.drawProgressBar(2, 30, 124, 5, (float)(data_.o2B1S1V/1.275*100.0));
    // Estado mezcla
    const char* mix1 = data_.o2B1S1V < 0.1 ? "POBRE" :
                       data_.o2B1S1V > 0.9 ? "RICO"  : "OK";
    oled_.drawString(80, 21, mix1, 1);

    oled_.drawLine(0, 37, OLED_WIDTH-1, 37, true);

    // B2S1
    oled_.drawString(2, 39, "BANCO 2  S1", 1);
    oled_.drawString(2, 49, "Volt:", 1);
    oled_.drawString(34, 49, fmtDouble(data_.o2B2S1V,3)+"V", 1);
    oled_.drawProgressBar(2, 58, 124, 5, (float)(data_.o2B2S1V/1.275*100.0));
    const char* mix2 = data_.o2B2S1V < 0.1 ? "POBRE" :
                       data_.o2B2S1V > 0.9 ? "RICO"  : "OK";
    oled_.drawString(80, 49, mix2, 1);
}

// ─── PÁGINA GM ───────────────────────────────────────────────────────────────
// Odómetro, Voltaje batería, Torque, Presión combustible
void OLEDDisplay::renderGM()
{
    std::lock_guard<std::mutex> lock(data_.mtx);
    drawHeader("DATOS GM", 5);

    // Odómetro — más prominente
    oled_.drawString(2, 11, "ODO:", 1);
    // Truncar a 15 chars
    std::string km = data_.gmKm;
    if (km.length() > 14) km = km.substr(0,14);
    oled_.drawString(28, 10, km, 1);

    oled_.drawLine(0, 21, OLED_WIDTH-1, 21, true);

    // Voltaje batería
    oled_.drawString(2, 23, "BATT:", 1);
    std::string volt = data_.gmVoltage;
    if (volt.length() > 10) volt = volt.substr(0,10);
    oled_.drawString(34, 23, volt, 1);

    // Torque
    oled_.drawString(2, 33, "TORQ:", 1);
    std::string torq = data_.gmTorque;
    if (torq.length() > 10) torq = torq.substr(0,10);
    oled_.drawString(34, 33, torq, 1);

    // Presión combustible
    oled_.drawString(2, 43, "FUEL:", 1);
    std::string fp = data_.gmFuelPress;
    if (fp.length() > 13) fp = fp.substr(0,13);
    oled_.drawString(34, 43, fp, 1);

    // Temperatura catalizador
    oled_.drawString(2, 53, "CAT:", 1);
    std::string cat = data_.gmCatTemp;
    if (cat.length() > 12) cat = cat.substr(0,12);
    oled_.drawString(28, 53, cat, 1);

    drawFooter();
}

// ─── PÁGINA DTC ──────────────────────────────────────────────────────────────
// Lista de códigos de error
void OLEDDisplay::renderDTC()
{
    std::lock_guard<std::mutex> lock(data_.mtx);
    drawHeader("CODIGOS DTC", 6);

    if (data_.dtcCodes.empty() ||
        (data_.dtcCodes.size()==1 && data_.dtcCodes[0]=="NONE")) {
        oled_.drawStringCentered(25, "Sin codigos", 1);
        oled_.drawStringCentered(38, "activos", 1);
        // Checkmark
        oled_.drawString(58, 50, "OK", 2);
        return;
    }

    // MIL activa
    if (data_.milActive) {
        oled_.drawString(2, 11, "MIL: ACTIVA", 1);
        oled_.drawRect(0, 10, 80, 10, true);
    }

    int y = data_.milActive ? 23 : 12;
    int shown = 0;
    for (auto& code : data_.dtcCodes) {
        if (y + 9 > 62) {
            oled_.drawString(2, y, "...", 1);
            break;
        }
        oled_.drawString(2, y, code, 1);
        y += 10;
        shown++;
    }
    drawFooter();
}

// ─── PÁGINA DEBUG ────────────────────────────────────────────────────────────
// Última línea TX y RX en tiempo real
void OLEDDisplay::renderDebug()
{
    std::lock_guard<std::mutex> lock(data_.mtx);
    drawHeader("DEBUG BT", 7);

    // TX — azul en terminal, aquí solo texto
    oled_.drawString(2, 11, "TX:", 1);
    std::string tx = data_.lastTX;
    if (tx.length() > 18) tx = tx.substr(tx.length()-18); // últimos 18 chars
    oled_.drawString(22, 11, tx, 1);

    oled_.drawLine(0, 21, OLED_WIDTH-1, 21, true);

    // RX
    oled_.drawString(2, 23, "RX:", 1);
    std::string rx = data_.lastRX;
    if (rx.length() > 18) rx = rx.substr(rx.length()-18);
    oled_.drawString(22, 23, rx, 1);

    oled_.drawLine(0, 33, OLED_WIDTH-1, 33, true);

    // Protocolo
    oled_.drawString(2, 35, "PROTO:", 1);
    std::string proto = data_.protocol;
    if (proto.length() > 14) proto = proto.substr(0,14);
    oled_.drawString(40, 35, proto, 1);

    // Estado y timestamp
    oled_.drawString(2, 45, data_.connected ? "ONLINE" : "OFFLINE", 1);

    // Barra de actividad
    static int actBar = 0;
    actBar = (actBar + 1) % OLED_WIDTH;
    oled_.drawPixel(actBar, 55, true);
    oled_.drawPixel((actBar+1)%OLED_WIDTH, 55, false);
    oled_.drawPixel((actBar+2)%OLED_WIDTH, 55, false);
}
CPP

# ─────────────────────────────────────────────────────────────────────────────
# SRC: main.cpp — Aplicación principal
# ─────────────────────────────────────────────────────────────────────────────
cat > src/main.cpp << 'CPP'
// main.cpp — OBD2 Raspberry Pi 2W + SSD1306 SPI OLED
//
// Uso:
//   ./obd2_rpi [MAC_BT] [SPI_DEVICE] [PIN_DC] [PIN_RST]
//
// Ejemplo:
//   ./obd2_rpi 00:1D:A5:07:23:6E /dev/spidev0.0 25 17
//
// Controles (stdin, sin bloquear):
//   n → siguiente página
//   p → página anterior
//   a → toggle auto-rotación
//   q → salir
//   l → log CSV
//   g → actualizar datos GM
//   d → leer DTCs
//   c → borrar DTCs

#include "elm327.hpp"
#include "gm_commands.hpp"
#include "logger.hpp"
#include "ssd1306.hpp"
#include "oled_display.hpp"

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <memory>
#include <sstream>
#include <iomanip>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// ─── Señal de parada ─────────────────────────────────────────────────────────
static std::atomic<bool> g_running(true);
static void sigHandler(int) { g_running = false; }

// ─── Terminal non-blocking ───────────────────────────────────────────────────
struct TermRaw {
    struct termios old_tio;
    TermRaw() {
        tcgetattr(STDIN_FILENO, &old_tio);
        struct termios tio = old_tio;
        tio.c_lflag &= ~(ICANON | ECHO);
        tio.c_cc[VMIN]  = 0;
        tio.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &tio);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    }
    ~TermRaw() {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
        fcntl(STDIN_FILENO, F_SETFL, 0);
    }
};

// ─── Hilo de polling OBD2 ────────────────────────────────────────────────────
void obdPollThread(ELM327* elm, GMCommands* gm, VehicleData* vd, std::atomic<bool>& running)
{
    int cycle = 0;
    while (running) {
        auto t0 = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(vd->mtx);
            vd->connected = elm->isConnected();
        }
        if (!elm->isConnected()) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        // ── Lectura rápida (cada ciclo ~800ms) ──
        int rpm    = elm->getRPM();
        int speed  = elm->getSpeed();
        int temp   = elm->getCoolantTemp();
        int load   = elm->getEngineLoad();
        double thr = elm->getThrottlePosition();
        double map = elm->getIntakePressure();

        // ── Lectura media (cada 3 ciclos ~2.4s) ──
        double maf=0, fuel=0, baro=0, timing=0;
        double stft1=0, stft2=0, ltft1=0, ltft2=0;
        double o2v1=0, o2v2=0;
        int intakeT=0;
        if (cycle % 3 == 0) {
            maf    = elm->getMAF();
            fuel   = elm->getFuelLevel();
            baro   = elm->getBarometricPressure();
            timing = elm->getTimingAdvance();
            intakeT= elm->getIntakeTemp();
            stft1  = elm->getShortTermTrimBank1();
            stft2  = elm->getShortTermTrimBank2();
            ltft1  = elm->getLongTermTrimBank1();
            ltft2  = elm->getLongTermTrimBank2();
            auto o2b1 = elm->getO2Sensor(1,1);
            auto o2b2 = elm->getO2Sensor(2,1);
            o2v1 = o2b1.voltage;
            o2v2 = o2b2.voltage;
        }

        // ── Lectura lenta GM (cada 10 ciclos ~8s) ──
        std::string gmKm="--", gmV="--", gmTq="--", gmCat="--", gmFP="--";
        if (cycle % 10 == 0) {
            gmKm  = gm->getKilometers();
            gmV   = gm->getECUVoltage();
            gmTq  = gm->getEngineTorque();
        }
        if (cycle % 30 == 0) {
            gmCat = gm->getCatalystTemp();
            gmFP  = gm->getFuelPressure();
        }

        // DTCs (cada 60 ciclos ~48s)
        std::vector<std::string> dtcList;
        bool milActive = false;
        if (cycle % 60 == 0) {
            milActive = elm->checkMIL();
            auto dtcs = elm->getDTCs();
            for (auto& d : dtcs) {
                if (d.code != "NONE") dtcList.push_back(d.code);
            }
        }

        // ── Actualizar VehicleData ──
        {
            std::lock_guard<std::mutex> lock(vd->mtx);
            if (rpm > 0)    vd->rpm = rpm;
            if (speed >= 0) vd->speed = speed;
            if (temp > -90) vd->coolantTemp = temp;
            if (load >= 0)  vd->engineLoad = load;
            if (thr >= 0)   vd->throttle = thr;
            if (map >= 0)   vd->intakePressure = map;

            if (cycle % 3 == 0) {
                if (maf  >= 0)   vd->maf = maf;
                if (fuel >= 0)   vd->fuelLevel = fuel;
                if (baro >= 0)   vd->baroPressure = baro;
                if (timing > -90) vd->timingAdvance = timing;
                if (intakeT > -90) vd->intakeTemp = intakeT;
                if (stft1 > -999) vd->stftBank1 = stft1;
                if (stft2 > -999) vd->stftBank2 = stft2;
                if (ltft1 > -999) vd->ltftBank1 = ltft1;
                if (ltft2 > -999) vd->ltftBank2 = ltft2;
                if (o2v1 >= 0)   vd->o2B1S1V = o2v1;
                if (o2v2 >= 0)   vd->o2B2S1V = o2v2;
            }
            if (cycle % 10 == 0) {
                if (gmKm != "--") vd->gmKm = gmKm;
                if (gmV  != "--") vd->gmVoltage = gmV;
                if (gmTq != "--") vd->gmTorque = gmTq;
            }
            if (cycle % 30 == 0) {
                if (gmCat != "--") vd->gmCatTemp = gmCat;
                if (gmFP  != "--") vd->gmFuelPress = gmFP;
            }
            if (cycle % 60 == 0) {
                vd->milActive = milActive;
                if (!dtcList.empty()) vd->dtcCodes = dtcList;
            }
            vd->lastUpdate = std::chrono::steady_clock::now();
        }

        cycle++;
        // Calcular tiempo de espera para mantener ~800ms por ciclo
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
        int wait = 800 - (int)dt;
        if (wait > 0) std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    }
}

// ─── MAIN ────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    std::signal(SIGINT,  sigHandler);
    std::signal(SIGTERM, sigHandler);

    // Parámetros
    std::string mac        = (argc>1) ? argv[1] : "00:1D:A5:07:23:6E";
    std::string spiDevice  = (argc>2) ? argv[2] : "/dev/spidev0.0";
    int pinDC              = (argc>3) ? std::stoi(argv[3]) : 25;
    int pinRst             = (argc>4) ? std::stoi(argv[4]) : 17;

    std::cout << "╔══════════════════════════════════╗\n";
    std::cout << "║   OBD2 RPi 2W + SSD1306 SPI      ║\n";
    std::cout << "╚══════════════════════════════════╝\n\n";
    std::cout << "MAC BT  : " << mac << "\n";
    std::cout << "SPI dev : " << spiDevice << "\n";
    std::cout << "GPIO DC : " << pinDC << "  RST: " << pinRst << "\n\n";

    // ── Inicializar OLED ──────────────────────────────────────────────────
    SSD1306::Config oledCfg;
    oledCfg.spiDevice   = spiDevice;
    oledCfg.pinDC       = pinDC;
    oledCfg.pinReset    = pinRst;
    oledCfg.spiSpeedHz  = 8000000;

    SSD1306 oled(oledCfg);
    if (!oled.begin()) {
        std::cerr << "[ERROR] No se pudo inicializar la pantalla OLED.\n";
        std::cerr << "Verificar:\n";
        std::cerr << "  sudo raspi-config → Interface Options → SPI → Enable\n";
        std::cerr << "  Conexiones físicas: DIN→GPIO10, CLK→GPIO11, CS→GPIO8\n";
        std::cerr << "  DC→GPIO"<<pinDC<<"  RES→GPIO"<<pinRst<<"\n";
        return 1;
    }

    // ── Datos compartidos ──────────────────────────────────────────────────
    VehicleData vehicleData;

    // ── Display ──────────────────────────────────────────────────────────
    OLEDDisplay display(oled, vehicleData);
    display.setAutoRotate(true, 6);
    display.setPage(OLEDPage::SPLASH);
    display.start(400); // refresh cada 400ms

    // ── Conexión BT ───────────────────────────────────────────────────────
    auto elm = std::make_unique<ELM327>(mac);
    auto gm  = std::make_unique<GMCommands>(elm.get());

    std::cout << "[INFO] Conectando al ELM327 (" << mac << ")...\n";
    if (!elm->connectBT()) {
        std::cerr << "[ERROR] No se pudo conectar vía Bluetooth.\n";
        std::cerr << "Verificar: bluetoothctl → pair " << mac << "\n";
        std::cerr << "Esperando 30s y reintentando...\n";
        for (int i=0; i<30 && g_running; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (!elm->connectBT()) {
            display.stop();
            return 1;
        }
    }

    std::cout << "[OK] Conectado al ELM327\n";
    std::cout << "Protocolo: " << elm->getProtocol() << "\n\n";

    {
        std::lock_guard<std::mutex> lock(vehicleData.mtx);
        vehicleData.connected = true;
        vehicleData.protocol  = elm->getProtocol();
    }
    display.setPage(OLEDPage::MAIN);

    // ── Logger CSV ────────────────────────────────────────────────────────
    Logger logger;

    // ── Hilo de polling OBD2 ─────────────────────────────────────────────
    std::thread pollThread(obdPollThread, elm.get(), gm.get(),
                           &vehicleData, std::ref(g_running));

    // ── Terminal raw para entrada de teclado ──────────────────────────────
    TermRaw termRaw;

    std::cout << "Controles:\n";
    std::cout << "  n/p  = siguiente/anterior página OLED\n";
    std::cout << "  a    = toggle auto-rotación\n";
    std::cout << "  l    = iniciar/detener log CSV\n";
    std::cout << "  g    = forzar lectura GM ahora\n";
    std::cout << "  d    = leer DTCs\n";
    std::cout << "  c    = borrar DTCs (con confirmación)\n";
    std::cout << "  q    = salir\n\n";

    bool logOpen = false;
    static bool autoRot = true;

    // ── Loop principal ────────────────────────────────────────────────────
    while (g_running) {
        // Leer tecla sin bloquear
        char key = 0;
        if (read(STDIN_FILENO, &key, 1) == 1) {
            switch (key) {
                case 'n': case 'N':
                    display.nextPage();
                    std::cout << "\r[OLED] Página: " << static_cast<int>(display.currentPage()) << "    \n";
                    break;
                case 'p': case 'P':
                    display.prevPage();
                    break;
                case 'a': case 'A':
                    autoRot = !autoRot;
                    display.setAutoRotate(autoRot, 6);
                    std::cout << "\r[OLED] Auto-rotación: " << (autoRot?"ON":"OFF") << "    \n";
                    break;
                case 'l': case 'L':
                    if (!logOpen) {
                        if (logger.open()) {
                            logOpen = true;
                            std::cout << "\r[LOG] Iniciado\n";
                        }
                    } else {
                        logger.close();
                        logOpen = false;
                        std::cout << "\r[LOG] Detenido\n";
                    }
                    break;
                case 'g': case 'G': {
                    std::cout << "\r[GM] Leyendo datos GM...\n";
                    std::string km = gm->getKilometers();
                    std::string v  = gm->getECUVoltage();
                    std::string tq = gm->getEngineTorque();
                    std::string ct = gm->getCatalystTemp();
                    std::string fp = gm->getFuelPressure();
                    std::cout << "  Km: " << km << "\n";
                    std::cout << "  V:  " << v  << "\n";
                    std::cout << "  Tq: " << tq << "\n";
                    std::cout << "  Cat:" << ct << "\n";
                    std::cout << "  FP: " << fp << "\n";
                    std::lock_guard<std::mutex> lock(vehicleData.mtx);
                    vehicleData.gmKm        = km;
                    vehicleData.gmVoltage   = v;
                    vehicleData.gmTorque    = tq;
                    vehicleData.gmCatTemp   = ct;
                    vehicleData.gmFuelPress = fp;
                    display.setPage(OLEDPage::GM);
                    break;
                }
                case 'd': case 'D': {
                    auto dtcs = elm->getDTCs();
                    std::cout << "\r[DTC] " << dtcs.size() << " códigos:\n";
                    std::lock_guard<std::mutex> lock(vehicleData.mtx);
                    vehicleData.dtcCodes.clear();
                    for (auto& d : dtcs) {
                        std::cout << "  " << d.code << "\n";
                        if (d.code != "NONE") vehicleData.dtcCodes.push_back(d.code);
                    }
                    display.setPage(OLEDPage::DTC);
                    break;
                }
                case 'c': case 'C': {
                    std::cout << "\r[DTC] ¿Borrar DTCs? (s/N): ";
                    char confirm = 'n';
                    read(STDIN_FILENO, &confirm, 1);
                    if (confirm == 's' || confirm == 'S') {
                        bool ok = elm->clearDTCs();
                        std::cout << (ok ? " Borrados OK\n" : " Error\n");
                    }
                    break;
                }
                case 'q': case 'Q':
                    g_running = false;
                    break;
            }
        }

        // Logging CSV si está activo
        if (logOpen) {
            std::lock_guard<std::mutex> lock(vehicleData.mtx);
            logger.log("rpm",       vehicleData.rpm);
            logger.log("speed",     vehicleData.speed);
            logger.log("coolant",   vehicleData.coolantTemp);
            logger.log("load",      vehicleData.engineLoad);
            logger.log("throttle",  vehicleData.throttle);
            logger.log("stft_b1",   vehicleData.stftBank1);
            logger.log("ltft_b1",   vehicleData.ltftBank1);
            logger.log("o2_b1s1",   vehicleData.o2B1S1V);
        }

        // Mostrar estado en consola
        {
            std::lock_guard<std::mutex> lock(vehicleData.mtx);
            std::cout << "\rRPM:" << std::setw(5) << vehicleData.rpm
                      << " VEL:" << std::setw(3) << vehicleData.speed
                      << " TEMP:" << std::setw(3) << vehicleData.coolantTemp
                      << "C CARG:" << std::setw(2) << vehicleData.engineLoad << "%"
                      << " O2:" << std::fixed << std::setprecision(3) << vehicleData.o2B1S1V << "V"
                      << "  " << std::flush;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    // ── Cleanup ───────────────────────────────────────────────────────────
    std::cout << "\n\n[INFO] Deteniendo...\n";
    display.stop();
    if (pollThread.joinable()) pollThread.join();
    if (logOpen) logger.close();
    elm->disconnect();

    // Mensaje final en OLED
    oled.clear();
    oled.drawStringCentered(22, "APAGANDO", 1);
    oled.drawStringCentered(36, "OBD2 RPi", 1);
    oled.display();
    usleep(1500000);
    oled.clear();
    oled.display();

    std::cout << "[OK] Bye!\n";
    return 0;
}
CPP

# ─────────────────────────────────────────────────────────────────────────────
# CMakeLists.txt
# ─────────────────────────────────────────────────────────────────────────────
cat > CMakeLists.txt << 'CMAKE'
cmake_minimum_required(VERSION 3.16)
project(obd2_rpi CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimización para ARMv6 (Pi Zero) / ARMv7 (Pi 2W)
# Detectar automáticamente la arquitectura
execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
if (ARCH MATCHES "aarch64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=armv8-a")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(Threads REQUIRED)

# Buscar libgpiod
find_library(GPIOD_LIB gpiod REQUIRED)
find_path(GPIOD_INCLUDE gpiod.h REQUIRED)

include_directories(include ${GPIOD_INCLUDE})

file(GLOB SOURCES src/*.cpp)

add_executable(obd2_rpi ${SOURCES})

target_link_libraries(obd2_rpi
    bluetooth
    Threads::Threads
    ${GPIOD_LIB}
)

message(STATUS "Arquitectura : ${ARCH}")
message(STATUS "Binario en   : ${CMAKE_SOURCE_DIR}/bin/obd2_rpi")
CMAKE

# ─────────────────────────────────────────────────────────────────────────────
# Generar script de instalación de dependencias
# ─────────────────────────────────────────────────────────────────────────────
cat > install_deps.sh << 'SHELL'
#!/bin/bash
# install_deps.sh — Instalar dependencias en Raspberry Pi 2W (Raspberry Pi OS)
echo "[INFO] Actualizando sistema..."
sudo apt update

echo "[INFO] Instalando dependencias..."
sudo apt install -y \
    build-essential \
    cmake \
    git \
    libbluetooth-dev \
    libgpiod-dev \
    libgpiod2 \
    gpiod \
    bluez \
    bluetooth

echo "[INFO] Habilitando SPI..."
# Habilitar SPI en /boot/config.txt si no está habilitado
if ! grep -q "^dtparam=spi=on" /boot/config.txt; then
    echo "dtparam=spi=on" | sudo tee -a /boot/config.txt
    echo "[OK] SPI habilitado — se requiere reiniciar"
fi
# O usar raspi-config:
# sudo raspi-config nonint do_spi 0

echo "[INFO] Configurando Bluetooth..."
sudo systemctl enable bluetooth
sudo systemctl start bluetooth

echo ""
echo "╔══════════════════════════════════════════════╗"
echo "║  Para emparejar el ELM327:                   ║"
echo "║    bluetoothctl                              ║"
echo "║    power on                                  ║"
echo "║    scan on                                   ║"
echo "║    pair XX:XX:XX:XX:XX:XX                   ║"
echo "║    trust XX:XX:XX:XX:XX:XX                  ║"
echo "║    quit                                      ║"
echo "╚══════════════════════════════════════════════╝"
echo ""
echo "[IMPORTANTE] Si se habilitó SPI, reiniciar: sudo reboot"
SHELL
chmod +x install_deps.sh

# ─────────────────────────────────────────────────────────────────────────────
# Script de inicio automático (systemd service)
# ─────────────────────────────────────────────────────────────────────────────
cat > obd2_rpi.service << 'SERVICE'
[Unit]
Description=OBD2 Raspberry Pi 2W con OLED SSD1306
After=network.target bluetooth.target
Wants=bluetooth.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/obd2_rpi
ExecStart=/home/pi/obd2_rpi/bin/obd2_rpi 00:1D:A5:07:23:6E /dev/spidev0.0 25 17
Restart=on-failure
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
SERVICE

# Script para instalar el servicio systemd
cat > install_service.sh << 'SHELL'
#!/bin/bash
# Ajustar la MAC del ELM327 antes de instalar
SERVICE_FILE="obd2_rpi.service"
echo "MAC actual del ELM327 en el servicio:"
grep ExecStart $SERVICE_FILE
read -p "Ingrese la MAC del ELM327 (Enter para conservar actual): " mac
if [ -n "$mac" ]; then
    sed -i "s/[0-9A-Fa-f:]\{17\}/$mac/g" $SERVICE_FILE
    echo "MAC actualizada a: $mac"
fi
sudo cp $SERVICE_FILE /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable obd2_rpi
sudo systemctl start obd2_rpi
echo "[OK] Servicio instalado y arrancado"
echo "     sudo systemctl status obd2_rpi"
echo "     sudo journalctl -u obd2_rpi -f"
SHELL
chmod +x install_service.sh

# ─────────────────────────────────────────────────────────────────────────────
# README
# ─────────────────────────────────────────────────────────────────────────────
cat > README.md << 'MD'
# OBD2 Raspberry Pi 2W + SSD1306 SPI

## Conexión OLED SSD1306 SPI → Raspberry Pi 2W

```
SSD1306  │  RPi Pin  │  GPIO BCM  │  Función
─────────┼───────────┼────────────┼──────────
VCC      │  Pin 1    │  3.3V      │  Alimentación
GND      │  Pin 6    │  GND       │  Tierra
DIN      │  Pin 19   │  GPIO10    │  SPI0 MOSI
CLK      │  Pin 23   │  GPIO11    │  SPI0 SCLK
CS       │  Pin 24   │  GPIO8     │  SPI0 CE0
DC       │  Pin 22   │  GPIO25    │  Data/Command
RES      │  Pin 11   │  GPIO17    │  Reset
```

## Compilación

```bash
# 1. Instalar dependencias (solo primera vez)
./install_deps.sh

# 2. Compilar
mkdir -p build && cd build
cmake .. && make -j$(nproc)
cd ..

# 3. Ejecutar
./bin/obd2_rpi [MAC_ELM327] [SPI_DEV] [PIN_DC] [PIN_RST]
# Ejemplo:
./bin/obd2_rpi 00:1D:A5:07:23:6E /dev/spidev0.0 25 17
```

## Páginas OLED (rotación automática cada 6 segundos)

| Pág | Contenido |
|-----|-----------|
| 1 MOTOR | RPM grande con barra, velocidad, temperatura, carga |
| 2 ADMISIÓN | MAP, acelerador, timing, MAF, temp admisión, baro |
| 3 FUEL TRIM | STFT B1/B2, LTFT B1/B2, nivel combustible |
| 4 SENSOR O2 | Voltaje O2 B1S1/B2S1, barra visual, estado mezcla |
| 5 DATOS GM | Odómetro, batería, torque, presión combustible |
| 6 CÓDIGOS DTC | Lista de DTCs activos, estado MIL |
| 7 DEBUG BT | Última línea TX/RX Bluetooth en tiempo real |

## Controles por teclado (stdin)

| Tecla | Acción |
|-------|--------|
| n/p | Siguiente/anterior página OLED |
| a | Toggle auto-rotación |
| l | Iniciar/detener log CSV |
| g | Forzar lectura de datos GM ahora |
| d | Leer DTCs |
| c | Borrar DTCs |
| q | Salir |

## Inicio automático con systemd

```bash
./install_service.sh
```

## PIDs y fórmulas utilizadas

### OBD-II Estándar (Modo 01)
| Sensor | PID | Fórmula |
|--------|-----|---------|
| RPM | 010C | (A×256+B)/4 |
| Velocidad | 010D | A km/h |
| Temp. refrigerante | 0105 | A−40 °C |
| Carga motor | 0104 | A×100/255 % |
| STFT B1 | 0106 | ((A−128)×100)/128 % |
| STFT B2 | 0108 | ((A−128)×100)/128 % |
| LTFT B1 | 0107 | ((A−128)×100)/128 % |
| LTFT B2 | 0109 | ((A−128)×100)/128 % |
| O2 B1S1 Voltaje | 0114 | A×0.005 V |

### GM Modo 22 (UDS ISO 14229)
| Sensor | PID | Fórmula |
|--------|-----|---------|
| Odómetro | B100 | raw_32bit/10 km |
| Temp. catalizador | 01B4 | (raw_16bit×0.1)−40 °C |
| Presión combustible | 1180 | raw_16bit×4 kPa |
| Torque motor | 01A9 | (raw_16bit×0.5)−848 Nm |
| Voltaje ECU | 01A1 | raw_16bit×0.001 V |
MD

# ─────────────────────────────────────────────────────────────────────────────
# Compilar el proyecto
# ─────────────────────────────────────────────────────────────────────────────
echo ""
info "Proyecto generado. Verificando si se puede compilar en este sistema..."

mkdir -p bin

# Detectar si estamos en Raspberry Pi o en un sistema de desarrollo
if command -v cmake >/dev/null 2>&1 && \
   pkg-config --exists gpiod 2>/dev/null && \
   pkg-config --exists bluez 2>/dev/null; then

    info "Dependencias detectadas. Compilando..."
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    cd ..

    if [ -f "bin/obd2_rpi" ]; then
        log "✅ Compilación exitosa: bin/obd2_rpi"
    fi
else
    warn "No se compiló en este sistema (requiere RPi con libbluetooth-dev + libgpiod-dev)"
    warn "Copiar la carpeta '$PROJECT' a la Raspberry Pi y ejecutar:"
    echo ""
    echo -e "  ${BOLD}cd $PROJECT${NC}"
    echo -e "  ${BOLD}./install_deps.sh${NC}"
    echo -e "  ${BOLD}mkdir -p build && cd build && cmake .. && make -j4${NC}"
fi

cd ..

# ─────────────────────────────────────────────────────────────────────────────
# Resumen final
# ─────────────────────────────────────────────────────────────────────────────
echo ""
echo -e "${GREEN}╔══════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║   ✅  PROYECTO GENERADO: $PROJECT               ║${NC}"
echo -e "${GREEN}╠══════════════════════════════════════════════════════╣${NC}"
echo -e "${GREEN}║  Archivos:                                           ║${NC}"
echo -e "${GREEN}║    include/elm327.hpp        ← ELM327 interface      ║${NC}"
echo -e "${GREEN}║    include/gm_commands.hpp   ← Comandos GM           ║${NC}"
echo -e "${GREEN}║    include/ssd1306.hpp       ← Driver OLED SPI       ║${NC}"
echo -e "${GREEN}║    include/oled_display.hpp  ← Páginas OBD2          ║${NC}"
echo -e "${GREEN}║    include/logger.hpp        ← Logger CSV            ║${NC}"
echo -e "${GREEN}║    src/elm327.cpp            ← BT RFCOMM corregido   ║${NC}"
echo -e "${GREEN}║    src/gm_commands.cpp       ← PIDs GM corregidos    ║${NC}"
echo -e "${GREEN}║    src/ssd1306.cpp           ← Driver SPI + fuente   ║${NC}"
echo -e "${GREEN}║    src/oled_display.cpp      ← 7 páginas de display  ║${NC}"
echo -e "${GREEN}║    src/main.cpp              ← App principal         ║${NC}"
echo -e "${GREEN}║    CMakeLists.txt                                    ║${NC}"
echo -e "${GREEN}║    install_deps.sh           ← Instala en RPi        ║${NC}"
echo -e "${GREEN}║    install_service.sh        ← Servicio systemd      ║${NC}"
echo -e "${GREEN}║    README.md                                         ║${NC}"
echo -e "${GREEN}╠══════════════════════════════════════════════════════╣${NC}"
echo -e "${GREEN}║  En la Raspberry Pi:                                 ║${NC}"
echo -e "${GREEN}║    ./install_deps.sh                                 ║${NC}"
echo -e "${GREEN}║    mkdir build && cd build && cmake .. && make -j4   ║${NC}"
echo -e "${GREEN}║    ./bin/obd2_rpi 00:1D:A5:07:23:6E                 ║${NC}"
echo -e "${GREEN}╚══════════════════════════════════════════════════════╝${NC}"
echo ""
find "$PROJECT" -type f | sort | sed "s|^$PROJECT/|  |"
