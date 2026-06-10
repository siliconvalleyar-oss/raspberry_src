#include <iostream>
#include <bcm2835.h>

extern "C"{
	#include <time.h>
	#include <stdio.h>	
}
#include <memory>
#include <SSD1306_OLED.hpp>
#include <Bitmap_test_data.hpp>
#include "btc.hpp"
#include "ip_global.hpp"

#define myOLEDwidth  128
#define myOLEDheight 64
uint8_t fullscreenBuffer[1024];
const uint16_t I2C_Speed = 626;
const uint8_t I2C_Address = 0x3C;
SSD1306 myOLED(myOLEDwidth, myOLEDheight);

int8_t setup(void);
void EndTests(void);

int main(int argc, char **argv)
{
	(void)argc; (void)argv;
	if (!setup()) { return -1; }

	auto btc{std::make_unique<BTC::Btc_t>()};
	auto ip{std::make_unique<IP_GLOBAL::IpGlobal_t>()};

	myOLED.OLEDSetBufferPtr(myOLEDwidth, myOLEDheight,
		fullscreenBuffer, sizeof(fullscreenBuffer));

	std::string txt;
	ip->global();
	btc->function(txt);

	myOLED.OLEDclearBuffer();
	myOLED.setTextSize(1);
	myOLED.setFontNum(OLEDFont_Default);
	myOLED.setTextColor(WHITE);
	myOLED.setCursor(12, 32);
	myOLED.print(txt.c_str());
	myOLED.OLEDupdate();

	bcm2835_delay(5000);

	myOLED.OLEDclearBuffer();
	EndTests();
	return 0;
}

void EndTests()
{
	myOLED.OLEDPowerDown();
	bcm2835_close();
	printf("OLED End\r\n");
}

int8_t setup()
{
	if (!bcm2835_init())
	{
		printf("Error 1201 Cannot init bcm2835 library\n");
		return -1;
	}
	bcm2835_delay(50);
	printf("OLED Begin\r\n");
	myOLED.OLEDbegin(I2C_Speed, I2C_Address);
	myOLED.OLEDFillScreen(0x01, 0);
	bcm2835_delay(100);
	return 1;
}
