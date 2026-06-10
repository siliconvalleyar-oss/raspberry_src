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
