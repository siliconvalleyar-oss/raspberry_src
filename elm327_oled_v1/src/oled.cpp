
#include "oled.hpp"
#include <iostream>

OLED::OLED() : display(OLED_WIDTH, OLED_HEIGHT)
{
}

bool OLED::init()
{
    const uint16_t I2C_Speed = BCM2835_I2C_CLOCK_DIVIDER_626;
    const uint8_t I2C_Address = 0x3C;

    if(!bcm2835_init())
    {
        std::cerr << "Error bcm2835 init\n";
        return false;
    }

    if(!display.OLED_I2C_ON())
    {
        std::cerr << "Error I2C OLED\n";
        return false;
    }

    display.OLEDbegin(I2C_Speed, I2C_Address, false);

    if(!display.OLEDSetBufferPtr(OLED_WIDTH, OLED_HEIGHT, buffer, sizeof(buffer)))
    {
        std::cerr << "Error buffer OLED\n";
        return false;
    }

    display.OLEDclearBuffer();
    display.OLEDupdate();

    return true;
}

void OLED::clear()
{
    display.OLEDclearBuffer();
}

void OLED::showInitScreen()
{
    display.OLEDclearBuffer();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0,10);
    display.print("ELM327 INIT");

    display.OLEDupdate();
}

void OLED::showDashboard(int rpm, float maf, float map)
{
    display.OLEDclearBuffer();

    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0,0);
    display.print("RPM:");
    display.print(rpm);

    display.setCursor(0,10);
    display.print("MAF:");
    display.print(maf);

    display.setCursor(0,20);
    display.print("MAP:");
    display.print(map);

    display.OLEDupdate();
}
/*
#include "oled.hpp"
#include <iostream>


#define FULLSCREEN (OLED_WIDTH * (OLED_HEIGHT/8))

static uint8_t screenBuffer[FULLSCREEN];

OLED::OLED() : display(OLED_WIDTH, OLED_HEIGHT)
{
}

bool OLED::init()
{
    const uint16_t I2C_Speed = BCM2835_I2C_CLOCK_DIVIDER_626;
    const uint8_t I2C_Address = 0x3C;

    if(!bcm2835_init())
        return false;

    if(!display.OLED_I2C_ON())
        return false;

    display.OLEDbegin(I2C_Speed, I2C_Address, false);

    if(!display.OLEDSetBufferPtr(OLED_WIDTH, OLED_HEIGHT, screenBuffer, sizeof(screenBuffer)))
        return false;

    display.OLEDclearBuffer();
    display.OLEDupdate();

    return true;
}

void OLED::clear()
{
    display.OLEDclearBuffer();
}

void OLED::printRPM(int rpm)
{
    display.OLEDclearBuffer();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("RPM:");

    display.setCursor(0,16);
    display.print(rpm);

    display.OLEDupdate();
}

void OLED::printMAF(float maf)
{
    display.OLEDclearBuffer();

    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("MAF:");

    display.setCursor(0,12);
    display.print(maf);

    display.OLEDupdate();
}

void OLED::printMAP(float map)
{
    display.OLEDclearBuffer();

    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("MAP:");

    display.setCursor(0,12);
    display.print(map);

    display.OLEDupdate();
}




bool OLED::SetupOLED()
{
    const uint16_t I2C_Speed = BCM2835_I2C_CLOCK_DIVIDER_626;
    const uint8_t I2C_Address = 0x3C;
std::cout << "setup oled" << std::endl;

    if(!bcm2835_init())
    {
        std::cerr << "Error bcm2835 init" << std::endl;
        return false;
    }

    if(!myOLED.OLED_I2C_ON())
    {
        std::cerr << "Error I2C OLED" << std::endl;
        return false;
    }

    myOLED.OLEDbegin(I2C_Speed, I2C_Address, false);

    if(!myOLED.OLEDSetBufferPtr(OLED_WIDTH, OLED_HEIGHT,
                                screenBuffer,
                                sizeof(screenBuffer)))
    {
        std::cerr << "Error buffer OLED" << std::endl;
        return false;
    }

    myOLED.OLEDclearBuffer();
    myOLED.OLEDupdate();

    return true;
}

void OLED::DisplayDashboard(int rpm, float maf, float map)
{
    myOLED.OLEDclearBuffer();

    myOLED.setTextSize(1);
    myOLED.setTextColor(WHITE);

    myOLED.setCursor(0,0);
    myOLED.print("RPM:");
    myOLED.print(rpm);

    myOLED.setCursor(0,10);
    myOLED.print("MAF:");
    myOLED.print(maf);

    myOLED.setCursor(0,20);
    myOLED.print("MAP:");
    myOLED.print(map);

    myOLED.OLEDupdate();

}


    void OLED::OLEDclearBuffer(){
        myOLED.OLEDclearBuffer();
    }

    void OLED::setTextSize(int value){myOLED.setTextSize(value);}
    
    void OLED::setTextColor(int color){myOLED.setTextColor(color);}
    
    void OLED::setCursor(int value_x,int value_y){myOLED.setCursor(value_x,value_y);}
    
    void OLED::print(std::string text){myOLED.print(text);}

    void OLED::OLEDupdate(){myOLED.OLEDupdate();}

    */