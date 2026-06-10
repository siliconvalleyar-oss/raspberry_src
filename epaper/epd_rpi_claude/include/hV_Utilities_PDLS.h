///
/// @file hV_Utilities_PDLS.h
/// @brief PDLS utilities – Raspberry Pi port
///

#ifndef hV_UTILITIES_PDLS_RELEASE
#define hV_UTILITIES_PDLS_RELEASE 812

#include "hV_HAL_Peripherals.h"
#include "hV_Configuration.h"
#include "hV_List_Constants.h"
#include "hV_List_Screens.h"
#include "hV_Utilities_Common.h"
#include "hV_Board.h"

class hV_Utilities_PDLS : public hV_Board
{
  public:
    hV_Utilities_PDLS();

    ///
    /// @brief Invert display
    ///
    void invert(bool flag);

    ///
    /// @brief Number of colours
    ///
    uint8_t screenColours();

    ///
    /// @brief Screen number as string
    ///
    STRING_TYPE screenNumber();

    ///
    /// @brief Set temperature in Celsius
    ///
    void setTemperatureC(int8_t temperatureC);

    ///
    /// @brief Set temperature in Fahrenheit
    ///
    void setTemperatureF(int16_t temperatureF);

    ///
    /// @brief Check temperature vs update mode
    ///
    uint8_t checkTemperatureMode(uint8_t updateMode);

    ///
    /// @brief Set power profile
    ///
    void setPowerProfile(uint8_t mode, uint8_t scope);

    ///
    /// @brief Debug helper
    ///
    void debugVariant(uint8_t contextFilm);

  protected:
    void u_begin(pins_t board, uint8_t family, uint16_t delayCS);
    void u_WhoAmI(char* answer);
    void u_screenNumber(char* answer);

    eScreen_EPD_t u_eScreen_EPD = 0;

    uint8_t u_codeSize   = 0;
    uint8_t u_codeFilm   = 0;
    uint8_t u_codeDriver = 0;
    uint8_t u_codeExtra  = 0;

    int8_t  u_temperature  = 25;
    bool    u_invert       = false;
    bool    u_flagOTP      = false;

    uint8_t u_suspendMode  = POWER_MODE_AUTO;
    uint8_t u_suspendScope = POWER_SCOPE_GPIO_ONLY;

    uint32_t u_pageColourSize = 0;
    uint16_t u_bufferSizeH    = 0;
    uint16_t u_bufferSizeV    = 0;
    uint8_t  u_bufferDepth    = 1;
};

#endif // hV_UTILITIES_PDLS_RELEASE
