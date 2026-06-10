#pragma once

#include <bcm2835.h>
#include "SSD1306_OLED.hpp"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

class OLED
{
private:
    SSD1306 display;
    uint8_t buffer[OLED_WIDTH * (OLED_HEIGHT / 8)];

public:
    OLED();

    bool init();
    void clear();

    void showInitScreen();
    void showDashboard(int rpm, float maf, float map);
};

/*
#pragma once

#include <bcm2835.h>
#include "SSD1306_OLED.hpp"


#define OLED_WIDTH 128
#define OLED_HEIGHT 64

SSD1306 myOLED(OLED_WIDTH, OLED_HEIGHT);


class OLED
{
private:
    SSD1306 display;

public:
    OLED();

    bool init();
    void clear();

    void printRPM(int rpm);
    void printMAF(float maf);
    void printMAP(float map);
    bool SetupOLED();
    void DisplayDashboard(int rpm, float maf, float map);

void OLEDclearBuffer();    
void setTextSize(int);
void setTextColor(int);
void setCursor(int,int);
void print(std::string);
void OLEDupdate();
};
*/