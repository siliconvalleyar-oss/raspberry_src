//
// main.cpp
// Demo application – Raspberry Pi 2W / Pi 4
// EPaper 2.66"  152 x 296 px  Film P  fast update
//
// Compile:
//   g++ -O2 -std=c++14 -o epd_demo \
//       main.cpp hV_HAL_Peripherals.cpp hV_Board.cpp \
//       hV_Utilities_Common.cpp hV_Utilities_PDLS.cpp \
//       hV_Font_Terminal.cpp hV_Screen_Buffer.cpp \
//       Screen_EPD_EXT3.cpp \
//       -lbcm2835
//
// Run as root (required by bcm2835):
//   sudo ./epd_demo
//

#include "hV_HAL_Peripherals.h"
#include "hV_Configuration.h"
#include "hV_List_Boards.h"
#include "hV_List_Screens.h"
#include "Screen_EPD_EXT3.h"
#include <cstdio>
#include <csignal>

// ─── Global colour instance (declared extern in hV_Configuration.h) ───────────
Colours565 myColours;

// ─── Screen configuration for 2.66" Film P ────────────────────────────────────
// eScreen_EPD_266_PS_0C  = SCREEN(SIZE_266, FILM_P, DRIVER_C)
Screen_EPD_EXT3_Fast myScreen(eScreen_EPD_266_PS_0C, boardRaspberryPi2W);

// ─── Graceful shutdown ────────────────────────────────────────────────────────
static void handleSigInt(int)
{
    printf("\nShutting down...\n");
    debug_led1_set(false);
    debug_led2_set(false);
    bcm2835_close();
    exit(0);
}

// ─── Helper: draw a simple test pattern ──────────────────────────────────────
static void drawTestPattern()
{
    uint16_t w = myScreen.screenSizeX();
    uint16_t h = myScreen.screenSizeY();

    // Border rectangle
    myScreen.setPenSolid(false);
    myScreen.rectangle(2, 2, w - 3, h - 3, myColours.black);

    // Two diagonal lines
    myScreen.line(2, 2, w - 3, h - 3, myColours.black);
    myScreen.line(w - 3, 2, 2, h - 3, myColours.black);

    // Text labels
    myScreen.selectFont(1); // 8x12
    myScreen.gText(10, 10,  "RPi 2W port", myColours.black, myColours.white);
    myScreen.gText(10, 30,  "2.66\" EPD",   myColours.black, myColours.white);

    myScreen.selectFont(0); // 6x8
    myScreen.gText(10, 50,  myScreen.WhoAmI(), myColours.black, myColours.white);

    // Small filled rectangle in corner
    myScreen.setPenSolid(true);
    myScreen.rectangle(w - 30, h - 30, w - 5, h - 5, myColours.black);
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main()
{
    signal(SIGINT, handleSigInt);

    // Initialise bcm2835
    hV_HAL_begin();

    // Debug LEDs + buttons
    debug_leds_init();

    printf("=== EPaper Raspberry Pi 2W Demo ===\n");
    printf("Screen: 2.66\"  152x296 px  Film P\n");
    printf("GPIO map:\n");
    printf("  BUSY   = GPIO7  (PIN26)\n");
    printf("  DC     = GPIO8  (PIN24)\n");
    printf("  RESET  = GPIO25 (PIN22)\n");
    printf("  Flash_CS = GPIO22 (PIN15)\n");
    printf("  Panel_CS = GPIO27 (PIN13)\n");
    printf("  LED1   = GPIO20\n");
    printf("  LED2   = GPIO6\n");
    printf("  BTN1   = GPIO16\n");
    printf("  BTN2   = GPIO12\n\n");

    // ── Initialise screen ─────────────────────────────────────────────────────
    debug_led1_set(true);
    printf("Initialising screen...\n");
    myScreen.begin();
    printf("Screen: %s\n", myScreen.WhoAmI().c_str());
    printf("Size: %u x %u px\n", myScreen.screenSizeX(), myScreen.screenSizeY());

    // ── First frame: clear white ───────────────────────────────────────────────
    printf("Clearing screen...\n");
    myScreen.clear(myColours.white);
    myScreen.flush();
    delay(500);

    // ── Second frame: test pattern ────────────────────────────────────────────
    printf("Drawing test pattern...\n");
    debug_led2_set(true);
    myScreen.clear(myColours.white);
    drawTestPattern();
    myScreen.flush();
    printf("Pattern displayed.\n");
    debug_led2_set(false);

    // ── Interactive loop: buttons ─────────────────────────────────────────────
    printf("\nInteractive mode:\n");
    printf("  BTN1 (GPIO16) -> fast refresh with new frame\n");
    printf("  BTN2 (GPIO12) -> regenerate (ghost removal)\n");
    printf("  Ctrl+C to quit\n\n");

    uint32_t frameCount = 0;

    while (true)
    {
        if (debug_button1_pressed())
        {
            // Debounce
            delay(50);
            while (debug_button1_pressed()) delay(10);

            frameCount++;
            debug_led1_set(true);

            printf("Button 1 – fast update, frame %u\n", frameCount);

            myScreen.clear(myColours.white);
            myScreen.selectFont(1);
            myScreen.gText(10, 10, formatString("Frame %u", frameCount),
                           myColours.black, myColours.white);
            myScreen.selectFont(0);
            myScreen.gText(10, 35, "Fast update",
                           myColours.black, myColours.white);

            // Draw a small moving indicator bar
            uint16_t barX = (uint16_t)((frameCount * 7) % (myScreen.screenSizeX() - 20));
            myScreen.setPenSolid(true);
            myScreen.rectangle(barX, 60, barX + 15, 75, myColours.black);

            myScreen.flushMode(UPDATE_FAST);
            debug_led1_set(false);
        }

        if (debug_button2_pressed())
        {
            delay(50);
            while (debug_button2_pressed()) delay(10);

            printf("Button 2 – regenerate (ghost removal)\n");
            debug_led1_set(true);
            debug_led2_set(true);
            myScreen.regenerate();
            debug_led1_set(false);
            debug_led2_set(false);
        }

        delay(20); // ~50 Hz poll
    }

    // Never reached but tidy
    bcm2835_close();
    return 0;
}
