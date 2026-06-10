//
// Screen_EPD_EXT3.cpp
// Raspberry Pi 2W / Pi 4 port using bcm2835
//
// Original: Rei Vilo, Pervasive Displays Library Suite
// Port: adapted for Linux + bcm2835, no Arduino SDK required
//
// Screen: 2.66" EPD  152 x 296 px  Film P  frame-buffer = 5624 bytes
//

#include "Screen_EPD_EXT3.h"
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// COG – Medium screens (SIZE_343, SIZE_581, SIZE_741)
// ─────────────────────────────────────────────────────────────────────────────

void Screen_EPD_EXT3_Fast::COG_MediumP_reset()
{
    b_reset(5, 2, 4, 20, 5);
}

void Screen_EPD_EXT3_Fast::COG_MediumP_getDataOTP()
{
    uint16_t _readBytes = 0;
    uint8_t  ui8 = 0;
    u_flagOTP = false;

    COG_MediumP_reset();
    if (b_family == FAMILY_LARGE)
    {
        digitalWrite(b_pin.panelCSS, HIGH);
    }

    switch (u_codeDriver)
    {
        case DRIVER_B:
            _readBytes = 128;
            digitalWrite(b_pin.panelDC, LOW);
            digitalWrite(b_pin.panelCS, LOW);
            hV_HAL_SPI3_write(0xb9);
            delay(5);
            break;

        default:
            mySerial.println();
            mySerial.println(formatString("hV * OTP failed for screen %i-%cS-0%c",
                                          u_codeSize, u_codeFilm, u_codeDriver));
            mySerial.flush();
        while (true) {}
    }

    digitalWrite(b_pin.panelDC, HIGH);
    ui8 = hV_HAL_SPI3_read(); // dummy

    for (uint16_t i = 0; i < _readBytes; i++)
    {
        COG_data[i] = hV_HAL_SPI3_read();
    }

    digitalWrite(b_pin.panelCS, HIGH);

    uint8_t _chipId;
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_343_PS_0B:
        case eScreen_EPD_343_PS_0B_Touch:
        case eScreen_EPD_581_PS_0B:
            _chipId  = 0x10;
            u_flagOTP = (COG_data[0x00] == _chipId);
            break;

        default:
            _chipId  = 0x00;
            u_flagOTP = false;
            break;
    }

    if (u_flagOTP)
        mySerial.println("hV . OTP check passed");
    else
    {
        mySerial.println();
        mySerial.println(formatString("hV * OTP check failed - First byte 0x%02x, expected 0x%04x",
                                      COG_data[0x00], _chipId));
        mySerial.flush();
        while (true) {}
    }
}

void Screen_EPD_EXT3_Fast::COG_MediumP_initial(uint8_t updateMode)
{
    uint8_t workDCTL[2];
    workDCTL[0] = COG_data[0x10];
    workDCTL[1] = 0x00;
    b_sendIndexData(0x01, workDCTL, 2);
}

void Screen_EPD_EXT3_Fast::COG_MediumP_sendImageData(uint8_t updateMode)
{
    FRAMEBUFFER_TYPE nextBuffer     = s_newImage;
    FRAMEBUFFER_TYPE previousBuffer = s_newImage + u_pageColourSize;

    b_sendIndexData(0x13, &COG_data[0x15], 6);
    b_sendIndexData(0x90, &COG_data[0x0c], 4);

    b_sendIndexData(0x12, &COG_data[0x12], 3);
    b_sendIndexData(0x10, nextBuffer, u_pageColourSize);

    b_sendIndexData(0x12, &COG_data[0x12], 3);

    switch (updateMode)
    {
        case UPDATE_GLOBAL:
            b_sendIndexFixed(0x11, 0x00, u_pageColourSize);
            break;
        case UPDATE_FAST:
            b_sendIndexData(0x11, previousBuffer, u_pageColourSize);
            break;
        default:
            break;
    }

    memcpy(previousBuffer, nextBuffer, u_pageColourSize);
}

void Screen_EPD_EXT3_Fast::COG_MediumP_update(uint8_t updateMode)
{
    b_sendCommandData8(0x05, 0x7d);
    delay(50);
    b_sendCommandData8(0x05, 0x00);
    delay(1);
    b_sendCommandData8(0xd8, COG_data[0x1c]);
    b_sendCommandData8(0xd6, COG_data[0x1d]);

    b_sendCommandData8(0xa7, 0x10);
    delay(2);
    b_sendCommandData8(0xa7, 0x00);
    delay(10);

    b_sendCommandData8(0x44, 0x00);
    b_sendCommandData8(0x45, 0x80);

    b_sendCommandData8(0xa7, 0x10);
    delay(2);
    b_sendCommandData8(0xa7, 0x00);
    delay(10);

    uint8_t indexTemperature = 0;

    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_343_PS_0B:
        case eScreen_EPD_343_PS_0B_Touch:
            if (updateMode == UPDATE_FAST)
                indexTemperature = (u_temperature < 22) ? 0xc9 : 0xca;
            else
            {
                indexTemperature = (uint8_t)(2 * u_temperature + 0x50);
                indexTemperature = (uint8_t)checkRange(indexTemperature, (uint16_t)0x50, (uint16_t)0xb4);
            }
            // fall through
        case eScreen_EPD_581_PS_0B:
            if (updateMode == UPDATE_FAST)
            {
                indexTemperature = (uint8_t)(u_temperature + 0x28 + 0x80);
                indexTemperature = (uint8_t)checkRange(indexTemperature, (uint16_t)0xa8, (uint16_t)0xda);
            }
            else
            {
                indexTemperature = (uint8_t)(u_temperature + 0x28);
                indexTemperature = (uint8_t)checkRange(indexTemperature, (uint16_t)0x19, (uint16_t)0x64);
            }
            break;
        default:
            break;
    }

    b_sendCommandData8(0x44, 0x06);
    b_sendCommandData8(0x45, indexTemperature);

    b_sendCommandData8(0xa7, 0x10);
    delay(2);
    b_sendCommandData8(0xa7, 0x00);
    delay(10);

    b_sendCommandData8(0x60, COG_data[0x0b]);
    b_sendCommandData8(0x61, COG_data[0x1b]);
    b_sendCommandData8(0x02, COG_data[0x11]);

    uint8_t offsetFrame = 0x28;
    uint8_t _filter09   = 0xff;

    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_343_PS_0B:
        case eScreen_EPD_343_PS_0B_Touch:
            _filter09 = 0xfb;
            break;
        default:
            _filter09 = 0xff;
            break;
    }

    for (uint8_t stage = 0; stage < 4; stage++)
    {
        uint8_t offset   = offsetFrame + 0x08 * stage;
        uint8_t FORMAT   = COG_data[offset] & 0x80;
        uint8_t REPEAT   = COG_data[offset] & 0x7f;

        if (FORMAT > 0)
        {
            uint8_t  PHL_PHH[2]  = { COG_data[offset+1], COG_data[offset+2] };
            uint8_t  PHL_VAR     = COG_data[offset+3];
            uint8_t  PHH_VAR     = COG_data[offset+4];
            uint8_t  BST_SW_a    = COG_data[offset+5] & _filter09;
            uint8_t  BST_SW_b    = COG_data[offset+6] & _filter09;
            uint8_t  DELAY_SCALE = COG_data[offset+7] & 0x80;
            uint16_t DELAY_VALUE = COG_data[offset+7] & 0x7f;

            for (uint8_t i = 0; i < REPEAT; i++)
            {
                b_sendCommandData8(0x09, BST_SW_a);
                PHL_PHH[0] += PHL_VAR;
                PHL_PHH[1] += PHH_VAR;
                b_sendIndexData(0x51, PHL_PHH, 2);
                b_sendCommandData8(0x09, BST_SW_b);
                (DELAY_SCALE > 0) ? delay(DELAY_VALUE) : delayMicroseconds(10 * DELAY_VALUE);
            }
        }
        else
        {
            uint8_t  BST_SW_a     = COG_data[offset+1] & _filter09;
            uint8_t  BST_SW_b     = COG_data[offset+2] & _filter09;
            uint8_t  DELAY_a_SCALE = COG_data[offset+3] & 0x80;
            uint16_t DELAY_a_VALUE = COG_data[offset+3] & 0x7f;
            uint8_t  DELAY_b_SCALE = COG_data[offset+4] & 0x80;
            uint16_t DELAY_b_VALUE = COG_data[offset+4] & 0x7f;

            for (uint8_t i = 0; i < REPEAT; i++)
            {
                b_sendCommandData8(0x09, BST_SW_a);
                (DELAY_a_SCALE > 0) ? delay(DELAY_a_VALUE) : delayMicroseconds(10 * DELAY_a_VALUE);
                b_sendCommandData8(0x09, BST_SW_b);
                (DELAY_b_SCALE > 0) ? delay(DELAY_b_VALUE) : delayMicroseconds(10 * DELAY_b_VALUE);
            }
        }
    }

    b_waitBusy();
    b_sendCommandData8(0x15, 0x3c);
}

void Screen_EPD_EXT3_Fast::COG_MediumP_powerOff()
{
    b_waitBusy();
    b_sendCommandData8(0x09, 0x7b);
    b_sendCommandData8(0x05, 0x5d);
    b_sendCommandData8(0x09, 0x7a);
    delay(15);
    b_sendCommandData8(0x09, 0x00);
    b_waitBusy(HIGH);
}

// ─────────────────────────────────────────────────────────────────────────────
// COG – Small screens (all other sizes incl. 266)
// ─────────────────────────────────────────────────────────────────────────────

void Screen_EPD_EXT3_Fast::COG_SmallP_reset()
{
    b_reset(5, 5, 10, 5, 5);
}

void Screen_EPD_EXT3_Fast::COG_SmallP_getDataOTP()
{
    uint8_t  ui8 = 0;
    uint16_t _readBytes = 2;
    u_flagOTP = false;

    // Screens with embedded PSR (no OTP needed)
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
        case eScreen_EPD_290_KS_0F:
            u_flagOTP = true;
            mySerial.println("hV . OTP check passed - embedded PSR");
            return;
        default:
            break;
    }

    // Flag for register 0x50 fast-update tweak
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_154_PS_0C: case eScreen_EPD_154_KS_0C:
        case eScreen_EPD_206_KS_0E:
        case eScreen_EPD_213_PS_0E: case eScreen_EPD_213_KS_0E:
        case eScreen_EPD_266_PS_0C: case eScreen_EPD_266_KS_0C:
        case eScreen_EPD_271_KS_0C:
        case eScreen_EPD_370_PS_0C: case eScreen_EPD_370_PS_0C_Touch: case eScreen_EPD_370_KS_0C:
        case eScreen_EPD_437_PS_0C: case eScreen_EPD_437_KS_0C:
            s_flag50 = true;
            break;
        default:
            s_flag50 = false;
            break;
    }

    // Send OTP read command
    digitalWrite(b_pin.panelDC, LOW);
    digitalWrite(b_pin.panelCS, LOW);
    hV_HAL_SPI3_write(0xa2);
    digitalWrite(b_pin.panelCS, HIGH);
    delay(10);

    digitalWrite(b_pin.panelDC, HIGH);
    digitalWrite(b_pin.panelCS, LOW);
    ui8 = hV_HAL_SPI3_read(); // dummy
    digitalWrite(b_pin.panelCS, HIGH);

    digitalWrite(b_pin.panelCS, LOW);
    ui8 = hV_HAL_SPI3_read(); // first byte
    digitalWrite(b_pin.panelCS, HIGH);

    uint8_t  bank      = ((ui8 == 0xa5) ? 0 : 1);
    uint16_t offsetA5  = 0;
    uint16_t offsetPSR = 0;

    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_271_KS_09:
        case eScreen_EPD_271_KS_09_Touch:
            offsetPSR = 0x004b; offsetA5 = 0x0000;
            if (bank > 0) { COG_data[0]=0xcf; COG_data[1]=0x82; return; }
            break;

        case eScreen_EPD_271_PS_09:
        case eScreen_EPD_271_PS_09_Touch:
        case eScreen_EPD_287_PS_09:
            offsetPSR = 0x004b; offsetA5 = 0x0000;
            break;

        case eScreen_EPD_154_KS_0C: case eScreen_EPD_154_PS_0C:
        case eScreen_EPD_266_KS_0C: case eScreen_EPD_266_PS_0C:
        case eScreen_EPD_271_KS_0C:
        case eScreen_EPD_350_KS_0C:
        case eScreen_EPD_370_KS_0C: case eScreen_EPD_370_PS_0C: case eScreen_EPD_370_PS_0C_Touch:
        case eScreen_EPD_437_PS_0C:
            offsetPSR = (bank == 0) ? 0x0fb4 : 0x1fb4;
            offsetA5  = (bank == 0) ? 0x0000 : 0x1000;
            break;

        case eScreen_EPD_206_KS_0E: case eScreen_EPD_213_KS_0E: case eScreen_EPD_213_PS_0E:
            offsetPSR = (bank == 0) ? 0x0b1b : 0x171b;
            offsetA5  = (bank == 0) ? 0x0000 : 0x0c00;
            break;

        case eScreen_EPD_417_PS_0D: case eScreen_EPD_417_KS_0D:
            offsetPSR = (bank == 0) ? 0x0b1f : 0x171f;
            offsetA5  = (bank == 0) ? 0x0000 : 0x0c00;
            break;

        default:
            mySerial.println(formatString("hV * OTP check failed - Screen %i-%cS-0%c not supported",
                                          u_codeSize, u_codeFilm, u_codeDriver));
            mySerial.flush();
        while (true) {}
    }

    if (offsetA5 > 0x0000)
    {
        for (uint16_t i = 1; i < offsetA5; i++)
        {
            digitalWrite(b_pin.panelCS, LOW);
            ui8 = hV_HAL_SPI3_read();
            digitalWrite(b_pin.panelCS, HIGH);
        }
        digitalWrite(b_pin.panelCS, LOW);
        ui8 = hV_HAL_SPI3_read();
        digitalWrite(b_pin.panelCS, HIGH);
        if (ui8 != 0xa5)
        {
            mySerial.println(formatString("hV * OTP check failed - Bank %i, first 0x%02x, expected 0x%02x",
                                          bank, ui8, 0xa5));
            mySerial.flush();
        while (true) {}
        }
    }

    mySerial.println(formatString("hV . OTP check passed - Bank %i, first 0x%02x", bank, ui8));

    // Skip to PSR offset
    for (uint16_t i = offsetA5 + 1; i < offsetPSR; i++)
    {
        digitalWrite(b_pin.panelCS, LOW);
        ui8 = hV_HAL_SPI3_read();
        digitalWrite(b_pin.panelCS, HIGH);
    }

    for (uint16_t i = 0; i < _readBytes; i++)
    {
        digitalWrite(b_pin.panelCS, LOW);
        COG_data[i] = hV_HAL_SPI3_read();
        digitalWrite(b_pin.panelCS, HIGH);
    }

    u_flagOTP = true;
}

void Screen_EPD_EXT3_Fast::COG_SmallP_initial(uint8_t updateMode)
{
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
            b_sendCommand8(0x12);
            digitalWrite(b_pin.panelDC, LOW);
            b_waitBusy(LOW);
            b_sendCommandData8(0x1a, (uint8_t)u_temperature);
            if (updateMode == UPDATE_GLOBAL)
                b_sendCommandData8(0x22, 0xd7);
            else
            {
                b_sendCommandData8(0x3c, 0xc0);
                b_sendCommandData8(0x22, 0xdf);
            }
            break;

        default:
        {
            uint8_t indexTemperature;
            uint8_t index00_work[2];

            if (updateMode != UPDATE_GLOBAL)
            {
                indexTemperature  = (uint8_t)(u_temperature | 0x40);
                index00_work[0]   = COG_data[0] | 0x10;
                index00_work[1]   = COG_data[1] | 0x02;
            }
            else
            {
                indexTemperature  = (uint8_t)u_temperature;
                index00_work[0]   = COG_data[0];
                index00_work[1]   = COG_data[1];
            }

            b_sendCommandData8(0x00, 0x0e); // Soft-reset
            b_waitBusy();
            b_sendCommandData8(0xe5, indexTemperature);
            b_sendCommandData8(0xe0, 0x02);

            if (u_codeSize == SIZE_290)
            {
                b_sendCommandData8(0x4d, 0x55);
                b_sendCommandData8(0xe9, 0x02);
            }
            else
            {
                b_sendIndexData(0x00, index00_work, 2);
            }

            if (updateMode != UPDATE_GLOBAL)
                b_sendCommandData8(0x50, 0x07);
        }
        break;
    }
}

void Screen_EPD_EXT3_Fast::COG_SmallP_sendImageData(uint8_t updateMode)
{
    FRAMEBUFFER_TYPE nextBuffer     = s_newImage;
    FRAMEBUFFER_TYPE previousBuffer = s_newImage + u_pageColourSize;

    if (s_flag50)
        b_sendCommandData8(0x50, 0x27);

    b_sendIndexData(0x10, previousBuffer, u_pageColourSize);
    b_sendIndexData(0x13, nextBuffer,     u_pageColourSize);

    if (s_flag50)
        b_sendCommandData8(0x50, 0x07);

    memcpy(previousBuffer, nextBuffer, u_pageColourSize);
}

void Screen_EPD_EXT3_Fast::COG_SmallP_update(uint8_t updateMode)
{
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
            b_waitBusy(LOW);
            b_sendCommand8(0x20);
            digitalWrite(b_pin.panelCS, HIGH);
            b_waitBusy(LOW);
            break;

        default:
            b_waitBusy();
            b_sendCommand8(0x04); // Power on
            b_waitBusy();
            b_sendCommand8(0x12); // Display refresh
            b_waitBusy();
            break;
    }
}

void Screen_EPD_EXT3_Fast::COG_SmallP_powerOff()
{
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
            break;
        default:
            b_sendCommand8(0x02);
            b_waitBusy();
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Class implementation
// ─────────────────────────────────────────────────────────────────────────────

Screen_EPD_EXT3_Fast::Screen_EPD_EXT3_Fast(eScreen_EPD_t eScreen_EPD, pins_t board)
{
    u_eScreen_EPD = eScreen_EPD;
    b_pin         = board;
    s_newImage    = nullptr;
    COG_data[0]   = 0;
}

void Screen_EPD_EXT3_Fast::begin()
{
    u_codeSize   = SCREEN_SIZE(u_eScreen_EPD);
    u_codeFilm   = SCREEN_FILM(u_eScreen_EPD);
    u_codeDriver = SCREEN_DRIVER(u_eScreen_EPD);
    u_codeExtra  = SCREEN_EXTRA(u_eScreen_EPD);
    v_screenColourBits = 2;

    if (u_codeFilm != FILM_P)
    {
        debugVariant(FILM_P);
    }

    // Board family
    switch (u_codeSize)
    {
        case SIZE_343: case SIZE_581: case SIZE_741:
            b_begin(b_pin, FAMILY_MEDIUM, 0);
            break;
        case SIZE_969: case SIZE_1198:
            b_begin(b_pin, FAMILY_LARGE, 50);
            break;
        default:
            b_begin(b_pin, FAMILY_SMALL, 0);
            break;
    }

    // Physical screen dimensions (H = horizontal/narrow, V = vertical/long)
    switch (u_codeSize)
    {
        case SIZE_150: case SIZE_152:
            v_screenSizeV = 200; v_screenSizeH = 200; v_screenDiagonal = 150; break;
        case SIZE_154:
            v_screenSizeV = 152; v_screenSizeH = 152; v_screenDiagonal = 154; break;
        case SIZE_206:
            v_screenSizeV = 248; v_screenSizeH = 128; v_screenDiagonal = 206; break;
        case SIZE_213:
            v_screenSizeV = 212; v_screenSizeH = 104; v_screenDiagonal = 213; break;
        case SIZE_266:
            v_screenSizeV = 296; v_screenSizeH = 152; v_screenDiagonal = 266; break;
        case SIZE_271:
            v_screenSizeV = 264; v_screenSizeH = 176; v_screenDiagonal = 271; break;
        case SIZE_287:
            v_screenSizeV = 296; v_screenSizeH = 128; v_screenDiagonal = 287; break;
        case SIZE_290:
            v_screenSizeV = 384; v_screenSizeH = 168; v_screenDiagonal = 290; break;
        case SIZE_343:
            v_screenSizeV = 456; v_screenSizeH = 195; v_screenDiagonal = 343; break;
        case SIZE_350:
            v_screenSizeV = 384; v_screenSizeH = 184; v_screenDiagonal = 350; break;
        case SIZE_370:
            v_screenSizeV = 416; v_screenSizeH = 240; v_screenDiagonal = 370; break;
        case SIZE_417:
            v_screenSizeV = 300; v_screenSizeH = 400; v_screenDiagonal = 417; break;
        case SIZE_437:
            v_screenSizeV = 480; v_screenSizeH = 176; v_screenDiagonal = 437; break;
        case SIZE_581:
            v_screenSizeV = 720; v_screenSizeH = 256; v_screenDiagonal = 581; break;
        case SIZE_741:
            v_screenSizeV = 800; v_screenSizeH = 480; v_screenDiagonal = 741; break;
        case SIZE_969:
            v_screenSizeV = 672; v_screenSizeH = 960; v_screenDiagonal = 969; break;
        case SIZE_1198:
            v_screenSizeV = 768; v_screenSizeH = 960; v_screenDiagonal = 1198; break;
        default:
            mySerial.println(formatString("hV * Screen size %i not supported", u_codeSize));
            mySerial.flush();
        while (true) {}
    }

    // Buffer sizes (1 bit per pixel, packed bytes)
    u_bufferSizeH    = v_screenSizeH / 8;
    u_bufferSizeV    = v_screenSizeV;
    u_bufferDepth    = 2; // next + previous frame
    u_pageColourSize = (uint32_t)u_bufferSizeV * (uint32_t)u_bufferSizeH;

    // Allocate frame-buffer (2 pages: current + previous for fast update)
    if (s_newImage == nullptr)
    {
        s_newImage = new uint8_t[u_pageColourSize * u_bufferDepth];
    }
    memset(s_newImage, 0x00, u_pageColourSize * u_bufferDepth);

    setTemperatureC(25);
    b_fsmPowerScreen = FSM_OFF;

    if (b_pin.panelPower != NOT_CONNECTED)
    {
        setPowerProfile(POWER_MODE_MANUAL, POWER_SCOPE_GPIO_ONLY);
    }

    resume();

    hV_Screen_Buffer::begin();

    if (f_fontMax() > 0) f_selectFont(0);
    f_fontSolid = false;

    setOrientation(0);
    v_penSolid = false;
    u_invert   = false;
}

STRING_TYPE Screen_EPD_EXT3_Fast::WhoAmI()
{
    char work[64] = {0};
    u_WhoAmI(work);
    return formatString("iTC %i.%02i\"%s",
                        v_screenDiagonal / 100, v_screenDiagonal % 100, work);
}

void Screen_EPD_EXT3_Fast::suspend(uint8_t suspendScope)
{
    if (((suspendScope & FSM_GPIO_MASK) == FSM_GPIO_MASK) &&
        (b_pin.panelPower != NOT_CONNECTED))
    {
        if ((b_fsmPowerScreen & FSM_GPIO_MASK) == FSM_GPIO_MASK)
            b_suspend();
    }
}

void Screen_EPD_EXT3_Fast::resume()
{
    if (b_fsmPowerScreen != FSM_ON)
    {
        if ((b_fsmPowerScreen & FSM_GPIO_MASK) != FSM_GPIO_MASK)
        {
            b_resume();
            s_reset();
            b_fsmPowerScreen |= FSM_GPIO_MASK;
        }

        if (!u_flagOTP)
        {
            s_getDataOTP();
            s_reset();
        }

        hV_HAL_SPI_begin(8000000);
    }
}

void Screen_EPD_EXT3_Fast::s_reset()
{
    switch (b_family)
    {
        case FAMILY_MEDIUM: COG_MediumP_reset(); break;
        case FAMILY_SMALL:  COG_SmallP_reset();  break;
        default: break;
    }
}

void Screen_EPD_EXT3_Fast::s_getDataOTP()
{
    hV_HAL_SPI_end();
    hV_HAL_SPI3_begin();

    switch (b_family)
    {
        case FAMILY_MEDIUM: COG_MediumP_getDataOTP(); break;
        case FAMILY_SMALL:  COG_SmallP_getDataOTP();  break;
        default: break;
    }
}

void Screen_EPD_EXT3_Fast::s_flush(uint8_t updateMode)
{
    if (b_fsmPowerScreen != FSM_ON) resume();

    switch (b_family)
    {
        case FAMILY_MEDIUM:
            COG_MediumP_initial(updateMode);
            COG_MediumP_sendImageData(updateMode);
            COG_MediumP_update(updateMode);
            COG_MediumP_powerOff();
            break;
        case FAMILY_SMALL:
            COG_SmallP_initial(updateMode);
            COG_SmallP_sendImageData(updateMode);
            COG_SmallP_update(updateMode);
            COG_SmallP_powerOff();
            break;
        default:
            break;
    }

    if (u_suspendMode == POWER_MODE_AUTO)
        suspend(u_suspendScope);
}

uint8_t Screen_EPD_EXT3_Fast::flushMode(uint8_t updateMode)
{
    updateMode = checkTemperatureMode(updateMode);

    switch (updateMode)
    {
        case UPDATE_FAST:
        case UPDATE_GLOBAL:
            s_flush(updateMode);
            break;
        default:
            mySerial.println("hV ! PDLS - UPDATE_NONE invoked");
            break;
    }
    return updateMode;
}

void Screen_EPD_EXT3_Fast::flush()
{
    flushMode(UPDATE_FAST);
}

void Screen_EPD_EXT3_Fast::clear(uint16_t colour)
{
    if (colour == myColours.grey)
    {
        for (uint16_t i = 0; i < u_bufferSizeV; i++)
        {
            uint8_t pattern = (i % 2) ? 0b10101010 : 0b01010101;
            for (uint16_t j = 0; j < u_bufferSizeH; j++)
                s_newImage[i * u_bufferSizeH + j] = pattern;
        }
    }
    else if ((colour == myColours.white) ^ u_invert)
    {
        memset(s_newImage, 0x00, u_pageColourSize);
    }
    else
    {
        memset(s_newImage, 0xff, u_pageColourSize);
    }
}

void Screen_EPD_EXT3_Fast::regenerate(uint8_t mode)
{
    clear(myColours.black);  flush();  delay(100);
    clear(myColours.white);  flush();  delay(100);
}

void Screen_EPD_EXT3_Fast::s_setPoint(uint16_t x1, uint16_t y1, uint16_t colour)
{
    if (s_orientCoordinates(x1, y1) == RESULT_ERROR) return;

    bool flagOdd = ((x1 + y1) % 2 == 0);
    if (colour == myColours.grey)
        colour = flagOdd ? myColours.black : myColours.white;

    uint32_t z1 = s_getZ(x1, y1);
    uint16_t b1 = s_getB(x1, y1);

    if ((colour == myColours.white) ^ u_invert)
        bitClear(s_newImage[z1], b1);
    else if ((colour == myColours.black) ^ u_invert)
        bitSet(s_newImage[z1], b1);
}

void Screen_EPD_EXT3_Fast::s_setOrientation(uint8_t orientation)
{
    v_orientation = orientation % 4;
}

bool Screen_EPD_EXT3_Fast::s_orientCoordinates(uint16_t& x, uint16_t& y)
{
    bool result = RESULT_ERROR;
    switch (v_orientation)
    {
        case 3:
            if ((x < v_screenSizeV) && (y < v_screenSizeH))
            { x = v_screenSizeV - 1 - x; result = RESULT_SUCCESS; }
            break;
        case 2:
            if ((x < v_screenSizeH) && (y < v_screenSizeV))
            { x = v_screenSizeH-1-x; y = v_screenSizeV-1-y; hV_HAL_swap(x,y); result = RESULT_SUCCESS; }
            break;
        case 1:
            if ((x < v_screenSizeV) && (y < v_screenSizeH))
            { y = v_screenSizeH - 1 - y; result = RESULT_SUCCESS; }
            break;
        default:
            if ((x < v_screenSizeH) && (y < v_screenSizeV))
            { hV_HAL_swap(x,y); result = RESULT_SUCCESS; }
            break;
    }
    return result;
}

uint32_t Screen_EPD_EXT3_Fast::s_getZ(uint16_t x1, uint16_t y1)
{
    uint32_t z1 = 0;
    switch (u_codeSize)
    {
        case SIZE_969: case SIZE_1198:
            if (y1 >= (v_screenSizeH >> 1))
            { y1 -= (v_screenSizeH >> 1); z1 += (u_pageColourSize >> 1); }
            z1 += (uint32_t)x1 * (u_bufferSizeH >> 1) + (y1 >> 3);
            break;
        default:
            z1 = (uint32_t)x1 * u_bufferSizeH + (y1 >> 3);
            break;
    }
    return z1;
}

uint16_t Screen_EPD_EXT3_Fast::s_getB(uint16_t x1, uint16_t y1)
{
    return 7 - (y1 % 8);
}

uint16_t Screen_EPD_EXT3_Fast::s_getPoint(uint16_t x1, uint16_t y1)
{
    return 0x0000;
}
