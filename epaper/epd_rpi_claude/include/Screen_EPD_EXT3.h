///
/// @file Screen_EPD_EXT3.h
/// @brief Driver for Pervasive Displays iTC screens with embedded fast update
///        Ported to Raspberry Pi 2W / Pi 4 using bcm2835
///
/// Screen: 2.66" EPD  152 x 296 pixels  Film P  5624 bytes frame-buffer
///

#ifndef SCREEN_EPD_EXT3_RELEASE
#define SCREEN_EPD_EXT3_RELEASE 820

#define SCREEN_EPD_EXT3_VARIANT "Basic-Fast"

#include "hV_HAL_Peripherals.h"
#include "hV_Configuration.h"
#include "hV_Screen_Buffer.h"
#include "hV_Board.h"
#include "hV_Utilities_PDLS.h"

#define WITH_FAST
#define WITH_FAST_FRIENDS

///
/// @brief Screen driver class
///
class Screen_EPD_EXT3_Fast final : public hV_Screen_Buffer, public hV_Utilities_PDLS
{
  public:
    Screen_EPD_EXT3_Fast(eScreen_EPD_t eScreen_EPD, pins_t board);

    void begin();
    void suspend(uint8_t suspendScope = POWER_SCOPE_GPIO_ONLY);
    void resume();

    virtual STRING_TYPE WhoAmI();

    void    clear(uint16_t colour = myColours.white);
    void    flush();
    void    regenerate(uint8_t mode = UPDATE_FAST);
    uint8_t flushMode(uint8_t updateMode = UPDATE_FAST);

  protected:
    void     s_setOrientation(uint8_t orientation);
    bool     s_orientCoordinates(uint16_t& x, uint16_t& y);
    void     s_setPoint(uint16_t x1, uint16_t y1, uint16_t colour);
    uint16_t s_getPoint(uint16_t x1, uint16_t y1);
    void     s_reset();
    void     s_getDataOTP();
    void     s_flush(uint8_t updateMode = UPDATE_FAST);
    uint32_t s_getZ(uint16_t x1, uint16_t y1);
    uint16_t s_getB(uint16_t x1, uint16_t y1);

    // COG functions – Medium screens (581 PS 0B etc.)
    void COG_MediumP_reset();
    void COG_MediumP_getDataOTP();
    void COG_MediumP_initial(uint8_t updateMode);
    void COG_MediumP_sendImageData(uint8_t updateMode);
    void COG_MediumP_update(uint8_t updateMode);
    void COG_MediumP_powerOff();

    // COG functions – Small screens (266 PS 0C etc.)
    void COG_SmallP_reset();
    void COG_SmallP_getDataOTP();
    void COG_SmallP_initial(uint8_t updateMode);
    void COG_SmallP_sendImageData(uint8_t updateMode);
    void COG_SmallP_update(uint8_t updateMode);
    void COG_SmallP_powerOff();

    uint8_t COG_data[128];
    bool    s_flag50 = false;
};

#endif // SCREEN_EPD_EXT3_RELEASE
